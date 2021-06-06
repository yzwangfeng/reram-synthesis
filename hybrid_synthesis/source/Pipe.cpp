/*
 * Pipe.cpp
 *
 *  Created on: 2020/7/31
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#include "Pipe.h"
#include "Primitive.h"

////////////////////////////////////////////////////////////////////////
///                         PIPE SKELETON                            ///
////////////////////////////////////////////////////////////////////////

Pipe::Pipe(int bit_width_, int iter_, int shift_len_)
        :
        Skeleton(PIPE, bit_width_, shift_len_), iter(iter_) {
}

Pipe::~Pipe() {

}

vector<Data*>& Pipe::append(Skeleton *s) {
    if (children.size() == 0) {
        inputs.assign(s->inputs.begin(), s->inputs.end());
    }
    outputs.assign(s->outputs.begin(), s->outputs.end());
    children.push_back(s);
    return outputs;
}

int Pipe::partition() {
    for (Skeleton *child : children) {
        hybrid |= child->partition();
    }
    return hybrid;
}

void Pipe::reduce_stateful_logic() {
    for (Skeleton *child : children) {
        child->reduce_stateful_logic();
    }

    int start = 0;
    for (int end = 0; end <= children.size(); ++end) {
        if (end == children.size() || children[end]->st != PRIMITIVE) {
            /*
             * TO DO:
             * invoke STAR to synthesize children from start to end
             */
            start = end + 1;
        }
    }

    cycle_compute = children[0]->cycle_compute;
    for (int i = 1; i < children.size(); ++i) {
        /*
         * TO DO:
         * merge shared operations between children[i - 1] and children[i]
         */
        children[i]->cycle_compute -= 2;
        cycle_compute += children[i]->cycle_compute;
    }
    cycle_compute *= iter;
}

void Pipe::assign_parallelism(int multiplier) {
    parallel_parent = multiplier;
    for (Skeleton *s : children) {
        s->assign_parallelism(parallel_parent);
    }
}

void Pipe::allocate_bounding_box_digital() {
    for (Skeleton *child : children) {
        child->allocate_bounding_box_digital();
    }

    /*
     * for image convolution only
     */
    int width = 0, height = 0;
    for (Skeleton *skeleton : children) {
        width += skeleton->bounding_box.width;
        height = max(height, skeleton->bounding_box.height);
    }
    bounding_box = Box(new Xbar(), -1, -1, width, bit_width, height);
}

void Pipe::reduce_mac() {

}

void Pipe::compare() {

}

void Pipe::allocate_bounding_box_analog() {

}

void Pipe::simulate() {
    /*
     * TO DO: output statistics
     */
}
