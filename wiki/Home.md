# Banana Native wiki

Banana Native runs Switch 2 games from your own dumps on Android phones. It emulates no hardware: the game's own ARM64 code runs on the phone's CPU, and Banana Native supplies the system services the game calls and translates its graphics to Vulkan. This wiki is for players: how to install, set up, play and fix things. It is unofficial and experimental, and not affiliated with Nintendo, nxnative or Eden.

![Donkey Kong Bananza on Banana Native](images/donkey-kong-bananza.jpg)

![Banana Native Home: a row of game tiles with the dock at the bottom and the round Add game button](images/home-launcher.jpg)

## Start here

New? Follow the [Quick start](Quick-Start). New words are explained in the [Glossary](Glossary). The longer path:

1. Check the [requirements](Installation#what-you-need) (Android 13+, Snapdragon/Adreno GPU, 12 GB RAM or more).
2. [Install the app](Installation).
3. [Add and Prepare your game](First-Start-and-Prepare), once per game.
4. [Play](Playing).

## Pages

| Topic | Pages |
|---|---|
| Setup | [Quick start](Quick-Start), [Glossary](Glossary), [Installation](Installation), [First start and Prepare](First-Start-and-Prepare), [Updates](Updates), [Live shader building](Live-Shader-Building) |
| Playing | [Playing](Playing), [Saves and storage](Saves-and-Storage), [GPU drivers](GPU-Drivers) |
| Settings and speed | [Settings overview](Settings-Overview), [Getting higher FPS](Getting-Higher-FPS), [Heat and battery](Heat-and-Battery), [Cheats](Cheats), [Mods](Mods) |
| Games | [Games](Games) |
| Devices | [Device reports](Device-Reports), [Tested devices](Tested-Devices) |
| Problems | [Troubleshooting](Troubleshooting), [FAQ](FAQ) |

## Good to know

> **Fork note.** This page comes from Banana Native (moceure). Banana Native itself takes no donations. This repository is OoT Native, an independent port maintained by PiSCES, which accepts voluntary donations ([Support](https://github.com/P1R4N351/oot-android#support)); they do not go to Banana Native.

- The only official place for upstream Banana Native is [git.eden-emu.dev/moceure/BananaNative](https://git.eden-emu.dev/moceure/BananaNative). Download the OoT Native APK only from this repository's [releases](https://github.com/P1R4N351/oot-android/releases). The project has no Discord, subreddit, social media account or website, and nobody connected to it will ask you for money, keys or game files.
- The project takes only a dump folder (or a `.nup` file of one) that you made from your own console. It contains no Nintendo code, firmware or keys. Please don't ask for games, keys or dumps.
- Questions and reports go to [issues](https://git.eden-emu.dev/moceure/BananaNative/issues).
