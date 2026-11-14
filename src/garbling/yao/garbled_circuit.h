
#ifndef GARBLING_YAO_GARBLED_CIRCUIT_H
#define GARBLING_YAO_GARBLED_CIRCUIT_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <vector>

#include "crypto/label.h"
#include "type.h"

namespace basic_gc {

struct GarbledGate {
  std::vector<std::size_t> inputs;  // Input wire indices, except for
                                    // EQ gate: inputs[0] is constant value.
  std::size_t output;
  std::vector<Ciphertext> ciphertexts;
};

struct GarbledCircuit {
  std::size_t num_gates;
  std::size_t num_wires;
  std::vector<std::size_t> input_sizes;
  std::vector<std::size_t> output_sizes;

  std::vector<GarbledGate> gates;

  std::size_t GetNumInputs() const {
    return std::accumulate(
        input_sizes.begin(), input_sizes.end(), std::size_t{0});
  }

  std::size_t GetNumOutputs() const {
    return std::accumulate(
        output_sizes.begin(), output_sizes.end(), std::size_t{0});
  }
};

}  // namespace basic_gc

#endif  // GARBLING_YAO_GARBLED_CIRCUIT_H
