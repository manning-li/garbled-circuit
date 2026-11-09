#include "crypto/label.h"

#include <openssl/rand.h>
#include <stdexcept>

namespace crypto {

Label GenerateLabel() {
  Label label{};

  if (RAND_bytes(
          reinterpret_cast<unsigned char*>(label.data()),
          static_cast<int>(label.size())) != 1) {
    throw std::runtime_error("Failed to generate label.");
  }

  return label;
}

WireLabels GenerateWireLabels() {
  WireLabels labels;

  labels.zero = GenerateLabel();
  labels.one = GenerateLabel();

  while (labels.zero == labels.one) {
    labels.one = GenerateLabel();
  }

  return labels;
}

}  // namespace crypto
