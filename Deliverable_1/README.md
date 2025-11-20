# Analysis of OpenMP Scheduling Strategies for SpMV

**Author:** Samuele Morandi  
**Student ID:** 245067  
**Email:** `samuele.morandi@studenti.unitn.it`  

---

## 1. Project Overview

This project presents a performance analysis of OpenMP-based parallelization for Sparse Matrix-Vector Multiplication (SpMV) using the **Compressed Sparse Row (CSR)** format.

The core of the analysis investigates the trade-off between scheduling overhead and load imbalance by benchmarking three OpenMP scheduling clauses:
* `schedule(static)`
* `schedule(dynamic)`
* `schedule(guided)`

The study concludes that while the low-overhead `static` scheduler provides robust performance for regular matrices, **`dynamic` scheduling with chunk size non-default** (chunk=100) proves superior for large, irregular matrices (e.g., `nlpkkt240`), achieving a speedup of 8.38x vs 6.5x of static.

## 2. Repository Structure

The repository is organized as follows:

```
Deliverable_1/
├── Matrices/         # (Local only) Input matrices (.mtx format)
├── source/           # C++ source code (.cpp files)
├── scripts/          # PBS scripts for cluster execution (.pbs files)
├── results/          # Output files (stdout, stderr, perf logs)
│   └── perf_results/
├── report/           # Final project report PDF (Morandi_245067_D1.pdf)
├── plots/            # Table with the times for each matrix (.xlsx and .csv) and tables of speedup
└── README.md         # This file
```

### 2.1 Source files

The 'source' directory contains 6 files:
* sequential.cpp
* scheduleStatic.cpp
* scheduleDynamic.cpp
* scheduleDynamic_100.cpp
* scheduleGuided.cpp
* scheduleGuided_100.cpp

They all differ in the type of scheduling (sequential, static, dynamic or guided) and on the value of chunk_size (default or 100).

### 2.2 Scripts

The 'scripts' directory contains 26 .pbs files:

* script for the sequential code.
* 25 scripts for the parallel code, covering all 5 parallel source files (static, dynamic, dynamic_100, etc.), each tested with 5 different thread counts.

PBS files differ for the number of threads used during the execution.

## 3. Requirements

* **Compiler:** `gcc` (v9.1 or later) package, specifically using the `g++` command for C++ compilation.
* **Cluster Environment:** PBS (Portable Batch System)
* **Modules:** `gcc91`, `perf`

## 4. Compilation

The C++ source code is compiled (and then executed) automatically by the PBS scripts. The compilation flags used are:

```bash
g++ -std=c++11 -O3 -march=native -fopenmp $SOURCE -o $EXECUTABLE
```

## 5. Execution and Reproducibility

To get started, first clone the repository to your local machine or cluster environment:

```bash
git clone https://github.com/SamuMorandi/PARCO-Computing-2026-245067.git
cd PARCO-Computing-2026-245067
```
### 5.0 Data Setup (Important)
Due to the large file size, the matrix datasets are **not included** in this repository.
Before running the code, you must:

1.  Create a directory named `Matrices` in the root of the project (Deliverable_1):
    ```bash
    mkdir Matrices
    ```
2.  Download the matrices listed in the **7. Dataset** section.
3.  Place the downloaded `.mtx` files inside the `Matrices/` folder.

### 5.1 Local Execution

You can compile and run the code locally for development or testing.

1.  **Compile:**
    Navigate to the `PARCO-Computing-2026-245067` directory and compile the desired source file:
    ```bash
    # First gcc91 must be installed (or loaded if you are on the cluster)
    # To discover the installed version (if there is one)
    gcc --version
    
    # If the version is 9.1.0 or later, compile (example for static schedule)
    g++ -std=c++11 -O3 -march=native -fopenmp source/scheduleStatic.cpp -o static.out
    ```

2.  **Run:**
    Set the number of threads via the environment variable and provide the path to the matrix as an argument:
    ```bash
    # Set OpenMP threads
    export OMP_NUM_THREADS=4
    
    # Run the executable, adding the path of the desired matrix (located in the Matrices directory)
    ./static.out Matrices/bmwcra_1.mtx
    ```

### 5.2 Cluster Execution

To reproduce the final benchmark results, use the provided PBS scripts.

1.  **Navigate to the scripts directory:**
    ```bash
    cd scripts/
    ```

2.  **Submit a job:**
    Submit the desired PBS script to the cluster queue.
    ```bash
    # Example: run the 'static' schedule test with 4 threads
    qsub staticSchedule_4.pbs
    
    # Example: run the 'dynamic' schedule (chunk 100) with 8 threads
    qsub dynamicSchedule_8_100.pbs
    ```

3.  **Check Results:**
    All output (standard out, standard error, and `perf stat` logs) will be saved in the `results/` and `results/perf_results/` directories.

## 6. How to Change Parameters

Parameters can be modified as follows:

* **Number of Threads/CPUs:** The thread count is defined by which PBS script you run. Each script is hardcoded with the number of threads (e.g., `#PBS -l select=1:ncpus=4` and `THREADS=4` in the script body). To run with 8 threads, you must execute the corresponding `..._8.pbs` script. If you want to use a different number of threads for a determined source file while you are on the cluster, it is necessary to change the value of the variable THREADS inside the PBS file.
* **Scheduling Strategy & Chunk Size:** These parameters are strictly linked by the source file that the PBS script compiles (e.g., `SOURCE="../source/scheduleDynamic_100.cpp"`). For this reason, changing the scheduling clause means compiling a different source file while changing the chunk size means modifying the second parameter of the scheduling clause.
* **Matrices:** The set of matrices to be tested is defined inside each PBS script in the `set=(...)` array. Different matrices (in .mtx format) can be added to the `Matrices/` directory and then added to the `set=(...)` array inside the PBS scripts to be included in the tests.

## 7. Dataset

The experiments use five matrices with diverse sparsity patterns from the **SuiteSparse Matrix Collection**:

| Matrice                                                | Righe      | NNZ         | Densità (%) | Dominio            |
| ------------------------------------------------------ | ---------- | ----------- | ----------- | ------------------ |
| [nlpkkt240](https://sparse.tamu.edu/Schenk/nlpkkt240)  | 27,993,600 | 760,648,352 | 9.71 × 10−7 | Optimization       |
| [ML_Geer](https://sparse.tamu.edu/Janna/ML_Geer)       | 1,504,002  | 110,686,677 | 4.89 × 10−5 | Machine learning   |
| [PFlow_742](https://sparse.tamu.edu/Janna/PFlow_742)   | 742,793    | 37,138,461  | 6.73 × 10−5 | Fluid dynamics     |
| [msdoor](https://sparse.tamu.edu/INPRO/msdoor)         | 415,863    | 19,173,163  | 1.11 × 10−4 | Structural problem |
| [bmwcra_1](https://sparse.tamu.edu/GHS_psdef/bmwcra_1) | 148,770    | 10,641,602  | 4.81 × 10−4 | Structural problem |

Note: Please download the matrices in "Matrix Market" format and ensure they are placed in the Deliverable_1/Matrices/ directory.

## 8. I/O Formats

This section contains the format of both input and output data.

### 8.1 Input Format

The program expects ONLY ONE input file in the **Matrix Market (.mtx) format**. This is a standard text-based file format for sparse matrices.

The code's parser reads the **Coordinate (COO)** data from the `.mtx` file and converts it internally to the **Compressed Sparse Row (CSR)** format before performing the multiplication.

### 8.2 Output Format

The scripts generate three types of output files in the `results/` folder:

1.  **Standard Output (`results/*.txt`):**
    * Defined by the `#PBS -o` directive.
    * Contains the standard output (stdout) of the C++ executable. Each file contains 10 comment lines indicating the testing session, each followed by 5 output lines indicating the matrix used as parameter, the CPU time and the real time. Each data is separated by the symbol ":" in order to create a smaller, but still easy to read, output file.
    ```bash
    # In dynamicSchedule_4.txt
    
    #Testing session 1
    ../Matrices/bmwcra_1.mtx:0.140000:0.036094
    ../Matrices/ML_Geer.mtx:1.460000:0.365342
    ../Matrices/msdoor.mtx:0.770000:0.219520
    ../Matrices/nlpkkt240.mtx:21.260000:6.002818
    ../Matrices/PFlow_742.mtx:0.650000:0.161398
    #Testing session 2
    ...
    ```
    
2.  **Standard Error (`results/*.err`):**
    * Defined by the `#PBS -e` directive.
    * Contains any error messages (stderr) generated by the executable.
    * At the end of the execution the error file should be empty (and can be deleted).

3.  **Perf Stat Logs (`results/perf_results/perf-*.txt`):**
    * Defined by the `PERF_OUTPUT_FILE` variable.
    * Contains the hardware counter analysis from the `perf stat` command.
    * **Note:** The output of `perf stat` (which writes to stderr) is appended (`2>>`) to this file for each of the 10 testing sessions, resulting in a single log file per configuration. For this reason, errors caused by perf are also added to this file.
