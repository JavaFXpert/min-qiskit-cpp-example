/*
 * Minimal Qiskit C++ Example - Bell State with Runtime Sampler
 *
 * This program creates a simple Bell state circuit and runs it on
 * IBM Quantum hardware using the Qiskit C++ interface (Qiskit 2.2+).
 *
 * (C) Copyright IBM 2024.
 * Licensed under the Apache License, Version 2.0.
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>

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

int main(int argc, char* argv[]) {
    // Configuration
    std::string backend_name = "ibm_torino";  // Default backend
    int num_shots = 1024;

    // Parse command line arguments
    if (argc > 1) {
        backend_name = argv[1];
    }
    if (argc > 2) {
        num_shots = std::atoi(argv[2]);
    }

    std::cout << "Bell State Circuit Example" << std::endl;
    std::cout << "==========================" << std::endl;
    std::cout << "Backend: " << backend_name << std::endl;
    std::cout << "Shots: " << num_shots << std::endl << std::endl;

    // Create a 2-qubit Bell state circuit
    QuantumRegister qr(2);
    ClassicalRegister cr(2, std::string("meas"));
    QuantumCircuit circ(
        std::vector<QuantumRegister>({qr}),
        std::vector<ClassicalRegister>({cr})
    );

    // Build Bell state: |Φ+⟩ = (|00⟩ + |11⟩) / √2
    circ.h(0);        // Hadamard gate on qubit 0: |0⟩ → (|0⟩ + |1⟩) / √2
    circ.cx(0, 1);    // CNOT gate: control=0, target=1
    circ.measure(qr, cr);  // Measure both qubits

    // Print the circuit in QASM3 format
    std::cout << "Circuit (QASM3):" << std::endl;
    std::cout << circ.to_qasm3() << std::endl;

    // Connect to IBM Quantum Runtime
    // Credentials are read from $HOME/.qiskit/qiskit-ibm.json
    // or environment variables QISKIT_IBM_TOKEN and QISKIT_IBM_INSTANCE
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
    for (const auto& c : counts) {
        double percentage = (100.0 * c.second) / num_shots;
        std::cout << "  |" << c.first << "⟩: " << c.second
                  << " (" << std::fixed << std::setprecision(1)
                  << percentage << "%)" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Expected: ~50% |00⟩ and ~50% |11⟩ (Bell state entanglement)" << std::endl;

    return 0;
}
