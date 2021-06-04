/*
 * MatrixXbar.cpp
 *
 *  Created on: 2017/3/13
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#include "MatrixXbar.h"

//define DEBUG_MATRIX_XBAR

MatrixXbar::MatrixXbar(int index_, int word_len_, int array_size_)
        : BaseXbar(index_, word_len_, array_size_) {
    assert(word_len <= 64);
    assert(array_size % word_len == 0);
}

MatrixXbar::~MatrixXbar() {
}

Stats MatrixXbar::add_element(WORD& sum, int row, int col, int row_width, int col_width) {
    assert(col % word_len == 0);

#ifdef DEBUG_MATRIX_XBAR
    printf("[Xbar %d Debug] ADD ELEMENT ", index);
    print_matrix(row, col, row_width, col_width);
    cout << endl;
#endif

    Stats pre_stats = stats;
    int sum_col = redundance[COL]->find_clean(word_len);
    vector<int> addend_col(col_width);
    for (int i = 0; i < col_width; ++i) {
        addend_col[i] = col + i * word_len;
    }
    add_num_row(addend_col, sum_col, row, 1, row_width);

    int sum_row = redundance[ROW]->find_clean();
    vector<int> addend_row(row_width);
    for (int i = 0; i < row_width; ++i) {
        addend_row[i] = row + i;
    }
    add_num_col(addend_row, sum_row, sum_col);

    read_word(sum, sum_row, sum_col);
    redundance[COL]->set_status(DIRTY, sum_col, word_len);
    redundance[ROW]->set_status(DIRTY, sum_row, 1);
    clean_redundance();

#ifdef DEBUG_MATRIX_XBAR
    printf("[Xbar %d Debug] SUM %lu\n", index, sum);
    int real_sum = 0;
    for (int i = 0; i < row_width; ++i) {
        for (int j = 0; j < col_width; ++j) {
            real_sum += print_word(row + i, col + j * word_len);
        }
    }
    assert(equal(real_sum, sum));
#endif

    return stats - pre_stats;
}

Stats MatrixXbar::add_matrix(vector<int> addend_row, int sum_row, int col, int row_width, int col_width) {
    assert(col % word_len == 0);

#ifdef DEBUG_MATRIX_XBAR
    WORD real_sum[row_width][col_width];
    for (int i = 0; i < row_width; ++i) {
        for (int j = 0; j < col_width; ++j) {
            real_sum[i][j] = 0;
            for (int k = 0; k < (int) addend_row.size(); ++k) {
                real_sum[i][j] += print_word(addend_row[k] + i, col + j * word_len);
            }
        }
    }
    printf("[Xbar %d Debug] ADD ", index);
    for (int i = 0; i < (int) addend_row.size(); ++i) {
        print_matrix(addend_row[i], col, row_width, col_width);
        if (i == addend_row.size() - 1) {
            cout << endl;
        } else {
            cout << " + ";
        }
    }
#endif

    Stats pre_stats = stats;

    for (int bias = 0; bias < row_width; ++bias) {
        vector<int> bias_addend_row;
        for (int i = 0; i < (int) addend_row.size(); ++i) {
            bias_addend_row.push_back(addend_row[i] + bias);
        }
        while (bias_addend_row.size() > 2) {
            vector<int> shift_row;
            do {
                vector<int> carry_row;
                do {
                    int carry_in = bias_addend_row.size() == 2 ? -1 : bias_addend_row[2];
                    int sum_pos = redundance[ROW]->find_clean();
                    int carry_out_pos = redundance[ROW]->find_clean();

                    add_bit(bias_addend_row[0], bias_addend_row[1], carry_in, sum_pos, carry_out_pos, col, 1,
                            word_len * col_width, COL);

                    for (int i = 0; i < 3; ++i) {
                        if (!bias_addend_row.empty()) {
                            redundance[ROW]->set_status(DIRTY, bias_addend_row[0]);
                            bias_addend_row.erase(bias_addend_row.begin());
                        }
                    }

                    bias_addend_row.push_back(sum_pos);
                    carry_row.push_back(carry_out_pos);
                } while (bias_addend_row.size() > 1);
                shift_row.push_back(bias_addend_row[0]);
                bias_addend_row.assign(carry_row.begin(), carry_row.end());
            } while (bias_addend_row.size() > 1);

            shift_row.push_back(bias_addend_row[0]);
            for (int i = 0; i < (int) shift_row.size(); ++i) {
                bool buffer[word_len * col_width];
                read(buffer, shift_row[i], col, word_len * col_width);
                for (int j = word_len * col_width - 1; j >= 0; --j) {
                    buffer[j] = j % word_len >= i ? buffer[j % word_len - i] : 0;
                }
                write(buffer, shift_row[i], col, word_len * col_width);
            }
            bias_addend_row.assign(shift_row.begin(), shift_row.end());

            clean_redundance();
        }

        if (bias_addend_row.size() == 1) {
            execute(AND, bias_addend_row[0], -1, sum_row, col, 1, word_len * col_width, COL);
        } else if (bias_addend_row.size() == 2) {
            add_2_num_col(bias_addend_row[0], bias_addend_row[1], sum_row, col, col_width);
        }

        clean_redundance();
    }

#ifdef DEBUG_MATRIX_XBAR
    printf("[Xbar %d Debug] SUM ", index);
    print_matrix(sum_row, col, row_width, col_width);
    cout << endl;
    for (int i = 0; i < row_width; ++i) {
        for (int j = 0; j < col_width; ++j) {
            assert(equal(real_sum[i][j], print_word(sum_row + i, col + word_len * j)));
        }
    }
#endif

    return stats - pre_stats;
}

Stats MatrixXbar::multiply_scalar(int multiplier_row, int product_row, int col, WORD scalar, int row_width,
        int col_width) {
    assert(col % word_len == 0);

// 上下的debug
#ifdef DEBUG_MATRIX_XBAR
    WORD a[row_width][col_width], real_res[row_width][col_width];
    for (int i = 0; i < row_width; ++i) {
        for (int j = 0; j < col_width; ++j) {
            a[i][j] = print_word(multiplier_row + i, col + j * word_len);
            real_res[i][j] = a[i][j] * scalar;
        }
    }

    printf("[Xbar %d Debug] scalar_multiply ", index);
    print_matrix(multiplier_row, col, row_width, col_width);
    cout << " * ";
    cout << scalar << endl;
#endif

    Stats pre_stats = stats;
// 申请一块add的空间   add（开始行号） = redundance[ROW]->find_clean(scalar里面1的个数);
// addend.push_back(add+0,1,2,3...)
    int one_num = 0;
    int mask = 1;
    for (int i = 0; i < word_len; i++) {
        if (mask & scalar)
            one_num++;
        mask <<= 1;
    }

// for：i=0 to row_width
    for (int u = 0; u < row_width; u++) {
        // 构造加数（考虑从哪一位开始赋值）：每次读出一行，和scalar的对应位and，写到addend[0], [1], ...
        int add = redundance[ROW]->find_clean(one_num);
        vector<int> addend;
        for (int i = 0; i < one_num; i++)
            addend.push_back(add + i);
        bool buffer[word_len * col_width];
        read(buffer, multiplier_row + u, col, word_len * col_width);
        //cout << endl;
        //print_matrix(multiplier + u, col ,1, col_width);
        //cout << "res:" << endl;
        bool* mul_tmp = new bool[word_len * col_width];
        mask = 1;
        int cnt = 0;
        for (int i = 0; i < word_len; i++) {
            if (mask & scalar) {
                //cout << i << ":" << endl;
                for (int j = 0; j < col_width; j++) {
                    for (int k = 0; k < i; k++)
                        mul_tmp[j * word_len + k] = 0;
                    for (int k = i; k < word_len; k++) {
                        mul_tmp[j * word_len + k] = buffer[j * word_len + k - i];
                    }
                }
                write(mul_tmp, addend[cnt], col, word_len * col_width);
                //print_matrix(addend[cnt], col ,1, col_width);
                cnt++;
            }
            mask <<= 1;
        }
        //  cout << addend.size() << endl;
        // add_num_col改成每行同时做多个数
        while (addend.size() > 2) {
            //cout << ">2" << endl;
            vector<int> shift;
            do {
                vector<int> carry;
                // 每次3个数相加，sum作为加数继续加，carry_out放到carry
                do {
                    int carry_in = addend.size() == 2 ? -1 : addend[2];
                    int sum_pos = redundance[ROW]->find_clean();
                    int carry_out_pos = redundance[ROW]->find_clean();

                    add_bit(addend[0], addend[1], carry_in, sum_pos, carry_out_pos, col, 1,
                            word_len * col_width, COL);

                    for (int i = 0; i < 3; ++i) {
                        if (!addend.empty()) {
                            redundance[ROW]->set_status(DIRTY, addend[0]);
                            addend.erase(addend.begin());
                        }
                    }

                    addend.push_back(sum_pos);
                    carry.push_back(carry_out_pos);
                } while (addend.size() > 1);  // 最终只有一个加数
                shift.push_back(addend[0]);   // shift[i]对应最外层循环中要平移i位
                addend.assign(carry.begin(), carry.end());   // carry部分继续相加，之后再平移
            } while (addend.size() > 1);

            // shift里的数平移
            shift.push_back(addend[0]);
            for (int i = 0; i < (int) shift.size(); ++i) {
                bool buffer1[word_len * col_width];
                read(buffer1, shift[i], col, word_len * col_width);
                // ************for col_width
                for (int q = 0; q < col_width; q++) {
                    for (int j = word_len - 1; j >= 0; --j) {
                        buffer1[j + q * word_len] = j >= i ? buffer1[j + q * word_len - i] : 0;
                    }
                }
                write(buffer1, shift[i], col, word_len * col_width);
            }
            addend.assign(shift.begin(), shift.end());

            // 然后将平移后的数相加
            clean_redundance();
        }

        //print_matrix(addend[0], col ,1, col_width);
        //print_matrix(addend[1], col ,1, col_width);

        if (addend.size() == 1) {
            execute(AND, addend[0], -1, product_row + u, col, 1, word_len * col_width, COL);
            //print_matrix(product + u, col ,1, col_width);
        } else if (addend.size() == 2) {
            //***************

            vector<int> as(7);
            for (int i = 0; i < 7; ++i) {
                as[i] = redundance[ROW]->find_clean();
            }
            // 一起做前五步
            execute(NAND, addend[0], addend[1], as[0], col, 1, word_len * col_width, COL);  // 并行，并行度是word_len
            execute(NAND, addend[0], as[0], as[1], col, 1, word_len * col_width, COL);
            execute(NAND, addend[1], as[0], as[2], col, 1, word_len * col_width, COL);
            execute(NAND, as[1], as[2], as[3], col, 1, word_len * col_width, COL);
            execute(NAND, as[3], -1, as[4], col, 1, word_len * col_width, COL);

            //**********for col, col+word_len, col+word_len*2, ...(col_width个)
            //一开始的低位carry_in
            for (int i = 0; i < col_width; i++) {
                write(false, as[6], col + i * word_len);
            }

            //一开始低位的cary_out
            execute(NAND, as[0], -1, as[5], col, word_len, col_width, COL);

            // 第6、7步（进位平移）
            for (int i = col + 1, pos = as[5]; i < col + word_len; ++i) {   // propagate carries in series
                //*****for i, i+word_len, i+2*word_len, ... (col_width个)
                for (int j = 0; j < col_width; j++) {
                    execute(AND, i + j * word_len - 1, -1, i + j * word_len, pos, 1, 1);
                }

                execute(NAND, as[3], pos, product_row + u, i, word_len, col_width, COL);
                pos = as[5] + as[6] - pos;
                execute(NAND, as[0], product_row + u, pos, i, word_len, col_width, COL);
            }

            // 第8、9步（并行）
            execute(NAND, as[6], -1, as[4], col, 2, (col_width * word_len) >> 1, COL);
            execute(NAND, as[5], -1, as[4], col + 1, 2, (col_width * word_len) >> 1, COL);
            execute(NAND, as[4], -1, product_row + u, col, 1, (col_width * word_len), COL);

            redundance[ROW]->set_status(DIRTY, as);

            clean_redundance();
        }
    }

#ifdef DEBUG_MATRIX_XBAR
// 输出结果
    printf("[Xbar %d Debug] MUL ", index);
// sum，col：结果的左上角
    print_matrix(product_row, col, row_width, col_width);
    cout << endl;
    for (int i = 0; i < row_width; ++i) {
        for (int j = 0; j < col_width; ++j) {
            // 比较两个数 print_word(开始的坐标)
            assert(equal(real_res[i][j], print_word(product_row + i, col + word_len * j)));
        }
    }
#endif

    return stats - pre_stats;
}

Stats MatrixXbar::multiply_matrix(int multiplier_row1, int multiplier_row2, int product_row, int col,
        int row_width1, int row_width2, int col_width) {
    assert(col % word_len == 0);

#ifdef DEBUG_MATRIX_XBAR
    WORD m1[row_width1][col_width], m2[row_width2][col_width], real_product[row_width1][row_width2];
    for (int i = 0; i < col_width; ++i) {
        for (int row1 = 0; row1 < row_width1; ++row1) {
            m1[row1][i] = print_word(multiplier_row1 + row1, col + i * word_len);
        }
        for (int row2 = 0; row2 < row_width2; ++row2) {
            m2[row2][i] = print_word(multiplier_row2 + row2, col + i * word_len);
        }
    }
    for (int row1 = 0; row1 < row_width1; ++row1) {
        for (int row2 = 0; row2 < row_width2; ++row2) {
            real_product[row1][row2] = 0;
            for (int i = 0; i < col_width; ++i) {
                real_product[row1][row2] += m1[row1][i] * m2[row2][i];
            }
        }
    }
    printf("[Xbar %d Debug] MULTIPLY ", index);
    print_matrix(multiplier_row1, col, row_width1, col_width);
    cout << " * ";
    print_matrix(multiplier_row2, col, row_width2, col_width);
    cout << endl;
#endif

    Stats pre_stats = stats;
    bool* buffer = new bool[word_len * col_width];
    for (int row1 = 0; row1 < row_width1; ++row1) {  // execute matrix-vector multiplication for row_width1 times
        /*
         * transform multiplication to addition. construct addends
         */
        bool value1[word_len * col_width];
        read(value1, multiplier_row1 + row1, col, word_len * col_width);
        vector<int> as(word_len * row_width2);
        as[0] = redundance[ROW]->find_clean(word_len * row_width2);

        for (int row2 = 0; row2 < row_width2; ++row2) {
            bool value2[word_len * col_width];
            read(value2, multiplier_row2 + row2, col, word_len * col_width);
            for (int i = 0; i < word_len; ++i) {
                for (int j = 0; j < col_width * word_len; ++j) {
                    buffer[j] = j % word_len >= i ? value1[j - i] & value2[j - j % word_len + i] : false;
                }
                as[word_len * row2 + i] = as[0] + word_len * row2 + i;
                write(buffer, as[word_len * row2 + i], col, word_len * col_width);
            }
        }

        /*
         * add from left to right, then from top to bottom
         */
        int sum = col;
        if (col_width != 1) {
            sum = redundance[COL]->find_clean(word_len);
            vector<int> addend(col_width);
            for (int i = 0; i < col_width; ++i) {
                addend[i] = col + word_len * i;
            }
            add_num_row(addend, sum, as[0], 1, word_len * row_width2);
        }

        for (int i = 0; i < row_width2; ++i) {
            int as_product = redundance[ROW]->find_clean();
            vector<int> addend(word_len);
            for (int j = 0; j < word_len; ++j) {
                addend[j] = as[word_len * i + j];
            }
            add_num_col(addend, as_product, sum);
            read(buffer, as_product, sum, word_len);
            write(buffer, product_row + row1, col + word_len * i, word_len);
            redundance[ROW]->set_status(DIRTY, as_product);
        }

        if (col_width != 1) {
            redundance[COL]->set_status(DIRTY, sum, word_len);
        }
        redundance[ROW]->set_status(DIRTY, as);
        clean_redundance();
    }
    delete[] buffer;

    clean_redundance();

#ifdef DEBUG_MATRIX_XBAR
    printf("[Xbar %d Debug] PRODUCT ", index);
    print_matrix(product_row, col, row_width1, row_width2);
    cout << endl;
    for (int i = 0; i < row_width1; ++i) {
        for (int j = 0; j < row_width2; ++j) {
            assert(equal(real_product[i][j], print_word(product_row + i, col + word_len * j)));
        }
    }
#endif

    return stats - pre_stats;
}

Stats MatrixXbar::transpose_matrix(int before_row, int after_row, int col, int row_width, int col_width) {
    assert(col % word_len == 0);

#ifdef DEBUG_MATRIX_XBAR
    printf("[Xbar %d Debug] TRANSPOSE ", index);
    print_matrix(before_row, col, row_width, col_width);
    cout << endl;
#endif

    Stats pre_stats = stats;
    bool* buffer = new bool[word_len];
    for (int i = 0; i < row_width; ++i) {
        for (int j = 0; j < col_width; ++j) {
            read(buffer, before_row + i, col + j * word_len);
            write(buffer, after_row + j, col + i * word_len);
        }
    }

#ifdef DEBUG_MATRIX_XBAR
    printf("[Xbar %d Debug] AFTER TRANSPOSE ", index);
    print_matrix(after_row, col, col_width, row_width);
    cout << endl;
    for (int i = 0; i < row_width; ++i) {
        for (int j = 0; j < col_width; ++j) {
            assert(
                    equal(print_word(before_row + i, col + j * word_len),
                            print_word(after_row + j, col + i * word_len)));
        }
    }
#endif

    return stats - pre_stats;
}

Stats MatrixXbar::max_min_element(WORD& x, int row, int col, int row_width, int col_width, bool is_max) {
#ifdef DEBUG_MATRIX_XBAR
    printf("[Xbar %d Debug] MAX_MIN_ELEMENT ", index);
    print_matrix(row, col, row_width, col_width);
    cout << endl;
#endif

    Stats pre_stats = stats;
    int ans_row = redundance[COL]->find_clean(word_len);
    max_min_row(row, col, col_width, is_max, 1, row_width, ans_row);
    int ans = redundance[ROW]->find_clean();
    max_min_col(row, ans_row, row_width, is_max, ans);
    read_word(x, ans, ans_row);

#ifdef DEBUG_MATRIX_XBAR
    printf("[Xbar %d Debug] MAX_MIN_ELEMENT %lu", index, x);
    if (is_max)
        cout << "(max)" << endl << endl;
    else
        cout << "(min)" << endl << endl;
    WORD res;
    read_word(res, row, col);
    for (int i = 1; i < row_width; i++) {
        for (int j = 0; j < col_width; j++) {
            WORD tmp;
            read_word(tmp, row + i, col + j * word_len);
            if (is_max && tmp > res)
                res = tmp;
            if ((!is_max) && tmp < res)
                res = tmp;
        }
    }
    //assert(res == read_word(ans, col));
#endif

    redundance[COL]->set_status(DIRTY, ans_row, word_len);
    redundance[ROW]->set_status(DIRTY, ans, 1);
    clean_redundance();

    return stats - pre_stats;
}

Stats MatrixXbar::add_bit(int addend1, int addend2, int carry_in, int sum, int carry_out, int start,
        int stride, int parallelism, Direction row_or_col) {
    Stats pre_stats = stats;
    vector<int> as(5);
    for (int i = 0; i < 5; ++i) {
        as[i] = redundance[1 - row_or_col]->find_clean();
    }

    execute(NAND, addend1, addend2, as[0], start, stride, parallelism, row_or_col);
    execute(NAND, addend1, as[0], as[1], start, stride, parallelism, row_or_col);
    execute(NAND, addend2, as[0], as[2], start, stride, parallelism, row_or_col);
    execute(NAND, as[1], as[2], as[3], start, stride, parallelism, row_or_col);
    if (carry_in != -1) {
        execute(NAND, as[3], carry_in, sum, start, stride, parallelism, row_or_col);
    }
    execute(NAND, as[0], sum, carry_out, start, stride, parallelism, row_or_col);
    execute(NAND, as[3], -1, as[4], start, stride, parallelism, row_or_col);
    if (carry_in != -1) {
        execute(NAND, carry_in, -1, as[4], start, stride, parallelism, row_or_col);
    }
    execute(NAND, as[4], -1, sum, start, stride, parallelism, row_or_col);

    redundance[1 - row_or_col]->set_status(DIRTY, as);
    clean_redundance();

    return stats - pre_stats;
}

Stats MatrixXbar::add_2_num_row(int addend1, int addend2, int sum, int start, int stride, int parallelism) {
#ifdef DEBUG_MATRIX_XBAR
    WORD a1[parallelism], a2[parallelism];
    printf("[Xbar %d Debug] ADD", index);
    for (int i = 0; i < parallelism; ++i) {
        a1[i] = print_word(start + stride * i, addend1);
        a2[i] = print_word(start + stride * i, addend2);
        printf(" %lu + %lu", a1[i], a2[i]);
    }
    cout << endl;
#endif

    Stats pre_stats = stats;
    int carry_in = -1;
    for (int i = 0; i < word_len; ++i) {
        int carry_out = redundance[COL]->find_clean();
        add_bit(addend1 + i, addend2 + i, carry_in, sum + i, carry_out, start, stride, parallelism, ROW);
        carry_in = carry_out;
        redundance[COL]->set_status(DIRTY, carry_in);
        if (i == word_len - 1) {
            redundance[COL]->set_status(DIRTY, carry_out);
        }
    }
    clean_redundance();

#ifdef DEBUG_MATRIX_XBAR
    WORD s[parallelism];
    printf("[Xbar %d Debug] SUM", index);
    for (int i = 0; i < parallelism; ++i) {
        s[i] = print_word(start + stride * i, sum);
        printf(" %lu", s[i]);
        assert(equal(a1[i] + a2[i], s[i]));
    }
    cout << endl;
#endif

    return stats - pre_stats;
}

Stats MatrixXbar::add_2_num_col(int addend1, int addend2, int sum, int col, int parallelism) {
#ifdef DEBUG_MATRIX_XBAR
    WORD a1[parallelism], a2[parallelism];
    printf("[Xbar %d Debug] ADD", index);
    for (int i = 0; i < parallelism; ++i) {
        a1[i] = print_word(addend1, col + word_len * i);
        a2[i] = print_word(addend2, col + word_len * i);
        printf(" %lu + %lu", a1[i], a2[i]);
    }
    cout << endl;
#endif

    Stats pre_stats = stats;
    vector<int> as(7);
    for (int i = 0; i < 7; ++i) {
        as[i] = redundance[ROW]->find_clean();
    }

    execute(NAND, addend1, addend2, as[0], col, 1, word_len * parallelism, COL);
    execute(NAND, addend1, as[0], as[1], col, 1, word_len * parallelism, COL);
    execute(NAND, addend2, as[0], as[2], col, 1, word_len * parallelism, COL);
    execute(NAND, as[1], as[2], as[3], col, 1, word_len * parallelism, COL);
    execute(NAND, as[3], -1, as[4], col, 1, word_len * parallelism, COL);

    for (int j = 0; j < parallelism; ++j) {
        write(false, as[6], col + j * word_len);
        execute(NAND, as[0], -1, as[5], col + j * word_len, 1, 1, COL);
        for (int i = col + j * word_len + 1, pos = as[5]; i < col + j * word_len + word_len; ++i) {  // propagate carries in series
            execute(AND, i - 1, -1, i, pos, 1, 1);
            execute(NAND, as[3], pos, sum, i, 1, 1, COL);
            pos = as[5] + as[6] - pos;
            execute(NAND, as[0], sum, pos, i, 1, 1, COL);
        }
    }

    execute(NAND, as[6], -1, as[4], col, 2, (word_len >> 1) * parallelism, COL);
    execute(NAND, as[5], -1, as[4], col + 1, 2, (word_len >> 1) * parallelism, COL);
    execute(NAND, as[4], -1, sum, col, 1, word_len * parallelism, COL);

    redundance[ROW]->set_status(DIRTY, as);
    clean_redundance();

#ifdef DEBUG_MATRIX_XBAR
    WORD s[parallelism];
    printf("[Xbar %d Debug] SUM", index);
    for (int i = 0; i < parallelism; ++i) {
        s[i] = print_word(sum, col + word_len * i);
        printf(" %lu", s[i]);
        assert(equal(a1[i] + a2[i], s[i]));
    }
    cout << endl;
#endif

    return stats - pre_stats;
}

Stats MatrixXbar::add_num_row(vector<int> addend_col, int sum_col, int start_row, int stride,
        int parallelism) {
#ifdef DEBUG_MATRIX_XBAR
    WORD real_s[parallelism];
    printf("[Xbar %d Debug] ADD", index);
    for (int i = 0; i < parallelism; ++i) {
        real_s[i] = 0;
        for (int j = 0; j < (int) addend_col.size(); ++j) {
            WORD a = print_word(start_row + stride * i, addend_col[j]);
            real_s[i] += a;
            printf(" %s%lu", j == 0 ? "" : "+ ", a);
        }
    }
    cout << endl;
#endif

    Stats pre_stats = stats;
    if (addend_col.size() == 1 && sum_col != addend_col[0]) {
        for (int i = 0; i < word_len; ++i) {
            execute(AND, addend_col[0] + i, -1, sum_col + i, start_row, stride, parallelism, ROW);
        }
    } else if (addend_col.size() == 2) {
        add_2_num_row(addend_col[0], addend_col[1], sum_col, start_row, stride, parallelism);
    } else {
        int tmp_sum_col = redundance[COL]->find_clean(word_len);
        int x = addend_col.size() & 1 ? tmp_sum_col : sum_col, y = tmp_sum_col + sum_col - x;
        add_2_num_row(addend_col[0], addend_col[1], x, start_row, stride, parallelism);
        for (int i = 2; i < (int) addend_col.size(); ++i) {
            if ((i & 1) == 0) {
                add_2_num_row(addend_col[i], x, y, start_row, stride, parallelism);
                set(true, x, word_len, start_row, stride, parallelism);
            } else {
                add_2_num_row(addend_col[i], y, x, start_row, stride, parallelism);
                set(true, y, word_len, start_row, stride, parallelism);
            }
        }
        redundance[COL]->set_status(DIRTY, tmp_sum_col, word_len);
    }

    clean_redundance();

#ifdef DEBUG_MATRIX_XBAR
    WORD s[parallelism];
    printf("[Xbar %d Debug] SUM", index);
    for (int i = 0; i < parallelism; ++i) {
        s[i] = print_word(start_row + stride * i, sum_col);
        printf(" %lu", s[i]);
        assert(equal(real_s[i], s[i]));
    }
    cout << endl;
#endif

    return stats - pre_stats;
}

Stats MatrixXbar::add_num_col(vector<int> addend_row, int sum_row, int col) {
#ifdef DEBUG_MATRIX_XBAR
    printf("[Xbar %d Debug] ADD", index);
    WORD real_sum = 0;
    for (int i = 0; i < (int) addend_row.size(); ++i) {
        WORD a = print_word(addend_row[i], col);
        real_sum += a;
        printf(" %s%lu", i == 0 ? "" : "+ ", a);
    }
    cout << endl;
#endif

    Stats pre_stats = stats;
    /*
     * carry save adder
     */
    while (addend_row.size() > 2) {
        vector<int> shift_row;
        do {
            vector<int> carry_row;
            do {
                int carry_in = addend_row.size() == 2 ? -1 : addend_row[2];
                int sum_pos = redundance[ROW]->find_clean();
                int carry_out_pos = redundance[ROW]->find_clean();

                add_bit(addend_row[0], addend_row[1], carry_in, sum_pos, carry_out_pos, col, 1, word_len,
                        COL);

                for (int i = 0; i < 3; ++i) {
                    if (!addend_row.empty()) {
                        redundance[ROW]->set_status(DIRTY, addend_row[0]);
                        addend_row.erase(addend_row.begin());
                    }
                }

                addend_row.push_back(sum_pos);
                carry_row.push_back(carry_out_pos);
            } while (addend_row.size() > 1);
            shift_row.push_back(addend_row[0]);
            addend_row.assign(carry_row.begin(), carry_row.end());
        } while (addend_row.size() > 1);

        shift_row.push_back(addend_row[0]);
        for (int i = 0; i < (int) shift_row.size(); ++i) {
            bool buffer[word_len];
            read(buffer, shift_row[i], col, word_len);
            for (int j = word_len - 1; j >= 0; --j) {
                buffer[j] = j >= i ? buffer[j - i] : 0;
            }
            write(buffer, shift_row[i], col, word_len);
        }
        addend_row.assign(shift_row.begin(), shift_row.end());

        clean_redundance();
    }

    if (addend_row.size() == 1) {
        execute(AND, addend_row[0], -1, sum_row, col, 1, word_len, COL);
    } else if (addend_row.size() == 2) {
        add_2_num_col(addend_row[0], addend_row[1], sum_row, col);
    }

    clean_redundance();

#ifdef DEBUG_MATRIX_XBAR
    WORD s = print_word(sum_row, col);
    printf("[Xbar %d Debug] SUM %lu\n", index, s);
    assert(equal(real_sum, s));
#endif

    return stats - pre_stats;
}

Stats MatrixXbar::max_min_2_ele(int row, int ele1, int ele2, bool is_max, int stride, int parallelism,
        int ans) {
#ifdef DEBUG_MATRIX_XBAR
    printf("[Xbar %d Debug] MAX_MIN_2_ELE ", index);
    for (int i = 0; i < parallelism; i++) {
        cout << "line1:" << endl;
        print_word(row + i * stride, ele1, 1);
        print_word(row + i * stride, ele2, 1);
    }
#endif

    Stats pre_stats = stats;
    // xi:as[4] yi:as[6] ai!bi:as[5] xn...xi+1:res[0] yn...yi:res[1]
    int res = redundance[COL]->find_clean(2);
    for (int i = word_len - 1; i >= 0; i--) {
        int as = redundance[COL]->find_clean(7);
        write(1, row, as + 7, ROW);
        execute(NAND, ele1 + i, -1, as, row, stride, parallelism, ROW);
        //cout << "!a" << i << ":" << *read(row, as, 1, 0, ROW) << " ";
        execute(NAND, ele2 + i, -1, as + 1, row, stride, parallelism, ROW);
        //cout << "!b" << i << ":" << *read(row, as + 1, 1, 0, ROW) << " ";
        execute(NAND, ele1 + i, ele2 + i, as + 2, row, stride, parallelism, ROW);
        //cout << "nand-ab" << i <<":"<< *read(row, as + 2, 1, 0, ROW) << " ";
        execute(NAND, as, as + 1, as + 3, row, stride, parallelism, ROW);
        execute(NAND, as + 2, as + 3, as + 4, row, stride, parallelism, ROW);    // xi
        //cout << "a" << i<<":" << *read(row, ele1 + i, 1, 0, ROW) << " ";
        //cout << "b" << i <<":"<< *read(row, ele2 + i, 1, 0, ROW) << " ";
        //cout << "x" << i <<":"<< *read(row, as + 4, 1, 0, ROW) << endl;
        execute(AND, ele1 + i, as + 1, as + 5, row, stride, parallelism, ROW);
        execute(NAND, res, as + 5, as + 6, row, stride, parallelism, ROW);  // yi
        execute(AND, res, as + 4, res, row, stride, parallelism, ROW);
        execute(AND, res + 1, as + 6, res + 1, row, stride, parallelism, ROW);
        redundance[COL]->set_status(DIRTY, as, 7);
    }
    for (int i = 0; i < parallelism; i++) {
        bool p[1];
        read(p, row + i * stride, res + 1, 1);
        WORD tmp;
        // ele1 is min
        if (*p) {
            if (is_max) {
                read_word(tmp, row + i * stride, ele2);
                write_word(tmp, row + i * stride, ans);
            } else {
                read_word(tmp, row + i * stride, ele1);
                write_word(tmp, row + i * stride, ans);
            }
        } else {
            if (is_max) {
                read_word(tmp, row + i * stride, ele1);
                write_word(tmp, row + i * stride, ans);
            }

            else {
                read_word(tmp, row + i * stride, ele2);
                write_word(tmp, row + i * stride, ans);
            }
        }
#ifdef DEBUG_MATRIX_XBAR
        printf("[Xbar %d Debug] \n", index);
        WORD a1;
        read_word(a1, row + i * stride, ele1);
        WORD a2;
        read_word(a2, row + i * stride, ele2);
        if (a1 > a2) {
            if (is_max) {
                assert(tmp == a1);
            } else {
                assert(tmp == a2);
            }
        }
        if (a1 < a2) {
            if (is_max) {
                assert(tmp == a2);
            } else {
                assert(tmp == a1);
            }
        }
#endif

    }
    clean_redundance();

    return stats - pre_stats;
}

Stats MatrixXbar::max_min_row(int row, int col, int col_width, bool is_max, int stride, int parallelism,
        int ans) {
#ifdef DEBUG_MATRIX_XBAR
    printf("[Xbar %d Debug] MAX_MIN_ROW ", index);
    for (int j = 0; j < parallelism; j++) {
        cout << "line" << j << endl;
        for (int i = 0; i < col_width; i++) {
            print_word(row + j * stride, col + i * word_len, 1);
        }
    }
#endif

    Stats pre_stats = stats;
    set(!is_max, ans, word_len, row, stride, parallelism, ROW);
    for (int i = 0; i < col_width; i++) {
        max_min_2_ele(row, ans, col + i * word_len, is_max, stride, parallelism, ans);
    }

#ifdef DEBUG_MATRIX_XBAR
    printf("[Xbar %d Debug] MAX_MIN_ROW \n\n", index);
    for (int i = 0; i < parallelism; i++) {
        WORD x;
        read_word(x, row + i * stride, col);
        for (int j = 1; j < col_width; ++j) {
            WORD tmp;
            read_word(tmp, row + i * stride, col + j * word_len);
            if (is_max && tmp > x) {
                x = tmp;
            }
            if ((!is_max) && tmp < x) {
                x = tmp;
            }
        }
        WORD real_x;
        read_word(real_x, row + i * stride, ans);
        assert(x == real_x);
    }
#endif

    return stats - pre_stats;
}

Stats MatrixXbar::max_min_col(int row, int col, int row_width, bool is_max, int ans) {
#ifdef DEBUG_MATRIX_XBAR
    printf("[Xbar %d Debug] MAX_MIN_COL ", index);
    for (int i = 0; i < row_width; i++)
        print_word(row + i, col, 1);
#endif

    Stats pre_stats = stats;
    WORD tmp;
    read_word(tmp, row, col);
    write_word(tmp, ans, col);

    for (int i = 1; i < row_width; i++) {
        int as = redundance[ROW]->find_clean(6);
        execute(NAND, ans, -1, as, col, 1, word_len, COL);
        execute(NAND, row + i, -1, as + 1, col, 1, word_len, COL);
        execute(NAND, ans, row + i, as + 2, col, 1, word_len, COL);
        execute(NAND, as, as + 1, as + 3, col, 1, word_len, COL);
        execute(NAND, as + 2, as + 3, as + 4, col, 1, word_len, COL);  // x

        /*for (int j = 0; j < word_len; j++){
         cout << "a" << j<<":" << *read(ans, col + j, 1, 0, ROW) << " ";
         cout << "b" << j <<":"<< *read(row + i, col + j, 1, 0, ROW) << " ";
         cout << "x" << j <<":"<< *read(as + 4, col + j, 1, 0, ROW) << endl;
         }*/
        execute(AND, ans, as + 1, as + 5, col, 1, word_len, COL);  // ai!bi
        int ai_nbi = redundance[COL]->find_clean(word_len);
        WORD as_word;
        read_word(as_word, as + 5, col);
        write_word(as_word, as + 4, ai_nbi);

        int yi = redundance[COL]->find_clean(word_len);
        execute(NAND, ai_nbi + word_len - 1, -1, yi + word_len - 1, as + 4, 1, 1, ROW);
        for (int j = word_len - 2; j >= 0; j--) {
            execute(NAND, col + j + 1, ai_nbi + j, yi + j, as + 4, 1, 1, ROW);
            execute(AND, col + j + 1, col + j, col + j, as + 4, 1, 1, ROW);
        }

        for (int j = word_len - 1; j > 0; j--) {
            execute(AND, yi + j, yi + j - 1, yi + j - 1, as + 4, 1, 1, ROW);
        }

        bool p[1];
        read(p, as + 4, yi, 1);
        // cout << *p << endl;
        if (*p) {  // i > ans
            if (is_max) {
                WORD u;
                read_word(u, row + i, col);
                //cout << u << endl;
                write_word(u, ans, col);
            }
        } else {
            if (!is_max) {
                WORD u;
                read_word(u, row + i, col);
                //cout << u << endl;
                write_word(u, ans, col);
            }
        }
        redundance[ROW]->set_status(DIRTY, as, 6);
        redundance[COL]->set_status(DIRTY, ai_nbi, word_len);
        redundance[COL]->set_status(DIRTY, yi, word_len);
    }
    clean_redundance();

#ifdef DEBUG_MATRIX_XBAR
    WORD max_min_col;
    read_word(max_min_col, ans, col);
    printf("[Xbar %d Debug] MAX_MIN_COL %lu", index, max_min_col);
    if (is_max)
        cout << "(max)" << endl << endl;
    else
        cout << "(min)" << endl << endl;
    WORD res;
    read_word(res, row, col);
    for (int i = 1; i < row_width; i++) {
        WORD tmp;
        read_word(tmp, row + i, col);
        if (is_max && tmp > res)
            res = tmp;
        if ((!is_max) && tmp < res)
            res = tmp;
    }
    WORD real_res;
    read_word(real_res, ans, col);
    assert(res == real_res);
#endif

    return stats - pre_stats;
}

void MatrixXbar::print_matrix(int row, int col, int row_width, int col_width) {
    cout << "(" << endl;
    for (int i = 0; i < row_width; ++i) {
        cout << (i == 0 ? "(" : " (");
        for (int j = 0; j < col_width; ++j) {
            cout << (j == 0 ? "" : " ") << print_word(row + i, col + word_len * j);
        }
        cout << ")" << endl;
    }
    cout << ")";
}
