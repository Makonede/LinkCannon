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


export module server;

export import botw;
export import utility;

#define _GNU_SOURCE

#include <algorithm>
#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include <cstddef>
#include <cstdlib>

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <nn/nifm.h>
#include <nn/os.h>
#include <nn/socket.h>

using namespace std::literals;


inline auto HandleConnProxy(void *server) noexcept;


export class Server {
  // Wait for and accept incoming connections and receive and send data
  auto HandleConnection() noexcept -> void {
    // Wait for an incoming connection
    Poll(end::SERVER);

    // A connection has been made; accept it
    clientSocket = nn::socket::Accept(serverSocket, nullptr, nullptr);

    // Set the client socket as non-blocking
    auto flags = nn::socket::Fcntl(clientSocket, F_GETFL);
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
            inPackets[currentMessageId] = data;
          }

          // Notify Server::Read to return
          inPacketCv.notify_all();
          break;
        }

        case sig::SEND: {
          // Send data
          auto dataIt = outPackets.find(currentMessageId);
          auto data = dataIt->second;

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
        }
      }
    }
  }


  friend inline auto HandleConnProxy(void *server) noexcept;

  public:
    enum class sig : unsigned char {
      READ,
      SEND
    };

    enum class end : unsigned char {
      CLIENT,
      SERVER
    };

  private:
    inline auto Poll(end endpoint) noexcept {
      int sock;

      switch (endpoint) {
        case end::CLIENT: {
          sock = clientSocket;
          break;
        }

        case end::SERVER: {
          sock = serverSocket;
        }
      }

      constexpr auto READY = static_cast<short>(POLLIN);
      pollfd socketFd{.fd = sock, .events = READY};

      do [[unlikely]] {
        Yield();
      } while (!(nn::socket::Poll(
        &socketFd, 1ul, 0
      ) > 0 && socketFd.revents & READY));
    }

    inline auto Reconnect() noexcept {
      // Close and reset the socket
      nn::socket::Close(clientSocket);
      clientSocket = -1;

      // Wait for a new connection
      Poll(end::SERVER);

      // A connection has been made; accept it
      clientSocket = nn::socket::Accept(serverSocket, nullptr, nullptr);
    }

    const std::vector<unsigned char> HANDSHAKE{
      static_cast<unsigned char>('L'), static_cast<unsigned char>('C'),
      static_cast<unsigned char>('\0')
    };
    const std::vector<unsigned char> ACK{
      static_cast<unsigned char>('L'), static_cast<unsigned char>('C'),
      static_cast<unsigned char>('\1')
    };
    const std::vector<unsigned char> NACK{
      static_cast<unsigned char>('L'), static_cast<unsigned char>('C'),
      static_cast<unsigned char>('\2')
    };

    const std::vector<std::string> CLIENT_MESSAGES{"ADDR"s, "RADD"s, "DATA"s};
    const std::vector<std::string> SERVER_MESSAGES{"DATA"s};

    int serverSocket = -1;
    int clientSocket = -1;
    bool connected = false;
    int messageId = 0;

    std::map<int, sig> signals;
    std::mutex signalMutex;
    std::condition_variable signalCv;

    std::map<int, std::vector<unsigned char>> inPackets;
    std::mutex inPacketMutex;
    std::condition_variable inPacketCv;
    std::map<int, std::vector<unsigned char>> outPackets;
    std::mutex outPacketMutex;
    std::condition_variable outPacketCv;
    std::map<int, std::size_t> lengths;
    std::mutex lengthMutex;

  public:
    // Initialize the server
    auto Init(unsigned short port) noexcept -> bool {
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
      while (nn::nifm::IsNetworkRequestOnHold()) [[unlikely]] {
        Yield();
      }

      if (!nn::nifm::IsNetworkAvailable()) [[unlikely]] {
        nn::socket::Finalize();
        free(pool);

        return false;
      }

      // Create a socket
      serverSocket = nn::socket::Socket(
        AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0
      );

      if (serverSocket < 0) [[unlikely]] {
        nn::socket::Finalize();
        free(pool);

        return false;
      }

      // Bind to all available network interfaces
      sockaddr_in address{
        .sin_family = AF_INET, .sin_port = nn::socket::InetHtons(port)
      };
      nn::socket::InetAton("0.0.0.0", reinterpret_cast<nn::socket::InAddr *>(
        &address.sin_addr.s_addr
      ));

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


    // Start the connection handler
    auto Connect() noexcept -> bool {
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
    auto Read(
      const std::size_t length
    ) noexcept -> std::vector<unsigned char> {
      auto currentMessageId = ++messageId;

      // Set the length
      {
        const std::lock_guard<std::mutex> lock(lengthMutex);
        lengths[currentMessageId] = length;
      }

      // Broadcast the signal
      {
        const std::lock_guard<std::mutex> lock(signalMutex);
        signals[currentMessageId] = sig::READ;
      }

      signalCv.notify_one();

      // Wait for a response
      std::unique_lock<std::mutex> lock(inPacketMutex);
      inPacketCv.wait(lock, [this, currentMessageId] {
        return inPackets.contains(currentMessageId);
      });

      // Return the response
      auto packetIt = inPackets.find(currentMessageId);
      auto packet = packetIt->second;
      inPackets.erase(packetIt);
      lock.unlock();

      return packet;
    }


    // Send data to the client
    auto Send(const std::vector<unsigned char> data) noexcept -> void {
      auto currentMessageId = ++messageId;

      // Set the data
      {
        const std::lock_guard<std::mutex> lock(outPacketMutex);
        outPackets[currentMessageId] = data;
      }

      // Broadcast the signal
      {
        const std::lock_guard<std::mutex> lock(signalMutex);
        signals[currentMessageId] = sig::SEND;
      }

      signalCv.notify_one();

      // Wait for the packet to be sent
      std::unique_lock<std::mutex> lock(outPacketMutex);
      outPacketCv.wait(lock, [this, currentMessageId] {
        return !outPackets.contains(currentMessageId);
      });

      lock.unlock();
    }


    inline auto Ack() noexcept -> void {
      Send(ACK);
    }
    inline auto Nack() noexcept -> void {
      Send(NACK);
    }
    inline auto ReadAck() noexcept -> bool {
      return Read(3uz) == ACK;
    }


    // Start a message from the passed endpoint
    auto StartMessage(
      const end endpoint, std::string &code
    ) noexcept -> bool {
      switch (endpoint) {
        case end::CLIENT: {
          // Read the message code
          auto codeVec = Read(4uz);
          code = std::string(codeVec.begin(), codeVec.end());

          // Check if the code exists
          if (std::find(
            CLIENT_MESSAGES.begin(), CLIENT_MESSAGES.end(), code
          ) == CLIENT_MESSAGES.end()) [[unlikely]] {
            return false;
          }

          Ack();
          break;
        }

        case end::SERVER: {
          // Check if the code exists
          if (std::find(
            SERVER_MESSAGES.begin(), SERVER_MESSAGES.end(), code
          ) == SERVER_MESSAGES.end()) [[unlikely]] {
            return false;
          }

          // Send the message code
          Send(std::vector<unsigned char>(code.begin(), code.end()));

          // Check if the client received the message
          return ReadAck();
        }
      }

      return false;
    }


    std::map<std::size_t, std::size_t> watched;
    std::mutex watchedMutex;
    std::condition_variable watchedCv;
};


inline auto HandleConnProxy(void *server) noexcept {
  static_cast<Server *>(server)->HandleConnection();
}
