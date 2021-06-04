/*
 * Global.h
 *
 *  Created on: 2017/3/15
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#ifndef INCLUDE_GLOBAL_H_
#define INCLUDE_GLOBAL_H_

#define WORD unsigned long

enum Direction {
    ROW, COL
};

enum PrimitiveType {
    AND, NAND, OR, NOR, XOR, XNOR, NOT, READ, SET, RESET
};

const int ARRAY_QUANTITY = 8;
const int ARRAY_SIZE = 512;
const int WORD_LEN = 8;  // no more than 64

const double TIME[14] = { };
const double ENERGY[14] = { };
const double CURRENT[14] = { };

#define INF 1000000

#endif /* INCLUDE_GLOBAL_H_ */
