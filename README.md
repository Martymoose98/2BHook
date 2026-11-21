# 2BHook

A mod menu for NieR:Automata: an ImGui overlay injected into the game, with gameplay, camera,
visual, spawning and debugging tools.

<!--- [![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE) --->

## Table of Contents
- [Supported Game Versions](#supported-game-versions)
- [Features](#features)
- [Building](#building)
- [Installing](#installing)
- [Usage](#usage)
- [Configuration](#configuration)
- [Troubleshooting](#troubleshooting)
- [Releasing](#releasing)
- [Todo](#todo)
- [Contact](#contact)

## Supported Game Versions

The `Debug` and `Release` configurations support:

- **Steam build 26534214** (the November 2026 update). It has no SteamStub or Denuvo wrapper,
  so `NieRAutomata.exe` works as installed.
- **Steam v1.02**, unpacked with Steamless (`NieRAutomata.exe.unpacked.exe`).

The running build is detected from the executable's hash at startup and logged.
`NieRAutomataCompat.exe` from build 26534214 is detected but untested.

The old Denuvo Steam build is no longer supported. The `Debug (Denuvo)` and `Release (Denuvo)`
project configurations are kept for reference, but they aren't in `2BHook.slnx` and don't build.

## Features

Legend: 🧪 experimental, ☢️ known not to work on the current build.

**Player**
- Godmode: take no damage. Uses the game's own debug invincibility flags.
- No Enemy Damage / No Fall Damage.
- Invincible Hacking Ship: the ship takes no hits in hacking and in the Ending E credits shooter. Godmode covers this too.
- Level, experience and money editing.
- Tick Base: scale the game's time.
- Speedhack: scale the game clock (QueryPerformanceCounter) by a multiplier.
- No Model Collision and Model Gravity toggles.
- Change Player, Play Animation, Airstuck.

**Camera**
- Free Camera: the game's own debug free camera, flown with keyboard and mouse (or the gamepad). 2B stays put while you fly.
- First Person: a head-mounted view on the normal follow camera. Head meshes are hidden, she turns to face the view while idle, and pitch is clamped.
- Camera FOV override.

**Visuals**
- ESP for the player and enemies: a box fitted to the skeleton (2D or 3D), HP bar, skeleton and trace line.
- Chams: enemies through walls, red where something hides them and green where you can see them, and NPC Chams for NPCs and Emil in blue and cyan. Use Shader switches the seen colour between flat and the game's own shading.
- Glow and NPC Glow: a glow around enemies (red) and NPCs and Emil (blue), through walls too.
- Enemy, NPC and world info text.
- Collision info: the loaded collision trees and the live collision objects' bounds.
- Wireframe.
- Rainbow Hair and Rainbow Pod.
- Hide Pod.
- Per-mesh colour and visibility, and Model Tint Color for the whole model (its alpha makes it see-through).
- Material Tint: give each material of a mesh its own colour (e.g. 2B's boots or tights apart from her skin). It multiplies with the mesh colour, so Rainbow Hair still shows.
- Move Sun: hold the sun, and the light and shadows it casts, at the elevation and azimuth you set.
- Motion: play one of the camera entity's own motions, or a .mot file from disk, through the game's own motion system.
- Menu themes: Default, YoRHa (the game's own menu colours) or Custom, with per-colour overrides, a resizable menu and UI scaling that follows DPI and resolution.

**Spawning**
- Entity spawner: every object in the game's construction table (enemies, NPCs, pods, weapons, items, map objects...), grouped by type, with a search box. Anything in it can be spawned; entries whose assets aren't loaded on the current map are marked, and their files are loaded before they're spawned.
- Spawn blacklist: stop chosen entities from being created.
- Item spawning by id or name, with Instant Equip.
- Buddy switching (2B, A2 or 9S as buddy), Unlink Buddy, Destroy Buddy.
- Unlock Pod B / Pod C.
- Enemy level balancing, and setting an entity's level.
- Entity list with delete.

**Miscellaneous**
- Play Sound / Emit Sound.
- 🧪 Emit Particle.
- Mount a custom CPK.
- Unlock All Achievements.
- No Tutorial Dialogs.
- Anti-VSync.
- Anti-Framerate Cap: removes the 60 FPS limiter. The game logic is frame-locked, so it also runs faster above 60 FPS.
- Frame Budget.
- Backup Save: copies the chosen save slot.
- Raw editors for the game's debug, camera and game flags.
- Heap Scale: make every game memory heap 1x to 4x bigger, for loading more than a map plans for. It takes effect the next time the game starts, and only with 2BHook in the `mods` folder.
- Console, with logging, and Show Game Errors for the game's CRI errors and warnings.

## Building

### Requirements
- Windows 10 or 11, x64.
- Visual Studio 2026 (v18) with the **Desktop development with C++** workload. The projects use the `v145` toolset, C++17 and the Windows 10/11 SDK.
- Git, with SSH access to GitHub for the `Fluorine` submodule (see below).

### Getting the source
```
git clone --recurse-submodules https://github.com/Martymoose98/2BHook.git
```
Two submodules are included:
- `imgui`: Dear ImGui.
- `Fluorine`: the memory and hooking library.

`.gitmodules` points Fluorine at its SSH URL (`git@github.com:Martymoose98/Fluorine.git`). Without an SSH key on GitHub, run this before updating submodules:
```
git config submodule.Fluorine.url https://github.com/Martymoose98/Fluorine.git
git submodule update --init --recursive
```

### Building
Open `2BHook.slnx` in Visual Studio, choose `Debug|x64` or `Release|x64`, and build. Fluorine builds first as a dependency.

From a Developer command prompt:
```
msbuild 2BHook.slnx -p:Configuration=Release -p:Platform=x64 -m
```
The DLL lands in `x64\<Configuration>\2BHook.dll`.

**Post-build copy:** a post-build step copies the DLL into a `mods` folder, by default `Z:\NieRAutomata\mods\` (the author's game folder). Point it at your own game with the `ModsDir` property, or an environment variable of the same name:
```
msbuild 2BHook.slnx -p:Configuration=Release -p:Platform=x64 -p:ModsDir="C:\Games\NieRAutomata\mods"
```
Turn it off for one build with `-p:PostBuildEventUseInBuild=false`. It is off automatically in CI, where the `CI` environment variable is `true`.

`Debug` builds add a few debug-only menu controls.

## Installing

2BHook is a DLL, so something has to load it into the game. Use one of these:

- **The 2B Hook Launcher** (recommended): the official launcher, currently v1.5.1. It's open source, and you can get it from the Files tab of the [2B Hook Nexus Mods page](https://www.nexusmods.com/nierautomata/mods/10?tab=files) or from GitHub at [Martymoose98/2B-Mod-Launcher](https://github.com/Martymoose98/2B-Mod-Launcher).
  1. Put `2BHook.dll` in the `mods` folder next to `NieRAutomata.exe`. The launcher creates that folder the first time it runs if it's missing.
  2. Run the launcher. It finds the game, starts it (or attaches to it if it's already running), and loads every DLL in `mods`.
- **A proxy DLL loader**: a DLL the game loads by itself, which then loads the DLLs from a `mods` folder.
  1. Put the loader in the game folder.
  2. Put `2BHook.dll` in `<game folder>\mods\`.
  3. Start the game.
- **A DLL injector** (e.g. Xenos or Extreme Injector).
  1. Start the game and wait for the main menu.
  2. Inject `2BHook.dll`. Injecting earlier can fail.

Prebuilt releases are on the [Releases](https://github.com/Martymoose98/2BHook/releases) page: `2BHook-<version>.zip` (the DLL and this README). They're also on the [Nexus Mods page](https://www.nexusmods.com/nierautomata/mods/10). Otherwise, build from source as above.

2BHook can run alongside other DLL mods. Mods with hard-coded addresses for an older game build, such as old builds of NASA, can crash the game on build 26534214, so update them first.

## Usage

Press **Insert** to open or close the menu. Closing it with the window's **X** also works. While the menu is open, the game doesn't receive keyboard or mouse input; Ignore Input in the Config tab turns that off.

The menu tabs are Gameplay, Visuals, Spawn, Miscellaneous, Config and Console.

Default keybinds (change them in the Config tab):

| Key | Action |
|---|---|
| Insert | Open or close the menu |
| F1 | First person on or off |
| F3 | Unlink buddy |
| F5 | Play the selected animation |
| F6 | Change player |
| F7 | Airstuck |
| F9 | Model gravity |
| F10 / F11 | Teleport forward / backward |
| Up / Down | Raise / lower the player |

Free camera controls:

| Input | Action |
|---|---|
| W / S | Forward / back |
| A / D | Strafe |
| Space / Left Ctrl | Up / down |
| Left Shift | 4× speed |
| Left Alt | ¼ speed |
| Mouse | Look |

The gamepad works through the game's own free camera controls.

## Configuration

Settings are saved as XML next to the DLL. `default.xml` loads at startup, and the Config tab can save and load named configs. Configs from older versions are migrated on load.

## Troubleshooting

- **Log:** `2BHook.log` in the game folder records the detected game build and runtime messages; check it first if a feature does nothing.
- **Crash dumps:** they're written to the game folder as `NieRAutomata_<date>_<time>.dmp`. Attach the dump and the log to a [GitHub issue](https://github.com/Martymoose98/2BHook/issues). A dump may not be written if you close the "has stopped working" dialog too early.
- **Wrong game version:** if the log says the build isn't recognised, see [Supported Game Versions](#supported-game-versions).

## Releasing

CI runs on GitHub Actions, on the `windows-2025-vs2026` runner (Visual Studio 2026, MSVC v145):

- **Build** (`.github/workflows/build.yml`) runs on every push and pull request to `master`. It builds Release x64 and keeps the DLL and PDB as a workflow artifact for 14 days. It never releases.
- **Release** (`.github/workflows/release.yml`) runs when a `v*` tag is pushed. It:
  1. takes the release notes from `.github/release-notes/<tag>.md` if the tagged commit has that file, or otherwise writes a changelog from the commits since the previous `v*` tag. It also checks the notes' Nexus summary (see below), before building, so bad notes fail in seconds;
  2. builds Release x64;
  3. packages `2BHook-<tag>.zip` (the DLL and README). The PDB isn't released; the Build workflow's artifact has it;
  4. creates the GitHub release with the zip and the notes;
  5. uploads the zip to Nexus Mods, if that is configured. It's uploaded as a new version of the main file, with the name `2BHook <tag>`, the version from the tag (`v1.2.0` becomes `1.2.0`), and the summary as its description. The mod page's version is set to match, and the changes are added to the mod's changelog as plain text.

To cut a release:
1. Set the version in `Version.h`. It's the only place it's set: the DLL's version resource, the menu title and the log all read it. A hotfix sets `HOOK_VERSION_HOTFIX` and is tagged with all four numbers (`v1.2.3.1`); its Nexus upload archives the version it replaces.
2. Optionally, add hand-written notes as `.github/release-notes/v<version>.md`, with a Nexus summary (see below).
3. Commit, then push Fluorine first if it has new commits, then 2BHook.
4. Push the matching tag:
   ```
   git tag v1.2.0
   git push origin v1.2.0
   ```

The Release workflow refuses a tag that doesn't match `Version.h`.
To release an existing tag again (for example after fixing the workflow), run **Release** from the Actions tab with that tag. It replaces the GitHub release's files and notes, but on Nexus it adds another version of the file, so delete the duplicate there, or delete the `NEXUSMODS_FILE_ID` variable first to skip the Nexus step.

#### The Nexus summary

Nexus shows a short description under each file, at most 255 characters. Put it in the notes file in a `nexus-summary` block. The block can go anywhere in the file, and its lines are joined with spaces:

````markdown
## 2BHook v1.2.0

```nexus-summary
November 2026 Update. Supports the new Steam build and v1.02 unpacked.
```

The rest of the notes...
````

The block is taken out of the GitHub release notes. The job fails before building if the summary is over 255 characters (counted the way the Nexus API counts them), if it's empty or not closed, or if there's more than one block. Without a block, the summary is the notes' opening paragraph (the first text that isn't a heading), shortened to 255 characters at a word boundary. For a release with no notes file, that's the first few commit subjects.

### Repository secrets and variables

Set these under **Settings > Secrets and variables > Actions**. The GitHub release needs none of them; it uses the workflow's own token. Without all three, the Nexus Mods step is skipped with a notice.

| Name | Kind | Value |
|---|---|---|
| `NEXUSMODS_API_KEY` | Secret | A Nexus Mods API key, from your account's API key settings (Site Preferences, API Keys). |
| `NEXUSMODS_MOD_ID` | Variable | The mod's v3 API id. This is not the number in the mod page URL: look it up with `curl -H "apikey: <key>" https://api.nexusmods.com/v3/games/<game>/mods/<url number>` and take `data.id` (2B Hook: `8375186227210`). |
| `NEXUSMODS_FILE_ID` | Variable | The v3 id of the file that each release adds a new version to. This is not the file id in download links: list them with `curl -H "apikey: <key>" https://api.nexusmods.com/v3/mods/<mod id>/files` and take the active file's `id` (2B Hook: `820696`). |

The upload goes through `Nexus-Mods/upload-action`, which uses Nexus Mods' v3 Upload API (currently in open beta). It adds a new version to an existing file, so upload the first file to the mod page by hand.

## Todo

- More features.
- Teleport to waypoints, points of interest and objectives.
- Fix or remove the controls listed as not working.

## Old Denuvo IDA Database
https://mega.nz/folder/PiRDHSiZ#6BLoGaVzAEuVKdoJGTcmTA

## Contact

- [NieR:Automata Modding Discord](https://discord.gg/v8Yyrrg): Martino#1552 (probably the best way).
- GitHub: open an issue here.
