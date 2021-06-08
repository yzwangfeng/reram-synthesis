/*
 * reduce.cpp
 *
 *  Created on: 2020/7/31
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#include "reduce.h"
#include "primitive.h"

////////////////////////////////////////////////////////////////////////
///                        REDUCE SKELETON                           ///
////////////////////////////////////////////////////////////////////////

Reduce::Reduce(int bit_width_, Skeleton *child, int shift_len_)
        :
        Skeleton(REDUCE, bit_width_, shift_len_) {
    children.push_back(child);
    inputs.assign(children[0]->inputs.begin(), children[0]->inputs.end());
    outputs.assign(children[0]->outputs.begin(), children[0]->outputs.end());
}

Reduce::~Reduce() {
}

int Reduce::partition() {
    if (children[0]->st == PRIMITIVE) {
        if (((Primitive*) children[0])->pt == ADD) {
            hybrid = 2;
        } else {
            hybrid = 1;
        }
    } else {
        children[0]->partition();
    }
    hybrid = children[0]->hybrid;
    return hybrid;
}

void Reduce::reduce_stateful_logic() {
    children[0]->reduce_stateful_logic();
    cycle_compute = children[0]->cycle_compute * 4;
}

void Reduce::assign_parallelism(int multiplier) {
    parallel_parent = multiplier;
    children[0]->assign_parallelism(inputs.size() / 2 * parallel_parent);
}

void Reduce::allocate_bounding_box_digital() {
    for (Skeleton *child : children) {
        child->allocate_bounding_box_digital();
    }

    /*
     * for image convolution only
     */
    bounding_box = Box(new Xbar(), -1, -1, 30, bit_width, 3);
}

void Reduce::reduce_mac() {

}

void Reduce::compare() {

}

void Reduce::allocate_bounding_box_analog() {

}

void Reduce::simulate() {
    /*
     * TO DO: output statistics
     */
}
