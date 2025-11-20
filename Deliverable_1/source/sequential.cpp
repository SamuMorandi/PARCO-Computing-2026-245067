#include <iostream>
#include <stdio.h>   
#include <stdlib.h>   
#include <cstring>
#include <vector>
#include <random>
#include <algorithm>
#include <ctime>

using namespace std;

struct Node {
    int row, col;
    double value;
};

int main(int argc, char* argv[]) {
    srand(time(NULL));

    struct timespec start, end;
    clock_t start2, end2;

    double execution_time_CPU, execution_time_REAL;
/*CHECK ON THE ARGUMENT (THE FILE OF THE SPARSE MATRIX)*/
    if(argc != 2){
        fprintf(stderr,"[ERR] Missing argument (or extra argument added) when executing the file\n");
        return 1;
    }
    char* filename = argv[1];
    size_t len = strlen(filename);
    size_t ext_len = 4; // Lunghezza di ".mtx"

    if (len <= ext_len || strcmp(filename + len - ext_len, ".mtx") != 0) {
        fprintf(stderr, "[ERR] Il file non ha l'estensione .mtx: %s\n", filename);
        return 1;
    }

/*OPENING THE FILE*/
    FILE* file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr,"[ERR] Error while opening the file\n");
        return 1;
    }

/*CATCH IF THE MATRIX IS SYMMETRIC (ONLY FIRST LINE). SKIPS THE COMMENTS AND READ THE FIRST LINE OF THE FILE, WHICH CONTAINS #ROWS, #COLUMNS, #NON ZERO VALUES*/
    char line[1024];
    fgets(line, sizeof(line), file);
    int is_symmetric = strstr(line, "symmetric") != NULL;

    do {
        if (!fgets(line, sizeof(line), file)) {
            fprintf(stderr,"[ERR] Empty file or error while reading\n");
            fclose(file);
            return 1;
        }
    } while (line[0] == '%');

    int rows_number, columns_number, nnz;
    sscanf(line, "%d %d %d", &rows_number, &columns_number, &nnz);
    //printf("INFORMATION FROM FILE!!\nSymmetric:%d\nRows: %d\nColumns: %d\nNon zero values: %d\n\n",is_symmetric,rows_number, columns_number, nnz);

    
/*CREATES A LIST OF NODES WITH ALL THE NON ZERO ELEMENTS OF THE MATRIX AND EVENTUALLY ADAPTS TO ITS SYMMETRY*/

    vector<Node> matrix;
    //since I alredy know the actual size of the vector, I reserve some memory to speed up the allocation.
    matrix.reserve(is_symmetric ? 2 * nnz : nnz);
    
    Node node;
    int tmp_r, tmp_c;
    for(int i=0; i<nnz; i++){
        if(fgets(line, sizeof(line), file) == NULL){
            fprintf(stderr, "[ERR] Something went wrong while reading the file (unexpected EOF at line %d)\n",i);
            fclose(file);
            return 1;
        }
        sscanf(line, "%d %d %lf", &node.row, &node.col, &node.value);
        node.row--;
        tmp_r=node.row;
        node.col--;
        tmp_c=node.col;
        matrix.push_back(node);       

        if(is_symmetric && tmp_r != tmp_c){
            node.row = tmp_c;
            node.col = tmp_r;
            matrix.push_back(node);
        }
    }

/*SORTS THE ELEMENTS BASED FIRST ON ROWS AND EVENTUALLY ON  COLUMNS*/
    sort(matrix.begin(), matrix.end(), [](const Node &a, const Node &b) {
        if (a.row != b.row) return a.row < b.row;
        return a.col < b.col;
    });

/*CREATION OF THE CSR REPRESENTATION*/
    vector<int> rows_ptr ((rows_number+1), 0);
    vector<int> cols;
    vector<double> values;

    if(is_symmetric){
        cols.reserve(2*nnz);
        values.reserve(2*nnz);
    }
    else{
        cols.reserve(nnz);
        values.reserve(nnz);
    }

    for (const Node &n : matrix) {
        cols.push_back(n.col);
        values.push_back(n.value);

        //Count of the values in each row
        rows_ptr[n.row + 1]++;
    }

    //using the offset in the array
    for(int r=0; r<rows_number; r++)
        rows_ptr[r+1] += rows_ptr[r];

/*CREATION OF A RANDOM ARRAY*/
    vector<double> random_array (rows_number);
    for(int i = 0; i < rows_number; i++) {
        random_array[i] = rand() % (9) + 1;
    }

/*MATRIX-ARRAY MULTIPLICATION*/
    vector<double> result (rows_number, 0);

    //from here starts the real computation of the CSR and this is why the time of execution starts here
    start2=clock();
    clock_gettime(CLOCK_MONOTONIC, &start);

    for(int r = 0; r < rows_number; r++){
        for(int idx = rows_ptr[r]; idx < rows_ptr[r+1]; idx++){
            result[r] += values[idx] * random_array[cols[idx]];
        }
    }

    //The execution finishes, this is why time stops here.
    clock_gettime(CLOCK_MONOTONIC, &end);
    end2=clock();

    //Print the resulting vector
    /*for(int r = 0; r < rows_number; r++)
        printf("result[%d] = %.9lf\n", r, result[r]);
    */

    execution_time_REAL = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    execution_time_CPU = static_cast<double>(end2 - start2)/CLOCKS_PER_SEC;
    printf("%s:%.6f:%.6f\n", argv[1],execution_time_CPU, execution_time_REAL);

    fclose(file);
    return 0;
}