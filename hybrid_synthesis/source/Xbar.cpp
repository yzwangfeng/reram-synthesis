/*
 * Xbar.cpp
 *
 *  Created on: 2020/9/2
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#include "Xbar.h"

Xbar::Xbar(int x_, int y_)
        :
        x(x_), y(y_), value(NULL) {
}

Box::Box(Xbar *xbar_, int x_, int y_, int width_, int width_valid_, int height_)
        :
        xbar(xbar_), x(x_), y(y_), width(width_), width_valid(width_valid_), height(height_) {
}

ostream& operator<<(ostream &out, Box &box) {
    out << box.width << ' ' << box.height << endl;
    return out;
}

Data::Data()
        :
        value(0), in_xbar(false) {
}

void Data::set_value(int value_) {
    value = value_;
}

void Data::set_position(bool in_xbar_, Box pos_) {
    in_xbar = in_xbar_;
    if (in_xbar) {
        pos = pos_;
    }
}
