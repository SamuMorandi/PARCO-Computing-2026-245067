# Parallel Computing (PARCO) Coursework

**Student:** Samuele Morandi  
**ID:** 245067  
**University of Trento** **Academic Year:** 2025/2026  

---

## Repository Overview

This repository contains the source code, scripts, and reports for the practical assignments ("provette") of the Parallel Computing course. The work is divided into two main deliverables:

### [Deliverable 1: OpenMP SpMV Analysis](./Deliverable_1)
**Topic:** Analysis of OpenMP Scheduling Strategies for Sparse Matrix-Vector Multiplication (CSR format).

* **Goal:** Investigate the trade-off between scheduling overhead and load imbalance using `static`, `dynamic`, and `guided` strategies.
* **Key Results:** Achieved **8.38x speedup** on irregular matrices (nlpkkt240) using optimized dynamic scheduling compared to the sequential version.
* **Technologies:** C++, OpenMP, GCC, PBS.
* 👉 **[Go to Deliverable 1 Documentation](./Deliverable_1/README.md)**

### Deliverable 2: MPI SpMV Analysis
*(Coming soon / Work in Progress)*

* **Topic:** [TBD - e.g., MPI Implementation / CUDA / Hybrid Programming]
* **Status:** Pending
* 👉 **[Go to Deliverable 2 Folder](./Deliverable_2)** *(Link will be active once uploaded)*

---

## Directory Structure

```text
PARCO-Computing-2026-245067/
├── Deliverable_1/       # Project 1: OpenMP SpMV
│
├── Deliverable_2/       # Project 2: MPI SpMV
│
└── README.md            # This file (General Overview)
