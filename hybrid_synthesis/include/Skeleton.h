/*
 * Skeleton.h
 *
 *  Created on: 2020/6/3
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#ifndef INCLUDE_SKELETON_H_
#define INCLUDE_SKELETON_H_

#include <vector>

#include "Xbar.h"
using namespace std;

enum SkeletonType {
    PRIMITIVE, MAP, REDUCE, FARM, PIPE
};

enum PrimitiveType {
    ADD, SUB, MUL, DIV, AND, NAND, OR, NOR, XOR, XNOR, NOT, READ
};

struct Skeleton {
    /*
     * input information
     */
    SkeletonType st;
    int bit_width;
    int shift_len;
    vector<Data*> inputs;
    vector<Data*> outputs;
    vector<Skeleton*> children;

    /*
     * computed in partition()
     */
    int hybrid;  // 0: unidentified, 1: digital, 2: analog, 3: digital + analog

    /*
     * computed in reduce_stateful_logic()
     */
    int cycle_compute;
    int cycle_move;

    /*
     * computed in assign_parallelism()
     */
    int parallel_parent;

    /*
     * computed in allocate_bounding_box_digital()
     */
    int parallel_own;  // <= XBAR_LENGTH / parallel_parent
    Box bounding_box;

    /*
     * computed in map_to_crossbar()
     */
    vector<Box> pos;

    Skeleton();
    Skeleton(SkeletonType st_, int bit_width_, int shift_len_ = 0);
    virtual ~Skeleton();

    Data* operator[](int index);

    virtual vector<Data*>& append(Skeleton *s);

    void synthesize();

    virtual int partition() = 0;  // identify the potential analog part

    /*
     * optimize the whole application in the digital mode in three steps
     */
    virtual void reduce_stateful_logic() = 0;
    virtual void assign_parallelism(int multiplier = 1) = 0;
    virtual void allocate_bounding_box_digital() = 0;

    /*
     * optimize the potential analog part in three steps
     */
    virtual void reduce_mac() = 0;
    virtual void compare() = 0;
    virtual void allocate_bounding_box_analog() = 0;

    /*
     * split the bounding box to fit the crossbar size
     * the optimization objective can be throughput or area
     */
    void map_to_crossbar(int obj = 1);
    void map_to_crossbar_throughput();
    void map_to_crossbar_area();
    pair<int, int> greedy_map_throughput(int x, int y);
    pair<int, int> greedy_map_area(int x, int y);
    void calc_pos();

    virtual void simulate();

    void set_pos(int x, int y, int nocx, int nocy);
};

#endif /* INCLUDE_SKELETON_H_ */
