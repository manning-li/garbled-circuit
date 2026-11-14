#ifndef TYPE_H
#define TYPE_H

#include <array>
#include <cstdint>

constexpr std::size_t kAesKeySize = 32;
constexpr std::size_t kBlockSize = 16;
constexpr std::size_t kLabelSize = 16;

constexpr std::size_t kPlaintextSize = 2 * kBlockSize;
constexpr std::size_t kCiphertextSize = 2 * kBlockSize;

using AesKey = std::array<std::uint8_t, kAesKeySize>;
using CheckValue = std::array<std::uint8_t, kBlockSize>;
using Ciphertext = std::array<std::uint8_t, kPlaintextSize>;
using Label = std::array<std::uint8_t, kLabelSize>;
using Plaintext = std::array<std::uint8_t, kPlaintextSize>;

struct WireLabels {
  Label zero;
  Label one;
};

#endif  // TYPE_H