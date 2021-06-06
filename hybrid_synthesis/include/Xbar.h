/*
 * Xbar.h
 *
 *  Created on: 2020/9/2
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#ifndef SYNTHESIS_INCLUDE_XBAR_H_
#define SYNTHESIS_INCLUDE_XBAR_H_

#define XBAR_LENGTH 512

#include <cstring>
#include <iostream>
using namespace std;

struct Xbar {
    int x;  // which xbar
    int y;
    bool **value;

    Xbar(int x_ = -1, int y_ = -1);
};

struct Box {
    Xbar *xbar;
    int x;  // position in xbar
    int y;
    int width;
    int width_valid;
    int height;

    Box(Xbar *xbar_ = NULL, int x_ = -1, int y_ = -1, int width_ = 0,
        int width_valid_ = 0, int height_ = 0);

    friend ostream &operator<<(ostream &out, Box &box);
};

struct Data {
    int value;

    bool in_xbar;
    Box pos;

    Data();

    void set_value(int value_);
    void set_position(bool xbar_, Box pos_ = Box());
};

#endif /* SYNTHESIS_INCLUDE_XBAR_H_ */
