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

bool CheckText(const std::array<std::uint8_t, crypto::kPlaintextSize>& text) {
  for (std::size_t i = crypto::kLabelSize; i < text.size(); ++i) {
    if (text[i] != 0) {
      return false;
    }
  }
  return true;
}

std::array<std::uint8_t, crypto::kPlaintextSize> RowToText(const GarbledRow& row) {
  std::array<std::uint8_t, crypto::kPlaintextSize> ciphertext{};
  std::copy(row.ciphertext.begin(), row.ciphertext.end(), ciphertext.begin());
  std::copy(
      row.check.begin(), row.check.end(),
      ciphertext.begin() + crypto::kBlockSize);
  return ciphertext;
}
      
} // namespace


std::vector<crypto::Label> Evaluator::Evaluate(
    const std::vector<crypto::Label>& e_input_labels) const {

  // TODO: Make sure g_input_labels_ and garbled_circuit_ are loaded.

  const std::size_t e_party_id = 1 - g_party_id_;

  if (g_input_labels_.size() != garbled_circuit_.input_sizes[g_party_id_] ||
      e_input_labels.size() != garbled_circuit_.input_sizes[e_party_id]) {
    throw std::runtime_error("Invalid input labels.");
  }

  std::vector<std::optional<crypto::Label>> wires(garbled_circuit_.num_wires);

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
    std::vector<crypto::Label> input_labels;
    for (std::size_t input : gate.inputs) {
      if (!wires[input].has_value()) {
        throw std::runtime_error(
            "Input wire not assigned: wires[" + std::to_string(input) + "].");
      }
      input_labels.push_back(*wires[input]);
    }

    const auto key = crypto::DeriveKey(input_labels);
    bool has_valid_text = false;
    crypto::Label output_label{};
    for (const GarbledRow& row : gate.ciphertexts) {
      const auto plaintext = crypto::Decrypt(key, RowToText(row));
      if (CheckText(plaintext)) {
        std::copy(
            plaintext.begin(), plaintext.begin() + crypto::kLabelSize,
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

  std::vector<crypto::Label> output_labels;
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
    for (GarbledRow& row : gate.ciphertexts) {
      file.read(
          reinterpret_cast<char*>(row.ciphertext.data()),
          row.ciphertext.size());
      file.read(reinterpret_cast<char*>(row.check.data()), row.check.size());
      if (!file) {
        throw std::runtime_error("Failed to read garbled row.");
      }
    }

    circuit.gates.push_back(std::move(gate));
  }
  garbled_circuit_ = std::move(circuit);
}

void Evaluator::LoadInputLabels(const std::string& filename) {

  // TODO: Make sure garbled_circuit_ is loaded.

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

  std::vector<crypto::Label> labels;
  labels.reserve(num_labels);
  for (std::size_t i = 0; i < num_labels; ++i) {
    crypto::Label label{};
    file.read(reinterpret_cast<char*>(label.data()), label.size());
    if (!file) {
      throw std::runtime_error("Failed to read input label.");
    }
    labels.push_back(label);
  }
  g_input_labels_ = std::move(labels);
}

}