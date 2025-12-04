/*
 * Qiskit C++ Example - N-Qubit GHZ State with Runtime Sampler
 *
 * This program creates an N-qubit GHZ (Greenberger-Horne-Zeilinger) state
 * and runs it on IBM Quantum hardware using the Qiskit C++ interface.
 *
 * GHZ state: |GHZ⟩ = (|00...0⟩ + |11...1⟩) / √2
 *
 * Usage: ghz_20q <num_qubits> <backend> [shots]
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cstdlib>
#include <map>

#include "circuit/quantumcircuit.hpp"
#include "primitives/backend_sampler_v2.hpp"
#include "service/qiskit_runtime_service.hpp"
#include "compiler/transpiler.hpp"

using namespace Qiskit;
using namespace Qiskit::circuit;
using namespace Qiskit::providers;
using namespace Qiskit::primitives;
using namespace Qiskit::service;
using namespace Qiskit::compiler;

using Sampler = BackendSamplerV2;

void print_usage(const char* program_name) {
    std::cerr << "Usage: " << program_name << " <num_qubits> <backend> [shots]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Arguments:" << std::endl;
    std::cerr << "  num_qubits  Number of qubits in the GHZ state (2-127)" << std::endl;
    std::cerr << "  backend     IBM Quantum backend name (e.g., ibm_fez, ibm_torino)" << std::endl;
    std::cerr << "  shots       Number of shots (default: 1024)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Examples:" << std::endl;
    std::cerr << "  " << program_name << " 20 ibm_fez" << std::endl;
    std::cerr << "  " << program_name << " 50 ibm_torino 2048" << std::endl;
}

int main(int argc, char* argv[]) {
    // Check for required arguments
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    // Parse command line arguments
    int num_qubits = std::atoi(argv[1]);
    std::string backend_name = argv[2];
    int num_shots = (argc > 3) ? std::atoi(argv[3]) : 1024;

    // Validate num_qubits
    if (num_qubits < 2 || num_qubits > 127) {
        std::cerr << "Error: num_qubits must be between 2 and 127" << std::endl;
        return 1;
    }

    std::cout << num_qubits << "-Qubit GHZ State Example" << std::endl;
    std::cout << "==========================" << std::endl;
    std::cout << "Backend: " << backend_name << std::endl;
    std::cout << "Shots: " << num_shots << std::endl;
    std::cout << "Qubits: " << num_qubits << std::endl << std::endl;

    // Create an N-qubit circuit
    QuantumRegister qr(num_qubits);
    ClassicalRegister cr(num_qubits, std::string("meas"));
    QuantumCircuit circ(
        std::vector<QuantumRegister>({qr}),
        std::vector<ClassicalRegister>({cr})
    );

    // Build GHZ state: |GHZ⟩ = (|00...0⟩ + |11...1⟩) / √2
    // Step 1: Hadamard on qubit 0 to create superposition
    circ.h(0);

    // Step 2: CNOT cascade from qubit 0 to all others
    for (int i = 1; i < num_qubits; i++) {
        circ.cx(0, i);
    }

    // Measure all qubits
    circ.measure(qr, cr);

    // Print circuit info
    std::cout << "Circuit: H(0)";
    for (int i = 1; i < num_qubits; i++) {
        std::cout << ", CX(0," << i << ")";
    }
    std::cout << ", Measure" << std::endl << std::endl;

    // Print the circuit in QASM3 format (only for small circuits)
    if (num_qubits <= 10) {
        std::cout << "Circuit (QASM3):" << std::endl;
        std::cout << circ.to_qasm3() << std::endl;
    } else {
        std::cout << "(QASM3 output suppressed for circuits > 10 qubits)" << std::endl << std::endl;
    }

    // Connect to IBM Quantum Runtime
    auto service = QiskitRuntimeService();
    auto backend = service.backend(backend_name);

    // Transpile circuit for the target backend
    auto transpiled_circ = transpile(circ, backend);

    // Create sampler and run the circuit
    auto sampler = Sampler(backend, num_shots);
    auto job = sampler.run({SamplerPub(transpiled_circ)});

    if (job == nullptr) {
        std::cerr << "Error: Failed to submit job" << std::endl;
        return -1;
    }

    std::cout << "Job submitted. Waiting for results..." << std::endl;

    // Get results
    auto result = job->result();
    auto pub_result = result[0];
    auto meas_bits = pub_result.data("meas");
    auto counts = meas_bits.get_counts();

    // Print measurement results
    std::cout << std::endl << "Measurement Results:" << std::endl;
    std::cout << "-------------------" << std::endl;

    // Count results - for GHZ we expect mostly all-0s and all-1s
    int count_all_zeros = 0;
    int count_all_ones = 0;
    int count_other = 0;

    // Expected bitstrings for N-qubit GHZ
    std::string all_zeros(num_qubits, '0');
    std::string all_ones(num_qubits, '1');

    // Calculate expected hex values for all-ones
    // For num_qubits bits all set to 1, the hex value is (2^num_qubits - 1)
    std::stringstream hex_all_ones;
    hex_all_ones << "0x" << std::hex << ((1ULL << num_qubits) - 1);
    std::string hex_ones_str = hex_all_ones.str();

    for (const auto& c : counts) {
        if (c.first == all_zeros || c.first == "0x0" || c.first == "0") {
            count_all_zeros = c.second;
        } else if (c.first == all_ones || c.first == hex_ones_str) {
            count_all_ones = c.second;
        } else {
            count_other += c.second;
        }

        // Print top results (> 1%)
        double percentage = (100.0 * c.second) / num_shots;
        if (percentage > 1.0) {
            std::cout << "  |" << c.first << "⟩: " << c.second
                      << " (" << std::fixed << std::setprecision(1)
                      << percentage << "%)" << std::endl;
        }
    }

    std::cout << std::endl << "Summary:" << std::endl;
    std::cout << "  All 0s: " << count_all_zeros << " ("
              << std::fixed << std::setprecision(1)
              << (100.0 * count_all_zeros / num_shots) << "%)" << std::endl;
    std::cout << "  All 1s: " << count_all_ones << " ("
              << std::fixed << std::setprecision(1)
              << (100.0 * count_all_ones / num_shots) << "%)" << std::endl;
    std::cout << "  Other (noise): " << count_other << " ("
              << std::fixed << std::setprecision(1)
              << (100.0 * count_other / num_shots) << "%)" << std::endl;

    std::cout << std::endl;
    std::cout << "Expected: ~50% all-0s and ~50% all-1s" << std::endl;
    std::cout << "(Other results indicate decoherence/noise)" << std::endl;

    return 0;
}
