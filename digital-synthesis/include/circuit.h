/*
 * circuit.h
 *
 *
 */

#ifndef CIRCUIT_H_
#define CIRCUIT_H_

#include <map>
#include <set>
#include <queue>
#include <fstream>
#include <iostream>
#include <cmath>

#include "abc.h"

#define INF 1000000

struct Var {
    /*
     * basic variables and functions
     */
    string name;
    bool is_in, is_out;

    vector<string> pre;
    vector<string> next;

    int out_degree;

    string gate;

    Var(string name_, bool is_in_, bool is_out_, string gate_ = "NAND", bool visit_ = false);

    /*
     * variables and functions used in technology mapping (DOM)
     */

    // TO DO

    /*
     * variables and functions used in physical mapping (STAR)
     */
    bool visit;
    int min_cycle;
    int left_out_degree;
    bool need;
    vector<string> erase_pre;
};

struct Circuit {
    /*
     * basic variables and functions
     */
    string benchmark;
    int abc_iter;

    map<string, Var*> graph;
    int ope_num;

    vector<string> input;
    vector<string> output;

    Circuit(string benchmark_, bool cover_input_ = false, double alpha_ = 1, int max_set_ = INF);
    ~Circuit();

    void abc_synthesize();  // synthesize a serial operation using abc library
    void read_blif();

    void write_dot();   // visualize the circuit

    /*
     * variables and functions used in technology mapping (DOM)
     */

    // TO DO

    /*
     * variables and functions used in physical mapping (STAR)
     */
    bool cover_input;
    double alpha;
    int max_set;

    vector<vector<string> > sequence;

    void mark_pre(string now, string target);
    void compute_erase_pre(string target);
    void minimize_set_sequence();  // reorganize the operation sequence to minimize the set operations

    pair<int, int> minimize_cell();    // compute the minimal cell number
    pair<int, int> optimize_cell(bool align = true);  // compute the optimal cell number (minimize cycle * area)
    int compute_set(int cell);

    int mark_min_cycle(string now);
    double compute_max_parallelism();
};

#endif /* CIRCUIT_H_ */
