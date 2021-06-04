/*
 * Star.h
 *
 * Synthesis Targeting high ARea utilization
 */

#ifndef SYNTHESIS_H_
#define SYNTHESIS_H_

#include <map>
#include <set>
#include <queue>
#include <fstream>
#include <iostream>
#include <cmath>
#include "ABC.h"

//#define OUTPUT_SET

#define INF 1000000

struct Var {
    string name;
    bool is_in, is_out;

    int min_cycle;

    vector<string> pre;
    vector<string> next;

    int out_degree;
    int left_out_degree;

    bool visit;

    string gate;

    bool need;
    vector<string> erase_pre;

    Var(string name_, bool is_in_, bool is_out_, bool visit_, string gate_ = "NOR");
};

struct Circuit {
    string benchmark;
    int abc_iter;

    map<string, Var*> graph;
    int ope_num;

    vector<string> input;
    vector<string> output;

    bool cover_input;
    double alpha;
    int max_set;

    vector<vector<string> > sequence;

    Circuit(string benchmark_, bool cover_input_ = false, double alpha_ = 1, int max_set_ = INF);
    ~Circuit();

    void abc_synthesize();  // synthesize a serial operation using abc library
    void read_blif();

    void mark_pre(string now, string target);
    void compute_erase_pre(string target);
    void minimize_set_sequence();  // reorganize the operation sequence to minimize the set operations

    pair<int, int> minimize_cell();    // compute the minimal cell number
    pair<int, int> optimize_cell(bool align = true);    // compute the optimal cell number (minimize cycle * area)
    int compute_set(int cell);

    void write_dot();   // visualize the circuit

    int mark_min_cycle(string now);
    double compute_max_parallelism();
};

#endif /* SYNTHESIS_H_ */
