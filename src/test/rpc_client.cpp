#include "test/rpc_client.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "utils/rpc_utils.h"

using namespace utils;

namespace basic_gc {

void RpcClient::Connect() {
  if (socket_ >= 0) {
    return;
  }

  socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_ < 0) {
    throw std::runtime_error("Failed to create client socket.");
  }

  sockaddr_in server_address{};
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(static_cast<std::uint16_t>(port_));

  if (inet_pton(AF_INET, host_.c_str(), &server_address.sin_addr) <= 0 ||
      connect(socket_, reinterpret_cast<sockaddr*>(&server_address),
              sizeof(server_address)) < 0) {
    close(socket_);
    socket_ = -1;
    throw std::runtime_error("Failed to connect to server.");
  }
}

void RpcClient::CloseSockets() {
  if (socket_ >= 0) {
    close(socket_);
    socket_ = -1;
  }
}

void RpcClient::RequestGarbledCircuit(const std::string& filename) {
  SendUint32(socket_, kRequestGarbledCircuit);
  ReceiveFile(socket_, filename);
}

void RpcClient::RequestInputLabels(
    const std::vector<std::uint8_t>& inputs,
    std::size_t party_id,
    const std::string& filename) {
  SendUint32(socket_, kRequestInputLabels);
  SendUint64(socket_, party_id);
  SendUint64(socket_, inputs.size());
  SendAll(socket_, inputs.data(), inputs.size());

  ReceiveFile(socket_, filename);
}

std::vector<std::uint8_t> RpcClient::RequestOutput(
    const std::vector<Label>& output_labels) {
  SendUint32(socket_, kRequestOutput);
  SendUint64(socket_, output_labels.size());
  SendAll(
      socket_, output_labels.data(),
      output_labels.size() * sizeof(Label));

  const std::uint64_t num_outputs = ReceiveUint64(socket_);
  std::vector<std::uint8_t> output(static_cast<std::size_t>(num_outputs));
  ReceiveAll(socket_, output.data(), output.size());

  return output;
}

}  // namespace basic_gc
