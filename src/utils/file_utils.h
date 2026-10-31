#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <fstream>
#include <sstream>
#include <string>

namespace utils {

bool IsBlankLine(const std::string& line);
bool GetNextLine(std::ifstream& file, std::istringstream& line);

}  // namespace utils

#endif // FILE_UTILS_H