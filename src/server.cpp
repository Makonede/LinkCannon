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


#include <algorithm>

#include <cstdlib>

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <nn/nifm.h>
#include <nn/os.h>

#include <server.hpp>


// Initialize the server
auto Server::Init(unsigned short port) noexcept -> bool {
  // Initialize the Network Interface Module
  if (nn::nifm::Initialize().IsFailure()) [[unlikely]] {
    return false;
  }

  // Allocate memory for the socket pool
  constexpr auto POOL_SIZE = 0x100000uz;
  constexpr auto ALIGNMENT = 0x1000uz;
  auto *pool = std::aligned_alloc(ALIGNMENT, POOL_SIZE);

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
    std::free(pool);
    return false;
  }

  // Request a network interface
  nn::nifm::SubmitNetworkRequest();
  while (nn::nifm::IsNetworkRequestOnHold()) [[unlikely]] {
    Yield();
  }

  if (!nn::nifm::IsNetworkAvailable()) [[unlikely]] {
    nn::socket::Finalize();
    std::free(pool);

    return false;
  }

  // Create a socket
  serverSocket = nn::socket::Socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

  if (serverSocket < 0) [[unlikely]] {
    nn::socket::Finalize();
    std::free(pool);

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
    std::free(pool);

    return false;
  }

  // Listen for incoming connections
  if (nn::socket::Listen(serverSocket, 1) < 0) [[unlikely]] {
    nn::socket::Close(serverSocket);
    serverSocket = -1;
    nn::socket::Finalize();
    std::free(pool);

    return false;
  }

  return true;
}


// Start a message from the passed endpoint
auto Server::StartMessage(
  const end endpoint, std::string &code
) noexcept -> bool {
  switch (endpoint) {
    case end::CLIENT: {
      // Read the message code
      code = Read<std::string>(4uz);

      // Check if the code exists
      if (std::find(
        CLIENT_MESSAGES.begin(), CLIENT_MESSAGES.end(), code
      ) == CLIENT_MESSAGES.end()) [[unlikely]] {
        Nack();
        return false;
      }

      Ack();
      return true;
    }

    case end::SERVER: {
      // Check if the code exists
      if (std::find(
        SERVER_MESSAGES.begin(), SERVER_MESSAGES.end(), code
      ) == SERVER_MESSAGES.end()) [[unlikely]] {
        return false;
      }

      // Send the message code
      Send(code);

      // Check if the client received the message
      return ReadAck();
    }
  }

  std::unreachable();
}


// Wait for and accept incoming connections and receive and send data
auto Server::HandleConnection() noexcept -> void {
  // Wait for an incoming connection
  Poll(end::SERVER);

  // A connection has been made; accept it
  clientSocket = nn::socket::Accept(serverSocket, nullptr, nullptr);

  // Set the client socket as non-blocking
  const auto flags = nn::socket::Fcntl(clientSocket, F_GETFL);
  if (flags < 0) [[unlikely]] {
    return;
  }

  nn::socket::Fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK);

  // Receive a handshake
  Poll(end::CLIENT);

  std::vector<unsigned char> handshake(3uz);
  auto handshakeResult = nn::socket::Recv(
    clientSocket, static_cast<void *>(handshake.data()),
    static_cast<unsigned long long>(handshake.size()), 0
  );

  while (handshake != HANDSHAKE) [[unlikely]] {
    if (handshakeResult > 0) [[unlikely]] {
      // A proper handshake was not received
      Reconnect();

      // Retry the handshake
      Poll(end::CLIENT);
      handshakeResult = nn::socket::Recv(
        clientSocket, static_cast<void *>(handshake.data()),
        static_cast<unsigned long long>(handshake.size()), 0
      );
    }

    while (handshakeResult <= 0) [[likely]] {
      // result == 0: Client closed the connection
      // result < 0: An error occurred
      if (!handshakeResult) [[likely]] {
        // Connection has been lost
        Reconnect();
      }

      Poll(end::CLIENT);
      handshakeResult = nn::socket::Recv(
        clientSocket, static_cast<void *>(handshake.data()),
        static_cast<unsigned long long>(handshake.size()), 0
      );
    }
  }

  // Acknowledge the handshake
  auto ackResult = nn::socket::Send(
    clientSocket, static_cast<const void *>(ACK.data()),
    static_cast<unsigned long long>(ACK.size()), 0
  );
  Poll(end::CLIENT);

  while (ackResult <= 0) [[unlikely]] {
    if (!ackResult) [[likely]] {
      Reconnect();
    }

    ackResult = nn::socket::Send(
      clientSocket, static_cast<const void *>(ACK.data()),
      static_cast<unsigned long long>(ACK.size()), 0
    );
    Poll(end::CLIENT);
  }

  connected = true;

  while (true) [[likely]] {
    // Wait for a signal
    std::unique_lock<std::mutex> lock(signalMutex);
    signalCv.wait(lock, [this] {
      return !signals.empty();
    });

    // Execute next signal
    const auto signalIt = signals.begin();
    const auto signal = signalIt->second;
    const auto currentPacketId = signalIt->first;
    signals.erase(signalIt);
    lock.unlock();

    switch (signal) {
      case sig::READ: {
        // Receive data
        const auto lengthIt = lengths.find(currentPacketId);
        const auto length = lengthIt->second;
        std::vector<unsigned char> data(length);

        Poll(end::CLIENT);
        auto result = nn::socket::Recv(
          clientSocket, static_cast<void *>(data.data()),
          static_cast<unsigned long long>(data.size()), 0
        );

        while (result <= 0) [[unlikely]] {
          if (!result) [[likely]] {
            Reconnect();
          }

          Poll(end::CLIENT);
          result = nn::socket::Recv(
            clientSocket, static_cast<void *>(data.data()),
            static_cast<unsigned long long>(data.size()), 0
          );
        }

        // Add packet to queue
        {
          const std::lock_guard<std::mutex> lock(inPacketMutex);
          inPackets[currentPacketId] = data;
        }

        // Notify Server::Read to return
        inPacketCv.notify_all();
        break;
      }

      case sig::SEND: {
        // Send data
        const auto dataIt = outPackets.find(currentPacketId);
        const auto data = dataIt->second;

        auto result = nn::socket::Send(
          clientSocket, static_cast<const void *>(data.data()),
          static_cast<unsigned long long>(data.size()), 0
        );
        Poll(end::CLIENT);

        while (result <= 0) [[unlikely]] {
          if (!result) [[likely]] {
            Reconnect();
          }

          result = nn::socket::Send(
            clientSocket, static_cast<const void *>(data.data()),
            static_cast<unsigned long long>(data.size()), 0
          );
          Poll(end::CLIENT);
        }

        // Remove packet from queue
        {
          const std::lock_guard<std::mutex> lock(outPacketMutex);
          outPackets.erase(dataIt);
        }

        // Notify Server::Send to return
        outPacketCv.notify_all();
        break;
      }

      default: std::unreachable();
    }
  }
}


// Start the connection handler
auto Server::Connect() noexcept -> bool {
  if (connected) [[unlikely]] {
    return true;
  }

  // Start the server thread
  if (!StartThread(HandleConnProxy, this)) [[unlikely]] {
    return false;
  }

  // Wait for a connection
  do [[unlikely]] {
    Yield();
  } while (!connected);

  return true;
}


// Receive data from the client
auto Server::Read(
  const std::size_t length
) noexcept -> std::vector<unsigned char> {
  const auto currentPacketId = ++packetId;

  // Set the length
  {
    const std::lock_guard<std::mutex> lock(lengthMutex);
    lengths[currentPacketId] = length;
  }

  // Broadcast the signal
  {
    const std::lock_guard<std::mutex> lock(signalMutex);
    signals[currentPacketId] = sig::READ;
  }

  signalCv.notify_one();

  // Wait for a response
  std::unique_lock<std::mutex> lock(inPacketMutex);
  inPacketCv.wait(lock, [this, currentPacketId] {
    return inPackets.contains(currentPacketId);
  });

  // Return the response
  const auto packetIt = inPackets.find(currentPacketId);
  const auto packet = packetIt->second;
  inPackets.erase(packetIt);
  lock.unlock();

  return packet;
}


// Send data to the client
auto Server::Send(const std::vector<unsigned char> data) noexcept -> void {
  const auto currentPacketId = ++packetId;

  // Set the data
  {
    const std::lock_guard<std::mutex> lock(outPacketMutex);
    outPackets[currentPacketId] = data;
  }

  // Broadcast the signal
  {
    const std::lock_guard<std::mutex> lock(signalMutex);
    signals[currentPacketId] = sig::SEND;
  }

  signalCv.notify_one();

  // Wait for the packet to be sent
  std::unique_lock<std::mutex> lock(outPacketMutex);
  outPacketCv.wait(lock, [this, currentPacketId] {
    return !outPackets.contains(currentPacketId);
  });

  lock.unlock();
}
