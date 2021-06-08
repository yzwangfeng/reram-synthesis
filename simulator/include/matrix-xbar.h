/*
 * matrix-xbar.h
 *
 *  Created on: 2017/3/13
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#ifndef INCLUDE_MATRIXXBAR_H_
#define INCLUDE_MATRIXXBAR_H_

#include "base-xbar.h"

class MatrixXbar: public BaseXbar {  // matrix operations in an array
public:
    MatrixXbar(int index_, int word_len_, int array_size_);
    ~MatrixXbar();

    /*
     * ADD all elements in a matrix
     */
    Stats add_element(WORD& sum, int row, int col, int row_width, int col_width);

    /*
     * ADD matrices
     *
     * addend, sum: row_width rows * (col_width * word_len) columns
     * col: the 1st column of matrices
     *
     * row_width = 1: vector + vector
     * row_width = col_width = 1: number + number
     *
     * this operation requires col % word_len == 0
     */
    Stats add_matrix(vector<int> addend_row, int sum_row, int col, int row_width = 1, int col_width = 1);

    /*
     * SCALAR MULTIPLICATION
     *
     * multiplier, product: row_width rows * (col_width * word_len) columns
     * col: the 1st column of matrices
     *
     * this operation requires col % word_len == 0
     */
    Stats multiply_scalar(int multiplier_row, int product_row, int col, WORD scalar, int row_width = 1,
            int col_width = 1);

    /*
     * MULTIPLY 2 matrices
     *
     * multiplier1: row_width1 rows * (col_width * word_len) columns
     * multiplier2': row_width2 rows * (col_width * word_len) columns
     * product: row_width1 rows * (row_width2 * word_len) columns
     * col: the 1st column of matrices
     *
     * row_width1 = 1 || row_width2 = 1: matrix * vector
     * row_width1 = row_width2 = 1: vector * vector
     * row_width1 = row_width2 = col_width: number * number
     *
     * this operation requires col % word_len == 0
     */
    Stats multiply_matrix(int multiplier_row1, int multiplier_row2, int product_row, int col, int row_width1 =
            1, int row_width2 = 1, int col_width = 1);

    /*
     * TRANSPOSE a matrix
     */
    Stats transpose_matrix(int before_row, int after_row, int col, int row_width, int col_width);

    /*
     * FIND maximum or minimum in a matrix
     */
    Stats max_min_element(WORD& res, int row, int col, int row_width, int col_width, bool is_max);

protected:
    /* FULL ADDER
     *
     * if row_or_col = COL
     * then (start, carry_out), (start, sum) = (start, addend1) + (start, addend2) + (start, carry_in)
     *      (start + stride, carry_out), (start + stride, sum) = (start + stride, addend1) + (start + stride, addend2) + (start + stride, carry_in)
     *      ......
     *
     * carry_in = -1 means no carry_in
     */
    Stats add_bit(int addend1, int addend2, int carry_in, int sum, int carry_out, int start, int stride = 1,
            int parallelism = 1, Direction row_or_col = COL);

    /*
     * ADD 2 numbers in several rows
     *
     * in the first row,
     *   addend1 is stored from (start, addend1) to (start, addend1 + word_len)
     *   addend2 is stored from (start, addend2) to (start, addend2 + word_len)
     *   sum is stored from (start, sum) to (start, sum + word_len)
     */
    Stats add_2_num_row(int addend1, int addend2, int sum, int start, int stride = 1, int parallelism = 1);

    /*
     * ADD 2 numbers in a column
     *
     * addend1 is stored from (addend1, col) to (addend1, col + word_len)
     * addend2 is stored from (addend2, col) to (addend2, col + word_len)
     * sum is stored from (sum, col) to (sum, col + word_len)
     */
    Stats add_2_num_col(int addend1, int addend2, int sum, int col, int parallelism = 1);

    /*
     * ADD several numbers in several rows
     *
     * parameters are similar to add_2_num_row
     */
    Stats add_num_row(vector<int> addend_col, int sum_col, int start_row, int stride = 1,
            int parallelism = 1);

    /*
     * ADD several numbers in a column
     *
     * parameters are similar to add_2_num_col
     */
    Stats add_num_col(vector<int> addend_row, int sum_row, int col);

    /*
     * min/max of two elements
     *
     */
    Stats max_min_2_ele(int row, int ele1, int ele2, bool is_max, int stride = 1, int parallelism = 1,
            int ans = 0);

    /*
     * min/max in a row
     *
     */
    Stats max_min_row(int row, int col, int col_width, bool is_max, int stride = 1, int parallelism = 1,
            int ans = 0);

    /*
     * min/max in a column
     *
     */
    Stats max_min_col(int row, int col, int row_width, bool is_max, int ans = 0);

    /*
     * PRINT a matrix when debugging
     */
    void print_matrix(int row, int col, int row_width, int col_width);
};

#endif /* INCLUDE_MATRIXXBAR_H_ */
