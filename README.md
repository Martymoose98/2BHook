# 2B Hook
A Mod Menu For NieR:Automata

#What it does:

2B Hook is a mod menu, which means it has multiple features inside one mod. Here are some of the features:

Godmode

No Fall

No Enemy Damage

Level Manipulation

Buddy Spawning

Item Spawning

Pod Spawning

Model Scale and Rotation and Tint Manipulation

Model parts colour manipulation

Protagonist Switching

Airstuck

Duplicate buddy as NPC

NPC, Yorha and enemy entity handle enumeration

Set NPC Follow

Set NPC Idle

Fps Uncapper

No Tutorial Dialogs

Save Backup

"Ghost Model"

Wireframe (D3D11)

If you want to help out or contribute contact me (See Below). If it gets enough interest I'll most likely make it open source.
Special thanks to Marky for play testing and an a lot of good ideas!

Installation:

Since this mod is a dynamic link library (DLL) it cannot be used on it's own and needs a DLL injector such as extreme injector or xenos etc... 
alternatively you can make your own.

Download/Make a DLL injector

Download the 2B Hook DLL here (Choose the latest release build unless you know what you are doing)

Open the injector and select the DLL

Open NieR:Automata (It's probably best to wait until the game is in the main menu. Not doing so could result in undefined behaviour!)

Inject!

Any crash dumps (.dmp files) are located in the same folder as NieR:Automata. They should be relayed to me (see Contact section) if you want it to get fixed!
It might not dump if you close the "NieR:Automata has stopped working" dialog prematurely.

# Hotkeys:

F6 - Change Player

F3 - Duplicate Buddy As NPC

F2 - Put buddy in flight suit (Can't reverse at the moment, might be removed due to lack of usefulness)

F8 - Airstuck (Allows you to phase through solid objects & get stuck in mid-air)

# To-Do:

 [OPTIONAL] Make an official injector/launcher
 
Add more features of course

Add a config for custom hotkeys

Add teleport (to waypoint, poi, or objective and forward)

Add an overlay renderer

# Change Log:

Version 0.01:

-Inital Release (Debug build only)

Version 0.02:

-Added the Release version

-Fixed the dependency to load a save file before injecting

Version 0.03:

-Added crashdumps (to help with error reporting). NOTE: It will also make a dump if my mod isn't at fault.

Version 0.04:

-Added the ability to rotate the main character model

-Added the ability to scale the main character model

-Added the "Ghost Model" feature

-Fixed leaking handles on writing a dump file

Version 0.05 (The Panic Fix Update):

-Fixed the "Ghost Model" feature so that it can be applied to more than the localplayer. With this fix came the decision
of unghosting the old camera-focused entity.

-Added custom model tints

Version 0.06:

-Added Buddy spawning feature (2B, A2, 9S)

-Added Buddy destroy feature

-Added Make Buddy (EM, NPC, Yorha) (Experimental)

-Added Duplicate buddy as NPC feature (Hotkey: F3)

Version 0.07:

-Fixed all crashes where the enity pointer would invalidate due to death/save reload etc...

-Fixed weird artifacts pertaining to "Ghost Model"

Version 0.08:

-Fixed a crash where if you opened the menu in a loading/any state where localplayer was invalidated

# Contact:

There are mutiple ways to contact me:

NieR:Automata Modding
https://discord.gg/v8Yyrrg

Martino#1552 - on discord (probably the best way)

Reddit  - (PM or post on the thread)

GitHub

