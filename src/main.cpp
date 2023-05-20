/*
main.cpp - Main C++ file.
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


#include <controller.hpp>
#include <server.hpp>
#include <utility.hpp>

#include <nn/os.h>

#include <cstdlib>


// Main loop thread
// The function is named _main in order to not be treated as the main function.
[[noreturn]] auto _main([[maybe_unused]] auto *unused) noexcept {
  ksys::evt::Manager *eventManager = nullptr;
  do [[unlikely]] {
    eventManager = ksys::evt::Manager::instance();
    Yield();
  } while (eventManager == nullptr);

  sead::ControllerMgr *controllerManager = nullptr;
  do [[unlikely]] {
    controllerManager = sead::ControllerMgr::instance();
    Yield();
  } while (controllerManager == nullptr);

  sead::Controller *controller = nullptr;
  do [[unlikely]] {
    controller = controllerManager->getController(0);
    Yield();
  } while (controller == nullptr);

  const auto BUTTON_COMBO = btn::R | btn::ZR | btn::Y;
  const auto EVENT_METADATA = ksys::evt::Metadata("LinkCannon");

  while (true) [[likely]] {
    // Wait until the button combination is pressed
    while (!controller->isHoldAll(
      static_cast<unsigned int>(BUTTON_COMBO)
    )) [[likely]] {
      Yield();
    }

    eventManager->callEvent(EVENT_METADATA);

    // Wait until at least one of the buttons in the combination is released
    while (controller->isHoldAll(
      static_cast<unsigned int>(BUTTON_COMBO)
    )) [[unlikely]] {
      Yield();
    }
  }
}


// Initialization function (entrypoint)
extern "C" auto LinkCannon_init() noexcept {
  static nn::os::ThreadType mainThread;

  // Allocate memory for the main thread stack
  constexpr auto STACK_SIZE = 0x80000uz;
  constexpr auto ALIGNMENT = 0x1000uz;
  auto *stack = aligned_alloc(ALIGNMENT, STACK_SIZE);

  if (stack == nullptr) [[unlikely]] {
    return;
  }

  // Attempt to create the main thread
  constexpr auto PRIORITY = 16;

  if (nn::os::CreateThread(
    &mainThread, _main, nullptr, stack,
    static_cast<unsigned long long>(STACK_SIZE), PRIORITY, 0
  ).IsFailure()) [[unlikely]] {
    // Free the thread stack if it fails
    free(stack);
    return;
  }

  // Start the thread
  nn::os::StartThread(&mainThread);
}


// Finalization function
extern "C" auto LinkCannon_fini() noexcept {}
