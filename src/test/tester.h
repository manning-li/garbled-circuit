#ifndef TESTER_H
#define TESTER_H

#include <string>

namespace test{

void RunPlainCircuit(
    const std::string& circuit_file, const std::string& sample_file);

void RunBasicGC(
    const std::string& circuit_file, const std::string& sample_file);

}  // namespace test

#endif  // TESTER_H