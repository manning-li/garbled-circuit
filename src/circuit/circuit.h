#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <cstddef>
#include <numeric>
#include <string>
#include <vector>

namespace circuit{

enum class GateType {
  AND, EQ, EQW, ERR, INV, NAND, NOR, NOT, OR, XNOR, XOR
};

struct Gate {
  GateType type;
  std::vector<std::size_t> inputs;  // Input wire indices, except for
                                    // EQ gate: inputs[0] is constant value.
  std::size_t output;               // Output wire index
};

struct Circuit {
  std::size_t num_gates = 0;
  std::size_t num_wires = 0;

  // Number of wires for each input/output party.
  std::vector<std::size_t> input_sizes;
  std::vector<std::size_t> output_sizes;

  std::vector<Gate> gates;

  std::size_t GetNumInputs() const {
    return std::accumulate(
        input_sizes.begin(), input_sizes.end(), std::size_t{0});
  }

  std::size_t GetNumOutputs() const {
    return std::accumulate(
        output_sizes.begin(), output_sizes.end(), std::size_t{0});
  }
};

}  // namespace circuit

#endif // CIRCUIT_H
