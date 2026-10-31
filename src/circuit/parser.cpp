#include "circuit/parser.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "utils/file_utils.h"

namespace circuit{

namespace {

std::istringstream GetNextLine(std::ifstream& file) {
  std::istringstream line;
  if (!utils::GetNextLine(file, line)) {
    throw std::runtime_error("Failed to get next line.");
  }
  return line;
}

GateType ParseGateType(const std::string& type) {
  if (type == "AND") return GateType::AND;
  if (type == "EQ") return GateType::EQ;
  if (type == "EQW") return GateType::EQW;
  if (type == "INV") return GateType::INV;
  if (type == "NAND") return GateType::NAND;
  if (type == "NOR") return GateType::NOR;
  if (type == "NOT") return GateType::NOT;
  if (type == "OR") return GateType::OR;
  if (type == "XNOR") return GateType::XNOR;
  if (type == "XOR") return GateType::XOR;

  throw std::runtime_error("Invalid gate type: " + type + ".");
}

std::size_t GetExpectedNumInput(GateType type) {
  switch (type) {
    case GateType::AND:
    case GateType::NAND:
    case GateType::NOR:
    case GateType::OR:
    case GateType::XNOR:
    case GateType::XOR:
      return 2;

    case GateType::EQ:
    case GateType::EQW:
    case GateType::INV:
    case GateType::NOT:
      return 1;
  }

  throw std::runtime_error("Invalid gate type.");
}

std::vector<std::size_t> ParsePartyWireSizes(std::istringstream& line) {
  std::size_t num_parties;
  if (!(line >> num_parties)) {
    throw std::runtime_error("Failed to read number of parties.");
  }

  std::vector<std::size_t> sizes(num_parties);
  for (std::size_t& size : sizes) {
    if (!(line >> size)) {
      throw std::runtime_error("Failed to read wire sizes.");
    }
  }

  return sizes;
}

Gate ParseGate(std::istringstream& line, std::size_t num_wires) {
  std::size_t num_input;
  std::size_t num_output;

  if (!(line >> num_input >> num_output)) {
    throw std::runtime_error("Failed to read gate header.");
  }

  if (num_input > 2) {
    throw std::runtime_error("Input wires number larger than expected.");
  }
  if (num_output != 1) {
    throw std::runtime_error("Only support singular output.");
  }

  Gate gate;

  gate.inputs.resize(num_input);
  for (std::size_t& wire : gate.inputs) {
    if (!(line >> wire)) {
      throw std::runtime_error("Failed to read input wires.");
    }
  }

  if (!(line >> gate.output)) {
    throw std::runtime_error("Failed to read output wire.");
  }

  std::string type;
  if (!(line >> type)) {
    throw std::runtime_error("Failed to read gate type.");
  }

  gate.type = ParseGateType(type);

  if (num_input != GetExpectedNumInput(gate.type)) {
    throw std::runtime_error(
        "Invalid input wires number: " + std::to_string(num_input)
        + ", gate type: " + type);
  }

  // Check if wire number is out of the range [0, num_wires).
  
  if (gate.type == GateType::EQ) {
    if (gate.inputs[0] > 1) {
      throw std::runtime_error(
          "Invalid EQ constant value: " + std::to_string(gate.inputs[0]) + ".");
    }
  } else {
    for (std::size_t wire : gate.inputs) {
      if (wire >= num_wires) {
        throw std::runtime_error("Input wire index out of range.");
      }
    }
  }

  if (gate.output >= num_wires) {
    throw std::runtime_error("Output wire index out of range.");
  }

  return gate;
}

}  // namespace

Circuit ParseCircuit(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filename);
  }

  Circuit circuit;

  auto header_line = GetNextLine(file);
  if (!(header_line >> circuit.num_gates >> circuit.num_wires)) {
    throw std::runtime_error("Failed to read file header.");
  }

  auto input_line = GetNextLine(file);
  circuit.input_sizes = ParsePartyWireSizes(input_line);

  auto output_line = GetNextLine(file);
  circuit.output_sizes = ParsePartyWireSizes(output_line);

  circuit.gates.reserve(circuit.num_gates);
  for (std::size_t i = 0; i < circuit.num_gates; ++i) {
    auto gate_line = GetNextLine(file);
    circuit.gates.push_back(
        ParseGate(gate_line, circuit.num_wires));
  }

  return circuit;
}

}  // namespace circuit
