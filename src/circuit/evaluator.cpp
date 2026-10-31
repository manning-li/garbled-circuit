#include "circuit/evaluator.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace circuit{

namespace {

std::uint8_t EvaluateGate(const Gate& gate, 
                          const std::vector<std::uint8_t>& wires) {
  std::size_t input_wire1 = gate.inputs[0];
  std::size_t input_wire2 = 
      gate.inputs.size() == 2 ? gate.inputs[1] : 0;

  switch (gate.type) {
    case GateType::AND:
      return wires[input_wire1] && wires[input_wire2];

    case GateType::EQ:
      if (input_wire1 > 1) {
        throw std::runtime_error("Invalid EQ constant.");
      }
      return input_wire1 == 1;

    case GateType::EQW:
      return wires[input_wire1];

    case GateType::INV:
    case GateType::NOT:
      return !wires[input_wire1];

    case GateType::NAND:
      return !(wires[input_wire1] && wires[input_wire2]);

    case GateType::NOR:
      return !(wires[input_wire1] || wires[input_wire2]);

    case GateType::OR:
      return wires[input_wire1] || wires[input_wire2];

    case GateType::XNOR:
      return wires[input_wire1] == wires[input_wire2];

    case GateType::XOR:
      return wires[input_wire1] != wires[input_wire2];
  }

  throw std::runtime_error("Invalid gate type.");
}

}  // namespace

std::vector<std::uint8_t> EvaluateCircuit(
    const Circuit& circuit,
    const std::vector<std::uint8_t>& inputs) {

  if (inputs.size() != circuit.GetNumInputs()) {
    throw std::runtime_error(
        "Invalid input size: " + std::to_string(inputs.size()) + ".");
  }

  std::vector<std::uint8_t> wires(circuit.num_wires, 0);

  for (std::size_t i = 0; i < inputs.size(); ++i) {
    if (inputs[i] > 1) {
      throw std::runtime_error("Invalid input value.");
    }
    wires[i] = inputs[i];
  }

  for (const Gate& gate : circuit.gates) {
    wires[gate.output] = EvaluateGate(gate, wires);
  }

  // Output index range is based on Bristol Fashion.
  std::size_t index = circuit.num_wires - circuit.GetNumOutputs();
  return std::vector<std::uint8_t>(
      wires.begin() + index, wires.end());
}

}  // namespace circuit
