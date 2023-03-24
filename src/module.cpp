/*
module.cpp - C++ module configuration file.
Copyright (C) 2023  Makonede

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/


#define MODULE_NAME "Link Cannon"
#define MODULE_NAME_LEN 11


struct ModuleName {
  int unknown;
  int name_length;
  char name[MODULE_NAME_LEN + 1];
};


__attribute__((section(".nx-module-name")))
const ModuleName s_ModuleName = {.unknown = 0, .name_length = MODULE_NAME_LEN, .name = MODULE_NAME};
