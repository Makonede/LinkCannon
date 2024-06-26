# Link Cannon

<picture>
  <img alt="BTB" src="https://repository-images.githubusercontent.com/594929966/db09df55-f0ea-40e7-bd3e-dc1a84c60455">
</picture>

A mod for The Legend of Zelda: Breath of the Wild (Switch 1.6.0) that aims to
help with BTB setups.

## Installation

### Prerequisites

- A homebrewed Nintendo Switch running [Atmosphère](https://github.com/Atmosphere-NX/Atmosphere)
- [Python] 3.12+
- A dump of `Pack/Bootup.pack`, `Pack/Bootup_USen.pack`, and `System/Resource/ResourceSizeTable.product.srsizetable`
from Breath of the Wild 1.6.0

To install Link Cannon:

- Download the [latest release](https://github.com/Makonede/LinkCannon/releases/latest)'s
`.zip` file (alternatively, the `.tar.xz` file if on UNIX) and extract it.
- Place your dumps of `Bootup.pack` and `Bootup_USen.pack` in the `dump/Pack` folder
in the folder that you extracted the `.zip` (or `.tar.xz`) to — the folder also
containing `patch.py`, `patches`, and `content`. Place your dump of
`ResourceSizeTable.product.srsizetable` in `dump/System/Resource`.
- Run `patch.py`.
- If you have an FTP server such as [`sys-ftpd`](https://github.com/cathery/sys-ftpd)
on your Switch, fill in the fields in `ftp.toml` and run `ftp.py` to install directly
to your Switch.
- If you don't, extract the resulting `patched.zip` (or `patched.tar.xz`) file to
the root of your microSD card.

> [!NOTE]
> If you are asked to overwrite existing files, click `Yes`.

> [!WARNING]
> There is a high chance that this mod will not work with other mods. Using other
> mods in combination with Link Cannon is not supported — do so at your own risk.

## Updating the mod

To update the mod, simply uninstall the mod with the instructions below and then
install the latest version.

## Uninstallation

> [!WARNING]
> If you are using Link Cannon with other mods, this step will remove all of your
> other Breath of the Wild mods.

To uninstall Link Cannon, delete the folder `/atmosphere/contents/01007EF00011E000`
from your microSD card.

## Usage

Once the mod is installed, it will automatically load when starting the game. To
launch Breath of the Wild with Link Cannon temporarily disabled, hold
<picture>
  <img alt="L" src="https://restite.org/dpad/l">
</picture>
after selecting a user.

<picture>
  <img alt="R, ZR, and Y" src="https://restite.org/dpad/rRy">
</picture>

This button combination will open the BTB simulation dialog — from there, just follow
the given instructions.

## Building from source

### Prerequisites

- Windows 7+, Linux, or macOS — `x86_64` architecture
- devkitPro [`switch-dev`](https://switchbrew.org/wiki/Setting_up_Development_Environment)
(devkitA64 and libnx)
- [Python] 3.12+

### Steps

1. Clone the repository recursively.
2. On UNIX:

   - Open a shell.
   - `cd` into the respository directory.
   - Run these commands:

   ```bash
   chmod +x compile setup_build
   ./setup_build
   ```

   On Windows:

   - Open File Explorer.
   - Navigate to the repository directory.
   - Double-click `setup_build.bat`.

3. Place your dumps of `Bootup.pack` and `Bootup_USen.pack` in `dump/Pack` and your
dump of `ResourceSizeTable.product.srsizetable` in `dump/System/Resource`.
4. To build on UNIX, run:

   ```bash
   ./compile
   ```

   To build on Windows, double-click `compile.bat`.

### FTP installation

Link Cannon can be installed over FTP to your Switch.

1. Copy the configuration template from `config_templates/ftp.toml` to `config/`.
2. Fill in the fields (username and password are optional).
3. Run `ftp.py`.

## Credits

- [Pistonight](https://github.com/Pistonight) for his [save state mod](https://github.com/Pistonight/botw-save-state)
which I used as a scaffold, for his [`botw-link` tool](https://github.com/Pistonight/botw-symbols/tree/botw-link),
and for helping debug and find mistakes.
- [Shadów](https://github.com/shadowninja108) for helping with decompilation, for
his [exlaunch](https://github.com/shadowninja108/exlaunch) framework, for helping
debug crashes, and for his [`crt0` template](https://github.com/shadowninja108/exlaunch/blob/main/source/lib/init/crt0.s).
- [ZeldaRET](https://zelda64.dev) and [OpenEAD](https://github.com/open-ead)
for their decompilations of [Breath of the Wild](https://botw.link), [sead](https://github.com/open-ead/sead),
and the [Nintendo SDK](https://github.com/open-ead/nnheaders).
- [Prahaha](https://www.twitch.tv/prahaha) and [NotSoZany](https://www.twitch.tv/notsozany)
for the original inspiration to create this mod.
- [savage13](https://restite.org) for his [button image generator tool](https://restite.org/dpad).

[Python]: https://www.python.org/downloads/
