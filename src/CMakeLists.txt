add_executable(garbled_circuits
    main.cpp
    circuit/parser.cpp
    circuit/evaluator.cpp
    test/tester.cpp
    utils/file_utils.cpp
    crypto/label.cpp
    crypto/crypto_utils.cpp
    garbling/yao/garbler.cpp
    garbling/yao/evaluator.cpp
)

target_include_directories(garbled_circuits PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(garbled_circuits PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_compile_definitions(garbled_circuits PRIVATE
    PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)

target_link_libraries(garbled_circuits PRIVATE
    OpenSSL::Crypto
)