#ifndef TEST_RPC_CLIENT_H
#define TEST_RPC_CLIENT_H

#include <cstddef>
#include <cstdint>
#include <vector>

#include "test/rpc_server.h"
#include "type.h"

namespace basic_gc {

class RpcClient {
 public:
  RpcClient(const std::string& host, int port) : host_(host), port_(port) {}

  void Connect();
  void CloseSockets();

  void RequestGarbledCircuit(const std::string& filename);

  void RequestInputLabels(
      const std::vector<std::uint8_t>& inputs,
      std::size_t party_id,
      const std::string& filename);

  std::vector<std::uint8_t> RequestOutput(
      const std::vector<Label>& output_labels);

 private:
  std::string host_;
  int port_;
  int socket_ = -1;
};

}  // namespace basic_gc

#endif  // TEST_RPC_CLIENT_H