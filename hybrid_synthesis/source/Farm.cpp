/*
 * Farm.cpp
 *
 *  Created on: 2020/7/31
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#include "Farm.h"
#include "Primitive.h"

////////////////////////////////////////////////////////////////////////
///                         FARM SKELETON                            ///
////////////////////////////////////////////////////////////////////////

Farm::Farm(int bit_width_, int shift_len_)
        :
        Skeleton(FARM, bit_width_, shift_len_) {
}

Farm::~Farm() {
}

vector<Data*>& Farm::append(Skeleton *s) {
    children.push_back(s);
    if (inputs.empty()) {
        inputs.insert(inputs.end(), s->inputs.begin(), s->inputs.end());
    }
    outputs.insert(outputs.end(), s->outputs.begin(), s->outputs.end());
    return outputs;
}

int Farm::partition() {
    for (Skeleton *child : children) {
        if (child->st == PRIMITIVE) {
            if (((Primitive*) child)->pt == ADD || ((Primitive*) child)->pt == MUL) {
                child->hybrid = 2;
            } else {
                child->hybrid = 1;
            }
        } else {
            child->partition();
        }
        hybrid |= child->hybrid;
    }
    return hybrid;
}

void Farm::reduce_stateful_logic() {
    for (Skeleton *child : children) {
        child->reduce_stateful_logic();
    }
    cycle_compute = children[0]->cycle_compute;
}

void Farm::assign_parallelism(int multiplier) {
    parallel_parent = multiplier;
    for (Skeleton *s : children) {
        s->assign_parallelism(children.size() * parallel_parent);
    }
}

void Farm::allocate_bounding_box_digital() {

}

void Farm::reduce_mac() {

}

void Farm::compare() {

}

void Farm::allocate_bounding_box_analog() {

}

void Farm::simulate() {
    /*
     * TO DO: output statistics
     */
}
