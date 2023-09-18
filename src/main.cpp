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


#define _GNU_SOURCE

import std.compat;

#include <nn/os.h>

#include <controller.hpp>
#include <server.hpp>


constexpr auto PORT = static_cast<unsigned short>(52617u);


// Event caller thread
[[noreturn]] constexpr auto EventThread(
  [[maybe_unused]] auto *unused
) noexcept {
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


// Memory watcher thread
[[noreturn]] auto WatchThread(void *serverPtr) noexcept {
  auto *server = static_cast<Server *>(serverPtr);

  // Watch the requested memory addresses
  while (true) [[likely]] {
    // Wait for data to be available
    std::unique_lock<std::mutex> lock(server->watchedMutex);
    server->watchedCv.wait(lock, [server] {
      return !server->watched.empty();
    });

    // Read from each memory address
    auto watched = std::map(server->watched);
    lock.unlock();

    for (const auto &[address, size] : watched) [[likely]] {
      auto code = "DATA"s;

      if (!server->StartMessage(Server::end::SERVER, code)) [[unlikely]] {
        break;
      }

      // Send the address and size
      server->Send(std::vector(
        reinterpret_cast<const unsigned char *>(&address),
        reinterpret_cast<const unsigned char *>(&address) + 8uz
      ));
      server->Send(std::vector(
        reinterpret_cast<const unsigned char *>(&size),
        reinterpret_cast<const unsigned char *>(&size) + 8uz
      ));

      if (!server->ReadAck()) [[unlikely]] {
        break;
      }

      // Send the data
      server->Send(std::vector(
        botw::Memory + address, botw::Memory + address + size
      ));

      if (!server->ReadAck()) [[unlikely]] {
        break;
      }
    }
  }
}


// Network data thread
constexpr auto NetworkThread([[maybe_unused]] auto *unused) noexcept {
  // Initialize the server
  Server server;

  while (!server.Init(PORT)) [[unlikely]] {
    Yield();
  }

  // Connect to the client
  while (!server.Connect()) [[unlikely]] {
    Yield();
  }

  // Start the memory watcher thread
  static nn::os::ThreadType watchThread;
  constexpr auto STACK_SIZE = 0x80000uz;
  constexpr auto ALIGNMENT = 0x1000uz;
  auto *watchStack = aligned_alloc(ALIGNMENT, STACK_SIZE);

  if (watchStack == nullptr) [[unlikely]] {
    return;
  }

  constexpr auto PRIORITY = 16;

  if (nn::os::CreateThread(
    &watchThread, WatchThread, static_cast<void *>(&server), watchStack,
    static_cast<unsigned long long>(STACK_SIZE), PRIORITY, 0
  ).IsFailure()) [[unlikely]] {
    free(watchStack);
    return;
  }

  nn::os::StartThread(&watchThread);

  // Start processing messages
  while (true) [[likely]] {
    // Receive a message code
    std::string code;
    server.StartMessage(Server::end::CLIENT, code);

    if (code == "ADDR"s) {
      // ADDR - add address to watch
      auto address = *reinterpret_cast<std::size_t *>(server.Read(8uz).data());
      auto size = *reinterpret_cast<std::size_t *>(server.Read(8uz).data());

      // Start watching the address or update its maximum size
      {
        const std::lock_guard<std::mutex> lock(server.watchedMutex);
        server.watched[address] = size;
      }

      // Notify the watcher thread that data is available and acknowledge the
      // message
      server.watchedCv.notify_one();
      server.Ack();
    }
    else if (code == "RADD"s) {
      // RADD - stop watching address
      auto address = *reinterpret_cast<std::size_t *>(server.Read(8uz).data());

      // If the address is being watched, stop watching it
      {
        const std::lock_guard<std::mutex> lock(server.watchedMutex);
        auto watchedAddr = server.watched.find(address);

        if (watchedAddr != server.watched.end()) [[likely]] {
          server.watched.erase(watchedAddr);
          server.Ack();
        }
        else [[unlikely]] {
          server.Nack();
        }
      }

      server.watchedCv.notify_one();
    }
    else if (code == "DATA"s) {
      // DATA - write data to address
      auto address = *reinterpret_cast<std::size_t *>(server.Read(8uz).data());
      auto size = *reinterpret_cast<std::size_t *>(server.Read(8uz).data());
      server.Ack();

      // Read and write the data
      const auto *data = server.Read(size).data();
      std::memcpy(
        static_cast<void *>(botw::Memory + address),
        static_cast<const void *>(data), size
      );

      server.Ack();
    }
  }
}


// Initialization function (entrypoint)
extern "C" constexpr auto LinkCannon_init() noexcept {
  nn::os::ThreadType eventThread;
  nn::os::ThreadType networkThread;

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
[[maybe_unused]] extern "C" inline constexpr auto LinkCannon_fini() noexcept {}
