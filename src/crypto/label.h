#ifndef CRYPTO_LABEL_H
#define CRYPTO_LABEL_H

#include <array>
#include <cstddef>
#include <cstdint>

namespace crypto {

constexpr std::size_t kLabelSize = 16;
using Label = std::array<std::uint8_t, kLabelSize>;

struct WireLabels {
  Label zero;
  Label one;
};

Label GenerateLabel();
WireLabels GenerateWireLabels();

}  // namespace crypto

#endif  // CRYPTO_LABEL_H