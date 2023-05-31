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
#include <vector>

#include <cstddef>

using namespace std::string_literals;


[[noreturn]] auto HandleConnProxy(void *server) noexcept;


class Server {
  [[noreturn]] auto HandleConnection() noexcept;
  auto Reconnect() noexcept;

  friend auto HandleConnProxy(void *server) noexcept;

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

    const std::vector<std::string> MESSAGES{"ADDR"s, "RADD"s, "DATA"s};

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
    auto Init(unsigned short port) noexcept -> bool;
    auto Connect() noexcept -> bool;
    auto Read(const std::size_t length) noexcept -> std::vector<unsigned char>;
    auto Send(const std::vector<unsigned char> data) noexcept -> void;

    inline auto Ack() noexcept -> void {
      Send(ACK);
    }
    inline auto Nack() noexcept -> void {
      Send(NACK);
    }
    inline auto ReadAck() noexcept -> bool {
      return Read(3uz) == ACK;
    }

    auto StartMessage(end endpoint, std::string &code) noexcept -> bool;

    std::map<std::size_t, std::size_t> watched;
    std::mutex watchedMutex;
    std::condition_variable watchedCv;
};
