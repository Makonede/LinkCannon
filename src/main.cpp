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


#include <memory>

#include <cstdlib>
#include <cstring>

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
  auto server = *static_cast<std::shared_ptr<Server> *>(serverPtr);

  // Watch the requested memory addresses
  while (true) [[likely]] {
    // Wait for data to be available
    std::unique_lock<std::mutex> lock(server->watchedMutex);
    server->watchedCv.wait(lock, [server] {
      return !server->watched.empty();
    });

    // Read from each memory address
    const auto watched = std::map(server->watched);
    lock.unlock();
    auto code = "DATA"s;

    for (const auto &[address, size] : watched) [[likely]] {
      if (!server->StartMessage(Server::end::SERVER, code)) [[unlikely]] {
        break;
      }

      server->Send(address);
      server->Send(size);

      if (!server->ReadAck()) [[unlikely]] {
        break;
      }

      // Send the data
      server->Send(botw::Memory + address, size);

      if (!server->ReadAck()) [[unlikely]] {
        break;
      }
    }
  }
}


// Network data thread
constexpr auto NetworkThread([[maybe_unused]] auto *unused) noexcept {
  // Initialize the server
  auto server = std::make_shared<Server>();

  while (!server->Init(PORT)) [[unlikely]] {
    Yield();
  }

  // Connect to the client
  while (!server->Connect()) [[unlikely]] {
    Yield();
  }

  // Start the memory watcher thread
  if (!StartThread(WatchThread, &server)) [[unlikely]] {
    return;
  }

  // Start processing messages
  while (true) [[likely]] {
    // Receive a message code
    std::string code;
    if (!server->StartMessage(Server::end::CLIENT, code)) [[unlikely]] {
      continue;
    }

    // Lock the watcher thread
    const std::lock_guard<std::mutex> lock(server->watchedMutex);

    if (code == "ADDR"s) {
      // ADDR - add address to watch
      const auto address = server->Read<std::size_t>();
      const auto size = server->Read<std::size_t>();

      // Start watching the address or update its maximum size
      {
        const std::lock_guard<std::mutex> lock(server->watchedMutex);
        server->watched[address] = size;
      }

      // Notify the watcher thread that data is available and acknowledge the
      // message
      server->watchedCv.notify_one();
      server->Ack();
    }
    else if (code == "RADD"s) {
      // RADD - stop watching address
      const auto address = server->Read<std::size_t>();

      // If the address is being watched, stop watching it
      {
        const std::lock_guard<std::mutex> lock(server->watchedMutex);
        const auto watchedAddr = server->watched.find(address);

        if (watchedAddr != server->watched.end()) [[likely]] {
          server->watched.erase(watchedAddr);
          server->Ack();
        }
        else [[unlikely]] {
          server->Nack();
        }
      }

      server->watchedCv.notify_one();
    }
    else if (code == "DATA"s) {
      // DATA - write data to address
      const auto address = server->Read<std::size_t>();
      const auto size = server->Read<std::size_t>();
      server->Ack();

      // Read and write the data
      const auto *data = server->Read(size).data();
      std::memcpy(
        static_cast<void *>(botw::Memory + address),
        static_cast<const void *>(data), size
      );

      server->Ack();
    }
  }
}


// Initialization function (entrypoint)
extern "C" constexpr auto LinkCannon_init() noexcept {
  // Start all threads
  StartThread(EventThread);
  StartThread(NetworkThread);
}


// Finalization function
extern "C" constexpr auto LinkCannon_fini() noexcept {}
