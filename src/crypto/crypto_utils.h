#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "type.h"

namespace crypto {

AesKey DeriveKey(const std::vector<Label>& input_labels);
Ciphertext Encrypt(const AesKey& key, const Label& output_label);
Plaintext Decrypt(const AesKey& key, const Ciphertext& ciphertext);

}  // namespace crypto

#endif // CRYPTO_UTILS_H