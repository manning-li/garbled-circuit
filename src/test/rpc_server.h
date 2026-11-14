#ifndef TEST_RPC_SERVER_H
#define TEST_RPC_SERVER_H

#include <cstddef>
#include <cstdint>
#include <vector>

#include "garbling/yao/garbler.h"
#include "type.h"

namespace basic_gc {

class RpcServer {
 public:
  explicit RpcServer(Garbler& garbler) : garbler_(garbler) {}

  void Start(int port);

 private:
  void SendGarbledCircuit();
  void SendInputLabels();
  void SendOutput();

  void CloseSockets();

  Garbler& garbler_;

  int server_socket_ = -1;
  int client_socket_ = -1;
};

}  // namespace basic_gc

#endif  // TEST_RPC_SERVER_H