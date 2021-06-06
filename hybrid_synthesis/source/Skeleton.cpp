/*
 * Skeleton.cpp
 *
 *  Created on: 2020/6/3
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#include "Skeleton.h"
#include "Noc.h"
#include <cmath>
#include <iostream>

#define NOCW 16
#define NOCH 16
Noc noc[NOCW][NOCH];

////////////////////////////////////////////////////////////////////////
///                           SKELETON                               ///
////////////////////////////////////////////////////////////////////////

Skeleton::Skeleton()
        :
        st(PRIMITIVE), bit_width(1), shift_len(0), hybrid(0), cycle_compute(0), cycle_move(0), parallel_parent(
                1), parallel_own(1) {
}

Skeleton::Skeleton(SkeletonType st_, int bit_width_, int shift_len_)
        :
        st(st_), bit_width(bit_width_), shift_len(shift_len_), hybrid(0), cycle_compute(0), cycle_move(0), parallel_parent(
                1), parallel_own(1) {
}

Skeleton::~Skeleton() {
    for (Skeleton *child : children) {
        delete child;
    }
}

Data* Skeleton::operator[](int index) {
    return outputs[index];
}

vector<Data*>& Skeleton::append(Skeleton *s) {
    children.push_back(s);
    return outputs;
}

void Skeleton::synthesize() {
    partition();

    reduce_stateful_logic();
    assign_parallelism();
    allocate_bounding_box_digital();

    reduce_mac();
    compare();
    allocate_bounding_box_analog();

    map_to_crossbar();
}

void Skeleton::calc_pos() {
    pos.clear();
    for (int i = children.size() - 1; ~i; i--) {
        Skeleton *skel = children[i];
        if (skel->bounding_box.height > XBAR_LENGTH || skel->bounding_box.width > XBAR_LENGTH) {
            skel->calc_pos();
            for (int k = skel->pos.size() - 1; ~k; k--)
                pos.push_back(skel->pos[k]);
        } else
            pos.push_back(skel->bounding_box);
    }
}

void Skeleton::map_to_crossbar(int obj) {
    for (int i = 0; i < NOCW; i++)
        for (int j = 0; j < NOCH; j++)
            noc[i][j] = Noc(XBAR_LENGTH * XBAR_LENGTH);
    switch (obj) {
        case 1:
            map_to_crossbar_throughput();
            break;
        case 2:
            map_to_crossbar_area();
            break;
        default:
            map_to_crossbar_throughput();
    }
    calc_pos();
}

void Skeleton::set_pos(int x, int y, int nocx, int nocy) {
    bounding_box.x = x;
    bounding_box.y = y;
    bounding_box.xbar->x = nocx;
    bounding_box.xbar->y = nocy;
}

double calc_min_utils() {
    double utils = XBAR_LENGTH * XBAR_LENGTH;
    for (int i = 0; i < NOCW; i++)
        for (int j = 0; j < NOCH; j++)
            utils = min(utils, noc[i][j].utils);
    return utils;
}

pair<int, int> Skeleton::greedy_map_throughput(int x, int y) {
    for (int k = children.size() - 1; ~k; k--) {
        Skeleton *skel = children[k];
        if (skel->bounding_box.height > XBAR_LENGTH || skel->bounding_box.width > XBAR_LENGTH) {
            pair<int, int> p = skel->greedy_map_throughput(x, y);
            x = p.first;
            y = p.second;
            continue;
        }
        // cout << skel->bounding_box.height << ' ' << skel->bounding_box.width
        //      << endl;
        while (true) {
            if (y == NOCH - 1) {
                y = 0;
                if (x == NOCW - 1)
                    x = 0;
                else
                    x = x + 1;
            } else
                y = y + 1;
            bool flag = false;
            for (int i = 0; i < XBAR_LENGTH; i++) {
                for (int j = 0; j < XBAR_LENGTH; j++)
                    if (noc[x][y].check_avil(skel, i, j)) {
                        noc[x][y].add_skel(skel);
                        skel->set_pos(i, j, x, y);
                        flag = true;
                        break;
                    }
                if (flag)
                    break;
            }
            if (flag)
                break;
        }
    }
    return make_pair(x, y);
}
void Skeleton::map_to_crossbar_throughput() {
    greedy_map_throughput(NOCW - 1, NOCH - 1);
    double utils = calc_min_utils();
    double t = utils;
    double alpha = 0.95;
    int iter = 0;
    while (t > 1e-5) {
        iter++;
        // std::cout << "iter=" << iter << ' ' << t << ':' << utils << endl;
        int nocx, nocy, nocx1, nocy1, x, y, k;
        Skeleton *skel;
        while (true) {
            nocx = rand() % NOCW;
            nocy = rand() % NOCH;
            if (noc[nocx][nocy].skeleton.size() == 0)
                continue;
            k = rand() % noc[nocx][nocy].skeleton.size();
            skel = noc[nocx][nocy].skeleton[k];
            nocx1 = rand() % NOCW;
            nocy1 = rand() % NOCH;
            x = rand() % (XBAR_LENGTH - skel->bounding_box.width + 1);
            y = rand() % (XBAR_LENGTH - skel->bounding_box.height + 1);
            if (noc[nocx1][nocy1].check_avil(skel, x, y)) {
                break;
            }
        }
        noc[nocx][nocy].delete_skel(k);
        noc[nocx1][nocy1].add_skel(skel);
        double utils1 = calc_min_utils();
        if (utils1 < utils) {
            // choose with a certain probability
            double prob = exp((utils1 - utils) / t);
            // cout << prob << endl;
            if ((double) rand() / RAND_MAX < prob) {
                skel->set_pos(x, y, nocx1, nocy1);
                utils = utils1;
            } else {
                noc[nocx1][nocy1].delete_skel(noc[nocx1][nocy1].skeleton.size() - 1);
                noc[nocx][nocy].add_skel(skel);
            }
        } else {
            // choose the new strategy
            skel->set_pos(x, y, nocx1, nocy1);
            utils = utils1;
        }
        t = t * alpha;
    }
}

pair<int, int> Skeleton::greedy_map_area(int x, int y) {
    // cout << x << ' ' << y << endl;
    for (int k = children.size() - 1; ~k; k--) {
        Skeleton *skel = children[k];
        if (skel->bounding_box.height > XBAR_LENGTH || skel->bounding_box.width > XBAR_LENGTH) {
            pair<int, int> p = skel->greedy_map_area(x, y);
            x = p.first;
            y = p.second;
            continue;
        }
        // cout << skel->bounding_box.height << ' ' << skel->bounding_box.width
        //     << ' ' << x << ' ' << y << endl;
        while (true) {
            bool flag = false;
            for (int i = 0; i < XBAR_LENGTH; i++) {
                for (int j = 0; j < XBAR_LENGTH; j++)
                    if (noc[x][y].check_avil(skel, i, j)) {
                        noc[x][y].add_skel(skel);
                        skel->set_pos(i, j, x, y);
                        // cout << x << ' ' << y << endl;
                        flag = true;
                        break;
                    }
                if (flag)
                    break;
            }
            if (flag)
                break;
            if (y == NOCH - 1) {
                y = 0;
                if (x == NOCW - 1)
                    x = 0;
                else
                    x = x + 1;
            } else
                y = y + 1;
        }
    }
    return make_pair(x, y);
}

double calc_area() {
    double area = 0;
    for (int i = 0; i < NOCW; i++)
        for (int j = 0; j < NOCH; j++)
            if (noc[i][j].skeleton.size() > 0)
                area++;
    return area;
}
void Skeleton::map_to_crossbar_area() {
    greedy_map_area(0, 0);
    double utils = calc_area();
    double t = utils;
    double alpha = 0.95;
    int iter = 0;
    while (t > 1e-5) {
        iter++;
        // std::cout << "iter=" << iter << ' ' << t << ':' << utils << endl;
        int nocx, nocy, nocx1, nocy1, x, y, k;
        Skeleton *skel;
        while (true) {
            nocx = rand() % NOCW;
            nocy = rand() % NOCH;
            if (noc[nocx][nocy].skeleton.size() == 0)
                continue;
            k = rand() % noc[nocx][nocy].skeleton.size();
            skel = noc[nocx][nocy].skeleton[k];
            nocx1 = rand() % NOCW;
            nocy1 = rand() % NOCH;
            x = rand() % (XBAR_LENGTH - skel->bounding_box.width + 1);
            y = rand() % (XBAR_LENGTH - skel->bounding_box.height + 1);
            if (noc[nocx1][nocy1].check_avil(skel, x, y)) {
                break;
            }
        }
        noc[nocx][nocy].delete_skel(k);
        noc[nocx1][nocy1].add_skel(skel);
        double utils1 = calc_area();
        if (utils1 > utils) {
            // choose with a certain probability
            double prob = exp((utils - utils1) / t);
            // cout << prob << endl;
            if ((double) rand() / RAND_MAX < prob) {
                skel->set_pos(x, y, nocx1, nocy1);
                utils = utils1;
            } else {
                noc[nocx1][nocy1].delete_skel(noc[nocx1][nocy1].skeleton.size() - 1);
                noc[nocx][nocy].add_skel(skel);
            }
        } else {
            // choose the new strategy
            skel->set_pos(x, y, nocx1, nocy1);
            utils = utils1;
        }
        t = t * alpha;
    }
}

void Skeleton::simulate() {
}
