/*
botw.hpp - Header file for used functions and variables from Breath of the Wild.
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


#pragma once

#include <controller/seadControllerMgr.h>
#include <math/seadMatrix.hpp>
#include <prim/seadSafeString.hpp>


extern bool callEvent(
  void *, sead::SafeString *, sead::SafeString *, sead::Matrix34<float> *, bool,
  bool
) noexcept;

extern sead::ControllerMgr *controllerMgr;
