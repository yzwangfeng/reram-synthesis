/*
 * FloatXbar.h
 *
 *  Created on: 2017/8/4
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#ifndef INCLUDE_FLOATXBAR_H_
#define INCLUDE_FLOATXBAR_H_

#include "MatrixXbar.h"

class FloatXbar: public MatrixXbar {  // matrix operations in an array
public:
    FloatXbar(int index_, int word_len_, int array_size_);
    ~FloatXbar();

    /*
     * multiply two floats, parallel in row
     *
     */
    Stats mul_2_float_col(int multiplier1, int multiplier2, int ans, int col, int col_width);

    /*
     * in-place int <--> float
     *
     * assume a 32-bit number
     */
    Stats int2float(int row, int col, int row_width, int col_width);
    Stats float2int(int row, int col, int row_width, int col_width);
};

#endif /* INCLUDE_FLOATXBAR_H_ */
