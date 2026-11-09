#ifndef GARBLING_YAO_EVALUATOR_H
#define GARBLING_YAO_EVALUATOR_H

#include <cstddef>
#include <string>
#include <vector>

#include "crypto/label.h"
#include "garbling/yao/garbled_circuit.h"

namespace basic_gc {

class Evaluator {
 public:
  std::vector<crypto::Label> Evaluate(
      const std::vector<crypto::Label>& evaluator_input_labels) const;

  void LoadGarbledCircuit(const std::string& filename);
  void LoadInputLabels(const std::string& filename);

 private:
  GarbledCircuit garbled_circuit_;

  // Garbler inputs.
  std::vector<crypto::Label> g_input_labels_;
  std::size_t g_party_id_ = 0;
};

}  // namespace basic_gc

#endif  // GARBLING_YAO_EVALUATOR_H