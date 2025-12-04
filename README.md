# min-qiskit-cpp-example

Minimal Qiskit C++ example demonstrating a Bell state circuit with IBM Quantum Runtime sampler.

## Overview

This project creates a simple Bell state (|Φ+⟩ = (|00⟩ + |11⟩) / √2) and runs it on IBM Quantum hardware using the Qiskit C++ interface introduced in Qiskit 2.2.

## Prerequisites

### 1. System Requirements

- **Operating System**: Linux (Ubuntu 22.04+), macOS Sequoia 15.1+, or Windows
- **C++ Compiler**: GCC, Clang, or MSVC with C++17 support
- **CMake**: Version 3.16 or higher
- **Rust**: Version 1.85 or higher
- **Python**: Version 3.11 or higher

### 2. Install Qiskit and Build the C API

```bash
# Clone the Qiskit repository
git clone https://github.com/Qiskit/qiskit.git
cd qiskit

# Install Qiskit Python package
pip install -e .

# Build the C extension library
make c

# Note: This creates libraries in dist/c/lib/ and headers in dist/c/include/
```

### 3. Clone Qiskit C++ (included in Qiskit repo)

The qiskit-cpp headers are located in the Qiskit repository at `qiskit-cpp/src/`.

If not present, clone it:
```bash
cd /path/to/qiskit
git clone https://github.com/Qiskit/qiskit-cpp.git
```

### 4. Build qiskit-ibm-runtime-c Library

The sampler functionality requires the IBM Runtime C bindings (a separate repo):

```bash
# Clone qiskit-ibm-runtime-c (note: separate repo from qiskit-ibm-runtime)
git clone https://github.com/Qiskit/qiskit-ibm-runtime-c.git
cd qiskit-ibm-runtime-c

# Build the library
mkdir build && cd build
cmake ..
make
```

**Note:** Requires Rust, cargo, and cbindgen to be installed.

### 5. IBM Quantum Credentials

Create the credentials file at `$HOME/.qiskit/qiskit-ibm.json`:

```json
{
  "token": "YOUR_IBM_QUANTUM_API_TOKEN",
  "instance": "ibm-q/open/main"
}
```

To get your API token:
1. Go to [IBM Quantum](https://quantum.ibm.com/)
2. Sign in or create an account
3. Navigate to your account settings
4. Copy your API token

Alternatively, set environment variables:
```bash
export QISKIT_IBM_TOKEN="your-api-token"
export QISKIT_IBM_INSTANCE="ibm-q/open/main"
```

## Building

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
# Set both QISKIT_ROOT and QISKIT_IBM_RUNTIME_C_ROOT
cmake \
  -DQISKIT_ROOT=/path/to/qiskit \
  -DQISKIT_IBM_RUNTIME_C_ROOT=/path/to/qiskit-ibm-runtime-c \
  ..

# Build
make
```

## Usage

```bash
# Run with default backend (ibm_torino) and 1024 shots
./bell_state

# Run with a specific backend
./bell_state ibm_fez

# Run with specific backend and shot count
./bell_state ibm_torino 2048
```

## Expected Output

```
Bell State Circuit Example
==========================
Backend: ibm_torino
Shots: 1024

Circuit (QASM3):
OPENQASM 3.0;
include "stdgates.inc";
qubit[2] q;
bit[2] meas;
h q[0];
cx q[0], q[1];
meas = measure q;

Job submitted. Waiting for results...

Measurement Results:
-------------------
  |00⟩: 518 (50.6%)
  |11⟩: 506 (49.4%)

Expected: ~50% |00⟩ and ~50% |11⟩ (Bell state entanglement)
```

The Bell state demonstrates quantum entanglement: measuring qubit 0 as |0⟩ guarantees qubit 1 is also |0⟩, and similarly for |1⟩.

## Project Structure

```
min-qiskit-cpp-example/
├── CMakeLists.txt      # Build configuration
├── README.md           # This file
└── src/
    └── main.cpp        # Bell state circuit implementation
```

## Troubleshooting

### CMake can't find qiskit library
Ensure you've run `make c` in the Qiskit repository to build the C API.

### Sampler functionality not working
Make sure `QISKIT_IBM_RUNTIME_C_ROOT` points to the qiskit-ibm-runtime/c directory with the built library.

### Authentication errors
Verify your credentials in `$HOME/.qiskit/qiskit-ibm.json` or set the environment variables.

## References

- [Qiskit C++ GitHub](https://github.com/Qiskit/qiskit-cpp)
- [Qiskit IBM Runtime C](https://github.com/Qiskit/qiskit-ibm-runtime-c)
- [Qiskit C API Documentation](https://www.ibm.com/quantum/blog/c-api-enables-end-to-end-hpc-demo)
- [Qiskit 2.2 Release Notes](https://docs.quantum.ibm.com/guides/latest-updates)
- [IBM Quantum Documentation](https://docs.quantum.ibm.com/)
