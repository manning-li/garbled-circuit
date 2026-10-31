#include <exception>
#include <iostream>

#include "test/tester.h"

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <circuit_file> <sample_file>\n";
    return 1;
  }

  try {
    test::Run(argv[1], argv[2]);
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << '\n';
    return 1;
  }
}