# compile.ps1 - Autobuild PowerShell script.
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


$TID = '01007EF00011E000'
$RELEASE_PATH = "release\atmosphere\contents\$TID"

(Test-Path build) ? (Remove-Item build\* -Recurse -Force) : (
    New-Item build -ItemType Directory -Force
)
py -3 tools\botw-link config\botw-link.toml -cvV 160
if (Test-Path release) { Remove-Item release -Recurse -Force }
if (Test-Path patched) { Remove-Item patched -Recurse -Force }
New-Item $RELEASE_PATH\exefs -ItemType Directory -Force
Copy-Item build\LinkCannon.nso $RELEASE_PATH\exefs\subsdk9 -Recurse -Force
Copy-Item build\app.npdm $RELEASE_PATH\exefs\main.npdm -Recurse -Force
Copy-Item romfs $RELEASE_PATH -Recurse -Force
py -3 patch.py
Copy-Item patched\* $RELEASE_PATH\romfs -Recurse -Force

Write-Host @'

Build complete. To install, copy the contents of the release folder onto the
root of your microSD card.

Press any key to continue...
'@

[void][System.Console]::ReadKey($true)
