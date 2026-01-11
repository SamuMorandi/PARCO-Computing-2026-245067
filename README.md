# Parallel Computing (PARCO) Coursework

**Author:** Samuele Morandi  
**Student ID:** 245067  
**Email:** `samuele.morandi@studenti.unitn.it`  

---

## Repository Overview

This repository contains the source code, scripts, and reports for the practical assignments ("provette") of the Parallel Computing course. The work is divided into two main deliverables:

### [Deliverable 1: OpenMP SpMV Analysis](./Deliverable_1)
**Topic:** Analysis of OpenMP Scheduling Strategies for Sparse Matrix-Vector Multiplication (CSR format).

* **Goal:** Investigate the trade-off between scheduling overhead and load imbalance using `static`, `dynamic`, and `guided` strategies.
* **Technologies:** C++, OpenMP, GCC, PBS.
* **[Go to Deliverable 1 Documentation](./Deliverable_1/README.md)**

### [Deliverable 2: MPI SpMV Analysis](./Deliverable_2)
**Topic:** Performance Analysis of Distributed SpMV using MPI: Scalability and Process Mapping.

* **Goal:** Investigate the trade-off between memory bandwidth saturation and network latency by benchmarking **Dense** (packed processes) vs **Distributed** (spread processes) configurations.
* **Key Findings:** The analysis highlights a performance crossover where Distributed configurations outperform Dense ones at large scales (up to ~13x speedup) for bandwidth-bound kernels.
* **Technologies:** C++, MPI (MPICH), PBS.
* **[Go to Deliverable 2 Documentation](./Deliverable_2/README.md)**

---

## Directory Structure

```text
PARCO-Computing-2026-245067/
├── Deliverable_1/          # Project 1: OpenMP SpMV
│   ├── ...
│   └── README.md
│
├── Deliverable_2/          # Project 2: MPI SpMV
│   ├── ...
│   └── README.md           # Detailed instructions for D2
│
└── README.md            # This file (General Overview)
