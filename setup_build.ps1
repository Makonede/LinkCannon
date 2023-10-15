# setup_build.ps1 - Build setup PowerShell script for Windows.
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

[Diagnostics.CodeAnalysis.SuppressMessageAttribute(
    'PSUseDeclaredVarsMoreThanAssignments', ''
)]
$PSNativeCommandUseErrorActionPreference = $true
$ErrorActionPreference = 'Stop'


New-Item dump\Pack -ItemType Directory -Force
New-Item dump\System\Resource -ItemType Directory -Force

if (!(Test-Path .venv)) { py -3 -m venv .venv }
.venv\Scripts\Activate.ps1
py -3 -m pip install -U pip
py -3 -m pip install -U wheel
py -3 -m pip install -U setuptools
py -3 -m pip install -Ur tools\botw-link\requirements.txt
py -3 -m pip install -U vidua
