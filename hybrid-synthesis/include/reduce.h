/*
 * reduce.h
 *
 *  Created on: 2020/7/31
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#ifndef SYNTHESIS_INCLUDE_REDUCE_H_
#define SYNTHESIS_INCLUDE_REDUCE_H_

#include "skeleton.h"

struct Reduce: public Skeleton {
    Reduce(int bit_width_, Skeleton *child, int shift_len_ = 0);
    ~Reduce();

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

#endif /* SYNTHESIS_INCLUDE_REDUCE_H_ */
