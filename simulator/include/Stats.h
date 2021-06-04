/*
 * Stats.h
 *
 *  Created on: 2016/4/1
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#ifndef STATS_H_
#define STATS_H_

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cassert>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

class Stats {
    int index;  // = index of its BaseXbar
    double time;
    double energy;

public:
    int cycle;

    Stats(double time_ = 0, double energy_ = 0, int cycle_ = 0, int index_ = -1);

    Stats operator +(Stats s);
    void operator +=(Stats s);
    Stats operator -(Stats s);
    void operator -=(Stats s);
    Stats operator *(int times);
    void operator *=(int times);

    void clean_stats();
    void print_stats();
};

#endif /* STATS_H_ */
