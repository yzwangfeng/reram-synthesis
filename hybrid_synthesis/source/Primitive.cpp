/*
 * Primitive.cpp
 *
 *  Created on: 2020/7/31
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#include "Primitive.h"

#include <cassert>

////////////////////////////////////////////////////////////////////////
///                       PRIMITIVE CIRCUIT                          ///
////////////////////////////////////////////////////////////////////////

Primitive::Primitive(int bit_width_, PrimitiveType pt_, Data *input1,
                     Data *input2, int shift_len_)
    : Skeleton(PRIMITIVE, bit_width_, shift_len_), pt(pt_) {
    assert(pt == NOT || input2 != NULL);
    inputs.push_back(input1);
    if (input2 != NULL) {
        inputs.push_back(input2);
    }
    Data *output = new Data();
    outputs.push_back(output);
}

Primitive::Primitive(int bit_width_, PrimitiveType pt_, vector<Data *> &inputs_,
                     int shift_len_)
    : Skeleton(PRIMITIVE, bit_width_, shift_len_), pt(pt_) {
    for (Data *input : inputs_) {
        inputs.push_back(input);
    }
    Data *output = new Data();
    outputs.push_back(output);
}

Primitive::~Primitive() {
    for (Data *output : outputs) {
        delete output;
    }
}

int Primitive::partition() {
    hybrid = 1;
    return hybrid;
}

void Primitive::reduce_stateful_logic() {
    /*
     * TO DO:
     * 1. invoke STAR, parameters: pt, input1, input2, bit_width
     * 2. fill in operations
     * 3. assign cycle_compute
     */

    switch (pt) {
        case ADD:
            cycle_compute = bit_width * 9;
            break;
        case MUL:
            cycle_compute = bit_width * (bit_width + 1) * 9 / 2;
            break;
        case AND:
            cycle_compute = bit_width * 2;
            break;
        case XOR:
            cycle_compute = bit_width * 6;
            break;
        case NOT:
            cycle_compute = bit_width;
            break;
        default:
            cycle_compute = bit_width;
            break;
    }
}

void Primitive::assign_parallelism(int multiplier) {
    parallel_parent = multiplier;
}

void Primitive::allocate_bounding_box_digital() {
    /*
     * TO DO:
     * make a tradeoff between area and latency in STAR
     */
    bounding_box = Box(new Xbar(), -1, -1, cycle_compute, bit_width, 1);
}

void Primitive::reduce_mac() {}

void Primitive::compare() {}

void Primitive::allocate_bounding_box_analog() {}

void Primitive::simulate() {
    /*
     * TO DO: output statistics
     */
}
