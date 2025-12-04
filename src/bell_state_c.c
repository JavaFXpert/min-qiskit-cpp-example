/*
 * Minimal Qiskit C Example - Bell State with Runtime Sampler
 *
 * This program creates a simple Bell state circuit and runs it on
 * IBM Quantum hardware using the Qiskit C API (Qiskit 2.2+).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include <qiskit.h>
#include <qiskit_ibm_runtime/qiskit_ibm_runtime.h>

int main(int argc, char *argv[]) {
    const char *backend_name = "ibm_fez";  // Default backend
    int32_t num_shots = 1024;

    // Parse command line arguments
    if (argc > 1) {
        backend_name = argv[1];
    }
    if (argc > 2) {
        num_shots = atoi(argv[2]);
    }

    printf("Bell State Circuit Example (C API)\n");
    printf("===================================\n");
    printf("Backend: %s\n", backend_name);
    printf("Shots: %d\n\n", num_shots);

    // Create a 2-qubit, 2-classical-bit circuit
    QkCircuit *qc = qk_circuit_new(2, 2);

    // Build Bell state: |Φ+⟩ = (|00⟩ + |11⟩) / √2
    // Hadamard on qubit 0
    uint32_t h_qargs[1] = {0};
    qk_circuit_gate(qc, QkGate_H, h_qargs, NULL);

    // CNOT: control=0, target=1
    uint32_t cx_qargs[2] = {0, 1};
    qk_circuit_gate(qc, QkGate_CX, cx_qargs, NULL);

    // Measure both qubits
    qk_circuit_measure(qc, 0, 0);
    qk_circuit_measure(qc, 1, 1);

    printf("Circuit created: H(0), CX(0,1), Measure\n\n");

    // Connect to IBM Quantum Runtime
    int res = 0;
    Service *service;
    res = qkrt_service_new(&service);
    if (res != 0) {
        printf("ERROR: Failed to create service (code: %d)\n", res);
        goto cleanup_circuit;
    }

    // Search for available backends
    BackendSearchResults *results;
    res = qkrt_backend_search(&results, service);
    if (res != 0) {
        printf("ERROR: Backend search failed (code: %d)\n", res);
        goto cleanup_service;
    }

    uint64_t result_count = qkrt_backend_search_results_length(results);
    Backend **backends = qkrt_backend_search_results_data(results);

    printf("Available backends:\n");
    for (uint64_t i = 0; i < result_count; i++) {
        printf("  - %s\n", qkrt_backend_name(backends[i]));
    }
    printf("\n");

    // Find the requested backend
    Backend *selected_backend = NULL;
    for (uint64_t i = 0; i < result_count; i++) {
        if (strcmp(qkrt_backend_name(backends[i]), backend_name) == 0) {
            selected_backend = backends[i];
            break;
        }
    }

    if (selected_backend == NULL) {
        printf("ERROR: Backend '%s' not found\n", backend_name);
        goto cleanup_search;
    }

    printf("Using backend: %s\n", qkrt_backend_name(selected_backend));

    // Get backend target for transpilation
    QkTarget *target = qkrt_get_backend_target(service, selected_backend);
    if (target == NULL) {
        printf("ERROR: Failed to get backend target\n");
        goto cleanup_search;
    }

    // Transpile circuit for backend
    QkTranspileResult transpile_result = {NULL, NULL};
    char *error = NULL;
    QkTranspileOptions options = qk_transpiler_default_options();
    options.seed = 42;

    int transpile_code = qk_transpile(qc, target, &options, &transpile_result, &error);
    if (transpile_code != 0) {
        printf("ERROR: Transpilation failed: %s\n", error ? error : "unknown error");
        goto cleanup_target;
    }

    printf("Circuit transpiled successfully\n");

    // Submit job
    Job *job;
    res = qkrt_sampler_job_run(&job, service, selected_backend, transpile_result.circuit, num_shots, NULL);
    if (res != 0) {
        printf("ERROR: Job submission failed (code: %d)\n", res);
        goto cleanup_transpile;
    }

    printf("Job submitted! Waiting for results...\n");

    // Poll for job completion
    uint32_t status;
    do {
        printf("  Polling (waiting 10 seconds)...\n");
        sleep(10);
        res = qkrt_job_status(&status, service, job);
        if (res != 0) {
            printf("ERROR: Status poll failed (code: %d)\n", res);
            goto cleanup_job;
        }
        printf("  Status: %d\n", status);
    } while (status == 0 || status == 1);  // 0=queued, 1=running

    printf("\nJob completed with status: %d\n", status);

    // Get results
    Samples *samples;
    res = qkrt_job_results(&samples, service, job);
    if (res != 0) {
        printf("ERROR: Failed to get results (code: %d)\n", res);
        goto cleanup_job;
    }

    printf("\nMeasurement Results:\n");
    printf("-------------------\n");
    size_t num_samples = qkrt_samples_num_samples(samples);
    printf("Total shots: %zu\n\n", num_samples);

    // Count occurrences of each measurement outcome
    // For 2 qubits: 00=0x0, 01=0x1, 10=0x2, 11=0x3
    int counts[4] = {0, 0, 0, 0};

    for (size_t i = 0; i < num_samples; i++) {
        char *sample = qkrt_samples_get_sample(samples, i);
        if (sample != NULL) {
            // Parse hex value (format: "0x0", "0x1", "0x2", "0x3")
            int value = (int)strtol(sample, NULL, 0);
            if (value >= 0 && value < 4) {
                counts[value]++;
            }
            qkrt_str_free(sample);
        }
    }

    // Display counts
    printf("  |00⟩: %d (%.1f%%)\n", counts[0], 100.0 * counts[0] / num_samples);
    printf("  |01⟩: %d (%.1f%%)\n", counts[1], 100.0 * counts[1] / num_samples);
    printf("  |10⟩: %d (%.1f%%)\n", counts[2], 100.0 * counts[2] / num_samples);
    printf("  |11⟩: %d (%.1f%%)\n", counts[3], 100.0 * counts[3] / num_samples);

    printf("\nExpected: ~50%% |00⟩ and ~50%% |11⟩ (Bell state entanglement)\n");
    printf("(|01⟩ and |10⟩ indicate noise/errors)\n");

    qkrt_samples_free(samples);

cleanup_job:
    qkrt_job_free(job);
cleanup_transpile:
    if (transpile_result.circuit) qk_circuit_free(transpile_result.circuit);
    if (transpile_result.layout) qk_transpile_layout_free(transpile_result.layout);
cleanup_target:
    qk_target_free(target);
cleanup_search:
    qkrt_backend_search_results_free(results);
cleanup_service:
    qkrt_service_free(service);
cleanup_circuit:
    qk_circuit_free(qc);

    return res;
}
