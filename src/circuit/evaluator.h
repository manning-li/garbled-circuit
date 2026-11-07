#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "circuit/circuit.h"

#include <cstdint>
#include <vector>

namespace circuit{

std::vector<std::uint8_t> EvaluateCircuit(
    const Circuit& circuit,
    const std::vector<std::uint8_t>& inputs);

}  // namespace circuit

#endif  // EVALUATOR_H
