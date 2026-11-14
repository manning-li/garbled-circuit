#include "garbling/yao/evaluator.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "crypto/crypto_utils.h"
#include "utils/file_utils.h"

namespace basic_gc {

namespace{

bool IsValidText(const Plaintext& text) {
  for (std::size_t i = kLabelSize; i < text.size(); ++i) {
    if (text[i] != 0) {
      return false;
    }
  }
  return true;
}
      
} // namespace


std::vector<Label> Evaluator::Evaluate(
    const std::vector<Label>& e_input_labels) const {
  if (!garbled_circuit_loaded_ || !input_labels_loaded_) {
    throw std::runtime_error("Binary files not loaded.");
  }

  const std::size_t e_party_id = 1 - g_party_id_;

  if (g_input_labels_.size() != garbled_circuit_.input_sizes[g_party_id_] ||
      e_input_labels.size() != garbled_circuit_.input_sizes[e_party_id]) {
    throw std::runtime_error("Invalid input labels.");
  }

  std::vector<std::optional<Label>> wires(garbled_circuit_.num_wires);

  std::size_t g_wire_index = 0;
  for (std::size_t i = 0; i < g_party_id_; ++i) {
    g_wire_index += garbled_circuit_.input_sizes[i];
  }
  for (std::size_t i = 0; i < g_input_labels_.size(); ++i) {
    wires[g_wire_index + i] = g_input_labels_[i];
  }

  std::size_t e_wire_index = 0;
  for (std::size_t i = 0; i < e_party_id; ++i) {
    e_wire_index += garbled_circuit_.input_sizes[i];
  }
  for (std::size_t i = 0; i < e_input_labels.size(); ++i) {
    wires[e_wire_index + i] = e_input_labels[i];
  }

  for (const auto& gate : garbled_circuit_.gates) {
    std::vector<Label> input_labels;
    for (std::size_t input : gate.inputs) {
      if (!wires[input].has_value()) {
        throw std::runtime_error(
            "Input wire not assigned: wires[" + std::to_string(input) + "].");
      }
      input_labels.push_back(*wires[input]);
    }

    const auto key = crypto::DeriveKey(input_labels);
    bool has_valid_text = false;
    Label output_label{};
    for (const Ciphertext& text : gate.ciphertexts) {
      const auto decrypted = crypto::Decrypt(key, text);
      if (IsValidText(decrypted)) {
        std::copy(
            decrypted.begin(), decrypted.begin() + kLabelSize,
            output_label.begin());
        has_valid_text = true;
      }
    }
    if (!has_valid_text) {
      throw std::runtime_error("No valid garbled row!");
    }
    wires[gate.output] = output_label;
  }

  // Output index range is based on Bristol Fashion.
  const std::size_t num_outputs = garbled_circuit_.GetNumOutputs();
  const std::size_t num_wires = garbled_circuit_.num_wires;

  std::vector<Label> output_labels;
  for (std::size_t i = (num_wires - num_outputs); i < num_wires; ++i) {
    if (!wires[i].has_value()) {
      throw std::runtime_error(
          "Output wire not assigned: wires[" + std::to_string(i) + "].");
    }
    output_labels.push_back(*wires[i]);
  }

  return output_labels;
}

void Evaluator::LoadGarbledCircuit(const std::string& filename) {
  if (garbled_circuit_loaded_) {
    throw std::runtime_error("File garbled_circuit.bin already loaded.");
  }

  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open garbled_circuit.bin.");
  }

  char magic[4];
  file.read(magic, 4);
  if (!file || std::string(magic, 4) != "CHII") {
    throw std::runtime_error("Invalid garbled_circuit.bin.");
  }

  const std::uint32_t version = utils::ReadUint32(file);
  if (version != 1) {
    throw std::runtime_error("Invalid version.");
  }

  // Read header.

  GarbledCircuit circuit;
  circuit.num_wires = static_cast<std::size_t>(utils::ReadUint64(file));
  const std::size_t num_gates = static_cast<std::size_t>(utils::ReadUint64(file));
  
  const std::size_t num_input_parties = utils::ReadSize32(file);
  circuit.input_sizes.resize(num_input_parties);
  for (std::size_t& size : circuit.input_sizes) {
    size = static_cast<std::size_t>(utils::ReadUint64(file));
  }

  const std::size_t num_output_parties = utils::ReadSize32(file);
  circuit.output_sizes.resize(num_output_parties);
  for (std::size_t& size : circuit.output_sizes) {
    size = static_cast<std::size_t>(utils::ReadUint64(file));
  }

  // Read gates.

  circuit.gates.reserve(num_gates);

  for (std::size_t i = 0; i < num_gates; ++i) {
    GarbledGate gate;

    const std::size_t num_inputs = utils::ReadSize32(file);
    gate.inputs.resize(num_inputs);
    for (std::size_t& wire : gate.inputs) {
      wire = static_cast<std::size_t>(utils::ReadUint64(file));
    }
    gate.output = static_cast<std::size_t>(utils::ReadUint64(file));

    const std::size_t num_rows = utils::ReadSize32(file);
    gate.ciphertexts.resize(num_rows);
    for (Ciphertext& text : gate.ciphertexts) {
      file.read(reinterpret_cast<char*>(text.data()), text.size());
      if (!file) {
        throw std::runtime_error("Failed to read garbled row.");
      }
    }

    circuit.gates.push_back(std::move(gate));
  }
  garbled_circuit_ = std::move(circuit);
  garbled_circuit_loaded_ = true;
}

void Evaluator::LoadInputLabels(const std::string& filename) {
  if (input_labels_loaded_) {
    throw std::runtime_error("File input_labels.bin already loaded.");
  }
  if (!garbled_circuit_loaded_) {
    throw std::runtime_error("File garbled_circuit.bin not loaded.");
  }

  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open input_labels.bin.");
  }

  char magic[4];
  file.read(magic, 4);
  if (!file || std::string(magic, 4) != "KAWA") {
    throw std::runtime_error("Invalid input_labels.bin.");
  }

  const std::uint32_t version = utils::ReadUint32(file);
  if (version != 1) {
    throw std::runtime_error("Invalid version.");
  }

  g_party_id_ = utils::ReadUint32(file);
  if (g_party_id_ >= garbled_circuit_.input_sizes.size()) {
    throw std::runtime_error("Invalid party ID.");
  }

  const std::size_t num_labels =
      static_cast<std::size_t>(utils::ReadUint64(file));
  if (num_labels != garbled_circuit_.input_sizes[g_party_id_]) {
    throw std::runtime_error("Invalid input labels number.");
  }

  std::vector<Label> labels;
  labels.reserve(num_labels);
  for (std::size_t i = 0; i < num_labels; ++i) {
    Label label{};
    file.read(reinterpret_cast<char*>(label.data()), label.size());
    if (!file) {
      throw std::runtime_error("Failed to read input label.");
    }
    labels.push_back(label);
  }
  g_input_labels_ = std::move(labels);
  input_labels_loaded_ = true;
}

void Evaluator::ResetGarbledCircuit() {
  garbled_circuit_ = GarbledCircuit{};
  garbled_circuit_loaded_ = false;
}

void Evaluator::ResetInputLabels() {
  g_input_labels_ = {};
  input_labels_loaded_ = false;
}

}