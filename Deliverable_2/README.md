# Performance Analysis of Distributed SpMV using MPI: Scalability and Process Mapping

**Author:** Samuele Morandi  
**Student ID:** 245067  
**Email:** `samuele.morandi@studenti.unitn.it`  

---

## 1. Project Overview

This project presents a performance analysis of a distributed-memory parallelization for Sparse Matrix-Vector Multiplication (SpMV) using **MPI (Message Passing Interface)**.

The solver employs a **1D Cyclic Row-Partitioning** strategy on matrices stored in **Compressed Sparse Row (CSR)** format. The core of the analysis investigates the trade-off between memory bandwidth saturation and network latency by benchmarking two distinct process mapping strategies:
* **Dense Configuration:** Processes are packed into the minimum number of nodes.
* **Distributed Configuration:** Processes are spread across more nodes to maximize available memory bandwidth.

The study concludes that a **performance crossover** exists: while **Dense** configurations minimize latency at small scales (latency-bound), **Distributed** configurations prove superior at large scales (e.g., 128 processes) for bandwidth-bound kernels like SpMV, achieving speedups up to ~13x.

## 2. Repository Structure

The repository is organized as follows:
```
Deliverable_2/ 
├── Matrices/ # (Local only) Input matrices (.mtx format) 
├── plots/ # tables and images about the results
├── report/ # Project report (Morandi_245067_D2.pdf) 
├── results/ # Output files (.txt format) 
├── scripts/ # PBS scripts 
├── source/ # C++ source code 
├── support/ # C++ code that creates synthetic matrices
└── README.md # This file
```

### 2.1 Source files

The 'source' directory contains the implementation of the distributed solver:
* mpi_blocking.cpp: The core MPI implementation handling matrix parsing, distribution, and computation.

Unlike the OpenMP project, a single executable handles the logic; the behavior (Dense vs Distributed) is determined by the runtime environment configuration (PBS script parameters).

### 2.2 Scripts

The 'scripts' directory contains 24 PBS scripts, necessary to submit jobs to the cluster. 
A single PBS file presents both the kind of scaling it tests (strong or weak) and the actual configuration implemented.
Both pieces of information are expressed by the name of the file, which strictly follows this structure:
`spmv_<number_of_nodes>x<number_of_cores_per_node>_<scaling>.pbs`

## 3. Requirements

* **Compiler:** `mpic++` (wrapper for `g++`), provided by MPICH.
* **Cluster Environment:** PBS (Portable Batch System).
* **Modules:** `mpich-3.2.1--gcc-9.1.0`, `gcc91`.

## 4. Compilation

The C++ source code is automatically compiled using the MPI wrapper. The compilation flags recommended are:

```bash
mpic++ -std=c++11 -O3 -g "$SOURCE" -o "$EXECUTABLE"
```

The executable is then automatically launched by the PBS and removed when its execution is finished.

## 5. Execution and reproducibility
To get started, first clone the repository to your local machine or cluster environment:

```bash
git clone https://github.com/SamuMorandi/PARCO-Computing-2026-245067.git;
cd PARCO-Computing-2026-245067
```

### 5.0 Data Setup (Important)
Due to the large file size, the matrix datasets are not included in this repository. Before running the code, you must:

1. Create a directory named `Matrices` in the root of the project (`Deliverable_2`):

```bash
mkdir Matrices
```

2. **For strong scaling:** download the matrices listed in the **7. Dataset** section.

3. Place the downloaded `.mtx` files inside the `Matrices/` folder.

4. **For weak scaling:** on the cluster, compile and execute the matrix_generator.cpp code (contained in the support directory):

```bash
cd support;
g++ -std=c++11 -O3 matrix_generator.cpp -o generator;
./generator <num_proc>
```

`NOTE:` <num_proc> must be substituted with the number of processes that you are about to use (e.g. if you want to submit the spmv_4x16_weak.pbs file, you first need to run ./generator 64).

5. Move the generated matrices from support/ to the `Matrices/` directory.

### 5.1 Local Execution
It is possible to compile and run the code locally for testing.

1. **Compile:** Navigate to the `PARCO-Computing-2026-245067/Deliverable_2/` directory and compile the desired source file:

```bash
# First gcc91 must be installed (or loaded if you are on the cluster)
# To discover the installed version (if there is one)
gcc --version
# Then you should also check for mpic++
mpic++ --version
# It should show g++ and then the version
# Then change directory
cd source
# To compile the code simply do 
mpic++ -std=c++11 -O3 mpi_blocking.cpp -o exec
```

2.  **Run:** Run the executable

```bash
# NOTE: Do not overload your local machine. Ensure the number of processes matches your CPU cores (e.g., -n 4).

# To execute (for example) with 4 processes and test strong scaling for matrix nlpkkt240
mpiexec -n 4 ./exec ../Matrices/nlpkkt240.mtx
```

### 5.2 Cluster Execution
To reproduce the final benchmark results, use the provided PBS scripts.

1.  **Navigate to the scripts directory:**

```bash
cd Deliverable_2/scripts/
```
    
2.  **Submit a job:** Submit the desired PBS script to the cluster queue.

```bash
# Example: run the weak scaling with 2 nodes and 8 cores each.
qsub spmv_2x8_weak.pbs

# If you want to run them all, do this:
for f in *.pbs; do
    qsub "$f";
done
#if you want to run only weak (or strong) simply change the *.pbs with *weak.pbs
```

3.  **Check Results:** all the output files (and error files) will be insert into the `result/` folder. If everything worked, you shall see that all the .err files are empty while the .txt files contain the results.

## 6. How to change parameters
Parameters can be modified as follows:

1. **Number of Processes, nodes or memory:** The number of processes is defined inside the PBS script using the `#PBS -l select=...:ncpus=...:mpiprocs=...mem=...` directive. To change the total processes, or the configuration you must edit the PBS file or create a new one. Also the value of the variable PROC and (optionally) the output name must be edited. The amount of memory must be changed in the directive above.
`NOTE:`Changing the number of nodes and processes could result in changing the actual configuration (from Dense to Distributed)

2. **Matrices:** If another matrix has to be tested, it shall be insert into the `Matrices/` directory. At this point, if you are testing the strong scaling, you need to add (or change, depending on your need) its path to the set declared into the PBS file you want to execute. Else, if you are testing the weak scalability, you need to change the name of the matrix into the mpiexec directive.
```bash
# If testing strong scaling
set=(
    "../Matrices/new_matrix.mtx"
    ...
)

# If testing weak scaling
mpiexec -n "$PROC" ./"$EXECUTABLE" ../Matrices/new_matrix.mtx 
```

## 7. Dataset
The experiments use five matrices with diverse sparsity patterns from the **SuiteSparse Matrix Collection**:
    
| Matrix                                               | Rows      | NNZ         | Density (%) | Domain         |
| ------------------------------------------------------ | ---------- | ----------- | ----------- | ------------------ |
| [nlpkkt240](https://sparse.tamu.edu/Schenk/nlpkkt240)  | 27,993,600 | 760,648,352 | 9.71 × 10−7 | Optimization       |
| [ML_Geer](https://sparse.tamu.edu/Janna/ML_Geer)       | 1,504,002  | 110,686,677 | 4.89 × 10−5 | Machine learning   |
| [PFlow_742](https://sparse.tamu.edu/Janna/PFlow_742)   | 742,793    | 37,138,461  | 6.73 × 10−5 | Fluid dynamics     |
| [msdoor](https://sparse.tamu.edu/INPRO/msdoor)         | 415,863    | 19,173,163  | 1.11 × 10−4 | Structural problem |
| [bmwcra_1](https://sparse.tamu.edu/GHS_psdef/bmwcra_1) | 148,770    | 10,641,602  | 4.81 × 10−4 | Structural problem |

`Note:` Please download the matrices in "Matrix Market" (.mtx) format and ensure they are placed in the `Deliverable_2/Matrices/` directory.

## 8. I/O Formats
    
This section contains the format of both input and output data.

### 8.1 Input Format

The program expects ONLY ONE input file in the **Matrix Market (.mtx) format**. This is a standard text-based file format for sparse matrices. 

The code's parser reads the data (Coordinate format) and distributes it using a Buffered Strategy to convert it into local CSR structures on each MPI rank.


### 8.2 Output Format

The scripts generate two types of output files in the `results/` folder:

1.  **Standard Output (`results/*.txt`):**
    * Defined by the `#PBS -o` directive.
    * Contains the standard output (stdout) of the C++ executable. The format of the output file will depend on the type of scaling that is being tested. A strong scaling output file will contain 5 set of measurements (one for each matrix) divided by an empty line. Each set is composed by the repetition of 3 lines:
        * Line 1: Rank <> | <matrix_path>
        * Line 2: LocalNNZ: <> s | LocalPerf: <> GFLOPS
        * Line 3: <time_1> <time_2> ... <time_10> 

*(Note: The unit 's' printed after `LocalNNZ` is a known typo in the logging format; the value represents the raw count of non-zeros, not seconds.)*

```bash
# In spmv_1x16_strong.txt
    
Rank 0 | ../Matrices/bmwcra_1.mtx
LocalNNZ: 666452.000000 s | LocalPerf: 0.902905 GFLOPS
0.001858473 0.001502752 0.001444578 0.001453638 0.001439810 0.001410961 0.001428843 0.001447916 0.001417160 0.001358271 
Rank 1 | ../Matrices/bmwcra_1.mtx
LocalNNZ: 667346.000000 s | LocalPerf: 0.856621 GFLOPS
0.001888037 0.001577377 0.001525640 0.001552582 0.001521826 0.001490831 0.001512766 0.001534462 0.001501560 0.001475811 
Rank 2 | ../Matrices/bmwcra_1.mtx
...
```
    
2.  **Standard Error (`results/*.err`):**
    * Defined by the `#PBS -e` directive.
    * Contains any error messages (stderr) generated by the executable.
    * At the end of the execution the error file should be empty (and can be deleted).

