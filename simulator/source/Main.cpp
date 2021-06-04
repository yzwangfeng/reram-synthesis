/*
 * Main.cpp
 *
 */

#include "Global.h"
#include <map>
#include <set>
#include <queue>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <dirent.h>
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

int main() {
    double total_time;
    clock_t start = clock();
    /*
     * add a computation sequence and simulate its performance
     */
    clock_t finish = clock();
    total_time = (double) (finish - start) / CLOCKS_PER_SEC;
    cout << "Run time: " << total_time << "s" << endl;
}
