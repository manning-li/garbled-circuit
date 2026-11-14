#include "utils/rpc_utils.h"

#include <arpa/inet.h>
#include <sys/socket.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace utils {

void SendAll(int socket, const void* data, std::size_t size) {
  const auto* bytes = static_cast<const char*>(data);
  std::size_t sent = 0;

  while (sent < size) {
    const ssize_t result = send(socket, bytes + sent, size - sent, 0);
    if (result < 0) {
      if (errno == EINTR) {
        continue;
      }
      throw std::runtime_error("Socket send failed.");
    }
    if (result == 0) {
      throw std::runtime_error("Socket connection closed.");
    }
    sent += static_cast<std::size_t>(result);
  }
}

bool ReceiveAll(int socket, void* data, std::size_t size, bool allow_eof) {
  auto* bytes = static_cast<char*>(data);
  std::size_t received = 0;

  while (received < size) {
    const ssize_t result =
        recv(socket, bytes + received, size - received, 0);
    if (result < 0) {
      if (errno == EINTR) {
        continue;
      }
      throw std::runtime_error("Socket receive failed.");
    }
    if (result == 0) {
      if (received == 0 && allow_eof) {
        return false;
      }
      throw std::runtime_error("Socket connection closed.");
    }
    received += static_cast<std::size_t>(result);
  }
  return true;
}

void SendUint64(int socket, std::uint64_t value) {
  std::array<std::uint8_t, 8> bytes{};
  for (std::size_t i = 0; i < bytes.size(); ++i) {
    bytes[i] = static_cast<std::uint8_t>(value >> ((7 - i) * 8));
  }
  SendAll(socket, bytes.data(), bytes.size());
}

std::uint64_t ReceiveUint64(int socket) {
  std::array<std::uint8_t, 8> bytes{};
  ReceiveAll(socket, bytes.data(), bytes.size());

  std::uint64_t value = 0;
  for (std::uint8_t byte : bytes) {
    value = (value << 8) | byte;
  }
  return value;
}

void SendUint32(int socket, std::uint32_t value) {
  const std::uint32_t network_value = htonl(value);
  SendAll(socket, &network_value, sizeof(network_value));
}

void SendFile(int socket, const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filename);
  }

  const std::uint64_t file_size = std::filesystem::file_size(filename);
  SendUint64(socket, file_size);

  std::array<char, kBufferSize> buffer{};
  std::uint64_t remaining = file_size;

  while (remaining > 0) {
    const std::size_t chunk_size =
        std::min(remaining, static_cast<std::uint64_t>(buffer.size()));

    file.read(buffer.data(), static_cast<std::streamsize>(chunk_size));
    if (!file) {
      throw std::runtime_error("Failed to read file: " + filename);
    }

    SendAll(socket, buffer.data(), chunk_size);
    remaining -= chunk_size;
  }
}

void ReceiveFile(int socket, const std::string& filename) {
  const std::uint64_t file_size = ReceiveUint64(socket);

  std::ofstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to create file: " + filename);
  }

  std::array<char, kBufferSize> buffer{};
  std::uint64_t remaining = file_size;

  while (remaining > 0) {
    const std::size_t chunk_size =
        std::min(remaining, static_cast<std::uint64_t>(buffer.size()));

    ReceiveAll(socket, buffer.data(), chunk_size);

    file.write(buffer.data(), static_cast<std::streamsize>(chunk_size));
    if (!file) {
      throw std::runtime_error("Failed to write file: " + filename);
    }
    remaining -= chunk_size;
  }
}

}  // namespace utils