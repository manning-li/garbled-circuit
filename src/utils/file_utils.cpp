#include "utils/file_utils.h"

#include <cctype>
#include <limits>
#include <stdexcept>

namespace utils {

bool IsBlankLine(const std::string& line) {
  for (char c : line) {
    if (!std::isspace(static_cast<unsigned char>(c))) {
      return false;
    }
  }

  return true;
}

bool GetNextLine(std::ifstream& file, std::istringstream& line) {
  std::string str;
  while (std::getline(file, str)) {
    if (!IsBlankLine(str)) {
      line = std::istringstream(str);
      return true;
    }
  }
  return false;
}

void WriteUint32(std::ofstream& file, std::uint32_t value) {
  for (std::size_t i = 0; i < 4; ++i) {
    file.put(static_cast<char>((value >> (i * 8)) & 0xFF));
  }
  if (!file) {
    throw std::runtime_error("Failed to write uint32.");
  }
}

void WriteUint64(std::ofstream& file, std::uint64_t value) {
  for (std::size_t i = 0; i < 8; ++i) {
    file.put(static_cast<char>((value >> (i * 8)) & 0xFF));
  }
  if (!file) {
    throw std::runtime_error("Failed to write uint64.");
  }
}

void WriteSize32(std::ofstream& file, std::size_t value) {
  WriteUint32(file, static_cast<std::uint32_t>(value));
}

std::uint32_t ReadUint32(std::ifstream& file) {
  std::uint32_t value = 0;
  for (std::size_t i = 0; i < 4; ++i) {
    char byte;
    if (!file.get(byte)) {
      throw std::runtime_error("Failed to read uint32.");
    }
    const std::uint32_t tmp_byte = static_cast<std::uint8_t>(byte);
    value |= tmp_byte << (i * 8);
  }
  return value;
}

std::uint64_t ReadUint64(std::ifstream& file) {
  std::uint64_t value = 0;
  for (std::size_t i = 0; i < 8; ++i) {
    char byte;
    if (!file.get(byte)) {
      throw std::runtime_error("Failed to read uint64.");
    }
    const std::uint64_t tmp_byte = static_cast<std::uint8_t>(byte);
    value |= tmp_byte << (i * 8);
  }
  return value;
}

std::size_t ReadSize32(std::ifstream& file) {
  return static_cast<std::size_t>(ReadUint32(file));
}

}  // namespace utils