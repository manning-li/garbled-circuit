#include "utils/file_utils.h"

#include <cctype>
#include <stdexcept>

namespace utils {

bool IsBlankLine(const std::string& line) {
  for (char c : line) {
    if (!std::isspace(static_cast<unsigned char>(c))) {
      return false;
    }
  }

  return true;
}

bool GetNextLine(std::ifstream& file, std::istringstream& line) {
  std::string str;
  while (std::getline(file, str)) {
    if (!IsBlankLine(str)) {
      line = std::istringstream(str);
      return true;
    }
  }
  return false;
}

}  // namespace utils