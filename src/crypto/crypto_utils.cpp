#include "crypto/crypto_utils.h"

#include <algorithm>

#include <memory>
#include <stdexcept>

#include <openssl/evp.h>
#include <openssl/kdf.h>

namespace crypto {

constexpr char kInfo[] = "gc";

std::array<std::uint8_t, kAesKeySize> DeriveKey(
    const std::vector<Label>& input_labels) {
  std::vector<std::uint8_t> material;

  for (const Label& label : input_labels) {
    material.insert(material.end(), label.begin(), label.end());
  }

  // TODO: Support EQ gate.
  if (material.empty()) {
    throw std::runtime_error("Invalid input labels.");
  }

  std::array<std::uint8_t, kAesKeySize> key{};

  // Using HKDF-SHA256.

  std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> context(
    EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, nullptr), EVP_PKEY_CTX_free);
  if (context == nullptr) {
    throw std::runtime_error("Failed to create HKDF context.");
  }

  if (EVP_PKEY_derive_init(context.get()) <= 0 ||
      EVP_PKEY_CTX_set_hkdf_md(context.get(), EVP_sha256()) <= 0 ||
      EVP_PKEY_CTX_set1_hkdf_key(
          context.get(), material.data(),
          static_cast<int>(material.size())) <= 0) {
    throw std::runtime_error("Failed to set HKDF config.");
  }

  if (EVP_PKEY_CTX_add1_hkdf_info(
          context.get(), reinterpret_cast<const unsigned char*>(kInfo),
          static_cast<int>(sizeof(kInfo) - 1)) <= 0) {
    throw std::runtime_error("Failed to set HKDF info.");
  }

  std::size_t key_size = key.size();
  if (EVP_PKEY_derive(context.get(), key.data(), &key_size) <= 0 ||
      key_size != key.size()) {
    throw std::runtime_error("Failed to derive key.");
  }

  return key;
}

std::array<std::uint8_t, kPlaintextSize> Encrypt(
    const std::array<std::uint8_t, kAesKeySize>& key,
    const crypto::Label& output_label) {
  
  std::array<std::uint8_t, kPlaintextSize> plaintext{};  // Init with 0.
  std::copy(output_label.begin(), output_label.end(), plaintext.begin());

  std::array<std::uint8_t, kPlaintextSize> ciphertext{};

  std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> context(
      EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
  if (context == nullptr) {
    throw std::runtime_error("Failed to create AES context.");
  }

  int output_size = 0;
  int final_size = 0;

  if (EVP_EncryptInit_ex(
          context.get(), EVP_aes_256_ecb(), nullptr, key.data(), nullptr) <= 0 ||
      EVP_CIPHER_CTX_set_padding(context.get(), 0) <= 0 ||
      EVP_EncryptUpdate(
          context.get(), ciphertext.data(), &output_size, plaintext.data(),
          static_cast<int>(plaintext.size())) <= 0 ||
      EVP_EncryptFinal_ex(
          context.get(), ciphertext.data() + output_size, &final_size) <= 0 ||
      static_cast<std::size_t>(output_size + final_size) != ciphertext.size()) {
    throw std::runtime_error("Failed to encrypt.");
  }

  return ciphertext;
}

std::array<std::uint8_t, kPlaintextSize> Decrypt(
    const std::array<std::uint8_t, kAesKeySize>& key,
    const std::array<std::uint8_t, kPlaintextSize>& ciphertext) {
  
  std::array<std::uint8_t, kPlaintextSize> plaintext{};

  std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> context(
      EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
  if (context == nullptr) {
    throw std::runtime_error("Failed to create AES context.");
  }

  int output_size = 0;
  int final_size = 0;

  if (EVP_DecryptInit_ex(
          context.get(), EVP_aes_256_ecb(), nullptr, key.data(), nullptr) <= 0 ||
      EVP_CIPHER_CTX_set_padding(context.get(), 0) <= 0 ||
      EVP_DecryptUpdate(
          context.get(), plaintext.data(), &output_size, ciphertext.data(),
          static_cast<int>(ciphertext.size())) <= 0 ||
      EVP_DecryptFinal_ex(
          context.get(), plaintext.data() + output_size, &final_size) <= 0 ||
      static_cast<std::size_t>(output_size + final_size) != plaintext.size()) {
    throw std::runtime_error("Failed to decrypt.");
  }

  return plaintext;
}

}