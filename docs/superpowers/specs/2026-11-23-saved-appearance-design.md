# Saved appearance (colours that survive a relaunch)

Short design, 2026-11-23.

## Intent

Players asked for their colour customizations (hair and so on) to stay put. Today every colour
edit in the Gameplay tab's model panel writes straight into game memory, so the colours reset every launch,
and also whenever the model reloads (area change, save load). The edits should go in the config XML
next to every other setting.

## Goals

- Save, keyed by model name (`pl0000` = 2B, etc.) and mesh or material name:
  - mesh colours (the mesh colour picker, on any model the camera is on)
  - Model Tint (`CModelInfo::m_vTint`)
  - per-material tints (`MaterialTint`)
  - pod mesh colours (pods are entities with their own model name, so these are mesh colours too)
- A separate **Save Appearance** button in the model panel that writes only the appearance part of
  the selected config and leaves the rest of the file as it is on disk.
- The Config tab's Save still writes everything, appearance included. Load replaces the appearance
  (an absent `<Appearance>` means no customizations, the same way Load treats theme overrides).
- Saved colours are set **once**, when a model with saved entries appears.

## Non-goals

- Hidden meshes (the mesh "Enabled" checkbox) stay session-only, as they are now.
- No auto-save.
- No holding colours every frame. The game is free to change a colour after spawn, and Rainbow
  Hair, Rainbow Pod, first person's head hiding and Hide Pod keep working as they do.

## Anti-goals

- Saving first person's alpha 0 (head meshes) or Hide Pod's alpha 0 as the user's colour. That would
  leave hair invisible after a relaunch.
- Save Appearance writing any setting other than the appearance (unsaved toggles must not reach disk).
- A failed or half-written Save Appearance damaging the existing config file.
- Material tint slots running out because tints for models that have since unloaded are never freed.
  (They aren't freed today; spawn-time tints would make it happen after a few area loads.)

## Approaches considered

1. **Set once on spawn** (picked by Martin): the colours are written once, when an entity with saved
   entries first shows up with its meshes. The editors keep writing memory directly and also record
   the edit. Light, and it never competes with other features. Risk: if the game itself rewrites a
   mesh colour later (cutscene, costume change), that colour stays reverted until the model reloads.
2. **Hold every frame**: put back any RGB that differs from the saved colour on every frame. More
   robust, but it means walking the whole entity list and comparing colours every frame. Not picked.
3. **Snapshot at save time** (read the camera entity's colours when saving): would lose every model
   that isn't the camera entity at that moment. Also it can't tell an edited colour from the
   default. Dropped.

Save button: **appearance-only** rewrite (picked by Martin) rather than a shortcut to the full Save.

## Design

### XML

A new section, following the rule in `Configuration.h` that settings added after a section shipped
get a section of their own. Colours use the theme's `#RRGGBBAA` (`Theme::FormatColor` / `ParseColor`).

```xml
<Appearance num_children="1">
  <mesh model="pl0000" name="Hair">#FF4040FF</mesh>
  <mesh model="pl1010" name="Body">#40FF40FF</mesh>
  <model_tint model="pl0000">#FFE0E0FF</model_tint>
  <material model="pl0000" name="W_Skin" intensity="1.50">#FFD0C0FF</material>
</Appearance>
```

The config version stays 2: version 2 loaders already skip sections they don't know.

### Config loader and writer (`Configuration.h/.cpp`)

- `IConfigItemXml` gets a `virtual void ReadAttributes(IXmlReader*)`, which does nothing by default.
  `CConfigXml::Load` calls it when it finds an item's element at depth 2, before that element's text.
  One `CAppearanceItem` handles every element in `<Appearance>`. It matches any element name (so
  `FindItem` needs a way for an item to say so), reads the attributes, then takes the colour in
  `ReadValue` and adds the entry. Its `Write` writes every entry.
- `CConfigXml::SaveSection(file, section)`: streams the existing file through an `IXmlReader`
  into an `IXmlWriter` on a temp file in the same folder. It copies every node except that
  section's subtree and whitespace (the writer re-indents). It writes the fresh section just
  before `</Root>`, then `MoveFileExW(..., MOVEFILE_REPLACE_EXISTING)`. If the file doesn't parse,
  the original is left alone and the error is returned. If the file doesn't exist, it writes a
  Root holding only that section.

### Appearance module (new `Appearance.h/.cpp`)

- The table: entries keyed by model name plus mesh name, material name, or "model tint". Each
  entry holds the colour (plus intensity for materials) and, at runtime only, the original
  colour, captured the first time the entry touches that model.
- `Record*` functions, which the editors call when a picker reports a change: they capture the
  original from memory before the edit if the entry is new, then store the colour. When first
  person is hiding a head mesh, the alpha recorded is the one first person will put back, not 0.
- `Forget*` / reset functions: restore the original on the camera entity and drop the entry.
- `Update()`, called once a frame from hkPresent beside `MaterialTint::Update()`:
  - Walks the entity list. Any live entity whose model has entries, has meshes, and whose handle
    is not in the applied set gets its saved mesh colours, model tint and material tints written
    once, and its handle goes into the applied set.
  - Drops applied handles that no longer resolve (`GetEntityFromHandleGlobal`), and clears the
    material tints that entity owned, so tint slots are freed when models unload.
- On config Load: put the originals back on live entities for the old entries, clear the material
  tints it owns, clear the applied set, then read the new table (applied next frame).
- A dirty flag: set by any record or forget, cleared by Save, Save Appearance and Load.

### Menu (`Menu.cpp`)

- Mesh colour picker, pod mesh colour picker, Model Tint picker and material tint editor edit
  through the Appearance module (pickers work on a copy and record on change). They stop setting
  `m_bUpdate` every frame and set it only when the colour changes.
- A **Reset** button next to each colour picker. Unticking a material tint already removes it.
- **Save Appearance** in the model panel (`Save Appearance *` while dirty) writes to the same
  config the Config tab would save to. The name resolution is factored out of `ConfigTab` into
  a helper so both buttons share it.
- The Config tab's `Save *` marker and unsaved-document dot also show for appearance changes.

### MaterialTint

- `MAX_TINTS` 32 → 64: spawn-time tints apply to every copy of a model.
- Log once when slots run out.

## Constraints

- Steam build 26534214 and v1.02; no new signatures needed.
- `/W4` clean; unused parameters get `UNREFERENCED_PARAMETER`; struct layout checks use
  `VALIDATE_OFFSET` / `VALIDATE_SIZE`.
- Never push or release before Martin tests it in game.

## Left to the builder

- Check in IDA (NieRAutomata.i64) where `CMeshPart::m_vColor` is first set for a new model,
  so the spawn-time write lands after the game's own initialization. If the game writes it
  later than the first frame the meshes exist, wait for that point.
- How the item tells `FindItem` it takes any element name (a flag, or a section-wide fallback).
- Entity-list walk details (which slots hold models, the object-name lookup).
