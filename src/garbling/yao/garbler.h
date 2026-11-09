#ifndef GARBLING_YAO_GARBLER_H
#define GARBLING_YAO_GARBLER_H

#include <cstdint>
#include <vector>

#include "circuit/circuit.h"
#include "crypto/label.h"
#include "garbling/yao/garbled_circuit.h"

namespace basic_gc {

class Garbler {
 public:
  void GarbleCircuit(const circuit::Circuit& circuit);

  const GarbledCircuit& GetGarbledCircuit() const {
    return garbled_circuit_;
  }

  std::vector<crypto::Label> GetInputLabels(
      const std::vector<std::uint8_t>& inputs,
      std::size_t party_id) const;

  void SaveGarbledCircuit(const std::string& filename) const;
  void SaveInputLabels(
      const std::vector<crypto::Label>& labels,
      std::size_t party_id,
      const std::string& filename) const;
  
  // TODO: Remove this function.
  std::vector<std::uint8_t> DecodeOutputLabels(
      const std::vector<crypto::Label>& output_labels) const;

 private:
  GarbledCircuit garbled_circuit_;
  std::vector<crypto::WireLabels> wire_labels_;
};

}  // namespace basic_gc

#endif  // GARBLING_YAO_GARBLER_H
