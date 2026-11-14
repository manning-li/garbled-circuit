#include "test/rpc_server.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "utils/rpc_utils.h"

using namespace utils;

namespace basic_gc {

constexpr char kGarbledCircuitFile[] = "server_garbled_circuit.bin";
constexpr char kInputLabelsFile[] = "server_input_labels.bin";

void RpcServer::Start(int port) {
  server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket_ < 0) {
    throw std::runtime_error("Failed to create server socket.");
  }

  int reuse_address = 1;
  if (setsockopt(
          server_socket_, SOL_SOCKET, SO_REUSEADDR, &reuse_address,
          sizeof(reuse_address)) < 0) {
    throw std::runtime_error("Failed to set socket config.");
  }

  sockaddr_in server_address{};
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(static_cast<std::uint16_t>(port));

  try {
    if (bind(server_socket_, reinterpret_cast<sockaddr*>(&server_address),
             sizeof(server_address)) < 0 ||
        listen(server_socket_, 1) < 0) {
      throw std::runtime_error("Failed to init server socket.");
    }

    client_socket_ = accept(server_socket_, nullptr, nullptr);
    if (client_socket_ < 0) {
      throw std::runtime_error("Failed to accept client socket.");
    }

    while (true) {
      std::uint32_t request_type = 0;
      if (!ReceiveAll(
              client_socket_, &request_type, sizeof(request_type), true)) {
        break;
      }
      request_type = ntohl(request_type);

      switch (request_type) {
        case kRequestGarbledCircuit:
          SendGarbledCircuit();
          break;
        case kRequestInputLabels:
          SendInputLabels();
          break;
        case kRequestOutput:
          SendOutput();
          break;

        default:
          throw std::runtime_error("Invalid request.");
      }
    }
    CloseSockets();
  } catch (...) {
    CloseSockets();
    throw;
  }
}

// Send format:
// 1. Size of server_garbled_circuit.bin (uint64_t);
// 2. server_garbled_circuit.bin.
void RpcServer::SendGarbledCircuit() {
  garbler_.SaveGarbledCircuit(kGarbledCircuitFile);
  SendFile(client_socket_, kGarbledCircuitFile);
}
 
// Recv format:
// 1. Party ID (uint64_t);
// 2. Number of inputs (uint64_t);
// 3. Input (vector<uint8_t>).
//
// Send format:
// 1. Size of server_input_labels.bin;
// 2. server_input_labels.bin.
void RpcServer::SendInputLabels() {
  const std::uint64_t party_id = ReceiveUint64(client_socket_);
  const std::uint64_t num_inputs = ReceiveUint64(client_socket_);

  const std::size_t s_id = static_cast<std::size_t>(party_id);
  const std::size_t s_num = static_cast<std::size_t>(num_inputs);

  std::vector<std::uint8_t> inputs(s_num);
  ReceiveAll(client_socket_, inputs.data(), inputs.size());

  const auto labels = garbler_.GetInputLabels(inputs, s_id);
  garbler_.SaveInputLabels(labels, s_id, kInputLabelsFile);

  SendFile(client_socket_, kInputLabelsFile);
}

// Recv format:
// 1. Number of output labels (uint64_t);
// 2. Output labels (vector<Label>).
//
// Send format:
// 1. Number of output (uint64_t);
// 2. Output bits (vector<uint8_t>).
void RpcServer::SendOutput() {
  const std::uint64_t num_labels = ReceiveUint64(client_socket_);
  const std::size_t s_num = static_cast<std::size_t>(num_labels);

  std::vector<Label> output_labels(s_num);
  ReceiveAll(
      client_socket_, output_labels.data(),
      output_labels.size() * sizeof(Label));

  const auto output = garbler_.DecodeOutputLabels(output_labels);

  SendUint64(client_socket_, output.size());
  SendAll(client_socket_, output.data(), output.size());
}

void RpcServer::CloseSockets() {
  if (client_socket_ >= 0) {
    close(client_socket_);
    client_socket_ = -1;
  }

  if (server_socket_ >= 0) {
    close(server_socket_);
    server_socket_ = -1;
  }
}

}  // namespace basic_gc
