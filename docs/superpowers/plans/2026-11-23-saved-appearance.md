# Saved Appearance Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Colour edits from the model panel (mesh colours, Model Tint, material tints, pod mesh colours) are stored in a new `<Appearance>` section of the config XML. They are set once on each matching model when it appears. A separate Save Appearance button rewrites only that section.

**Architecture:** There are two pure files with no game headers: an XML section rewriter (`ConfigXmlSection`) and the appearance table with its config item (`AppearanceTable`). A scratch console harness tests both. A game-side `Appearance` module owns the table, the spawn-time apply, the original colours and material tint ownership. The config loader gains attribute support and a catch-all item. Menu pickers edit through `Appearance`.

**Tech Stack:** C++17 (MSVC v14x, /W4), XmlLite, Dear ImGui 1.92, the 2BHook SDK structs.

**Spec:** `docs/superpowers/specs/2026-11-23-saved-appearance-design.md`

## Global Constraints

- The config version stays `CONFIG_XML_VERSION 2`. The new section is `CATEGORY_APPEARANCE _CRT_WIDE("Appearance")`, with `CATEGORY_APPEARANCE "Appearance"` in the INI branch.
- Colours use `#RRGGBBAA` via `Theme::FormatColor` / `Theme::ParseColor`.
- Elements: `<mesh model="" name="">`, `<model_tint model="">`, `<material model="" name="" intensity="">`. The colour is the element's text.
- `/W4` clean in Debug and Release x64. Unused parameters get `UNREFERENCED_PARAMETER`. Never delete code to silence a warning.
- Struct layout checks use `VALIDATE_OFFSET` / `VALIDATE_SIZE`.
- Hidden meshes stay session-only. No auto-save. No per-frame holding of colours.
- Commit locally only. No push, tag or release before Martin tests in game.
- Build: `"C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" "C:\Users\Martin\source\repos\2BHook\2BHook\2BHook.slnx" -p:Configuration=<Debug|Release> -p:Platform=x64 -m -nodeReuse:false -p:PostBuildEventUseInBuild=false`.

## Review Focus

1. A config with a malformed or truncated file when Save Appearance is pressed: the original file is unchanged, the temp file is deleted, and a console warning is shown. Test in Task 1 (`MalformedLeavesOriginal`).
2. Editing Hair while first person hides it: the saved alpha is first person's restore alpha, never 0. Checked in game; the code path is pinned in Task 3 step 4.
3. Loading another config while customized models are on screen: the old colours go back to their originals and the new ones apply next frame, with no leftover material tints. Task 3 `BeginLoad`; in-game checklist in Task 5.
4. A config written by this build is loaded by a 1.2.3.1 build (version 2 loader): the unknown `<Appearance>` section is skipped, with no crash and no lost settings. In-game checklist in Task 5.
5. Area reloads with material tints saved: tint slots don't run out after many loads. In-game checklist in Task 5 (do 10+ area loads and watch for the "no free slots" warning).

---

### Task 1: XML section rewriter

**Files:**
- Create: `ConfigXmlSection.h`, `ConfigXmlSection.cpp`
- Modify: `2BHook.vcxproj`, `2BHook.vcxproj.filters` (add both)
- Test (scratch, not committed): `<scratchpad>\appearance-tests\test_main.cpp`, where `<scratchpad>` = `C:\Users\Martin\AppData\Local\Temp\claude\b79eaac4fabfa5f3\0c996697-f822-410c-a897-1744718bcdee\scratchpad`

**Interfaces:**
- Produces: `HRESULT RewriteXmlSection(LPCWSTR szPath, LPCWSTR szSection, const std::function<HRESULT(IXmlWriter*)>& WriteSection);`
  - `WriteSection` writes the whole section element: start, children, end.
  - Returns `HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)` if `szPath` doesn't exist. On any failure, `szPath` is untouched and `<szPath>.tmp` is deleted.

- [ ] **Step 1: Write the harness tests** (a minimal `CHECK(cond)` macro, temp files in `<scratchpad>\appearance-tests\run\`):
  - `ReplacesExistingSection`: input `<Root version="2" num_children="2"><Gameplay num_children="1"><t_godmode>TRUE</t_godmode></Gameplay><Appearance num_children="1"><mesh model="pl0000" name="Hair">#FF0000FF</mesh></Appearance></Root>`. The writer emits `<Appearance num_children="1"><mesh model="pl0000" name="Face">#00FF00FF</mesh></Appearance>`. Re-read with XmlLite: `t_godmode` is still `TRUE`; there is exactly one `Appearance`; there is no `Hair` and there is `Face`; Root `version` is still `2`.
  - `InsertsWhenAbsent`: the same input without `<Appearance>`. Afterwards `Appearance` exists as the last child of Root and `Gameplay` is unchanged.
  - `EmptyRoot`: input `<Root version="2"/>` → Root holds exactly the new section.
  - `MissingFile`: returns `HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)` and creates no file.
  - `MalformedLeavesOriginal`: input `<Root><Gameplay><t_godmode>TRUE</Gameplay>` → `FAILED(hr)`, the bytes are identical to before, and no `.tmp` is left.
- [ ] **Step 2: Build the harness** with `cmd /c ""C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" && cl /nologo /EHsc /std:c++17 /W4 /I C:\Users\Martin\source\repos\2BHook\2BHook test_main.cpp C:\Users\Martin\source\repos\2BHook\2BHook\ConfigXmlSection.cpp xmllite.lib shlwapi.lib"`. Expected: it fails to link (`RewriteXmlSection` unresolved).
- [ ] **Step 3: Implement `RewriteXmlSection`.** Open the reader (`SHCreateStreamOnFileEx` read-only, `DtdProcessing_Prohibit`) and the writer on `<szPath>.tmp` (`STGM_CREATE | STGM_WRITE`, `Indent TRUE`, `WriteStartDocument(XmlStandalone_Omit)`). Copy node by node:
  - Skip the XML declaration and whitespace.
  - Copy elements by hand: `WriteStartElement` plus an attribute loop (`MoveToFirstAttribute` / `MoveToNextAttribute` → `WriteAttributeString`). Read `IsEmptyElement` before moving to the attributes. An empty element gets `WriteEndElement`.
  - End element → `WriteFullEndElement`. Text → `WriteString`. Comment → `WriteComment`. CDATA → `WriteCData`.
  - At depth 1, skip the subtree of an element named `szSection` and remember it was found.
  - Call `WriteSection` just before Root's end element, or right after Root's start when Root is empty.
  - Release the reader and the writer streams, then `MoveFileExW(tmp, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)`.
  - Every failure path deletes the tmp file.
- [ ] **Step 4: Rebuild and run the harness.** Expected: `5/5 passed`.
- [ ] **Step 5: Add both files to the vcxproj and filters** (next to `Configuration.*`).

### Task 2: Config loader attributes, catch-all items, SaveSection

**Files:**
- Create: `ConfigItemXml.h`. `IConfigItemXml` moves here from `Configuration.h` ~l.484, needing only `<xmllite.h>` and a forward declaration `template<typename T> class CConfigItemXml;` for `MakePair`. `Configuration.h` includes it. This lets pure files and the harness implement config items without game headers.
- Modify: `Configuration.h` (the `CATEGORY_*` defines ~l.92 and ~l.110, CConfigXml ~l.1270), vcxproj and filters
- Modify: `Configuration.cpp` (`Load` ~l.346-353, `FindItem` l.385, `Save` l.398)

**Interfaces:**
- Consumes: `RewriteXmlSection` (Task 1).
- Produces:
  - `virtual void IConfigItemXml::ReadAttributes(IXmlReader* pReader)`: does nothing by default. Called with the reader on the item's element, before its text.
  - `virtual bool IConfigItemXml::MatchesAnyName(void) const`: returns `false` by default. `FindItem` falls back to the first item in the section that returns true, after the exact-name match fails.
  - `HRESULT CConfigXml::SaveSection(LPCWSTR szFilename, LPCWSTR szSection);`
  - `CATEGORY_APPEARANCE`.

- [ ] **Step 1: Add the two virtuals and `CATEGORY_APPEARANCE`.** Add a comment above `CATEGORY_APPEARANCE` in the style of the Hacking/Theme note.
- [ ] **Step 2: Wire them into `Load`.** At depth 2 with `!bEmpty`, after `FindItem`: if there is an item, call `pItem->ReadAttributes(m_pReader.Get())`, then `m_pReader->MoveToElement()`. Add the `MatchesAnyName` fallback to `FindItem`.
- [ ] **Step 3: Split `Save`** into `Save(szFilename)` → `WriteDocument(IStream*, LPCWSTR szOnlySection)` (NULL = all sections; the existing loop, filtered). Add `SaveSection`:
  - Call `SetFilename`.
  - If the file doesn't exist, call `WriteDocument(stream, szSection)`.
  - Otherwise call `RewriteXmlSection(m_szFilename, szSection, lambda)`. The lambda writes `CConfigItemXmlNamespace(writer, szSection, count)` plus each item's `Write`, as `Save` does. `CConfigItemXmlNamespace` takes a `ComPtr<IXmlWriter>`, so the lambda wraps the raw pointer.
- [ ] **Step 4: Build Debug.** Expected: 0 errors, no new warnings in `Configuration.*` or `ConfigXmlSection.*`.

### Task 3: Appearance table, Appearance module, first-person accessor

**Files:**
- Create: `AppearanceTable.h/.cpp` (pure: no SDK or game headers; `Theme.h` only for the colour helpers), `Appearance.h/.cpp` (game side)
- Modify: `Features.h` (`UpdateFirstPersonHead` ~l.529), `MaterialTint.cpp:64`, `Configuration.cpp` (`InitializeConfig`), `Hooks.cpp:655`, vcxproj and filters
- Test (scratch): extend `test_main.cpp`. It stubs `Theme::FormatColor` / `Theme::ParseColor` with copies of `Theme.cpp:536-567`, using a local float→U32 conversion.

**Interfaces:**
- Produces (AppearanceTable.h):
  ```cpp
  enum AppearanceKind : int32_t { APPEARANCE_MESH, APPEARANCE_MODEL_TINT, APPEARANCE_MATERIAL };
  struct AppearanceEntry
  {
      AppearanceKind m_Kind;
      std::string m_Model;          // object name, "pl0000"
      std::string m_Name;           // mesh or material name; empty for a model tint
      ImVec4 m_vColor;
      float m_flIntensity = 1.0f;   // materials only
      ImVec4 m_vOriginal;           // runtime only: the colour before the first write
      bool m_bHaveOriginal = false;
  };
  class CAppearanceTable
  {
  public:
      AppearanceEntry* Find(AppearanceKind Kind, const char* szModel, const char* szName);
      AppearanceEntry& Set(AppearanceKind Kind, const char* szModel, const char* szName, const ImVec4& vColor, float flIntensity = 1.0f); // keeps m_vOriginal of an existing entry
      bool Remove(AppearanceKind Kind, const char* szModel, const char* szName);
      void Clear(void);
      bool HasModel(const char* szModel) const;
      std::vector<AppearanceEntry>& GetEntries(void);
      bool ReadElement(LPCWSTR szElement, LPCWSTR szModel, LPCWSTR szName, LPCWSTR szIntensity, LPCWSTR szColor); // false = rejected, table unchanged
      HRESULT Write(IXmlWriter* pWriter) const;
  };
  class CAppearanceItem : public IConfigItemXml { public: explicit CAppearanceItem(CAppearanceTable& Table); ... };
  ```
- Produces (Appearance.h, `namespace Appearance`):
  ```cpp
  CAppearanceTable& GetTable(void);
  bool HasMeshColor(CBehaviorAppBase* pEntity, const CMeshPart* pMesh);
  void RecordMeshColor(CBehaviorAppBase* pEntity, CMeshPart* pMesh, const ImVec4& vColor);
  void ResetMeshColor(CBehaviorAppBase* pEntity, CMeshPart* pMesh);
  bool HasModelTint(CBehaviorAppBase* pEntity);
  void RecordModelTint(CBehaviorAppBase* pEntity, const ImVec4& vColor);
  void ResetModelTint(CBehaviorAppBase* pEntity);
  bool SetMaterialTint(CBehaviorAppBase* pEntity, const CModelShader* pShader, const ImVec4& vColor, float flIntensity);
  void ClearMaterialTint(CBehaviorAppBase* pEntity, const CModelShader* pShader);
  void Update(void);        // once a frame from hkPresent
  void BeginLoad(void);     // before CConfigXml::Load from CMenu::LoadConfig
  bool IsDirty(void);
  void MarkSaved(void);
  ```
- Produces (Features.h): `inline bool Features::GetFirstPersonRestoreAlpha(const CBehaviorAppBase* pEntity, const char* szMesh, float& flAlpha);`. Returns true while first person hides `szMesh` on `pEntity`, with the alpha it will put back.

- [ ] **Step 1: Write the harness tests:**
  - `TableRoundTrip`: `Set` a mesh entry (`pl0000`/`Hair`, (1,0.25,0.25,1)), a model tint (`pl0000`, white), and a material (`pl0000`/`W_Skin`, intensity 1.5). `Write` into an in-memory `IXmlWriter` (`CreateStreamOnHGlobal`). Read it back through a reader, calling `ReadElement` on each element of a fresh table. Expect 3 entries with equal kind, model, name and intensity, and colour within 1/255.
  - `RejectsBadInput`: `ReadElement(L"mesh", L"pl0000", NULL, ...)` → false. Bad colour `L"#12"` → false. Unknown element `L"hat"` → false. `model_tint` with a name → true, and the name is ignored.
  - `SetKeepsOriginal`: `Set`, set `m_vOriginal` and `m_bHaveOriginal`, `Set` again with a new colour → the original is unchanged.
- [ ] **Step 2: Build the harness.** Expected: it fails to link (CAppearanceTable unresolved).
- [ ] **Step 3: Implement `AppearanceTable.cpp`.** Names convert UTF-16 ↔ UTF-8 with `WideCharToMultiByte` / `MultiByteToWideChar` and `CP_UTF8`. Intensity is written with `%.3f` and read with `_wtof`; it is clamped to [0, 50], the editor slider's range. `CAppearanceItem` derives from `IConfigItemXml` and is declared in `AppearanceTable.h`:
  - `GetName` returns `L"appearance"`, and `MatchesAnyName` returns true.
  - `ReadAttributes` stores the element's local name and its `model`, `name` and `intensity` attributes into members.
  - `ReadValue` calls `ReadElement` and clears the stored attributes.
  - `Write` calls the table's `Write`.

  It includes `ConfigItemXml.h` (Task 2), not `Configuration.h`, so it stays free of game headers.
- [ ] **Step 4: Implement `Appearance.cpp`:**
  - State: one `CAppearanceTable`. An applied set, `std::vector<Applied { EntityHandle_t m_hEntity; std::vector<const CModelShader*> m_Tints; }>`. A dirty flag. A once-only "out of tint slots" warning flag.
  - Model name: `ObjectIdToObjectName(sz, sizeof(sz), pEntity->m_pInfo->m_ObjectId, NULL)`.
  - `Record*` and `SetMaterialTint`:
    - Capture the original from memory before writing, if the entry is new or has no original yet.
    - Write memory (mesh `m_vColor` plus `m_bUpdate = TRUE`; or `m_vTint` plus `MODELINFO_FLAG_TINT_DIRTY`; or `MaterialTint::Set`).
    - `Set` the entry, set the dirty flag, and make sure the entity is in the applied set. Material tints record the shader there.
    - Mesh alpha: if `GetFirstPersonRestoreAlpha` returns true, store that alpha in the entry and leave the mesh's current alpha alone.
  - `Reset*` and `ClearMaterialTint`: write the original back if there is one, `Remove` the entry, set the dirty flag. Clearing a material calls `MaterialTint::Clear` and drops the shader from the owner list.
  - `Update`:
    1. Prune applied handles where `GetEntityFromHandleGlobal` returns NULL, and `MaterialTint::Clear` their shaders.
    2. Return if the table is empty.
    3. Walk the slots `0..g_pEntityList->m_uItems-1` (live as in `Menu.cpp:1603-1609`). For each handle not yet applied whose model name `HasModel`, resolve it with `GetEntityFromHandleGlobal`. Skip it if it is NULL, `m_Work.m_nMeshes <= 0` or `!m_Work.m_pMeshes` (it is retried next frame). Otherwise apply every entry for that model: mesh via `GetModelMesh`, model tint, and materials by name over `m_pModelShaders[0..m_nMaterialShaders)`. Capture originals first. Then add the handle to the applied set.
    4. If `MaterialTint::Set` fails, log the warning once (`WARN` plus `g_pConsole->Warn`).
  - `BeginLoad`: for each applied entity still alive, write back the originals of its model's entries and `MaterialTint::Clear` its shaders. Then clear the applied set and the table, and clear the dirty flag.
- [ ] **Step 5: Add the first-person accessor.** Move `s_szHeadMeshes`, `s_pHidden` and `s_flSavedAlpha` out of `UpdateFirstPersonHead` into `inline` namespace-scope variables (`g_szFirstPersonHeadMeshes`, `g_pFirstPersonHidden`, `g_flFirstPersonSavedAlpha`), with no change in behaviour. Add `GetFirstPersonRestoreAlpha` below them.
- [ ] **Step 6: Wire it up.** Set `MAX_TINTS` 32 → 64 in `MaterialTint.cpp:64`. In `InitializeConfig`, add `m_Items.emplace(CATEGORY_APPEARANCE, new CAppearanceItem(Appearance::GetTable()));`. Call `Appearance::Update();` just before `MaterialTint::Update();` in `Hooks.cpp:655`, so tints set this frame upload this frame. Add the files to the vcxproj and filters.
- [ ] **Step 7: IDA check.** Open `Z:\NieR Automata Debug\NieRAutomata.i64` (force_headless, no auto-analysis). Find where a new model's `CMeshPart` array is filled; start from `CModelWork` init and the writers of `m_nMeshes` (+0x18). Confirm `m_vColor` is set there, before `m_nMeshes` becomes non-zero. If something writes the colours later during entity setup, add that condition to the "has meshes" test in `Update`. Close with save false and delete only that database's unpacked leftovers.
- [ ] **Step 8: Rebuild and run the harness** (expected `8/8 passed`). Then build Debug (expected: 0 errors, no new warnings).

### Task 4: Menu wiring

**Files:**
- Modify: `Menu.h` (CMenu: `SaveAppearance`, `GetTargetConfigName`), `Menu.cpp` (`LoadConfig` l.371, `SaveConfig` l.388, `Draw` l.281, `ConfigTab` l.1295-1372, `ApplyPodMods` l.1634, `MaterialTintEditor` l.1692, `ApplyModelMods` l.1756)

**Interfaces:**
- Consumes: the `Appearance::*` functions (Task 3) and `CConfigXml::SaveSection` (Task 2).
- Produces: `bool CMenu::GetTargetConfigName(wchar_t (&szName)[MAX_PATH]);` (the config the Config tab's Save writes to; false if none) and `void CMenu::SaveAppearance(void);` (public).

- [ ] **Step 1: Config name and save/load.**
  - Factor the `szConfig` resolution out of `ConfigTab` into `GetTargetConfigName`; `ConfigTab` keeps its "No config selected" text when it returns false.
  - `SaveAppearance` calls `m_pConfig->SaveSection(name, CATEGORY_APPEARANCE)`, logs like `SaveConfig`, calls `Appearance::MarkSaved()` on success, and warns with the HRESULT on failure.
  - `SaveConfig` also calls `Appearance::MarkSaved()` on success.
  - `LoadConfig` calls `Appearance::BeginLoad()` before `m_pConfig->Load`.
- [ ] **Step 2: Unsaved markers.** The Config tab's `Save *` label and the tab's `ImGuiTabItemFlags_UnsavedDocument` test become `(m_Theme != m_SavedTheme || Appearance::IsDirty())`.
- [ ] **Step 3: Mesh colour pickers** (`ApplyModelMods` mesh, `ApplyPodMods` pod mesh):
  - Picker on a local copy of `m_vColor`; when it returns true, call `Appearance::RecordMeshColor(entity, mesh, copy)`. The entity is the pod for the pod picker.
  - Remove the unconditional per-frame `m_bUpdate = TRUE`.
  - Under the picker, a `Reset##Mesh` / `Reset##PodMesh` button, disabled (`ImGui::BeginDisabled`) unless `HasMeshColor`, calls `ResetMeshColor`.
- [ ] **Step 4: Model Tint.** Picker on a copy; when it returns true, call `RecordModelTint`. Add a `Reset##ModelTint` button, enabled only when `HasModelTint`.
- [ ] **Step 5: Material tints.** In `MaterialTintEditor`, `MaterialTint::Set` / `Clear` become `Appearance::SetMaterialTint` / `ClearMaterialTint(pEntity, pShader, ...)`. Keep the "no free slots" console warning on failure.
- [ ] **Step 6: Save Appearance button.** Right after the Rainbow Hair checkbox in `ApplyModelMods`: `ImGui::Button(Appearance::IsDirty() ? "Save Appearance *###SaveAppearance" : "Save Appearance###SaveAppearance")` → `g_pMenu->SaveAppearance()`. Tooltip: "Saves only the colours to the selected config. Settings you haven't saved stay unsaved."
- [ ] **Step 7: Build Debug and Release.** Expected: 0 errors, and no warnings in the touched files beyond those already in a clean build of `HEAD` (compare against a `HEAD` build log if anything shows up).

### Task 5: Finish

- [ ] **Step 1: Run the harness one last time** (`8/8`) and both builds.
- [ ] **Step 2: Review.** One fresh reviewer, on the most capable model, reads the whole diff against the spec. Fix what it confirms.
- [ ] **Step 3: Commit** the spec, this plan and the code in one feature commit on `master`. The message ends with `Co-Authored-By: Claude Opus 5.5 (<effort>) <noreply@anthropic.com>` (effort as Martin confirms), plus `Reviewed-by-model:` for the reviewer. Do not push.
- [ ] **Step 4: In-game checklist for Martin** (handed over, not run by Claude):
  1. Colour 2B's hair, press Save Appearance, relaunch: the hair keeps its colour.
  2. Do the same for Model Tint, a material tint (W_Skin) and the pod's Body. All survive a relaunch and an area reload.
  3. Change a toggle without saving, press Save Appearance, relaunch: the toggle is not saved.
  4. Load a config without appearance while customized: the colours go back to default.
  5. Edit hair colour in first person, leave first person, relaunch: the hair is visible.
  6. Do 10+ area loads with material tints saved: no "no free slots" warning.
  7. Open the new config with the 1.2.3.1 DLL: it loads, and the other settings are intact.
