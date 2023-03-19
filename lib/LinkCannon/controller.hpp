/*
controller.hpp - Header file for helper functions for controllers.
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

#include <botw.hpp>


enum class btn : unsigned int {
  A = 0x00000001,
  B = 0x00000002,
  ZL = 0x00000004,
  X = 0x00000008,
  Y = 0x00000010,
  ZR = 0x00000020,
  R3 = 0x00000040,
  L3 = 0x00000080,
  MINUS = 0x00000200,
  PLUS = 0x00000400,
  HOME = 0x00000800,
  CAPTURE = 0x00001000,
  L = 0x00002000,
  R = 0x00004000,
  TOUCH = 0x00008000,
  UP = 0x00010000,
  DOWN = 0x00020000,
  LEFT = 0x00040000,
  RIGHT = 0x00080000,
  L_UP = 0x00100000,
  L_DOWN = 0x00200000,
  L_LEFT = 0x00400000,
  L_RIGHT = 0x00800000,
  R_UP = 0x01000000,
  R_DOWN = 0x02000000,
  R_LEFT = 0x04000000,
  R_RIGHT = 0x08000000
};


inline btn operator|(btn lhs, btn rhs) noexcept {
  return static_cast<btn>(
    static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs)
  );
}


inline auto holdingOnly(sead::Controller *controller, btn buttons) noexcept {
  return controller->isHoldAll(
    static_cast<unsigned int>(buttons)
  ) && !controller->isHold(
    ~static_cast<unsigned int>(buttons)
  );
}
