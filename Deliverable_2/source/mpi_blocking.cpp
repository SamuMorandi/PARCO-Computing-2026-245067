#include <mpi.h>
#include <iostream>
#include <stdio.h>   
#include <stdlib.h>   
#include <cstring>
#include <vector>
#include <random>
#include <algorithm>
#include <ctime>

#define BUFFER_SIZE 50000  //necessary for NOT exceeding the memory size
#define NUM_ITERATIONS 10

using namespace std;

struct Node {
    int row, col;
    double value;
};

//FUNCTION USED BY RANK_0 FOR SENDING VALUES TO DIFFERENT PROCESSES
void flush_buffer(int dest, vector<vector<int>>& rows, vector<vector<int>>& cols, vector<vector<double>>& vals) {
    int count = rows[dest].size();
    if (count > 0) {
        MPI_Send(&count, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
        MPI_Send(rows[dest].data(), count, MPI_INT, dest, 1, MPI_COMM_WORLD);
        MPI_Send(cols[dest].data(), count, MPI_INT, dest, 2, MPI_COMM_WORLD);
        MPI_Send(vals[dest].data(), count, MPI_DOUBLE, dest, 3, MPI_COMM_WORLD);
        
        rows[dest].clear();
        cols[dest].clear();
        vals[dest].clear();
    }
}

int main (int argc, char *argv[]){
    MPI_Init(&argc,&argv);

    int my_rank, num_proc;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    int is_symmetric=0;        
    int rows_number=0;
    int columns_number=0;
    int nnz=0;

    vector<double> values;
    vector<int> rows;
    vector<int> cols;

    FILE* file = NULL;
    char line[1024];

    double start, end, max_exec_time;
    double flops;

/*RANK_0 CHECKS PARAMETERS, READS THE HEADER OF THE FILE AND SHARES BASIC INFORMATION ABOUT THE MATRIX WITH OTHER PROCESSES */
    if(my_rank == 0){
        srand(time(NULL));

        /*CHECK ON THE ARGUMENT (THE FILE OF THE SPARSE MATRIX)*/
        if(argc != 2){
            fprintf(stderr,"[ERR] Missing argument (or extra argument added) when executing the file\n");
            MPI_Abort(MPI_COMM_WORLD,1);
        }

        char* filename = argv[1];
        size_t len = strlen(filename);
        size_t ext_len = 4; // Lunghezza di ".mtx"
        if (len <= ext_len || strcmp(filename + len - ext_len, ".mtx") != 0) {
            fprintf(stderr, "[ERR] File foesn't have .mtx extension: %s\n", filename);
            MPI_Abort(MPI_COMM_WORLD,1);
        }

        /*OPENING THE FILE*/
        file = fopen(argv[1], "r");
        if (!file) {
            fprintf(stderr,"[ERR] Error while opening the file\n");
            MPI_Abort(MPI_COMM_WORLD,1);
        }

        /*CATCH IF THE MATRIX IS SYMMETRIC (ONLY FIRST LINE). SKIPS THE COMMENTS AND READ THE FIRST LINE OF THE FILE, WHICH CONTAINS #ROWS, #COLUMNS, #NON ZERO VALUES*/
        fgets(line, sizeof(line), file);
        is_symmetric = strstr(line, "symmetric") != NULL;

        do {
            if (!fgets(line, sizeof(line), file)) {
                fprintf(stderr,"[ERR] Empty file or error while reading\n");
                fclose(file);
                MPI_Abort(MPI_COMM_WORLD,1);
            }
        } while (line[0] == '%');

        sscanf(line, "%d %d %d", &rows_number, &columns_number, &nnz);
    }

    /*RANK_0 SHARES MAIN INFORMATION OF THE MATRIX TO OTHER PROCESSES*/
    MPI_Bcast(&rows_number, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&columns_number, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&nnz,  1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&is_symmetric, 1, MPI_INT, 0, MPI_COMM_WORLD);

/*RANK_0 READS THE WHOLE FILE AND SENDS TO OTHER PROCESSES THE ROWS THAT BELONG TO THEM. SEND OPERATION ARE PERFORMED EVERY 50000 ELEMENTS*/
    if (my_rank == 0){
        vector<vector<double>> buffer_values(num_proc);
        vector<vector<int>> buffer_rows(num_proc);
        vector<vector<int>> buffer_cols(num_proc);

        int tmp_row, tmp_col, dest;
        double tmp_val;

        /*RANK_0 READS EACH LINE OF THE FILE AND INSERTS VALUES INTO THE BUFFER OF THE AIMED PROCESS (dest)*/
        for(int i=0; i<nnz; i++){
            if(fgets(line, sizeof(line), file) == NULL){
                fprintf(stderr, "[ERR] Something went wrong while reading the file (unexpected EOF at line %d)\n",i);
                fclose(file);
                return 1;
            }
            sscanf(line, "%d %d %lf", &tmp_row, &tmp_col, &tmp_val);
            tmp_row--;
            tmp_col--;

            dest = tmp_row % num_proc;//THE DESTINATION CAN RANGE FROM 0 (THIS PROCESS) TO N-1

            if (dest == 0){//ALREADY THERE, NO NEED FOR BUFFER
                values.push_back(tmp_val);
                rows.push_back(tmp_row);
                cols.push_back(tmp_col);
            }
            else{//IF DESTINATION != RANK_0, ROW, COLUMN AND VALUE ARE INSERT INTO THE BUFFER OF THE DESTINATION
                buffer_rows[dest].push_back(tmp_row);
                buffer_cols[dest].push_back(tmp_col);
                buffer_values[dest].push_back(tmp_val);

                if (buffer_rows[dest].size() >= BUFFER_SIZE) {//WHEN THE DESTINATION BUFFER REACHES BUFFER_SIZE, THE FUNCTION IS CALLED AND THE BUFFER GETS FREED
                        flush_buffer(dest, buffer_rows, buffer_cols, buffer_values);
                    }
            }

            if(is_symmetric && tmp_row != tmp_col){//IF THE MATRIX IS SYMMETRIC, ANOTHER ELEMENT HAS TO BE INSERT BUT IT WILL BELONG TO A DIFFERENT PROCESS
                dest = tmp_col % num_proc;

                if(dest == 0){
                    values.push_back(tmp_val);
                    rows.push_back(tmp_col);
                    cols.push_back(tmp_row);
                }
                else{
                    buffer_rows[dest].push_back(tmp_col);
                    buffer_cols[dest].push_back(tmp_row);
                    buffer_values[dest].push_back(tmp_val);

                    if (buffer_rows[dest].size() >= BUFFER_SIZE) {
                        flush_buffer(dest, buffer_rows, buffer_cols, buffer_values);
                    }
                }
            }
        }

        fclose(file);

        /*WHEN REACHING EOF, THE FUNCTION IS CALLED: ELEMENTS ARE SENT TO THE PROCESSES AND BUFFERS OF ALL PROCESSES ARE RELEASED*/
        for(int p=1; p<num_proc; p++){
            flush_buffer(p, buffer_rows, buffer_cols, buffer_values);
            
            /*RECEIVING VALUE 0 WILL TERMINATE A while(1) CYCLE INTO THE RECEIVING PROCESSES*/
            int stop = 0;
            MPI_Send(&stop, 1, MPI_INT, p, 0, MPI_COMM_WORLD);
        }
    }

/*OTHER PROCESSES RECEIVE ROWS SENT BY RANK_0. THE RECEIVING CYCLE IS TERMINATED BY A 0 VALUE*/
    else{
        while(1) {
            int count;
            /*FIRST RECEIVES HOW MANY ELEMENTS WILL ARRIVE*/
            MPI_Recv(&count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            /*IF ZERO ELEMENTS, STOP WAITING SINCE ALL THE ELEMENTS HAVE BEEN RECEIVED*/
            if (count == 0) break;

            /*IN ORDER TO AVOID HAVING TEMPORARY BUFFERS FOR THE ARRIVING ELEMENTS, ALL OF THEM ARE DIRECTLY INSERT INTO FINAL VECTORS*/
            size_t start_idx = rows.size();//NEED TO RESIZE THE VECTOR AND "MAKING SPACE" FOR NEW ARRIVALS
            rows.resize(start_idx + count);
            cols.resize(start_idx + count);
            values.resize(start_idx + count);

            /*ARRIVING ELEMENTS ARE INSERT INTO THE POSITION AFTER THE LAST ELEMENT rows.data() + start_idx*/
            MPI_Recv(rows.data() + start_idx, count, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(cols.data() + start_idx, count, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(values.data() + start_idx, count, MPI_DOUBLE, 0, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }

/*ELEMENTS OF EACH PROCESS ARE SORTED AND REPRESENTED IN CSR FORMAT (COMMON TO ALL PROCESSES)*/
    /*WAIT FOR EVERY PROCESS TO FINISH I/O OPERATION*/
    MPI_Barrier(MPI_COMM_WORLD);

    /*TO SIMPLIFY THE SORTING OPERATION (AND THE CSR REPRESENTATION), NEED TO CREATE A VECTOR OF NODES*/
    size_t n_elements = rows.size();
    vector<Node> elements(n_elements);

    for(size_t i = 0; i < n_elements; i++) {
        elements[i].row = rows[i];
        elements[i].col = cols[i];
        elements[i].value = values[i];
    }
    /*RELEASE MEMORY (AVOIDS OUT-OF-MEMORY SITUATIONS)*/
    rows.clear(); rows.shrink_to_fit();
    cols.clear(); cols.shrink_to_fit();
    values.clear(); values.shrink_to_fit();

    /*SORTING*/
    sort(elements.begin(), elements.end(), [](const Node &a, const Node &b) {
        if (a.row != b.row) return a.row < b.row;
        return a.col < b.col;
    });

    int max_local_rows = (rows_number / num_proc) + 1;
    vector<double> csr_values;
    vector<int> csr_col_ind;
    vector<int> csr_row_ptr(max_local_rows + 1, 0); // COMPLETELY INITIALIZED TO 0 (max_local_rows IS JUST AN EXTIMATION OF THE SPACE)

    /*CREATION OF CSR REPRESENTATION*/
    for (const Node &n : elements) {
        csr_values.push_back(n.value);
        csr_col_ind.push_back(n.col);

        /*MAPPING GLOBAL ROW INTO LOCAL ROW*/ 
        // Ex: Proc 0 manages rows 0, 4, 8,... -> become local 0, 1, 2,...
        int local_row = n.row / num_proc;

        csr_row_ptr[local_row + 1]++;
    }
    
    /*RELEASE MEMORY*/
    elements.clear(); elements.shrink_to_fit();

    //using the offset in the array
    for(int r = 0; r < max_local_rows; r++) {
        csr_row_ptr[r+1] += csr_row_ptr[r];
    }

/*DENSE ARRAY HAS TO BE CREATED AND MANAGED BY ALL PROCESSES*/ 
    /*EACH PROCESS CREATES ITS PART OF THE VECTOR*/
    int local_array_size = columns_number / num_proc; 
    if (my_rank < (columns_number % num_proc)) local_array_size++;

    vector<double> local_array(local_array_size);//DENSE VECTOR FOR THE SpMV
    vector<double> local_result(max_local_rows, 0.0);

    for(int i=0; i<local_array_size; i++) {
        local_array[i] = rand() % 9+1;
    }
    
    /*EVERY PROCESS COMMUNICATES HOW MANY ELEMENTS HAS TO SEND TO OTHERS*/
    vector<int> recv_counts(num_proc);//contains the number of elements of other processes
    vector<int> displs(num_proc);//contains the offset

    MPI_Allgather(&local_array_size, 1, MPI_INT, recv_counts.data(), 1, MPI_INT, MPI_COMM_WORLD);

    displs[0] = 0;
    for(int i = 1; i < num_proc; i++) {
        displs[i] = displs[i-1] + recv_counts[i-1];
    }

    vector<double> global_array(columns_number);//CONTAINS THE GLOBAL VECTOR

    vector<double> my_times;
    my_times.reserve(NUM_ITERATIONS);

    for(int iter = 0; iter < NUM_ITERATIONS; iter++) {
        
        MPI_Barrier(MPI_COMM_WORLD);

        start = MPI_Wtime();

        MPI_Allgatherv(local_array.data(), local_array_size, MPI_DOUBLE, global_array.data(), recv_counts.data(), displs.data(), MPI_DOUBLE, MPI_COMM_WORLD);

        for(int i = 0; i < max_local_rows; i++) {
            double dot_product = 0.0;
            int start_idx = csr_row_ptr[i];
            int end_idx   = csr_row_ptr[i+1];

            for(int k = start_idx; k < end_idx; k++) {
                dot_product += csr_values[k] * global_array[csr_col_ind[k]];
            }
            local_result[i] = dot_product; 
        }

        end = MPI_Wtime();
        my_times.push_back(end - start);
    }

    double total_time=0.0;
    for(double t : my_times)
        total_time+=t;
    double avg_time = total_time / NUM_ITERATIONS;

    double local_gflops = (2.0 * csr_values.size()) / (avg_time * 1e9);

    vector<double> all_times_buffer;
    vector<double> all_nnz_values;
    vector<double> all_gflops;

    if (my_rank == 0) {
        all_times_buffer.resize(num_proc * NUM_ITERATIONS);
        all_nnz_values.resize(num_proc);
        all_gflops.resize(num_proc);
    }

    double local_nnz = csr_values.size();

    MPI_Gather(my_times.data(), NUM_ITERATIONS, MPI_DOUBLE, all_times_buffer.data(), NUM_ITERATIONS, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gather(&local_nnz, 1, MPI_DOUBLE, all_nnz_values.data(), 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gather(&local_gflops, 1, MPI_DOUBLE, all_gflops.data(), 1, MPI_DOUBLE, 0, MPI_COMM_WORLD); // Corretto

    if (my_rank == 0) {
        for (int p = 0; p < num_proc; p++) {
            printf("Rank %d | %s\nLocalNNZ: %f | LocalPerf: %f GFLOPS\n", p, argv[1], all_nnz_values[p], all_gflops[p]);
            
            for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
                int index = p * NUM_ITERATIONS + iter;
                printf("%.9f ", all_times_buffer[index]);
            }
            printf("\n");
        }
        printf("\n\n");
    }

    MPI_Finalize();
    return 0;
}