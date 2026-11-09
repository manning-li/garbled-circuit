#include "garbling/yao/garbler.h"

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "circuit/evaluator.h"
#include "crypto/crypto_utils.h"
#include "utils/file_utils.h"

namespace basic_gc {

void Garbler::GarbleCircuit(const circuit::Circuit& circuit) {

  // initialized.

  wire_labels_.clear();
  wire_labels_.reserve(circuit.num_wires);
  for (std::size_t i = 0; i < circuit.num_wires; ++i) {
    wire_labels_.push_back(crypto::GenerateWireLabels());
  }

  garbled_circuit_ = {};
  garbled_circuit_.num_gates = circuit.num_gates;
  garbled_circuit_.num_wires = circuit.num_wires;
  garbled_circuit_.input_sizes = circuit.input_sizes;
  garbled_circuit_.output_sizes = circuit.output_sizes;
  garbled_circuit_.gates.reserve(circuit.num_gates);

  // Iterate gates and generate its garbled table (ciphertexts).

  for (const auto& gate : circuit.gates) {
    GarbledGate garbled_gate;
    garbled_gate.inputs = gate.inputs;
    garbled_gate.output = gate.output;

    const std::size_t num_inputs = garbled_gate.inputs.size();
    const std::size_t num_rows = 
        gate.type == circuit::GateType::EQ ? 0 : (1U << num_inputs);  // 2^n

    // TODO: Handle EQ gate.
    for (std::size_t row_index = 0; row_index < num_rows; ++row_index) {
      std::vector<std::uint8_t> input_values;
      std::vector<crypto::Label> input_labels;

      for (std::size_t i = 0; i < num_inputs; ++i) {
        const std::uint8_t bit =
            static_cast<std::uint8_t>((row_index >> i) & 1);

        input_values.push_back(bit);
        const std::size_t wire = garbled_gate.inputs[i];
        const crypto::WireLabels& labels = wire_labels_[wire];
        input_labels.push_back(bit == 0 ? labels.zero : labels.one);
      }

      const std::uint8_t output_value = 
          circuit::EvaluateGate(gate, input_values);
      const crypto::Label& output_label = 
          output_value == 0
              ? wire_labels_[gate.output].zero
              : wire_labels_[gate.output].one;

      const auto key = crypto::DeriveKey(input_labels);
      const auto encrypted = crypto::Encrypt(key, output_label);

      GarbledRow row;
      std::copy(
          encrypted.begin(),
          encrypted.begin() + crypto::kBlockSize,
          row.ciphertext.begin());
      std::copy(
          encrypted.begin() + crypto::kBlockSize,
          encrypted.end(),
          row.check.begin());
      garbled_gate.ciphertexts.push_back(row);
    }
    garbled_circuit_.gates.push_back(std::move(garbled_gate));
  }
}

std::vector<crypto::Label> Garbler::GetInputLabels(
    const std::vector<std::uint8_t>& inputs,
    std::size_t party_id) const {
  
  if (party_id >= garbled_circuit_.input_sizes.size()) {
    throw std::runtime_error("Invalid party id.");
  }
  if (inputs.size() != garbled_circuit_.input_sizes[party_id]) {
    throw std::runtime_error("Invalid input size.");
  }

  std::size_t wire_index = 0;
  for (std::size_t i = 0; i < party_id; ++i) {
    wire_index += garbled_circuit_.input_sizes[i];
  }

  std::vector<crypto::Label> labels;
  for (std::size_t i = 0; i < inputs.size(); ++i) {
    if (inputs[i] > 1) {
      throw std::runtime_error("Invalid input value.");
    }
    const crypto::WireLabels& wire_labels = wire_labels_[wire_index + i];
    labels.push_back(inputs[i] == 0 ? wire_labels.zero
                                    : wire_labels.one);
  }

  return labels;
}

void Garbler::SaveGarbledCircuit(const std::string& filename) const {
  std::ofstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open garbled_circuit.bin.");
  }

  file.write("CHII", 4);
  utils::WriteUint32(file, 1);
  utils::WriteUint64(file, garbled_circuit_.num_wires);
  utils::WriteUint64(file, garbled_circuit_.gates.size());
  utils::WriteSize32(file, garbled_circuit_.input_sizes.size());
  for (std::size_t size : garbled_circuit_.input_sizes) {
    utils::WriteUint64(file, size);
  }
  utils::WriteSize32(file, garbled_circuit_.output_sizes.size());
  for (std::size_t size : garbled_circuit_.output_sizes) {
    utils::WriteUint64(file, size);
  }

  for (const GarbledGate& gate : garbled_circuit_.gates) {
    utils::WriteSize32(file, gate.inputs.size());
    for (std::size_t wire : gate.inputs) {
      utils::WriteUint64(file, wire);
    }
    utils::WriteUint64(file, gate.output);
    utils::WriteSize32(file, gate.ciphertexts.size());

    for (const GarbledRow& row : gate.ciphertexts) {
      file.write(
          reinterpret_cast<const char*>(row.ciphertext.data()),
          row.ciphertext.size());
      file.write(
          reinterpret_cast<const char*>(row.check.data()),
          row.check.size());
    }
  }

  if (!file) {
    throw std::runtime_error("Failed to write garbled_circuit.bin.");
  }
}

void Garbler::SaveInputLabels(
    const std::vector<crypto::Label>& labels,
    std::size_t party_id,
    const std::string& filename) const {
  
  std::ofstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open input_labels.bin.");
  }

  file.write("KAWA", 4);
  utils::WriteUint32(file, 1);
  utils::WriteSize32(file, party_id);
  utils::WriteUint64(file, labels.size());

  for (std::size_t i = 0; i < labels.size(); ++i) {
    file.write(reinterpret_cast<const char*>(labels[i].data()), labels[i].size());
  }

  if (!file) {
    throw std::runtime_error("Failed to write input_labels.bin.");
  }
}

// TODO: Remove this function.
std::vector<std::uint8_t> Garbler::DecodeOutputLabels(
    const std::vector<crypto::Label>& output_labels) const {
  
  std::vector<std::uint8_t> outputs;

  const std::size_t num_outputs = garbled_circuit_.GetNumOutputs();
  const std::size_t index = garbled_circuit_.num_wires - num_outputs;

  for (std::size_t i = 0; i < num_outputs; ++i) {
    const auto& wire_labels = wire_labels_[index + i];
    if (output_labels[i] == wire_labels.zero) {
      outputs.push_back(0);
    } else if (output_labels[i] == wire_labels.one) {
      outputs.push_back(1);
    } else {
      throw std::runtime_error("Invalid output label");
    }
  }
  return outputs;
}

}  // namespace basic_gc