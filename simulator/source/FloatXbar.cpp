/*
 * FloatXbar.cpp
 *
 *  Created on: 2017/8/4
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#include "FloatXbar.h"

//#define DEBUG_FLOAT_XBAR

FloatXbar::FloatXbar(int index_, int word_len_, int array_size_)
        : MatrixXbar(index_, word_len_, array_size_) {
}

FloatXbar::~FloatXbar() {
}

Stats FloatXbar::mul_2_float_col(int multiplier1, int multiplier2, int ans, int col, int col_width) {
#ifdef DEBUG_FLOAT_XBAR
    printf("[Xbar %d Debug] MUL_2_FLOAT_COL\n", index);
    for (int i = 0; i < col_width; i++) {
        unsigned u1_pre = read_word(multiplier1, col + word_len * i);
        unsigned u2_pre = read_word(multiplier2, col + word_len * i);
        for (int j = 0; j < 32; j++) {
            cout << !!(u1_pre & (1 << j));
        }
        cout << endl;
        cout << "*" << endl;
        for (int j = 0; j < 32; j++) {
            cout << !!(u2_pre & (1 << j));
        }
        cout << endl;

        union {
            unsigned u;
            float f;
        }a_pre;

        a_pre.u = u1_pre;
        float f1_pre = a_pre.f;
        a_pre.u = u2_pre;
        float f2_pre = a_pre.f;
        // cout << f1_pre << "*" << f2_pre << "=" << endl;

        float res_pre = f1_pre * f2_pre;
        a_pre.f = res_pre;
        for (int j = 0; j < 32; j++) {
            cout << !!(a_pre.u & (1 << j));
        }
        cout << endl;
        //cout << res_pre << endl;
    }
#endif

    Stats pre_stats = stats;
    // 读出两个数，存在val1, val2中，确定将结果存在bool *res中
    bool val1[col_width * word_len];
    read(val1, multiplier1, col, col_width * word_len);
    bool val2[col_width * word_len];
    read(val2, multiplier2, col, col_width * word_len);
    /*
     for (int i = 0; i < col_width * word_len; i++)
     cout <<val1[i];
     cout << endl;
     for (int i = 0; i < col_width * word_len; i++)
     cout <<val2[i];
     cout << endl;
     */
    bool *res = new bool[col_width * word_len];

    int frac = 23;
    int exp = 8;

    // 判断是否是规格化数，结果存在nor1, nor2中，1表示规格化，0表示exp全零，1表示exp全一
    vector<int> nor1;
    vector<int> nor2;
    for (int i = 0; i < col_width; i++) {
        // 表示0，1是否出现过
        int flag1[2] = { 0 };
        int flag2[2] = { 0 };
        for (int j = frac; j < frac + exp; j++) {
            if (val1[i * word_len + j])
                flag1[1] = 1;
            else
                flag1[0] = 1;
            if (val2[i * word_len + j])
                flag2[1] = 1;
            else
                flag2[0] = 1;
        }
        if (!flag1[0])
            nor1.push_back(-1);
        else if (!flag1[1])
            nor1.push_back(0);
        else
            nor1.push_back(1);
        if (!flag2[0])
            nor2.push_back(-1);
        else if (!flag2[1])
            nor2.push_back(0);
        else
            nor2.push_back(1);
    }

    // 构造frac相乘得到的加数，字长为2*(frac+1)
    int frac_mul_adder = redundance[ROW]->find_clean(frac + 1);
    int frac_mul_len = 2 * (frac + 1);
    bool *buf = new bool[col_width * frac_mul_len];
    for (int i = 0; i < frac; i++) {
        // 先构造val2的frac部分每一位与val1的（frac+缺省的1或0）乘积
        for (int j = 0; j < col_width; j++) {
            for (int u = 0; u < col_width * frac_mul_len; u++)
                buf[u] = 0;
            if (val2[j * word_len + i]) {
                for (int u = 0; u < frac; u++)
                    buf[j * frac_mul_len + i + u] = val1[j * word_len + u];
                if (nor1[j])
                    buf[j * frac_mul_len + i + frac] = 1;
            }
        }
        write(buf, frac_mul_adder + i, 0, col_width * frac_mul_len, ROW);
    }
    // 构造val2缺省值与val1的乘积
    for (int u = 0; u < col_width * frac_mul_len; u++)
        buf[u] = 0;
    for (int j = 0; j < col_width; j++) {
        if (nor2[j]) {
            for (int u = 0; u < frac; u++)
                buf[j * frac_mul_len + frac + u] = val1[j * word_len + u];
            if (nor1[j])
                buf[j * frac_mul_len + frac + frac] = 1;
        }
    }
    write(buf, frac_mul_adder + frac, 0, col_width * frac_mul_len, ROW);
    delete[] buf;

    // 写回rram，进行整行的加法
    vector<int> addend_row;
    for (int i = 0; i <= frac; i++)
        addend_row.push_back(frac_mul_adder + i);
    int frac_mul_res = redundance[ROW]->find_clean(1);
    int tmp = word_len;
    word_len = col_width * frac_mul_len;
    add_num_col(addend_row, frac_mul_res, 0);
    word_len = tmp;

    // 读出结果，判断最高位是否为1，从最高位开始，向低位找第一个1，位置为t，将移位数存在shift<int>中，存t-(2(frac+1)-2)
    bool frac_mul[col_width * frac_mul_len];
    read(frac_mul, frac_mul_res, 0, col_width * frac_mul_len);
    redundance[ROW]->set_status(DIRTY, frac_mul_adder, frac + 1);
    redundance[ROW]->set_status(DIRTY, frac_mul_res, 1);
    /*
     cout << "res !!!" << endl;
     for (int i = 0; i < col_width * frac_mul_len; i++)
     cout << frac_mul[i];
     */
    vector<int> shift;
    for (int i = 0; i < col_width; i++) {
        int t = 0;
        for (int j = frac_mul_len - 1; j >= 0; j--) {
            if (frac_mul[i * frac_mul_len + j]) {
                t = j;
                break;
            }
        }
        shift.push_back(t - (frac_mul_len - 2));

        // 将这个1之后的frac个数复制到res的frac部分
        for (int u = 0; u < frac; u++)
            res[i * word_len + frac - 1 - u] = frac_mul[i * frac_mul_len + t - 1 - u];
    }

    // 复制val1, val2中exp部分，nor为0的写1，构造exp + 1位的带符号加法的加数
    vector<int> addend;
    int exp_add = redundance[ROW]->find_clean(4);
    for (int i = 0; i < 4; i++)
        addend.push_back(exp_add + i);
    bool *buf_exp = new bool[col_width * (exp + 1)];
    // val1
    for (int i = 0; i < col_width * (exp + 1); i++)
        buf_exp[i] = 0;
    for (int i = 0; i < col_width; i++) {
        if (nor1[i]) {
            for (int j = 0; j < (exp + 1); j++)
                buf_exp[i * (exp + 1) + j] = val1[i * word_len + frac + j];
        } else {
            buf_exp[i * (exp + 1)] = 1;
        }
    }
    write(buf_exp, exp_add, 0, col_width * (exp + 1), ROW);
    // val2
    for (int i = 0; i < col_width * (exp + 1); i++)
        buf_exp[i] = 0;
    for (int i = 0; i < col_width; i++) {
        if (nor2[i]) {
            for (int j = 0; j < (exp + 1); j++)
                buf_exp[i * (exp + 1) + j] = val2[i * word_len + frac + j];
        } else {
            buf_exp[i * (exp + 1)] = 1;
        }
    }
    write(buf_exp, exp_add + 1, 0, col_width * (exp + 1), ROW);

    // 写入-bias, shift，作为另两个加数
    // -bias
    for (int i = 0; i < col_width * (exp + 1); i++)
        buf_exp[i] = 0;
    for (int i = 0; i < col_width; i++) {
        buf_exp[i * (exp + 1)] = 1;
        buf_exp[i * (exp + 1) + exp] = 1;
        buf_exp[i * (exp + 1) + exp - 1] = 1;
    }
    write(buf_exp, exp_add + 2, 0, col_width * (exp + 1), ROW);
    // shift
    for (int i = 0; i < col_width * (exp + 1); i++)
        buf_exp[i] = 0;
    for (int i = 0; i < col_width; i++) {
        if (shift[i] == 0)
            continue;
        if (shift[i] == 1) {
            buf_exp[i * (exp + 1)] = 1;
        } else {
            unsigned tmp_shift = (unsigned) shift[i];
            int mask = 1;
            for (int j = 0; j < (exp + 1); j++) {
                buf_exp[i * (exp + 1) + j] = !!(mask & tmp_shift);
                mask <<= 1;
            }
        }
    }
    write(buf_exp, exp_add + 3, 0, col_width * (exp + 1), ROW);
    delete[] buf_exp;

    // 加法
    int exp_add_res = redundance[ROW]->find_clean(1);
    tmp = word_len;
    word_len = col_width * (exp + 1);
    add_num_col(addend, exp_add_res, 0);
    word_len = tmp;

    // 读出结果exp_sum
    bool exp_sum[col_width * (exp + 1)];
    read(exp_sum, exp_add_res, 0, col_width * (exp + 1));
    redundance[ROW]->set_status(DIRTY, exp_add, 4);
    redundance[ROW]->set_status(DIRTY, exp_add_res, 1);

    // 先处理符号位不为1的情况，将对应位复制到res的exp部分，若exp_sum中符号位为1且nor1和nor2为1，则将nor1和nor2改为-1
    vector<int> cpu_cal;
    for (int i = 0; i < col_width; i++) {
        if (!exp_sum[i * (exp + 1) + exp]) {
            for (int j = 0; j < exp; j++)
                res[i * word_len + frac + j] = exp_sum[i * (exp + 1) + j];
        } else {
            if ((!nor1[i]) && (!nor2[i])) {
                nor1[i] = -1;
                nor2[i] = -1;
            } else {
                cpu_cal.push_back(i);
            }
        }
    }
    // 将nor1或nor2为-1的情况，将res对应部分赋值为全1
    for (int i = 0; i < col_width; i++) {
        if (nor1[i] == -1 || nor2[i] == -1) {
            for (int j = 0; j < word_len; j++)
                res[i * word_len + j] = 1;
        }
    }

    // 写回res到rram
    write(res, ans, col, col_width * word_len, ROW);

    // 若exp_sum中符号位为1且nor1和nor2为至少有一个不为1，将对应的数用read_word再读出，用
    // union转化为float在cpu中运算，然后再将结果用union转化为unsigned，写回rram
    for (int i = 0; i < cpu_cal.size(); i++) {
        int pos = cpu_cal[i];
        WORD u1;
        read_word(u1, multiplier1, col + word_len * pos);
        WORD u2;
        read_word(u2, multiplier2, col + word_len * pos);

        union {
            unsigned u;
            float f;
        } a;

        a.u = u1;
        float f1 = a.f;
        a.u = u2;
        float f2 = a.f;

        float cpu_res = f1 * f2;
        a.f = cpu_res;

        write_word((WORD) a.u, ans, col + word_len * pos);
    }

    /*
     execute(NAND, ans, -1, as, col, 1, word_len, COL);
     execute(NAND, row + i, -1, as + 1, col, 1, word_len, COL);
     execute(NAND, ans, row + i, as + 2, col, 1, word_len, COL);
     execute(NAND, as, as + 1, as + 3, col, 1, word_len, COL);
     execute(NAND, as + 2, as + 3, as + 4, col, 1, word_len, COL); // x
     */
    // 处理符号位的同或
    int as = redundance[ROW]->find_clean(5);
    execute(NAND, multiplier1, -1, as, col + word_len - 1, word_len, col_width, COL);
    execute(NAND, multiplier2, -1, as + 1, col + word_len - 1, word_len, col_width, COL);
    execute(NAND, multiplier1, multiplier2, as + 2, col + word_len - 1, word_len, col_width, COL);
    execute(NAND, as, as + 1, as + 3, col + word_len - 1, word_len, col_width, COL);
    execute(NAND, as + 2, as + 3, as + 4, col + word_len - 1, word_len, col_width, COL);    // xi
    execute(NAND, as + 4, -1, ans, col + word_len - 1, word_len, col_width, COL);
    redundance[ROW]->set_status(DIRTY, as, 5);

    clean_redundance();
#ifdef DEBUG_FLOAT_XBAR
    printf("[Xbar %d Debug] MUL_2_FLOAT_COL", index);
    cout << "rram result:" << endl;
    for (int i = 0; i < col_width; i++) {
        unsigned tmp = read_word(ans, col + i * word_len);
        for (int j = 0; j < 32; j++) {
            cout << !!(tmp & (1 << j));
        }
        cout << endl;
        union {
            unsigned u;
            float f;
        }a_test;
        a_test.u = tmp;
        //cout << a_test.f << endl;
    }
#endif

    return stats - pre_stats;
}

Stats FloatXbar::int2float(int row, int col, int row_width, int col_width) {
#ifdef DEBUG_FLOAT_XBAR
    printf("[Xbar %d Debug] INT2FLOAT ", index);
    print_matrix(row, col, row_width, col_width);
    unsigned** test = new unsigned*[row_width + 5];
    for (int i = 0; i < row_width + 5; i++)
    test[i] = new unsigned[col_width + 5];
    for (int i = 0; i < row_width; i++) {
        for (int j = 0; j < col_width; j++) {
            test[i][j] = print_word(row + i, col + j * word_len);
        }
    }
#endif

    Stats pre_stats = stats;

    union {
        unsigned u;
        float f;
    } a;

    for (int i = 0; i < row_width; i++) {
        for (int j = 0; j < col_width; j++) {
            unsigned tmp = print_word(row + i, col + j * word_len);
            a.f = (float) tmp;
            write_word(a.u, row + i, col + j * word_len);
        }
    }

#ifdef DEBUG_FLOAT_XBAR
    printf("[Xbar %d Debug] \n", index);
    for (int i = 0; i < row_width; ++i) {
        for (int j = 0; j < col_width; ++j) {
            a.u = print_word(row + i, col + j * word_len);
            cout << (unsigned)a.f << " ";
            assert(equal(test[i][j], (unsigned)a.f));
        }
        cout << endl;
    }
#endif

    return stats - pre_stats;
}

Stats FloatXbar::float2int(int row, int col, int row_width, int col_width) {
#ifdef DEBUG_FLOAT_XBAR
    printf("[Xbar %d Debug] INT2FLOAT ", index);
    print_matrix(row, col, row_width, col_width);
    unsigned** test = new unsigned*[row_width + 5];
    for (int i = 0; i < row_width + 5; i++)
    test[i] = new unsigned[col_width + 5];
    for (int i = 0; i < row_width; i++) {
        for (int j = 0; j < col_width; j++) {
            test[i][j] = print_word(row + i, col + j * word_len);
        }
    }
#endif

    Stats pre_stats = stats;

    union {
        unsigned u;
        float f;
    } a;

    for (int i = 0; i < row_width; i++) {
        for (int j = 0; j < col_width; j++) {
            unsigned tmp = print_word(row + i, col + j * word_len);
            a.u = tmp;
            write_word((unsigned) a.f, row + i, col + j * word_len);
        }
    }

#ifdef DEBUG_FLOAT_XBAR
    printf("[Xbar %d Debug] \n", index);
    for (int i = 0; i < row_width; ++i) {
        for (int j = 0; j < col_width; ++j) {
            a.f = (float)print_word(row + i, col + j * word_len);
            cout << a.u << " ";
            assert(equal(test[i][j], a.u));
        }
        cout << endl;
    }
#endif

    return stats - pre_stats;
}
