/*
 * star.cpp
 *
 * Synthesis Targeting high ARea utilization
 */

#include <algorithm>
#include <cstring>
#include "circuit.h"

void Circuit::mark_pre(string now, string target) {
    if (graph[now]->visit || graph[now]->need || now == target) {
        return;
    }
    graph[now]->need = true;
    for (string s : graph[now]->pre) {
        mark_pre(s, target);
    }
    graph[target]->erase_pre.push_back(now);
}

void Circuit::compute_erase_pre(string target) {
    if (graph[target]->erase_pre.size() > 0) {
        vector<string> next_erase_pre;
        for (string s : graph[target]->erase_pre) {
            if (!graph[s]->visit) {
                next_erase_pre.push_back(s);
            }
        }
        graph[target]->erase_pre.clear();
        for (string s : next_erase_pre) {
            graph[target]->erase_pre.push_back(s);
        }
        return;
    }

    for (pair<string, Var*> p : graph) {
        graph[p.first]->need = false;
    }
    graph[target]->erase_pre.clear();

    for (string s : graph[target]->next) {
        mark_pre(s, target);
    }
}

void Circuit::minimize_set_sequence() {
    double total_time;
    clock_t start = clock();

#ifdef OUTPUT_SET
    char output_file[256];
    sprintf(output_file, "%s_%d_min.blif", benchmark.c_str(), abc_iter);
    ofstream fout(output_file, ios::out);
    if (!fout.is_open()) {
        return;
    }

    fout << ".model " << benchmark << endl << ".inputs";
    for (string s : input) {
        fout << " " << s;
    }
    fout << "\n.outputs";
    for (string s : output) {
        fout << " " << s;
    }
    fout << endl;
#endif

    for (pair<string, Var*> p : graph) {
        p.second->left_out_degree = p.second->out_degree;
        p.second->visit = p.second->is_in;
        p.second->erase_pre.clear();
    }

    vector<string> stored;
    if (cover_input) {
        for (string s : input) {
            stored.push_back(s);
            compute_erase_pre(s);
        }
    }

    for (int i = 0; i < ope_num; ++i) {
        vector<string> candidate;
        bool found = false;
        if (stored.size() > 0) {
            string to_erase;
            int min_erase_pre = INF;
            for (string s : stored) {
                if (graph[s]->left_out_degree > 0 && min_erase_pre > graph[s]->erase_pre.size()) {
                    min_erase_pre = graph[s]->erase_pre.size();
                    to_erase = s;
                    found = true;
                }
            }
            if (found) {
                min_erase_pre = INF;
                for (string s : graph[to_erase]->erase_pre) {
                    bool can_compute = true;
                    for (string ps : graph[s]->pre) {
                        if (!graph[ps]->visit) {
                            can_compute = false;
                            break;
                        }
                    }
                    if (can_compute) {
                        candidate.push_back(s);
                    }
                }
            }
        }

        if (stored.size() == 0 || !found) {
            for (pair<string, Var*> p : graph) {
                bool can_compute = !graph[p.first]->visit;
                for (string ps : graph[p.first]->pre) {
                    if (!graph[ps]->visit) {
                        can_compute = false;
                        break;
                    }
                }
                if (can_compute) {
                    candidate.push_back(p.first);
                }
            }
        }

        string to_compute;
        int min_erase_pre = INF;
        for (string s : candidate) {
            compute_erase_pre(s);
            if (min_erase_pre > graph[s]->erase_pre.size()) {
                min_erase_pre = graph[s]->erase_pre.size();
                to_compute = s;
            }
        }

#ifdef OUTPUT_SET
        fout << ".gate " << graph[to_compute]->gate << "\t";
        char no = 'A';
        for (string s : graph[to_compute]->pre) {
            fout << no++ << "=" << s << ' ';
        }
        fout << "Y=" << to_compute << endl;

        if (graph[to_compute]->visit) {
            cerr << "Duplicate gate!\n";
            return;
        }
        for (string s : graph[to_compute]->pre) {
            if (!graph[s]->visit) {
                cerr << "Not ready!\n";
                return;
            }
        }
#endif

        vector<string> gate;
        for (string s : graph[to_compute]->pre) {
            gate.push_back(s);
        }
        gate.push_back(to_compute);
        sequence.push_back(gate);

        graph[to_compute]->visit = true;
        for (string ps : graph[to_compute]->pre) {
            --graph[ps]->left_out_degree;
        }

        for (string s : stored) {
            vector<string>::iterator iter = find(graph[s]->erase_pre.begin(), graph[s]->erase_pre.end(),
                    to_compute);
            if (iter != graph[s]->erase_pre.end()) {
                graph[s]->erase_pre.erase(iter);
            }
        }
        stored.push_back(to_compute);
    }

    clock_t finish = clock();
    total_time = (double) (finish - start) / CLOCKS_PER_SEC;
    cout << "Run time: " << total_time << "s" << endl;
}

pair<int, int> Circuit::minimize_cell() {
    map<string, int> last_use;
    for (int i = 0; i < ope_num; ++i) {
        for (string var : sequence[i]) {
            last_use[var] = i;
        }
    }

    int cell = input.size(), min_cell = 0;
    for (int i = 0; i < ope_num; ++i) {
        ++cell;
        min_cell = max(cell, min_cell);
        for (string var : sequence[i]) {
            cell -= last_use[var] == i && !graph[var]->is_out && (cover_input || !graph[var]->is_in);
        }
    }

    return make_pair(min_cell, compute_set(min_cell));
}

pair<int, int> Circuit::optimize_cell(bool align) {
    pair<int, int> opt = minimize_cell();
    double opt_quotient = pow(ope_num + opt.second, alpha)
            / (double(input.size() + output.size()) / opt.first);
    for (int cell = align ? (opt.first / 8 + 1) * 8 : opt.first;; cell += align ? 8 : 1) {
        int set = compute_set(cell);
        double quotient = pow(ope_num + set, alpha) / (double(input.size() + output.size()) / cell);
        if (quotient > opt_quotient) {
            return opt;
        }
        opt = make_pair(cell, set);
        opt_quotient = quotient;
    }
    return make_pair(-1, -1);
}

int Circuit::compute_set(int cell) {
    if (cell < int(input.size())) {
        return -1;
    }

    for (pair<string, Var*> p : graph) {
        p.second->left_out_degree = p.second->out_degree;
        p.second->visit = p.second->is_in;
    }

    vector<string> stored;
    if (cover_input) {
        for (string s : input) {
            stored.push_back(s);
        }
    } else {
        cell -= input.size();
    }

    int set_time = 0;
    for (int ope = 0; ope < ope_num; ++ope) {
        if (stored.size() == cell) {  // SET
            vector<string> next_stored;
            for (string s : stored) {
                if (graph[s]->left_out_degree > 0 || graph[s]->is_out) {
                    next_stored.push_back(s);
                }
            }
            if (next_stored.size() == cell) {  // failed!
                return -1;
            }

            set_time += ceil(double(stored.size() - next_stored.size()) / max_set);

            stored.clear();
            for (string s : next_stored) {
                stored.push_back(s);
            }
        }

        string to_compute = *sequence[ope].rbegin();
        graph[to_compute]->visit = true;
        for (string ps : graph[to_compute]->pre) {
            --graph[ps]->left_out_degree;
        }
        stored.push_back(to_compute);
    }

    return set_time;
}

int Circuit::mark_min_cycle(string now) {
    if (graph[now]->min_cycle != INF) {
        return graph[now]->min_cycle;
    }
    int max_cycle = 0;
    for (string s : graph[now]->pre) {
        max_cycle = max(max_cycle, mark_min_cycle(s));
    }
    graph[now]->min_cycle = max_cycle + 1;
    return graph[now]->min_cycle;
}

double Circuit::compute_max_parallelism() {
    int max_cycle = 0;
    for (string s : output) {
        mark_min_cycle(s);
        max_cycle = max(max_cycle, graph[s]->min_cycle);
    }
    return double(ope_num) / max_cycle;
}
