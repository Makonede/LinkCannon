/*
botw.cpp - C++ file for used functions from Breath of the Wild.
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


export module botw;

#define _GNU_SOURCE

#include <KingSystem/Event/evtManager.h>
#include <KingSystem/Event/evtMetadata.h>

#include <controller/seadControllerMgr.h>


export namespace botw {
  extern unsigned char *Memory;
}

export namespace ksys::evt {
  Metadata::~Metadata() {
    if (mOrderParam) {
      delete mOrderParam;
      mOrderParam = nullptr;
    }
  }
}
