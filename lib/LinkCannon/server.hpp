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
#include <vector>

#include <cstddef>


[[noreturn]] auto HandleConnProxy(void *server) noexcept;


class Server {
  [[noreturn]] auto HandleConnection() noexcept;
  friend auto HandleConnProxy(void *server) noexcept;

  enum class sig : unsigned char {
    READ = 0,
    SEND = 1
  };

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
    auto Read(std::size_t length) noexcept -> std::vector<unsigned char>;
    auto Send(std::vector<unsigned char> data) noexcept -> void;
};
