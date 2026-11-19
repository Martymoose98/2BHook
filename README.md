# 2B Hook

A Mod Menu for NieR:Automata.

<!--- [![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE) --->

## Table of Contents
- [Introduction](#introduction)
- [Features](#features)
- [Supported Game Versions](#supported-game-versions)
- [Installation](#installation)
- [Usage](#usage)
- [Documentation](#documentation)
- [Contributing](#contributing)
<!---- [License](#license) --->
- [Acknowledgments](#acknowledgments)

## Introduction

2B Hook is a Mod Menu for NieR:Automata

## Features

Appendix: 🪲 - Buggy 🧪 - Exparimental ☢️ - Broken

- Godmode: Take no damage.
- No Fall Damage: No fall damage.
- No Enemy Damage: No damage from enemies.
- Invincible Hacking Ship: The ship takes no hits in hacking and in the Ending E credits shooter. Godmode covers this too.
- Level Manipulation: Configures enemy levels and local player level.
- Buddy Spawning: Spawn in the buddy of your choice.
- Item Spawning: Spawn in items by id or name.
- Pod Spawning: Spawn in any pod.
- Model Affine Matrix Manipulation:
- Model Parts Colour Manipulation:
- Protagonist Switching: Freely switch between 
- Airstuck: Float in the air.
- Duplicate Buddy as NPC: Duplicate the current buddy as an NPC.
- NPC, Yorha and Enemy Entity Handle enumeration
- No Localplayer Gravity: Removes gravity for the localplayer.
- No Localplayer Collision: Removes collision detection for the localplayer.
- 🧪 Set NPC Follow: 
- 🧪 Set NPC Idle:
- 🧪 Fps Uncapper: Removes the 60 FPS limit.
- 🧪 No Tutorial Dialogs: 
- 🧪 Save Backup: Makes a copy of the loaded save.
- Wireframe: Render the scene as a wireframe.


## Supported Game Versions

The `Debug` and `Release` configurations support:

- Steam build 26534214 (the November 2026 update). This build has no SteamStub or Denuvo wrapper, so `NieRAutomata.exe` works as installed.
- Steam v1.02 unpacked with Steamless (`NieRAutomata.exe.unpacked.exe`).

`NieRAutomataCompat.exe` from build 26534214 is detected but untested. The free camera and `SetLookAtNoDistance` signatures don't match it.

The old Denuvo Steam build is no longer supported. The `Debug (Denuvo)` / `Release (Denuvo)` project configurations are kept for reference, but they aren't in `2BHook.slnx` and don't currently build.

## Installation

Since this mod is a dynamic link library (DLL) it cannot be used on it's own and needs a DLL injector such as Extreme Injector or Xenos etc... 
Alternatively, a proxy DLL loader will also suffice.

Acquire a DLL injector or proxy DLL loader.

Download the 2B Hook DLL here (Choose the latest release build unless you know what you are doing)

Open the injector and select the DLL

Open NieR:Automata (It's probably best to wait until the game is in the main menu. Not doing so could result in undefined behaviour!)

Inject!

Any crash dumps (.dmp files) are located in the same folder as NieR:Automata. They should be relayed to me (see Contact section) or create a github issue if you want it to get fixed!
It might not dump if you close the "NieR:Automata has stopped working" dialog prematurely.

## Usage

## Todo

- Add more features of course
- Add teleport (to waypoint, poi, or objective and forward)
- Add an overlay renderer [In Progress]

## Old Denuvo IDA Database:
https://mega.nz/folder/PiRDHSiZ#6BLoGaVzAEuVKdoJGTcmTA

## Contact:

There are mutiple ways to contact me:

[NieR:Automata Modding](https://discord.gg/v8Yyrrg)

Martino#1552 - on discord (probably the best way)

GitHub - You are here :D

