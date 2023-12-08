#!/usr/bin/env python3
# -*- coding: utf_8 -*-

# ftp.py - FTP installation Python script.
# Copyright (C) 2023  Makonede
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


import tomllib
from ftplib import FTP
from pathlib import Path, PurePosixPath
from typing import TypedDict

TITLE_ID = '01007EF00011E000'
LAYEREDFS = PurePosixPath('atmosphere/contents')
RELEASE = Path('release') / LAYEREDFS


class Config(TypedDict):
    ip_address: str
    port: int
    username: str
    password: str


def upload_mod(ftp: FTP, path: Path=Path()) -> None:
    for child in (RELEASE / path).iterdir():
        if child.is_file():
            with open(child, 'rb') as file:
                print(f'Uploading {child.name}')
                ftp.storbinary(f'STOR {child.name}', file)
        elif child.is_dir():
            directory = f'{child.name}'
            print(f'Entering {directory}')
            ftp.mkd(directory)
            ftp.cwd(directory)
            upload_mod(ftp, path / child)

    if path != Path():
        ftp.cwd('..')


def main() -> None:
    with open(Path('config') / 'ftp.toml', 'rb') as toml:
        config: Config = tomllib.load(toml)  # type: ignore

    address: str = config['ip_address']
    port: int = config['port']
    username: str = config['username']
    password: str = config['password']

    print('Connecting...')

    with FTP(
        address, username, password, source_address=(address, port)
    ) as ftp:
        print(f'Connected to {address}:{port}')
        ftp.cwd(f'{LAYEREDFS}')

        # Check for Breath of the Wild LayeredFS
        if any(d[0] == TITLE_ID for d in ftp.mlsd()):
            # Check for files in Breath of the Wild LayeredFS
            if any(d[0] not in '..' for d in ftp.mlsd(TITLE_ID)):
                overwrite = input('Mods are installed, overwrite? (y/n) ')
                if not overwrite or overwrite[0] != 'y':
                    return

            # Remove Breath of the Wild LayeredFS
            ftp.rmd(TITLE_ID)

        # Upload Link Cannon
        upload_mod(ftp)

    print('\nLink Cannon was installed successfully.')


if __name__ == '__main__':
    main()
