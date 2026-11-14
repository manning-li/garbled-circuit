#ifndef GARBLING_YAO_EVALUATOR_H
#define GARBLING_YAO_EVALUATOR_H

#include <cstddef>
#include <string>
#include <vector>

#include "crypto/label.h"
#include "garbling/yao/garbled_circuit.h"
#include "type.h"

namespace basic_gc {

class Evaluator {
 public:
  std::vector<Label> Evaluate(
      const std::vector<Label>& g_input_labels,
      const std::vector<Label>& e_input_labels,
      std::size_t g_party_id, std::size_t e_party_id) const;

  void LoadGarbledCircuit(const std::string& filename);
  std::vector<Label> LoadInputLabels(const std::string& filename);

  void ResetGarbledCircuit();
  void ResetInputLabels();

 private:
  GarbledCircuit garbled_circuit_;
  bool garbled_circuit_loaded_ = false;
};

}  // namespace basic_gc

#endif  // GARBLING_YAO_EVALUATOR_H