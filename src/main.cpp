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


constexpr auto PORT = 52617;


// Event caller thread
[[noreturn]] auto EventThread([[maybe_unused]] auto *unused) noexcept {
  // Wait for the event manager instance
  ksys::evt::Manager *eventManager = nullptr;
  do [[unlikely]] {
    eventManager = ksys::evt::Manager::instance();
    Yield();
  } while (eventManager == nullptr);

  // Wait for the controller manager instance
  sead::ControllerMgr *controllerManager = nullptr;
  do [[unlikely]] {
    controllerManager = sead::ControllerMgr::instance();
    Yield();
  } while (controllerManager == nullptr);

  // Wait for the controller instance
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


// Network data thread
auto NetworkThread([[maybe_unused]] auto *unused) noexcept {
  // Initialize the server
  Server server;

  while (!server.Init(PORT)) [[unlikely]] {
    Yield();
  }

  // Connect to the client
  while (!server.Connect()) [[unlikely]] {
    Yield();
  }

  // TODO: Send meaningful data
  // Simple network cat: read data and send it back
  constexpr auto DATA_SIZE = 0x1000uz;

  for (auto i = 0; i < 10; ++i) [[likely]] {
    server.Send(server.Read(DATA_SIZE));
  }
}


// Initialization function (entrypoint)
extern "C" auto LinkCannon_init() noexcept {
  static nn::os::ThreadType eventThread;
  static nn::os::ThreadType networkThread;

  // Allocate memory for the thread stacks
  constexpr auto STACK_SIZE = 0x80000uz;
  constexpr auto ALIGNMENT = 0x1000uz;

  auto *eventStack = aligned_alloc(ALIGNMENT, STACK_SIZE);
  auto *networkStack = aligned_alloc(ALIGNMENT, STACK_SIZE);

  if (eventStack == nullptr || networkStack == nullptr) [[unlikely]] {
    return;
  }

  // Attempt to create the threads
  constexpr auto PRIORITY = 16;

  if (nn::os::CreateThread(
    &eventThread, EventThread, nullptr, eventStack,
    static_cast<unsigned long long>(STACK_SIZE), PRIORITY, 0
  ).IsFailure() || nn::os::CreateThread(
    &networkThread, NetworkThread, nullptr, networkStack,
    static_cast<unsigned long long>(STACK_SIZE), PRIORITY, 0
  ).IsFailure()) [[unlikely]] {
    // Free the thread stacks if it fails
    free(eventStack);
    free(networkStack);
    return;
  }

  // Start the threads
  nn::os::StartThread(&eventThread);
  nn::os::StartThread(&networkThread);
}


// Finalization function
extern "C" auto LinkCannon_fini() noexcept {}
