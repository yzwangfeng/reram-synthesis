/*
 * primitive.h
 *
 *  Created on: 2020/7/31
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#ifndef SYNTHESIS_INCLUDE_PRIMITIVE_H_
#define SYNTHESIS_INCLUDE_PRIMITIVE_H_

#include "skeleton.h"

struct Operation {
    PrimitiveType pt;
    Data *input1;
    Data *input2;
};

struct Primitive: public Skeleton {
    PrimitiveType pt;

    vector<Operation> operations;

    Primitive(int bit_width_, PrimitiveType pt_, Data *input1, Data *input2 = NULL, int shift_len_ = 0);
    Primitive(int bit_width_, PrimitiveType pt_, vector<Data*> &inputs_, int shift_len_ = 0);
    ~Primitive();

    virtual int partition();        // identify the potential analog part

    /*
     * optimize the whole application in the digital mode
     */
    virtual void reduce_stateful_logic();
    virtual void assign_parallelism(int multiplier = 1);
    virtual void allocate_bounding_box_digital();

    /*
     * optimize the potential analog part
     */
    virtual void reduce_mac();
    virtual void compare();
    virtual void allocate_bounding_box_analog();

    void simulate();
};

#endif /* SYNTHESIS_INCLUDE_PRIMITIVE_H_ */
