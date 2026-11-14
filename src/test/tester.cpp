#include "test/tester.h"

#include <cctype>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "circuit/parser.h"
#include "circuit/evaluator.h"
#include "config.h"
#include "crypto/label.h"
#include "garbling/yao/evaluator.h"
#include "garbling/yao/garbler.h"
#include "test/rpc_client.h"
#include "test/rpc_server.h"
#include "type.h"
#include "utils/file_utils.h"

namespace test{

namespace {

constexpr char kGarbledCircuitFile[] = "garbled_circuit.bin";
constexpr char kGarblerInputLabelsFile[] = "g_input_labels.bin";
constexpr char kEvaluatorInputLabelsFile[] = "e_input_labels.bin";
constexpr int kRpcPort = 5001;

std::vector<std::string> ParseFields(std::istringstream& line) {
  std::vector<std::string> fields;
  std::string field;
  while (std::getline(line, field, ',')) {
    fields.push_back(field);
  }
  return fields;
}

uint8_t HexToDecimal(char c) {
  if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
  if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10);
  if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);

  throw std::runtime_error("Invalid hexadecimal character.");
}

char DecimalToHex(uint8_t value) {
  if (value < 10) return static_cast<char>('0' + value);
  if (value < 16) return static_cast<char>('a' + value - 10);

  throw std::runtime_error("Invalid decimal value.");
}

std::vector<uint8_t> HexToBits(const std::string& hex,
                                std::size_t expected_size) {
  std::vector<uint8_t> bits;

  for (auto it = hex.rbegin(); it != hex.rend(); ++it) {
    const uint8_t value = HexToDecimal(*it);
    for (int i = 0; i <= 3; ++i) {
      bits.push_back((value >> i) & 1);
    }
  }

  if (bits.size() < expected_size) {
    bits.resize(expected_size, 0);
  } else if (bits.size() > expected_size) {
    bits.resize(expected_size);
  }

  return bits;
}

std::string BitsToHex(const std::vector<uint8_t>& bits) {
  std::string hex;

  const std::size_t padding_size = (4 - bits.size() % 4) % 4;
  std::vector<uint8_t> padded_bits(bits);
  padded_bits.resize(bits.size() + padding_size, 0);

  for (std::size_t i = 0; i < padded_bits.size(); i += 4) {
    uint8_t value = 0;
    for (std::size_t j = 0; j < 4; ++j) {
      if (padded_bits[i + j] == 1) {
        value += (1 << j);
      }
    }
    hex.push_back(DecimalToHex(value));
  }
  
  std::reverse(hex.begin(), hex.end());
  return hex;
}

}  // namespace

void RunPlainCircuit(
    const std::string& circuit_file, const std::string& sample_file) {

  std::ifstream file(sample_file);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open sample file: " + sample_file);
  }

  const circuit::Circuit circuit = circuit::ParseCircuit(circuit_file);

  std::istringstream line;
  std::size_t line_number = 0;
  std::size_t passed = 0;
  std::size_t failed = 0;

  std::cout << "Test starts: " << circuit_file << "\n";

  while (utils::GetNextLine(file, line)) {
    ++line_number;
    if (line_number == 1) {  // Skip header.
      continue;
    }

    const std::vector<std::string> fields = ParseFields(line);
    std::size_t num_input_parties = circuit.input_sizes.size();

    if (fields.size() != num_input_parties + 1) {
      throw std::runtime_error(
          "Invalid fields, line " + std::to_string(line_number));
    }

    // Parse input fields.

    std::vector<uint8_t> inputs;
  
    for (std::size_t i = 0; i < num_input_parties; ++i) {
      const std::vector<uint8_t> input =
          HexToBits(fields[i], circuit.input_sizes[i]);
      inputs.insert(inputs.end(), input.begin(), input.end());
    }

    const std::vector<uint8_t> expected_output =
        HexToBits(fields.back(), circuit.GetNumOutputs());
    const std::vector<uint8_t> output =
        circuit::EvaluateCircuit(circuit, inputs);

    if (kDebug) {
      std::cout << "Line [" << line_number << "], expected_output: "
                << fields.back() << ", output: " << BitsToHex(output) << "\n";
    }

    if (output == expected_output) {
      ++passed;
    } else {
      ++failed;
      std::cout << "[FAIL] Line: " << line_number << "\n";
    }
  }

  std::cout << "Results: \n";
  std::cout << "Total cases: " << line_number - 1 << "\n";
  std::cout << "Passed: " << passed << "\n";
  std::cout << "Failed: " << failed << "\n";
}

void RunBasicGC(
    const std::string& circuit_file, const std::string& sample_file) {

  std::ifstream file(sample_file);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open sample file: " + sample_file);
  }

  const circuit::Circuit circuit = circuit::ParseCircuit(circuit_file);

  if (circuit.input_sizes.size() != 2) {
    if (kDebug) {
      std::cout << "Input parties: " << circuit.input_sizes.size() << "\n";
    }
    throw std::runtime_error("Only supports 2 parties.");
  }

  basic_gc::Garbler garbler;
  garbler.GarbleCircuit(circuit);

  basic_gc::RpcServer server(garbler);
  std::thread server_thread([&server]() {
    server.Start(kRpcPort);
  });

  // Make sure that the server finished initilizaition, binding and listening.
  std::this_thread::sleep_for(std::chrono::seconds(1));

  basic_gc::RpcClient client("127.0.0.1", kRpcPort);
  client.Connect();

  basic_gc::Evaluator evaluator;

  client.RequestGarbledCircuit(kGarbledCircuitFile);
  evaluator.LoadGarbledCircuit(kGarbledCircuitFile);

  std::istringstream line;
  std::size_t line_number = 0;
  std::size_t passed = 0;
  std::size_t failed = 0;

  std::cout << "Test starts: " << circuit_file << "\n";

  while (utils::GetNextLine(file, line)) {
    ++line_number;
    if (line_number == 1) {  // Skip header.
      continue;
    }

    const std::vector<std::string> fields = ParseFields(line);
    const std::size_t num_input_parties = circuit.input_sizes.size();

    if (fields.size() != num_input_parties + 1) {
      throw std::runtime_error(
          "Invalid fields, line " + std::to_string(line_number));
    }

    // Assume that garbler represents input party 1
    // and evaluator represents input party 2.
    std::vector<std::uint8_t> g_inputs =
        HexToBits(fields[0], circuit.input_sizes[0]);
    std::vector<std::uint8_t> e_inputs =
        HexToBits(fields[1], circuit.input_sizes[1]);

    client.RequestInputLabels(g_inputs, 0, kGarblerInputLabelsFile);
    const auto g_input_labels =
        evaluator.LoadInputLabels(kGarblerInputLabelsFile);
    // TODO(low priority): Use OT instead.
    client.RequestInputLabels(e_inputs, 1, kEvaluatorInputLabelsFile);
    const auto e_input_labels =
        evaluator.LoadInputLabels(kEvaluatorInputLabelsFile);

    const std::vector<Label> output_labels = evaluator.Evaluate(
        g_input_labels, e_input_labels, 0, 1);
    // TODO(low priority): Use proper output decoding instead.
    const std::vector<std::uint8_t> output =
        client.RequestOutput(output_labels);

    const std::vector<std::uint8_t> expected_output =
        HexToBits(fields.back(), circuit.GetNumOutputs());

    if (output == expected_output) {
      ++passed;
    } else {
      ++failed;
      std::cout << "[FAIL] Line: " << line_number << "\n";
    }

    if (kDebug) {
      std::cout << "Line [" << line_number << "], expected_output: "
                << fields.back() << ", output: " << BitsToHex(output) << "\n";
    }
  }

  client.CloseSockets();
  server_thread.join();

  std::cout << "Results: \n";
  std::cout << "Total cases: " << line_number - 1 << "\n";
  std::cout << "Passed: " << passed << "\n";
  std::cout << "Failed: " << failed << "\n";
}

}  // namespace test
