/*
server.cpp - C++ file for the data socket server.
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


#include <server.hpp>

#include <utility.hpp>

#include <nn/nifm.h>
#include <nn/os.h>
#include <nn/socket.h>

#include <netinet/in.h>

#include <cstdlib>


// Initialize the server
auto Server::Init(unsigned short port) noexcept -> bool {
  // Initialize the Network Interface Module
  if (nn::nifm::Initialize().IsFailure()) [[unlikely]] {
    return false;
  }

  // Allocate memory for the socket pool
  constexpr auto POOL_SIZE = 0x100000uz;
  constexpr auto ALIGNMENT = 0x1000uz;
  auto *pool = aligned_alloc(ALIGNMENT, POOL_SIZE);

  if (pool == nullptr) [[unlikely]] {
    return false;
  }

  // Initialize the sockets API
  constexpr auto POOL_ALLOC_SIZE = 0x20000ull;
  constexpr auto CONCURRENCY_LIMIT = 4;

  if (nn::socket::Initialize(
    pool, static_cast<unsigned long long>(POOL_SIZE), POOL_ALLOC_SIZE,
    CONCURRENCY_LIMIT
  ).IsFailure()) [[unlikely]] {
    // Free the pool if it fails
    free(pool);
    return false;
  }

  // Request a network interface
  nn::nifm::SubmitNetworkRequest();
  while (nn::nifm::IsNetworkRequestOnHold()) {
    Yield();
  }

  if (!nn::nifm::IsNetworkAvailable()) [[unlikely]] {
    nn::socket::Finalize();
    free(pool);

    return false;
  }

  // Create a socket
  serverSocket = nn::socket::Socket(AF_INET, SOCK_STREAM, 0);

  if (serverSocket < 0) [[unlikely]] {
    nn::socket::Finalize();
    free(pool);

    return false;
  }

  // Bind to all available network interfaces
  sockaddr_in address{
    .sin_family = AF_INET, .sin_port = nn::socket::InetHtons(port)
  };
  nn::socket::InetAton(
    "0.0.0.0", reinterpret_cast<nn::socket::InAddr *>(&address.sin_addr.s_addr)
  );

  if (nn::socket::Bind(
    serverSocket, reinterpret_cast<sockaddr *>(&address), sizeof address
  ) < 0) [[unlikely]] {
    nn::socket::Close(serverSocket);
    serverSocket = -1;
    nn::socket::Finalize();
    free(pool);

    return false;
  }

  // Listen for incoming connections
  if (nn::socket::Listen(serverSocket, 1) < 0) [[unlikely]] {
    nn::socket::Close(serverSocket);
    serverSocket = -1;
    nn::socket::Finalize();
    free(pool);

    return false;
  }

  return true;
}


// Wait for and accept incoming connections and receive and send data
[[noreturn]] auto Server::HandleConnection() noexcept {
  // Accept an incoming connection
  do [[unlikely]] {
    clientSocket = nn::socket::Accept(serverSocket, nullptr, nullptr);
    Yield();
  } while (clientSocket < 0);

  connected = true;

  while (true) [[likely]] {
    // Wait for a signal
    std::unique_lock<std::mutex> lock(signalMutex);
    signalCv.wait(lock, [this] {
      return !signals.empty();
    });

    // Execute next signal
    auto signalIt = signals.begin();
    auto signal = signalIt->second;
    auto currentMessageId = signalIt->first;
    signals.erase(signalIt);
    lock.unlock();

    switch (signal) {
      case sig::READ: {
        // Receive data
        auto lengthIt = lengths.find(currentMessageId);
        auto length = lengthIt->second;
        std::vector<unsigned char> data(length);

        auto result = nn::socket::Recv(
          clientSocket, static_cast<void *>(data.data()),
          static_cast<unsigned long long>(data.size()), 0
        );

        while (result <= 0) [[unlikely]] {
          Yield();

          // result == 0: Client closed the connection
          // result < 0: An error occurred
          if (!result) [[likely]] {
            // Connection has been lost; reconnect
            clientSocket = -1;

            do [[unlikely]] {
              clientSocket = nn::socket::Accept(serverSocket, nullptr, nullptr);
              Yield();
            } while (clientSocket < 0);
          }

          // Retry the operation
          result = nn::socket::Recv(
            clientSocket, static_cast<void *>(data.data()),
            static_cast<unsigned long long>(data.size()), 0
          );
        }

        // Add packet to queue
        std::lock_guard<std::mutex> lock(inPacketMutex);
        inPackets[currentMessageId] = data;
      }

      case sig::SEND: {
        // Send data
        auto dataIt = outPackets.find(currentMessageId);
        auto data = dataIt->second;

        auto result = nn::socket::Send(
          clientSocket, static_cast<const void *>(data.data()),
          static_cast<unsigned long long>(data.size()), 0
        );

        while (result <= 0) [[unlikely]] {
          Yield();

          if (!result) [[likely]] {
            clientSocket = -1;

            do [[unlikely]] {
              clientSocket = nn::socket::Accept(serverSocket, nullptr, nullptr);
              Yield();
            } while (clientSocket < 0);
          }

          result = nn::socket::Send(
            clientSocket, static_cast<const void *>(data.data()),
            static_cast<unsigned long long>(data.size()), 0
          );
        }

        // Remove packet from queue
        std::lock_guard<std::mutex> lock(outPacketMutex);
        outPackets.erase(dataIt);
      }
    }
  }
}


// void (*)(void *) proxy for HandleConnection
[[noreturn]] auto HandleConnProxy(void *server) noexcept {
  static_cast<Server *>(server)->HandleConnection();
}


// Start the connection handler
auto Server::Connect() noexcept -> bool {
  if (connected) [[unlikely]] {
    return true;
  }

  static nn::os::ThreadType serverThread;

  // Allocate memory for the server thread stack
  constexpr auto STACK_SIZE = 0x80000uz;
  constexpr auto ALIGNMENT = 0x1000uz;
  auto *stack = aligned_alloc(ALIGNMENT, STACK_SIZE);

  if (stack == nullptr) [[unlikely]] {
    return false;
  }

  // Attempt to create the server thread
  constexpr auto PRIORITY = 16;

  if (nn::os::CreateThread(
    &serverThread, HandleConnProxy, static_cast<void *>(this), stack,
    static_cast<unsigned long long>(STACK_SIZE), PRIORITY, 0
  ).IsFailure()) [[unlikely]] {
    // Free the thread stack if it fails
    free(stack);
    return false;
  }

  // Start the thread
  nn::os::StartThread(&serverThread);

  // Wait for a connection
  do [[unlikely]] {
    Yield();
  } while (!connected);

  return true;
}


// Receive data from the client
auto Server::Read(std::size_t length) noexcept -> std::vector<unsigned char> {
  auto currentMessageId = ++messageId;

  // Set the length
  {
    std::lock_guard<std::mutex> lock(lengthMutex);
    lengths[currentMessageId] = length;
  }

  // Broadcast the signal
  {
    std::lock_guard<std::mutex> lock(signalMutex);
    signals[currentMessageId] = sig::READ;
  }

  signalCv.notify_one();

  // Wait for a response
  std::unique_lock<std::mutex> lock(inPacketMutex);
  inPacketCv.wait(lock, [this, currentMessageId] {
    return inPackets.find(currentMessageId) != inPackets.end();
  });

  // Return the response
  auto packetIt = inPackets.find(currentMessageId);
  auto packet = packetIt->second;
  inPackets.erase(packetIt);
  lock.unlock();

  return packet;
}


// Send data to the client
auto Server::Send(std::vector<unsigned char> data) noexcept -> void {
  auto currentMessageId = ++messageId;

  // Set the data
  {
    std::lock_guard<std::mutex> lock(outPacketMutex);
    outPackets[currentMessageId] = data;
  }

  // Broadcast the signal
  {
    std::lock_guard<std::mutex> lock(signalMutex);
    signals[currentMessageId] = sig::SEND;
  }

  signalCv.notify_one();

  // Wait for the packet to be sent
  std::unique_lock<std::mutex> lock(outPacketMutex);
  outPacketCv.wait(lock, [this, currentMessageId] {
    return outPackets.find(currentMessageId) == outPackets.end();
  });

  lock.unlock();
}
