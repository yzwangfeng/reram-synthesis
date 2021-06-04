#include "Star.h"

#include <algorithm>
#include <cstring>

vector<string> split(string str,
                     string separator) {  //  split a string by the separator
    vector<string> dest;
    string substring;
    string::size_type start = 0, index;
    do {
        index = str.find_first_of(separator, start);
        if (index != string::npos) {
            substring = str.substr(start, index - start);
            dest.push_back(substring);
            start = str.find_first_not_of(separator, index);
            if (start == string::npos) {
                return dest;
            }
        }
    } while (index != string::npos);
    substring = str.substr(start);  // 最后一个子串
    dest.push_back(substring);
    return dest;
}

Var::Var(string name_, bool is_in_, bool is_out_, bool visit_, string gate_)
    : name(name_), is_in(is_in_), is_out(is_out_), visit(visit_), gate(gate_) {
    min_cycle = is_in ? 0 : INF;
    out_degree = left_out_degree = 0;
    need = false;
}

Circuit::Circuit(string benchmark_, bool cover_input_, double alpha_,
                 int max_set_)
    : benchmark(benchmark_),
      cover_input(cover_input_),
      alpha(alpha_),
      max_set(max_set_) {
    abc_iter = ope_num = 0;
    abc_synthesize();
    read_blif();
    minimize_set_sequence();
    // circuit.write_dot();
}

Circuit::~Circuit() {
    char blif_file[256];
    sprintf(blif_file, "%s_%d.blif", benchmark.c_str(), abc_iter);
    remove(blif_file);
}

void Circuit::abc_synthesize() {
    int min_cycle = INF, last_cycle = INF;
    char file[256], new_file[256];
    for (abc_iter = 1; abc_iter <= 100; ++abc_iter) {
        if (abc_iter == 1) {
            sprintf(file, "%s", benchmark.c_str());
        } else {
            strcpy(file, new_file);
        }
        sprintf(new_file, "%s_%d.blif", benchmark.c_str(), abc_iter);
        abc_map(file, "write_blif", new_file, ABC_LIB);

        ifstream fin(new_file, ios::in);
        if (!fin.is_open()) {
            return;
        }
        string s;
        int cycle = 0;
        while (fin >> s) {
            cycle += s == ".gate";
        }

        fin.close();
        if (abc_iter != 1) {
            remove(file);
        }

        min_cycle = min(min_cycle, cycle);
        if (last_cycle <= cycle) {
            cout << "Total cycle: " << cycle << endl;
            break;
        }
        last_cycle = cycle;
    }
}

void Circuit::read_blif() {
    char blif_file[256];
    sprintf(blif_file, "%s_%d.blif", benchmark.c_str(), abc_iter);
    ifstream fin(blif_file, ios::in);
    if (!fin.is_open()) {
        return;
    }

    string s;
    while (fin >> s && s != ".inputs") {
    }
    while (fin >> s && s != ".outputs") {
        if (s != "\\") {
            input.push_back(s);
            graph[s] = new Var(s, true, false, true);
        }
    }
    while (fin >> s && s != ".gate") {
        if (s != "\\") {
            output.push_back(s);
            graph[s] = new Var(s, false, true, false);
        }
    }

    ope_num = 0;
    do {
        string gate;
        fin >> gate;

        getline(fin, s);
        vector<string> cells = split(s, " ");

        string out_cell = (*cells.rbegin()).substr(2);
        if (find(output.begin(), output.end(), out_cell) == output.end()) {
            graph[out_cell] = new Var(out_cell, false, false, false, gate);
        }

        cells.erase(cells.begin());

        for (string cell : cells) {
            cell = cell.substr(2);
            if (cell != out_cell) {
                graph[cell]->next.push_back(out_cell);
                ++graph[cell]->out_degree;
                graph[out_cell]->pre.push_back(cell);
            }
        }

        ++ope_num;
    } while (fin >> s && s == ".gate");
}

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
                if (graph[s]->left_out_degree > 0 &&
                    min_erase_pre > graph[s]->erase_pre.size()) {
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
            vector<string>::iterator iter =
                find(graph[s]->erase_pre.begin(), graph[s]->erase_pre.end(),
                     to_compute);
            if (iter != graph[s]->erase_pre.end()) {
                graph[s]->erase_pre.erase(iter);
            }
        }
        stored.push_back(to_compute);
    }

    clock_t finish = clock();
    total_time = (double)(finish - start) / CLOCKS_PER_SEC;
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
            cell -= last_use[var] == i && !graph[var]->is_out &&
                    (cover_input || !graph[var]->is_in);
        }
    }

    return make_pair(min_cell, compute_set(min_cell));
}

pair<int, int> Circuit::optimize_cell(bool align) {
    pair<int, int> opt = minimize_cell();
    double opt_quotient = pow(ope_num + opt.second, alpha) /
                          (double(input.size() + output.size()) / opt.first);
    for (int cell = align ? (opt.first / 8 + 1) * 8 : opt.first;;
         cell += align ? 8 : 1) {
        int set = compute_set(cell);
        double quotient = pow(ope_num + set, alpha) /
                          (double(input.size() + output.size()) / cell);
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

            set_time +=
                ceil(double(stored.size() - next_stored.size()) / max_set);

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

void Circuit::write_dot() {
    char dot_file[256];
    sprintf(dot_file, "%s.dot", benchmark.c_str());
    ofstream fout(dot_file, ios::out);
    if (!fout.is_open()) {
        return;
    }

    fout << "digraph G {\nsize = \"7.5,10\";\ncenter = true;" << endl;
    for (pair<string, Var*> p : graph) {
        fout << p.first;
        if (p.second->is_in) {
            fout << "[shape = invtriangle, color = coral, fillcolor = coral];"
                 << endl;
        } else if (p.second->is_out) {
            fout << "[shape = triangle, color = coral, fillcolor = coral];"
                 << endl;
        } else {
            fout << "[shape = ellipse];" << endl;
        }
    }
    fout << endl;
    for (pair<string, Var*> p : graph) {
        for (string s : p.second->pre) {
            fout << s << " -> " << p.first << endl;
        }
    }
    fout << '}' << endl;

    fout.close();
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
