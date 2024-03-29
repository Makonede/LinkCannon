/*
server.hpp - Header file for the data socket server.
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

#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <cstddef>

#include <nn/socket.h>

#include <botw.hpp>
#include <utility.hpp>

using namespace std::literals;


inline auto HandleConnProxy(void *server) noexcept;


class Server {
  auto HandleConnection() noexcept -> void;

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
        case end::CLIENT:
          sock = clientSocket;
          break;

        case end::SERVER:
          sock = serverSocket;
          break;

        default: std::unreachable();
      }

      constexpr auto READY = static_cast<short>(POLLIN);
      pollfd socketFd{.fd = sock, .events = READY};

      do [[unlikely]] {
        Yield();
      } while (!(nn::socket::Poll(
        &socketFd, 1ul, 0
      ) > 0 && socketFd.revents & READY));
    }

    const std::vector<unsigned char> HANDSHAKE{'L', 'C', '\0'};
    const std::vector<unsigned char> ACK{'L', 'C', '\1'};
    const std::vector<unsigned char> NACK{'L', 'C', '\2'};

    const std::vector<std::string> CLIENT_MESSAGES{"ADDR"s, "RADD"s, "DATA"s};
    const std::vector<std::string> SERVER_MESSAGES{"DATA"s};

    int serverSocket = -1;
    int clientSocket = -1;
    bool connected = false;
    unsigned int packetId = 0;

    std::map<unsigned int, sig> signals;
    std::mutex signalMutex;
    std::condition_variable signalCv;

    std::map<unsigned int, std::vector<unsigned char>> inPackets;
    std::mutex inPacketMutex;
    std::condition_variable inPacketCv;
    std::map<unsigned int, std::vector<unsigned char>> outPackets;
    std::mutex outPacketMutex;
    std::condition_variable outPacketCv;
    std::map<unsigned int, std::size_t> lengths;
    std::mutex lengthMutex;

  public:
    auto Init(unsigned short port) noexcept -> bool;
    auto Connect() noexcept -> bool;
    auto Read(const std::size_t length) noexcept -> std::vector<unsigned char>;
    auto Send(const std::vector<unsigned char> data) noexcept -> void;

    template <typename T>
    inline auto Read([[maybe_unused]] const std::size_t length = 0uz) noexcept {
      if constexpr (std::is_same_v<T, std::string>) {
        const auto data = Read(length);
        return std::string(data.begin(), data.end());
      }
      else if (std::is_integral_v<T>) {
        const auto data = Read(sizeof(T));
        return *reinterpret_cast<const T *const>(data.data());
      }

      std::unreachable();
    }

    template <typename T>
    inline auto Send(const T data) noexcept {
      if constexpr (std::is_same_v<T, std::string>) {
        Send(std::vector<unsigned char>(data.begin(), data.end()));
      }
      else {
        const auto *const dataPtr = reinterpret_cast<
          const unsigned char *const
        >(&data);
        Send(std::vector(dataPtr, dataPtr + sizeof(T)));
      }
    }

    inline auto Send(
      const unsigned char *const data, const std::size_t size
    ) noexcept {
      Send(std::vector(data, data + size));
    }

    inline auto Ack() noexcept {
      Send(ACK);
    }
    inline auto Nack() noexcept {
      Send(NACK);
    }
    inline const auto ReadAck() noexcept {
      return Read(ACK.size()) == ACK;
    }

    auto StartMessage(const end endpoint, std::string &code) noexcept -> bool;

    std::map<std::size_t, std::size_t> watched;
    std::mutex watchedMutex;
    std::condition_variable watchedCv;

  private:
    inline auto Reconnect() noexcept {
      // Nack the last message
      Nack();

      // Close and reset the socket
      nn::socket::Close(clientSocket);
      clientSocket = -1;

      // Wait for a new connection
      Poll(end::SERVER);

      // A connection has been made; accept it
      clientSocket = nn::socket::Accept(serverSocket, nullptr, nullptr);
    }
};


inline auto HandleConnProxy(void *server) noexcept {
  static_cast<Server *const>(server)->HandleConnection();
}
