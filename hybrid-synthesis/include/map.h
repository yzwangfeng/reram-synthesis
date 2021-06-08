/*
 * map.h
 *
 *  Created on: 2020/7/31
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#ifndef SYNTHESIS_INCLUDE_MAP_H_
#define SYNTHESIS_INCLUDE_MAP_H_

#include "skeleton.h"

struct Map: public Skeleton {
    Map(int bit_width_, int shift_len_ = 0);
    ~Map();

    vector<Data*>& append(Skeleton *s);

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

#endif /* SYNTHESIS_INCLUDE_MAP_H_ */
