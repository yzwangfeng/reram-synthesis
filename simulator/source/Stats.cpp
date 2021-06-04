/*
 * Stats.cpp
 *
 *  Created on: 2016/4/1
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#include "Stats.h"

Stats::Stats(double time_, double energy_, int cycle_, int index_)
        : index(index_), time(time_), energy(energy_), cycle(cycle_) {
}

Stats Stats::operator +(Stats s) {
    return Stats(time + s.time, energy + s.energy, cycle + s.cycle);
}

void Stats::operator +=(Stats s) {
    time += s.time;
    energy += s.energy;
    cycle += s.cycle;
}

Stats Stats::operator -(Stats s) {
    return Stats(time - s.time, energy - s.energy, cycle - s.cycle);
}

void Stats::operator -=(Stats s) {
    time -= s.time;
    energy -= s.energy;
    cycle -= s.cycle;
}

Stats Stats::operator *(int times) {
    return Stats(time, energy * times, cycle);
}

void Stats::operator *=(int times) {
    energy *= times;
}

void Stats::clean_stats() {
    time = 0;
    energy = 0;
    cycle = 0;
}

void Stats::print_stats() {
    string unit[10] = { " p", " n", " μ", " m", " " };
    double cout_time = time;
    int time_unit = 1;
    while (cout_time > 1e3 && time_unit <= 4) {
        cout_time /= 1e3;
        time_unit += 1;
    }
    cout << "[Xbar " << index << " Summary] Total time: " << cout_time << unit[time_unit] << "s\n";
    double cout_energy = energy;
    int energy_unit = 0;
    while (cout_energy > 1e3 && energy_unit <= 4) {
        cout_energy /= 1e3;
        energy_unit += 1;
    }
    cout << "[Xbar " << index << " Summary] Total energy: " << cout_energy << unit[energy_unit] << "J\n";
    cout << "[Xbar " << index << " Summary] Total cycle(s): " << cycle << endl;
}
