#!/usr/bin/env python3
# -*- coding: utf_8 -*-

# patch.py - BPS patching Python script.
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


from vidua import bps

from pathlib import Path
from shutil import copytree


def list_files(path: Path) -> list[Path]:
    return [p for p in path.rglob('*') if p.is_file()]


def main() -> None:
    copytree('dump', 'patched')
    dumped_files: list[Path] = list_files(Path('dump'))

    for file in dumped_files:
        with open(file, 'rb') as dump, open(
            f'{Path("patches", *file.parts[1:])}.bps', 'rb'
        ) as patch, open(Path('patched', *file.parts[1:]), 'wb') as patched:
            patched.write(bps.patch(dump, patch).read())
            print(f'Patched {Path(*file.parts[1:])}')


if __name__ == '__main__':
    main()
