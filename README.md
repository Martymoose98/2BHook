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
- Enemy, NPC and world info text.
- Collision info: the loaded collision trees and the live collision objects' bounds.
- Wireframe.
- Rainbow Hair and Rainbow Pod.
- Hide Pod.
- Per-mesh colour and visibility.
- Material Tint: give each material of a mesh its own colour (e.g. 2B's boots or tights apart from her skin). It multiplies with the mesh colour, so Rainbow Hair still shows.
- Menu themes: Default, YoRHa (the game's own menu colours) or Custom, with per-colour overrides, a resizable menu and UI scaling that follows DPI and resolution.

**Spawning**
- Entity spawner: every object in the game's construction table (enemies, NPCs, pods, weapons, items, map objects...), grouped by type, with a search box. Entries whose assets aren't loaded on the current map can't be spawned.
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
- Console, with logging.

**Not working on the current build:** ☢️ Chams, ☢️ Use Shader, ☢️ Model Tint Color, ☢️ Show Game Errors.

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

**Post-build copy:** a post-build step copies the DLL to `Z:\NieRAutomata\mods\`, the author's game folder. Change the path in the project's Post-Build Event to your own game's `mods` folder, or turn it off for a build:
```
msbuild 2BHook.slnx -p:Configuration=Release -p:Platform=x64 -p:PostBuildEventUseInBuild=false
```

`Debug` builds add a few debug-only menu controls.

## Installing

2BHook is a DLL, so something has to load it into the game. Use one of these:

- **A proxy DLL loader** (recommended): a DLL the game loads by itself that then loads the DLLs from a `mods` folder.
  1. Put the loader in the game folder.
  2. Put `2BHook.dll` in `<game folder>\mods\`.
  3. Start the game.

  The author's own loader, 2B Mod Loader, is built as `xinput1_4.dll` and isn't part of this repository. Any proxy loader that loads DLLs from a folder works.
- **A DLL injector** (e.g. Xenos or Extreme Injector).
  1. Start the game and wait for the main menu.
  2. Inject `2BHook.dll`.

  Injecting earlier can fail.

Prebuilt DLLs are published on the [Releases](https://github.com/Martymoose98/2BHook/releases) page when available; otherwise build from source as above.

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

## Todo

- More features.
- Teleport to waypoints, points of interest and objectives.
- Fix or remove the controls listed as not working.

## Old Denuvo IDA Database
https://mega.nz/folder/PiRDHSiZ#6BLoGaVzAEuVKdoJGTcmTA

## Contact

- [NieR:Automata Modding Discord](https://discord.gg/v8Yyrrg): Martino#1552 (probably the best way).
- GitHub: open an issue here.
