#ifndef RPC_UTILS_H
#define RPC_UTILS_H

#include <cstddef>
#include <cstdint>
#include <string>

namespace utils {

constexpr std::uint32_t kRequestGarbledCircuit = 1;
constexpr std::uint32_t kRequestInputLabels = 2;
constexpr std::uint32_t kRequestOutput = 3;

constexpr std::size_t kBufferSize = 64 * 1024;

void SendAll(int socket, const void* data, std::size_t size);
bool ReceiveAll(
    int socket, void* data, std::size_t size, bool allow_eof = false);

void SendUint64(int socket, std::uint64_t value);
std::uint64_t ReceiveUint64(int socket);

void SendUint32(int socket, std::uint32_t value);

void SendFile(int socket, const std::string& filename);
void ReceiveFile(int socket, const std::string& filename);

}  // namespace utils

#endif // RPC_UTILS_H