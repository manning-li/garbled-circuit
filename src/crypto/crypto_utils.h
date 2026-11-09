#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "crypto/label.h"

namespace crypto {

constexpr std::size_t kAesKeySize = 32;
constexpr std::size_t kBlockSize = 16;
constexpr std::size_t kPlaintextSize = 2 * kBlockSize;

std::array<std::uint8_t, kAesKeySize> DeriveKey(
    const std::vector<Label>& input_labels);

std::array<std::uint8_t, kPlaintextSize> Encrypt(
    const std::array<std::uint8_t, kAesKeySize>& key,
    const Label& output_label);

std::array<std::uint8_t, kPlaintextSize> Decrypt(
    const std::array<std::uint8_t, kAesKeySize>& key,
    const std::array<std::uint8_t, kPlaintextSize>& ciphertext);

}  // namespace crypto

#endif // CRYPTO_UTILS_H