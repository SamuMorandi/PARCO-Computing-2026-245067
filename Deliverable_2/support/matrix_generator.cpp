#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <string>
#include <cstdlib>

using namespace std;

const int BASE_ROWS = 5000; 
const int BASE_COLS = 5000; 
const int NNZ_PER_ROW = 40; 

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Using: " << argv[0] << " <number of processes>" << endl;
        return 1;
    }

    int procs = atoi(argv[1]);
    if (procs < 1) 
        procs = 1;

    long long total_rows = (long long)BASE_ROWS * procs;
    long long total_cols = (long long)BASE_COLS * procs;
    long long total_nnz = total_rows * NNZ_PER_ROW;

    string filename = "weak_scaling_" + to_string(procs) + "P.mtx";
    
    cout << "Generating a matrix for " << procs << " processes..." << endl;
    cout << "Dims: " << total_rows << "x" << total_cols << ", Total NNZ: " << total_nnz << endl;

    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "[Err] An error has occured while opening the file\n" << endl;
        return 1;
    }

    file << "%%MatrixMarket matrix coordinate real general\n";
    file << total_rows << " " << total_cols << " " << total_nnz << "\n";

    random_device rd;
    mt19937 gen(rd());

    uniform_int_distribution<long long> dis_col(1, total_cols); 
    uniform_real_distribution<double> dis_val(0.0, 1.0);

    for (long long r = 1; r <= total_rows; ++r) {
        for (int k = 0; k < NNZ_PER_ROW; ++k) {
            long long c = dis_col(gen);
            double v = dis_val(gen);
            
            file << r << " " << c << " " << v << "\n";
        }

        if (r % (total_rows / 10) == 0) {
            cout << "   ... " << (r * 100 / total_rows) << "% completato" << endl;
        }
    }

    file.close();
    cout << "Salvato: " << filename << endl << endl;

    return 0;
}
