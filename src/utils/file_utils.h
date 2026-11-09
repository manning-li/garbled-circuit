#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <fstream>
#include <sstream>
#include <string>

namespace utils {

bool IsBlankLine(const std::string& line);
bool GetNextLine(std::ifstream& file, std::istringstream& line);

void WriteUint32(std::ofstream& file, std::uint32_t value);
void WriteUint64(std::ofstream& file, std::uint64_t value);
void WriteSize32(std::ofstream& file, std::size_t value);

std::uint32_t ReadUint32(std::ifstream& file);
std::uint64_t ReadUint64(std::ifstream& file);
std::size_t ReadSize32(std::ifstream& file);

}  // namespace utils

#endif // FILE_UTILS_H