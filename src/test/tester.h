#ifndef TESTER_H
#define TESTER_H

#include <string>

namespace test{

void Run(const std::string& circuit_file,
         const std::string& sample_file);

}  // namespace test

#endif  // TESTER_H