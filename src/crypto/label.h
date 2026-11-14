#ifndef CRYPTO_LABEL_H
#define CRYPTO_LABEL_H

#include <array>
#include <cstddef>
#include <cstdint>

#include "type.h"

namespace crypto {

Label GenerateLabel();
WireLabels GenerateWireLabels();

}  // namespace crypto

#endif  // CRYPTO_LABEL_H