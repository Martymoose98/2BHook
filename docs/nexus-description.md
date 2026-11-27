[size=5][b]2B Hook[/b][/size]
[size=3]A mod menu for NieR:Automata[/size]

2B Hook is an in-game menu (Insert) with gameplay, camera, visual, spawning and debugging tools. It's open source: the code, releases and issue tracker are on [url=https://github.com/Martymoose98/2BHook]GitHub[/url].

[b]Version 1.2.4[/b] supports the November 2026 Steam update (build 26534214) and the v1.02 build unpacked with Steamless. It saves your colour edits (hair, Model Tint, material tints and the pod) in the config, so they come back every launch, and Heap Overflow keeps the game going when a memory heap fills up; 1.2.3 added a Glow ESP and chams through every wall, and 1.2.2 brought back Chams, Model Tint Color and Show Game Errors and added Move Sun, a motion player and Heap Scale. See the top of the changelog below. The old Denuvo build is no longer supported.

[b]If 2B Hook works for you, please endorse it![/b] Endorsements are how other players find it. Got an idea for a feature? Post it in the Posts tab.

[line]
[size=4][b]Features[/b][/size]

[b]Player[/b]
[list]
[*]Godmode (the game's own debug invincibility), No Enemy Damage, No Fall Damage
[*]Invincible Hacking Ship: the ship takes no hits in hacking or the Ending E credits shooter
[*]Level, experience and money editing
[*]Tick Base (time scale) and Speedhack
[*]No Model Collision, Model Gravity, Airstuck, Change Player, Play Animation
[/list]

[b]Camera[/b]
[list]
[*]Free Camera: the game's own debug free camera, flown with keyboard and mouse or the gamepad. 2B stays put while you fly.
[*]First Person: a head-mounted view on the normal camera. She turns to face where you look.
[*]Camera FOV override
[/list]

[b]Visuals[/b]
[list]
[*]ESP for the player and enemies: a box fitted to the skeleton (2D or 3D), HP bar, skeleton and trace line
[*]Chams: enemies through walls in red and green, NPCs and Emil in blue and cyan
[*]Glow: a glow around enemies (red) and NPCs (blue), through walls too
[*]Enemy, NPC and world info, plus collision info
[*]Rainbow Hair, Rainbow Pod, Hide Pod
[*]Per-mesh colour and visibility, and Model Tint Color for the whole model
[*]Material Tint: give each material its own colour, e.g. 2B's boots or tights apart from her skin
[*]Colour edits saved in the config and put back every launch (Save Appearance)
[*]Wireframe
[*]Move Sun: hold the sun, and the light and shadows it casts, where you want it
[*]Motion: play the camera entity's own motions, or a .mot file from disk, through the game's own motion system
[*]Menu themes: Default, YoRHa (the game's own menu colours) and Custom, with UI scaling
[/list]

[b]Spawning[/b]
[list]
[*]Entity spawner: every object in the game's construction table (enemies, NPCs, pods, weapons, items, map objects), grouped by type, with search. Anything in it can be spawned; objects whose assets aren't loaded on the current map are marked, and their files are loaded before they're spawned.
[*]Spawn blacklist
[*]Item spawning with Instant Equip
[*]Buddy switching (2B, A2, 9S), Unlink Buddy, Destroy Buddy
[*]Unlock Pod B / C
[*]Enemy level balancing, and setting an entity's level
[/list]

[b]Miscellaneous[/b]
[list]
[*]Play and emit sounds
[*]Mount a custom CPK
[*]Unlock All Achievements
[*]No Tutorial Dialogs
[*]Anti-VSync
[*]Anti-Framerate Cap (the game runs faster above 60 FPS)
[*]Backup Save
[*]Heap Scale: bigger game memory heaps, for loading more than a map plans for
[*]Heap Overflow: extra memory heaps when one fills up, instead of the out-of-memory box
[*]A console with logging, and Show Game Errors
[/list]

[line]
[size=4][b]Installation[/b][/size]

[b]With the 2B Hook Launcher (recommended)[/b]
The official launcher (v1.5.1) is open source. Get it from this page's Files tab or [url=https://github.com/Martymoose98/2B-Mod-Launcher]GitHub[/url].
[list=1]
[*]Put [b]2BHook.dll[/b] in the [b]mods[/b] folder next to NieRAutomata.exe. The launcher creates the folder if it's missing.
[*]Run the launcher. It finds the game, starts it (or attaches to it if it's already running) and loads every DLL in [b]mods[/b].
[/list]

[b]With a DLL injector[/b]
Start the game, wait for the main menu, then inject [b]2BHook.dll[/b] with an injector such as Xenos or Extreme Injector.

Each release on [url=https://github.com/Martymoose98/2BHook/releases]GitHub Releases[/url] has the DLL and the README.

[b]Note:[/b] other DLL mods with hard-coded addresses for older game builds can crash the game on the November 2026 update. Update them first.

[line]
[size=4][b]Controls[/b][/size]

Change the keybinds in the menu's Config tab.
[list]
[*][b]Insert[/b]: open or close the menu
[*][b]F1[/b]: first person
[*][b]F3[/b]: unlink buddy
[*][b]F5[/b]: play the selected animation
[*][b]F6[/b]: change player
[*][b]F7[/b]: airstuck
[*][b]F9[/b]: model gravity
[*][b]F10 / F11[/b]: teleport forward / backward
[*][b]Up / Down[/b]: raise / lower the player
[/list]

[b]Free camera[/b]: W/S/A/D to move, Space / Left Ctrl for up / down, Left Shift fast, Left Alt slow, mouse to look.

[line]
[size=4][b]Known issues[/b][/size]
[list]
[*]Chams and Glow colour an enemy's own model only; weapons and parts drawn from a separate model keep their normal look.
[*]First person also hides the head's shadow.
[/list]

[line]
[size=4][b]Bugs and crash reports[/b][/size]

Crash dumps (.dmp) and [b]2BHook.log[/b] are written to the game folder. Attach both to a [url=https://github.com/Martymoose98/2BHook/issues]GitHub issue[/url], or post in the [url=https://discord.gg/v8Yyrrg]NieR:Automata Modding Discord[/url] (xmmword).

[line]
[size=4][b]Endorsements and feature requests[/b][/size]

If 2B Hook makes the game more fun for you, an endorsement (the button at the top of this page) helps a ton. It's what pushes the mod up Nexus's lists so other players find it.

Want something added? Post your idea in the Posts tab, open a [url=https://github.com/Martymoose98/2BHook/issues]GitHub issue[/url], or message me on the [url=https://discord.gg/v8Yyrrg]NieR:Automata Modding Discord[/url] (xmmword). I read them all.

[line]
[size=4][b]Changelog[/b][/size]
[spoiler]
[b]Version 1.2.4[/b] (November 2026)
[i]New[/i]
[list]
[*][b]Saved colours:[/b] mesh colours (hair and the rest), Model Tint Color, material tints and the pod's mesh colours are saved in the config and put back whenever that model appears: after a relaunch, an area load or a respawn.
[*][b]Save Appearance[/b] (Visuals): saves only the colours, leaving the config's other settings as they are. The Config tab's Save saves them too.
[*]Reset buttons for mesh colours, Model Tint and pod mesh colours.
[*][b]Heap Overflow[/b] (Miscellaneous, off by default): when a game memory heap is full, what it can't fit goes to an extra heap instead of the out-of-memory box. Extra heaps are made 256 MB at a time, up to 4 GB. It works however 2BHook is loaded.
[/list]
[i]Fixes[/i]
[list]
[*][b]Material Tint:[/b] a tint's slot is freed when its model unloads, and there are 64 slots, up from 32.
[/list]

[b]Version 1.2.3.1[/b] (November 2026)
[list]
[*]Chams and Glow work with MSAA on. In 1.2.3 they showed nothing with it, and before that they could put the light's view of the enemies on the screen.
[/list]

[b]Version 1.2.3[/b] (November 2026)
[i]New[/i]
[list]
[*][b]Glow[/b] and [b]NPC Glow[/b] (Visuals): a soft glow around enemies (red) and NPCs and Emil (blue), through walls too.
[/list]
[i]Fixes[/i]
[list]
[*][b]Chams:[/b] the hidden colour shows through every wall, not only the walls the game drew before the enemy. Use Shader now only changes the seen colour.
[*]Closing the game with the window's X no longer hangs it.
[*][b]Spawner:[/b] Entity Scale starts at 1,1,1 (it was 0,0,0, so spawns were invisible until it was changed).
[*]Entity View is as wide as the window, and Display Enemy Info is three lines under the ESP box.
[/list]

[b]Version 1.2.2[/b] (November 2026)
[i]Working again[/i]
[list]
[*][b]Chams:[/b] an ESP for enemies, red where something is in front of them and green where you can see them. The new NPC Chams does the same for NPCs and Emil in blue and cyan. Use Shader switches between flat colours and the game's own shading.
[*][b]Model Tint Color:[/b] the picker changes the model's colour again, and its alpha makes the model see-through.
[*][b]Show Game Errors:[/b] the game's CRI errors and warnings show in the Console.
[/list]
[i]New[/i]
[list]
[*][b]Move Sun[/b] (Visuals): holds the sun, and the light and shadows it casts, at the elevation and azimuth you set.
[*][b]Motion[/b] (Visuals): plays one of the camera entity's own motions, or a .mot file from disk, through the game's own motion system, with a slot, blend, rate and loop.
[*][b]Heap Scale[/b] (Miscellaneous): makes every game memory heap 1x to 4x bigger. It takes effect the next time the game starts, and only with 2BHook in the mods folder. Each step adds about 4.4 GB of committed memory.
[/list]
[i]Fixes[/i]
[list]
[*]Deleting a save no longer builds its file name by printing the path into itself.
[*]The texture loader no longer writes the image through a pointer that was never allocated.
[/list]

[b]Version 1.2.1[/b] (November 2026)
[i]Fixes[/i]
[list]
[*][b]No Model Collision:[/b] turning it off no longer snaps you back to the first wall you walked through. You stay where you are, and collision starts again from there.
[/list]
[i]Entity spawner[/i]
[list]
[*]Anything in the list can be spawned again. When an object's assets aren't loaded on the current map, Create Entity has the game load its files first and spawns it once they're in, the way the game's own spawner does.
[*]Entries whose assets aren't loaded are still greyed with --, and the "Loaded only" filter still hides them.
[*]Effects (ef), map pieces (sc) and the game's no-preload objects can be spawned too. Their !! mark is now a warning: the game normally gives these their model and data when it creates them, so a free spawn may show the dummy model or not work.
[/list]
[i]Other[/i]
[list]
[*][b]Console:[/b] the log fills the Console tab and grows when you resize the menu or play at a higher resolution, instead of staying a small fixed box.
[*]The DLL's version info names the product 2BHook.
[/list]

[b]Version 1.2.0[/b] (November 2026: Steam build 26534214)
[i]Game support[/i]
[list]
[*][b]Steam build 26534214[/b], the November 2026 update. It has no DRM wrapper, so NieRAutomata.exe works as installed.
[*][b]v1.02 unpacked with Steamless[/b] still works with the same DLL. The build is detected at startup and written to 2BHook.log.
[*]The old Denuvo build is no longer supported.
[/list]
[i]Free camera[/i]
[list]
[*]Uses the game's own debug free camera instead of fighting the normal camera.
[*]Fly with [b]WASD[/b], [b]Space / Left Ctrl[/b] (up / down), [b]Shift[/b] (fast), [b]Alt[/b] (slow) and the mouse. The gamepad works too.
[*]2B stays put while you fly: the game doesn't receive keyboard and mouse input meanwhile, except Escape.
[/list]
[i]First person[/i]
[list]
[*]A head-mounted view on the normal follow camera; event cameras keep their own view.
[*]Pitch is clamped and the view is limited relative to her body. She turns to face where you look while idle.
[*]A small near plane is used, and the head meshes are hidden, so the camera doesn't clip into her. Arms and weapons stay visible.
[*]The F1 keybind toggles it.
[/list]
[i]ESP[/i]
[list]
[*]The box now fits the skeleton (2D or 3D), with the name under it and the HP bar beside it.
[*]Enemies get the box, HP bar and skeleton too, not just their info text.
[/list]
[i]Entity spawner[/i]
[list]
[*]Every object in the game's construction table (1,134 entries) can be spawned: enemies, NPCs, pods, weapons, items, map objects and more.
[*]Grouped by type, with a search box (name, object name, class or hex id) and a "Loaded only" filter.
[*]Uses the game's own asset check. Entries whose assets aren't loaded on the current map can't be spawned. Objects that only the map or effect systems can create are blocked.
[/list]
[i]Pods[/i]
[list]
[*]Rainbow Pod and the pod mesh controls work again.
[*]Hide Pod restores every pod it hid when unticked.
[*]Spawn Pod B/C are now [b]Unlock Pod B/C[/b], which is what they do.
[/list]
[i]Material tint (#16)[/i]
[list]
[*]Give each material of a mesh its own colour, e.g. 2B's boots or tights apart from her skin.
[*]An intensity setting can brighten near-black materials.
[*]Tints multiply with the mesh colour, so Rainbow Hair still shows through.
[/list]
[i]Menu and themes[/i]
[list]
[*]Theme presets: Default, YoRHa (the game's own menu colours) and Custom, with per-colour overrides.
[*]The menu is resizable and scales with DPI and resolution.
[*]Checkboxes have clear contrast in every theme.
[*]Configs are read by name and versioned, and older configs are migrated.
[/list]
[i]Fixes[/i]
[list]
[*][b]Godmode[/b] uses the game's own debug invincibility.
[*][b]Invincible Hacking Ship (#14):[/b] the ship takes no hits in hacking and the Ending E shooter, and normal ship damage comes back when the toggle is off.
[*][b]Rainbow Hair[/b] and the mesh [b]Enabled[/b] checkbox work again; both had been doing nothing since 2024.
[*][b]Set Level / Balance Enemy Levels[/b] no longer delete the enemy instead of levelling it.
[*][b]Unlock All Achievements, Instant Equip, Speedhack, Camera FOV, Create UI Element, NPC info and the Emil teleports[/b] work on this build.
[*][b]Collision info[/b] draws the boxes where the collision actually is.
[*][b]Anti-Framerate Cap[/b] really removes the 60 FPS limiter. The game's logic is frame-locked, so it also runs faster above 60 FPS.
[*][b]No Tutorial Dialogs, Load CPK and Backup Save[/b] work on both supported builds.
[*]Fixed a crash when the game exits, and a menu that never rendered.
[*]Fixed DirectInput locking with the Steam overlay.
[/list]
[i]Known issues[/i]
[list]
[*]Chams, Use Shader, Model Tint Color and Show Game Errors don't work on this build yet.
[*]First person hides the head's shadow along with the head.
[*]Mods with hard-coded addresses for older builds can crash the game on build 26534214. For example, NASA needs its build 26534214 update.
[/list]

[b]Version 1.1.0.0 (Revelations Update)[/b]
[list]
[*]Changed version scheme to Maj.Min.Patch.Hotfix (to be more normalized)
[*]Fixed level scaling
[*]Fixed a crash caused by valve's gamerendereroverlay64.dll!HookedDirectInput8Create (Big Shutouts)
[*]Code refactoring
[*]Upgraded internal SDK :) very private, much useful
[*]Added color themes
[*]Various fixes and crashes
[*]Fixed Airstuck
[*]Fixed Rainbow Hair
[/list]

[b]Version 0.23[/b]
[list]
[*]Fixed a bug that would cause NierAutomata to crash on dinput8.dll!Common_Release everytime the NierAutomata was closed normally instead of through the debug console
[*]Fixed the "CreateEntity" hook which fixes "Spawn Blacklist" and other cool things
[*]Fixed no clip reverting the player to the last collison position
[*]Fixed level scaling
[*]Code refactoring
[/list]

[b]Version 0.22[/b]
[list]
[*]Updated for new unpacked Steam version
[*]Fixed a bug with config menu
[*]Fixed a bug in SanitizePath function
[*]Added camera fov changer (default fov is 50.0 degrees)
[*]Code refactoring
[/list]

[b]Version 0.21[/b]
[list]
[*]Fixed a crash if 2BHook was injected before loading in a save.
[/list]

[b]Version 0.20[/b]
[list]
[*]Removed feature "Temporary level" since it was pretty much useless
[*]Removed feature "Ghost model" since it was again useless and caused game crashes. Additionally, the same effect can be achieved by modifying the mesh alpha values.
[*]Added firsperson camera mode breaks when the game forces the camera angle. Still have to find out why, bound to F1 by default
[*]Added loading custom textures from .dds files very primitive will improve in later updates.
[/list]

[b]Version 0.19[/b]
[list]
[*]Fixed traceline to properly locate the head bone
[*]Fixed the fatal assertion on navigation to the config tab
[*]Fixed the config system not loading, saving or overwriting configs when desired
[/list]

[b]Version 0.18[/b]
[list]
[*]Fixed a issue where a keybind wouldn't work if it wasn't bound in the config
[*]Improved world esp (OBB's for valid objects, the rest are AABB)
[*]Fixed a issue where items would spawn in a random quantity
[*]Added configuration enumerator for greater ease of access
[*]Added item spawn quantity
[*]Added chams
[/list]

[b]Version 0.17[/b]
[list]
[*]Added absolute level balancing mode
[*]Added Emil to spawnable entities
[*]Added Emil to show info when NPC info is toggled
[*]Added Unlock all achievements function
[*]Added 3D ESP box for protagonists
[*]Added World ESP
[*]Changed the Open Menu keybind to be user changeable
[/list]

[b]Version 0.16[/b]
[list]
[*]Menu overhaul
[*]Added a console (logs what the game creates etc... with filters)
[*]Fixed a bug with bones
[*]Added the ability to set each entity's level individually
[*]Updated config with new values for console
[/list]

[b]Version 0.15[/b]
[list]
[*]Added option to re-balance enemy levels with tolerance
[*]Added spawn blacklist
[*]Added an option for the user to choose if the menu blocks input
[*]Updated config with new values
[*]Miscellaneous optimizations
[*]Fixed a bone related crash
[/list]

[b]Version 0.14[/b]
[list]
[*]Fixed default.ini getting created in the game directory. Now all configs will be created in 2bhook.dll's folder.
[*]Fixed some miscellaneous config bugs
[/list]

[b]Version 0.13: ('18 Christmas Update)[/b]
[list]
[*]Added No Horizontal Collision
[*]Added UI support for Custom configs (save & load)
[*]Added Entity Spawning! (9S, A2 (probably will crash), 2B, White Moose & Boar, Moose & Boar) more on the way! With model scale control
[*]Added Entity Wet Option
[*]Added Pod Colour Control
[*]Added Rainbow pod
[*]Added Hide pod
[*]Made NPC Info Toggleable
[/list]

[b]Version 0.12[/b]
[list]
[*]Added a toggle for displaying enemy info
[*]Fixed speedmeister (speedhack)
[*]Added keybind vfunc support
[*]Added a keybind for playing animations
[*]Added teleport forward and play animation
[*]Improved the logging system (debug)
[/list]

[b]Version 0.11[/b]
[list]
[*]Fixed a crash due to stack alignment (square root function)
[*]Corrected distances due to the same square root function
[*]Added Trace Line with length control
[*]Added Bone enumeration
[*]Added Skeleton Esp
[*]Added Rainbow Hair
[*]Added A Configuration System
[*]Added Rainbow hair
[*]Added No Model Gravity
[*]Added Y Position Model Gravity Manipulation
[*]Unrestricted what save names can be (no banned words)
[/list]

[b]Version 0.10: (The spoiled update)[/b]
This update was a bad update on nexus mods because i uploaded the wrong version probably version 0.09
due to build paths.
[list]
[*]Game will now ingore any keyboard and mouse input when menu is open
[*]Added player position
[*]Made the overlay renderer work and added new functionality
[*]Added create entity log (debug only)
[*]Fixed a fatal crash that only occured on windows 7
[/list]

[b]Version 0.09[/b]
[list]
[*]Added Item Spawning (some items refuse to spawn, hopefully I can fix this later)
[*]Added Sound playing and emitting
[*]Added Pod Spawning
[*]Added No Fall Damage
[*]Added No Enemy Damage
[*]Fixed Entity handle enumeration (now more concise and and no null entites)
[*]Added Custom model part color manipulation (hair color has a odd artifact currently; This will be fixed in later releases!)
[*]Added An Animation Implementation (First 58 animations, more will be added) NOTE: 27 WILL CRASH YOU FOR SOME REASON this is the one marked "[Access Violation]"
[/list]

[b]Version 0.08[/b]
[list]
[*]Fixed a crash where if you opened the menu in a loading/any state where localplayer was invalidated
[/list]

[b]Version 0.07[/b]
[list]
[*]Fixed all crashes where the enity pointer would invalidate due to death/save reload etc...
[*]Fixed weird artifacts pertaining to "Ghost Model"
[/list]

[b]Version 0.06[/b]
[list]
[*]Added Buddy spawning feature (2B, A2, 9S)
[*]Added Buddy destroy feature
[*]Added Make Buddy/NPC Follow (EM, NPC, Yorha) (Experimental)
[*]Added Duplicate buddy as NPC feature (Hotkey: F3)
[/list]

[b]Version 0.05 (The Panic Fix Update)[/b]
[list]
[*]Fixed the "Ghost Model" feature so that it can be applied to more than the localplayer. With this fix came the decision of unghosting the old camera-focused entity.
[*]Added custom model tints
[/list]

[b]Version 0.04[/b]
[list]
[*]Added the ability to rotate the main character model
[*]Added the ability to scale the main character model
[*]Added the "Ghost Model" feature
[*]Fixed leaking handles on writing a dump file
[/list]

[b]Version 0.03[/b]
[list]
[*]Added crashdumps (to help with error reporting). NOTE: It will also make a dump if my mod isn't at fault if I recall correctly.
[/list]

[b]Version 0.02[/b]
[list]
[*]Added the Release version
[*]Fixed the dependency to load a save file before injecting
[/list]

[b]Version 0.01[/b]
[list]
[*]Inital Release (Debug build only)
[/list]
[/spoiler]
