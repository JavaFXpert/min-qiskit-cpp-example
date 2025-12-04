cmake_minimum_required(VERSION 3.16)
project(bell_state_example LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qiskit C++ requires the QISKIT_ROOT path to be set
if(NOT DEFINED QISKIT_ROOT)
    message(FATAL_ERROR "QISKIT_ROOT must be set to the Qiskit repository path.\n"
            "Example: cmake -DQISKIT_ROOT=/path/to/qiskit ..")
endif()

# For sampler functionality, QISKIT_IBM_RUNTIME_C_ROOT must also be set
if(NOT DEFINED QISKIT_IBM_RUNTIME_C_ROOT)
    message(WARNING "QISKIT_IBM_RUNTIME_C_ROOT not set - sampler functionality will not work.\n"
            "See README.md for setup instructions.")
endif()

# Fetch nlohmann/json (required by qiskit-cpp)
include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

# Create the executable
add_executable(bell_state src/main.cpp)

# Include directories
target_include_directories(bell_state PRIVATE
    ${QISKIT_ROOT}/dist/c/include
    ${QISKIT_ROOT}/qiskit-cpp/src
    ${QISKIT_IBM_RUNTIME_C_ROOT}/include
)

# Link libraries
if(APPLE)
    target_link_libraries(bell_state PRIVATE
        "-L${QISKIT_ROOT}/dist/c/lib -L${QISKIT_IBM_RUNTIME_C_ROOT}/build/cargo/debug -Wl,-rpath,${QISKIT_ROOT}/dist/c/lib -Wl,-rpath,${QISKIT_IBM_RUNTIME_C_ROOT}/build/cargo/debug"
        qiskit
        qiskit_ibm_runtime
        nlohmann_json::nlohmann_json
    )
elseif(UNIX)
    target_link_libraries(bell_state PRIVATE
        "-L${QISKIT_ROOT}/dist/c/lib -L${QISKIT_IBM_RUNTIME_C_ROOT}/build/cargo/debug -Wl,-rpath,${QISKIT_ROOT}/dist/c/lib -Wl,-rpath,${QISKIT_IBM_RUNTIME_C_ROOT}/build/cargo/debug"
        qiskit
        qiskit_ibm_runtime
        nlohmann_json::nlohmann_json
    )
elseif(MSVC)
    target_link_directories(bell_state PUBLIC
        ${QISKIT_ROOT}/target/release
        ${QISKIT_IBM_RUNTIME_C_ROOT}/build/cargo/debug
    )
    target_link_libraries(bell_state PRIVATE
        qiskit_cext.dll.lib
        qiskit_ibm_runtime.dll.lib
        nlohmann_json::nlohmann_json
    )
endif()

# Pass compile definition for runtime root
if(QISKIT_IBM_RUNTIME_C_ROOT)
    target_compile_definitions(bell_state PRIVATE
        QISKIT_IBM_RUNTIME_C_ROOT="${QISKIT_IBM_RUNTIME_C_ROOT}"
    )
endif()

# Pure C version using C API directly
add_executable(bell_state_c src/bell_state_c.c)

target_include_directories(bell_state_c PRIVATE
    ${QISKIT_ROOT}/dist/c/include
    ${QISKIT_IBM_RUNTIME_C_ROOT}/include
)

if(APPLE)
    target_link_libraries(bell_state_c PRIVATE
        "-L${QISKIT_ROOT}/dist/c/lib -L${QISKIT_IBM_RUNTIME_C_ROOT}/build/cargo/debug -Wl,-rpath,${QISKIT_ROOT}/dist/c/lib -Wl,-rpath,${QISKIT_IBM_RUNTIME_C_ROOT}/build/cargo/debug"
        qiskit
        qiskit_ibm_runtime
    )
elseif(UNIX)
    target_link_libraries(bell_state_c PRIVATE
        "-L${QISKIT_ROOT}/dist/c/lib -L${QISKIT_IBM_RUNTIME_C_ROOT}/build/cargo/debug -Wl,-rpath,${QISKIT_ROOT}/dist/c/lib -Wl,-rpath,${QISKIT_IBM_RUNTIME_C_ROOT}/build/cargo/debug"
        qiskit
        qiskit_ibm_runtime
    )
elseif(MSVC)
    target_link_directories(bell_state_c PUBLIC
        ${QISKIT_ROOT}/target/release
        ${QISKIT_IBM_RUNTIME_C_ROOT}/build/cargo/debug
    )
    target_link_libraries(bell_state_c PRIVATE
        qiskit_cext.dll.lib
        qiskit_ibm_runtime.dll.lib
    )
endif()

# Installation
install(TARGETS bell_state bell_state_c DESTINATION bin)
