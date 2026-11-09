#include "circuit/evaluator.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace circuit{

namespace{

constexpr std::uint8_t kUnassigned = 2;

std::vector<std::uint8_t> GetGateInputs(
    const Gate& gate, const std::vector<std::uint8_t>& wires) {
  
  std::vector<std::uint8_t> input_values;

  for (std::size_t i = 0; i < gate.inputs.size(); ++i) {
    if (gate.type == GateType::EQ && i == 0) {
      continue;
    }

    std::size_t input = gate.inputs[i];
    if (wires[input] == kUnassigned) {
      throw std::runtime_error(
          "Input wire not assigned: wires[" + std::to_string(input) + "].");
    }
    input_values.push_back(wires[input]);
  }

  return input_values;
}
 
}  // namespace

std::uint8_t EvaluateGate(
    const Gate& gate,
    const std::vector<std::uint8_t>& inputs) {
  std::size_t input1 = inputs[0];
  std::size_t input2 = inputs.size() == 2 ? inputs[1] : 0;

  switch (gate.type) {
    case GateType::AND:
      return input1 && input2;

    case GateType::EQ:
      if (input1 > 1) {
        throw std::runtime_error("Invalid EQ constant.");
      }
      return input1 == 1;

    case GateType::EQW:
      return input1;

    case GateType::INV:
    case GateType::NOT:
      return !input1;

    case GateType::NAND:
      return !(input1 && input2);

    case GateType::NOR:
      return !(input1 || input2);

    case GateType::OR:
      return input1 || input2;

    case GateType::XNOR:
      return input1 == input2;

    case GateType::XOR:
      return input1 != input2;
  }

  throw std::runtime_error("Invalid gate type.");
}

std::vector<std::uint8_t> EvaluateCircuit(
    const Circuit& circuit,
    const std::vector<std::uint8_t>& inputs) {

  if (inputs.size() != circuit.GetNumInputs()) {
    throw std::runtime_error(
        "Invalid input size: " + std::to_string(inputs.size()) + ".");
  }

  std::vector<std::uint8_t> wires(circuit.num_wires, kUnassigned);

  for (std::size_t i = 0; i < inputs.size(); ++i) {
    if (inputs[i] > 1) {
      throw std::runtime_error("Invalid input value.");
    }
    wires[i] = inputs[i];
  }

  for (const Gate& gate : circuit.gates) {
    wires[gate.output] = EvaluateGate(gate, GetGateInputs(gate, wires));
  }

  // Output index range is based on Bristol Fashion.
  std::size_t index = circuit.num_wires - circuit.GetNumOutputs();
  return std::vector<std::uint8_t>(
      wires.begin() + index, wires.end());
}

}  // namespace circuit
