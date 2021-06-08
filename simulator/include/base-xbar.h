/*
 * base-xbar.h
 *
 *  Created on: 2017/3/13
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#ifndef BASEXBAR_H_
#define BASEXBAR_H_

#include "global.h"
#include "stats.h"

/*
 * FIXED: input
 * TMP: temporary variables
 * CLEAN: 1s
 * DIRTY: useless temporary variables, waiting RESET
 */
enum LineStatus {
    FIXED, TMP, CLEAN, DIRTY
};

class Redundance {  // status of rows or columns
public:
    Redundance(int index_, string name_, int array_size_);
    ~Redundance();

    void set_status(LineStatus s, int start, int length = 1, bool strict = false);
    void set_status(LineStatus s, vector<int> pos, bool strict = false);

    /*
     * FIND continuous clean rows or columns to store temporary variables
     *
     * search from array_size - 1 to 0
     */
    int find_clean(int length = 1);

    /*
     * FIND continuous unused (clean or dirty) rows or columns to store input
     *
     * search from 0 to array_size - 1
     */
    int find_unused(int length = 1);

    vector<int> find_all(LineStatus s = DIRTY);

    void print_redundance();

private:
    int index;  // = index of its MatrixXbar
    string name;
    int array_size;
    LineStatus* status;
    int max_redundance;  // max lines to store temporary varibles
};

class BaseXbar {    // primitive operations in an array
public:
    BaseXbar(int index_, int word_len_, int array_size_);
    ~BaseXbar();

    /*
     * SET continuous rows or columns to be unused
     */
    void set_unused(Direction row_or_col, int start, int length);

    /*
     * READ a word
     */
    Stats read_word(WORD& res, int row, int col, bool aligned = false);

    /*
     * WRITE a word
     *
     * target rows and columns will be set FIXED
     */
    Stats write_word(WORD word, int row, int col, bool aligned = false);

    /*
     * INITIALIZE an array
     *
     * range from (0, 0) to (row, word_len * col)
     * mainly for test
     */
    void construct_random_data(int row, int col = 1);

    /*
     * PRINT stats and redundance[2] information
     */
    void print_stats_redundance();

    /*
     * CALCULATE a word
     *
     * if output, print debug information
     */
    WORD print_word(int row, int col, bool output = false, bool aligned = false);

    /*
     * print all values in this array
     *
     * hex: in hexadecimal or binary format
     */
    void print_array(bool hex = false);

protected:
    int index;
    int word_len;
    int array_size;

    Redundance* redundance[2];
    Stats stats;

    /*
     * READ a row (or a column)
     *
     * data: target array
     * addr: row (or column) number
     * range from start to start + length
     * debug: if false, stats won't change
     * row_or_col：ROW = read a row, COL = read a column
     */
    Stats read(bool* data, int addr, int start, int length = WORD_LEN, bool debug = false,
            Direction row_or_col = ROW);

    /*
     * WRITE a row (or a column)
     *
     * parameters are similar to READ
     */
    Stats write(bool* data, int addr, int start = 0, int length = WORD_LEN, Direction row_or_col = ROW);

    /*
     * WRITE a RS device
     *
     * if row_or_col = ROW, (row, col) = value
     */
    Stats write(bool value, int row, int col, Direction row_or_col = ROW);

    /*
     * SET or RESET
     *
     * value: true = RESET, false = RESET
     * range from col to col + length
     * start, stride, parallelism: activated rows (or columns) are start, start + stride, ..., start + stride * (parallelism - 1)
     * row_or_col：ROW = multiple rows, COL = multiple columns
     */
    Stats set(bool value, int col, int length, int start, int stride, int parallelism, Direction row_or_col =
            ROW);

    /*
     * EXECUTE a primitive logic operation
     *
     * type: AND or NAND
     * operand1, operand2: operand columns (or rows), if operand2 = -1, no operand2
     * res: result columns (or rows)
     * start, stride, parallelism: activated rows (or columns) are start, start + stride, ..., start + stride * (parallelism - 1)
     * row_or_col：ROW = multiple rows, COL = multiple columns
     */
    Stats execute(PrimitiveType type, int operand1, int operand2, int res, int start, int stride = 1,
            int parallelism = 1, Direction row_or_col = ROW);

    /*
     * JUDGE if x = y (mod (1 << word_len))
     */
    bool equal(WORD x, WORD y);

    /*
     * clean dirty rows and columns, make them CLEAN
     */
    void clean_redundance();

private:
    bool **array;   // little endian
};

#endif /* BASEXBAR_H_ */
