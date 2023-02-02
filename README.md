# Link Cannon
<picture>
  <img alt="BTB" src="https://repository-images.githubusercontent.com/594929966/db09df55-f0ea-40e7-bd3e-dc1a84c60455">
</picture>

A mod for The Legend of Zelda: Breath of the Wild (Switch 1.6.0) that aims to help with BTB setups.

## Installation
### Prerequisite
- A homebrewed Nintendo Switch running [Atmosphère](https://github.com/Atmosphere-NX/Atmosphere)

To install Link Cannon, download the [latest release](https://github.com/Makonede/LinkCannon/releases/latest) and extract it onto the root of your microSD card.

> **Note**
> If you are asked to overwrite existing files, click `Yes`.

> **Warning**
> There is a high chance that this mod will not work with other mods. Using other mods in combination with Link Cannon is not supported — do so at your own risk.

## Updating the mod
To update the mod, simply uninstall the mod with the instructions below and then install the latest version.

## Uninstallation
> **Warning**
> If you are using Link Cannon with other mods, this step will likely remove all of your other Breath of the Wild mods.

To uninstall Link Cannon, delete the folder `/atmosphere/contents/01007EF00011E000` from your microSD card.

## Usage
Once the mod is installed, it will automatically load when starting the game. To launch Breath of the Wild with Link Cannon temporarily disabled, hold ![L](https://restite.org/dpad/l) after selecting a user.

![D-pad Up and Right Stick](https://restite.org/dpad/8o)

This button combination will open the BTB simulation dialog — from there, just follow the given instructions.

> **Note**
> The button combination will not work if any other buttons are pressed.

## Building from source
### Prerequisite
- devkitPro [`switch-dev`](https://switchbrew.org/wiki/Setting_up_Development_Environment) (devkitA64 and libnx)

### Steps
1. Clone the repository.
2. To build on UNIX:
- Open a shell.
- `cd` into the repository directory.
- Run these commands:
```bash
chmod +x compile
./compile
```
To build on Windows:
- Open File Explorer.
- Navigate to the repository directory.
- Double-click `compile.bat`.
