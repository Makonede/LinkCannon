#!/usr/bin/env python3
# -*- coding: utf_8 -*-

# patch.py - IPS patching Python script.
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


import ips

from pathlib import Path
from shutil import copytree


def list_files(path: Path) -> list[Path]:
    return [p for p in path.rglob('*') if p.is_file()]


def main() -> None:
    copytree('dump', 'patched')
    dumped_files: list[Path] = list_files(Path('patched'))

    for file in dumped_files:
        ips.apply(f'{Path("patches", *file.parts[1:])}.ips', f'{file}')


if __name__ == '__main__':
    main()
