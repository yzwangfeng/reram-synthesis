/*
 * main.cpp
 *
 */

#include <dirent.h>

#include "../include/circuit.h"
using namespace std;

void get_file_name(string path, vector<string> &files) {
    struct dirent *ptr;
    DIR *dir = opendir(path.c_str());
    while ((ptr = readdir(dir)) != NULL) {
        if (ptr->d_name[0] != '.') {  // skip . and ..
            files.push_back(ptr->d_name);
        }
    }
    closedir(dir);
}

void run_star(string dir, bool cover_input, double alpha, int max_set, int mode) {
    vector<string> benchmark;
    get_file_name(dir, benchmark);
    for (string s : benchmark) {
        Circuit circuit(dir + "/" + s, cover_input, alpha, max_set);
        circuit.minimize_set_sequence();
        cout << circuit.benchmark << ":\n";
        if (mode == 0) {
            pair<int, int> min = circuit.minimize_cell();
            cout << "Minimal cell: " << min.first << " cells with " << min.second << " set cycles\n";
        } else if (mode == 1) {
            pair<int, int> opt = circuit.optimize_cell();
            cout << "Optimal cell: " << opt.first << " cells with " << opt.second << " set cycles\n";
        } else {
            int cell = 0;
            while (cell != -1) {
                cout << "Cells for " << s << " : ";
                cin >> cell;
                int set = circuit.compute_set(cell);
                cout << "Set cycles: " << set << endl;
            }
        }
    }
}

int main() {
    run_star("benchmark/EPFL", false, 1.0, INF, 0);
    return 0;
}
