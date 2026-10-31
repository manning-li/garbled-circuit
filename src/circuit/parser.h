#ifndef PARSER_H
#define PARSER_H

#include <string>

#include "circuit/circuit.h"

namespace circuit{

Circuit ParseCircuit(const std::string& filename);

}  // namespace circuit

#endif  // PARSER_H
