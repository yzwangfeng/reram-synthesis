/*
 * Crossbar.cpp
 *
 *  Created on: 2016/4/1
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#include "BaseXbar.h"

//#define DEBUG_BASE_XBAR

Redundance::Redundance(int index_, string name_, int array_size_)
        :
        index(index_), name(name_), array_size(array_size_), max_redundance(0) {
    status = new LineStatus[array_size];
    for (int i = 0; i < array_size; ++i) {
        status[i] = CLEAN;
    }
}

Redundance::~Redundance() {
    delete[] status;
}

void Redundance::set_status(LineStatus s, int start, int length, bool strict) {
    assert(start >= 0 && start + length <= array_size);

    for (int i = 0; i < length; ++i) {
        if (status[start + i] != FIXED || strict) {
            status[start + i] = s;
        }
    }
}

void Redundance::set_status(LineStatus s, vector<int> pos, bool strict) {
    for (int i = 0; i < (int) pos.size(); ++i) {
        set_status(s, pos[i], 1, strict);
    }
}

int Redundance::find_clean(int length) {
    for (int i = array_size - 1; i >= length - 1; --i) {
        bool found = true;
        for (int j = i; j > i - length; --j) {
            if (status[j] != CLEAN) {
                found = false;
                break;
            }
        }
        if (found) {
            for (int j = i; j > i - length; --j) {
                status[j] = TMP;
            }
            max_redundance = max(max_redundance, array_size - i + length - 1);
            return i - length + 1;
        }
    }
    assert(false);  // if failed, expand the array size
    return -1;
}

int Redundance::find_unused(int length) {
    for (int i = 0; i <= array_size - length; ++i) {
        bool found = true;
        for (int j = i; j < i + length; ++j) {
            if (status[j] != CLEAN && status[j] != DIRTY) {
                found = false;
                break;
            }
        }
        if (found) {
            return i;
        }
    }
    assert(false);  // if failed, expand the array size
    return -1;
}

vector<int> Redundance::find_all(LineStatus s) {
    vector<int> all;
    for (int i = array_size - 1; i >= 0; --i) {
        if (status[i] == s) {
            all.push_back(i);
        }
    }
    return all;
}

void Redundance::print_redundance() {
    cout << "[Xbar " << index << " Summary] Max redundant " << name << "(s): " << max_redundance << endl;
}

BaseXbar::BaseXbar(int index_, int word_len_, int array_size_)
        :
        index(index_), word_len(word_len_), array_size(array_size_), stats(0, 0, 0, index_) {
    array = new bool*[array_size];
    for (int i = 0; i < array_size; ++i) {
        array[i] = new bool[array_size];
        for (int j = 0; j < array_size; ++j) {
            array[i][j] = true;
        }
    }
    redundance[ROW] = new Redundance(index, "row", array_size);
    redundance[COL] = new Redundance(index, "column", array_size);
}

BaseXbar::~BaseXbar() {
    for (int i = 0; i < array_size; ++i) {
        delete[] array[i];
    }
    delete[] array;
    redundance[ROW]->~Redundance();
    redundance[COL]->~Redundance();
}

void BaseXbar::set_unused(Direction row_or_col, int start, int length) {
    redundance[row_or_col]->set_status(DIRTY, start, length, true);
    clean_redundance();
}

Stats BaseXbar::read_word(WORD &res, int row, int col, bool aligned) {
    if (aligned) {
        assert(col % word_len == 0);
    }

    Stats pre_stats = stats;
    res = 0;
    bool buffer[word_len];
    read(buffer, row, col, word_len, true);
    for (int i = 0; i < word_len; ++i) {
        res |= ((WORD) buffer[i]) << i;
    }
    return stats - pre_stats;
}

Stats BaseXbar::write_word(WORD word, int row, int col, bool aligned) {
    if (aligned)
        assert(col % word_len == 0);

    Stats pre_stats = stats;
    bool *buffer = new bool[word_len];
    for (int i = 0; i < word_len; ++i) {
        buffer[i] = (word >> i) & 1;
    }
    write(buffer, row, col, word_len);

    redundance[ROW]->set_status(FIXED, row);
    redundance[COL]->set_status(FIXED, col, word_len);

    assert(equal(word, print_word(row, col)));

    return stats - pre_stats;
}

void BaseXbar::construct_random_data(int row, int col) {
    srand((unsigned) time(0));

    bool *buffer = new bool[word_len * col];
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < word_len * col; ++j) {
            buffer[j] = rand() & 1;
        }
        write(buffer, i, 0, word_len * col);
    }
    delete[] buffer;

    redundance[ROW]->set_status(FIXED, 0, row);
    redundance[COL]->set_status(FIXED, 0, word_len * col);
}

void BaseXbar::print_stats_redundance() {
    stats.print_stats();
    redundance[ROW]->print_redundance();
    redundance[COL]->print_redundance();
}

WORD BaseXbar::print_word(int row, int col, bool output, bool aligned) {
    if (aligned)
        assert(col % word_len == 0);

    WORD res = 0;
    bool buffer[word_len];
    read(buffer, row, col, word_len, true);
    for (int i = 0; i < word_len; ++i) {
        res |= ((WORD) buffer[i]) << i;
    }
    if (output) {
        printf("[Xbar %d Debug] Array(%d, %d) = %lu\n", index, row, col, res);
    }
    return res;
}

void BaseXbar::print_array(bool hex) {
    cout << "[Xbar " << index << " Information]" << endl;
    for (int i = 0; i < array_size; ++i) {
        if (hex) {
            for (int j = 0; j < array_size; j += 4) {
                int num = 0;
                for (int k = 0; k < 4; ++k) {
                    num += array[i][j + 3 - k] << k;
                }
                cout << hex << num << ' ';
            }
        } else {
            for (int j = 0; j < array_size; ++j) {
                cout << array[i][j];
            }
        }
        cout << endl;
    }
}

Stats BaseXbar::read(bool *data, int addr, int start, int length, bool debug, Direction row_or_col) {
    assert(addr >= 0 && addr < array_size);
    assert(start >= 0 && start < array_size);
    assert(length >= 0 && length <= array_size);

#ifdef DEBUG_BASE_XBAR
    if (!debug) {
        if (row_or_col == ROW) {
            printf("[Xbar %d Cycle %d] READ (%d, %d) to (%d, %d)\n", index, stats.cycle, addr, start, addr,
                    start + length - 1);
        } else {
            printf("[Xbar %d Cycle %d] READ (%d, %d) to (%d, %d)\n", index, stats.cycle, start, addr,
                    start + length - 1, addr);
        }
    }
#endif

    if (row_or_col == ROW) {
        for (int i = 0; i < length; ++i) {
            data[i] = array[addr][start + i];
        }
    } else {
        for (int i = 0; i < length; ++i) {
            data[i] = array[start + i][addr];
        }
    }

    if (!debug) {
        stats += Stats(TIME[READ], ENERGY[READ], 1);
        return Stats(TIME[READ], ENERGY[READ], 1);
    }
    return Stats(0, 0, 0);
}

Stats BaseXbar::write(bool *data, int addr, int start, int length, Direction row_or_col) {
    assert(addr >= 0 && addr < array_size);
    assert(data != NULL);
    assert(start >= 0 && start < array_size);
    assert(length >= 0 && length <= array_size);

#ifdef DEBUG_BASE_XBAR
    if (row_or_col == ROW) {
        printf("[Xbar %d Cycle %d] WRITE (%d, %d) to (%d, %d)\n", index, stats.cycle, addr, start, addr,
                start + length - 1);
    } else {
        printf("[Xbar %d Cycle %d] WRITE (%d, %d) to (%d, %d)\n", index, stats.cycle, start, addr,
                start + length - 1, addr);
    }
#endif

    int set_num = 0, reset_num = 0;
    if (row_or_col == ROW) {
        for (int i = 0; i < length; ++i) {
            set_num += array[addr][start + i] && !data[i];
            reset_num += !array[addr][start + i] && data[i];
            array[addr][start + i] = data[i];
        }
    } else {
        for (int i = 0; i < length; ++i) {
            set_num += array[start + i][addr] && !data[i];
            reset_num += !array[start + i][addr] && data[i];
            array[start + i][addr] = data[i];
        }
    }

    stats += Stats(TIME[SET], ENERGY[SET], 1) * set_num + Stats(TIME[RESET], ENERGY[RESET], 1) * reset_num;
    return Stats(TIME[SET], ENERGY[SET], 1) * set_num + Stats(TIME[RESET], ENERGY[RESET], 1) * reset_num;
}

Stats BaseXbar::write(bool value, int row, int col, Direction row_or_col) {
    assert(row >= 0 && row < array_size);
    assert(col >= 0 && col < array_size);

#ifdef DEBUG_BASE_XBAR
    if (row_or_col == ROW) {
        printf("[Xbar %d Cycle %d] WRITE (%d, %d) = %d\n", index, stats.cycle, row, col, value);
    } else {
        printf("[Xbar %d Cycle %d] WRITE (%d, %d) = %d\n", index, stats.cycle, col, row, value);
    }
#endif

    if (row_or_col == ROW) {
        array[row][col] = value;
    } else {
        array[col][row] = value;
    }

    PrimitiveType ope = value ? RESET : SET;
    stats += Stats(TIME[ope], ENERGY[ope], 1);
    return Stats(TIME[ope], ENERGY[ope], 1);
}

Stats BaseXbar::set(bool value, int col, int length, int start, int stride, int parallelism,
        Direction row_or_col) {
    assert(col >= 0 && col < array_size);
    assert(length >= 0 && length <= array_size);
    assert(start >= 0 && start < array_size);
    assert(start + stride * (parallelism - 1) >= 0 && start + stride * (parallelism - 1) < array_size);

#ifdef DEBUG_BASE_XBAR
    printf("[Xbar %d Cycle %d] %s %s %d to %d, activated %s: %d to %d, stride: %d\n", index, stats.cycle,
            value ? "RESET" : "SET", row_or_col == ROW ? "column" : "row", col, col + length,
            row_or_col == ROW ? "row" : "column", start, start + stride * (parallelism - 1), stride);
#endif

    int modified = 0;
    if (row_or_col == ROW) {
        for (int i = start, j = 0; j < parallelism; i += stride, ++j) {
            for (int k = col; k < col + length; ++k) {
                modified += array[i][k] != value;
                array[i][k] = value;
            }
        }
    } else {
        for (int i = start, j = 0; j < parallelism; i += stride, ++j) {
            for (int k = col; k < col + length; ++k) {
                modified += array[k][i] != value;
                array[k][i] = value;
            }
        }
    }

    PrimitiveType ope = value ? RESET : SET;
    stats += Stats(TIME[ope], 0, 1) + Stats(0, ENERGY[ope], 0) * modified;
    return Stats(TIME[ope], 0, 1) + Stats(0, ENERGY[ope], 0) * modified;
}

Stats BaseXbar::execute(PrimitiveType type, int operand1, int operand2, int res, int start, int stride,
        int parallelism, Direction row_or_col) {
    assert(operand1 >= 0 && operand1 < array_size);
    assert(operand2 >= -1 && operand2 < array_size);
    assert(res >= 0 && res < array_size);
    assert(start >= 0 && start < array_size);
    assert(start + stride * (parallelism - 1) >= 0 && start + stride * (parallelism - 1) < array_size);

#ifdef DEBUG_BASE_XBAR
    printf("[Xbar %d Cycle %d] %s operand %s: %d & %d, result %s: %d, activated %s: %d to %d, stride: %d\n",
            index, stats.cycle, type == AND ? "AND" : "NAND", row_or_col == ROW ? "column" : "row", operand1,
            operand2, row_or_col == ROW ? "column" : "row", res, row_or_col == ROW ? "row" : "column", start,
            start + stride * (parallelism - 1), stride);
#endif

    if (row_or_col == ROW) {
        for (int i = start, j = 0; j < parallelism; i += stride, ++j) {
            int value = operand2 == -1 ? 1 : array[i][operand2];
            if (type == AND) {
                array[i][res] &= array[i][operand1] & value;
            } else {
                array[i][res] &= !(array[i][operand1] & value);
            }
        }
    } else {
        for (int i = start, j = 0; j < parallelism; i += stride, ++j) {
            int value = operand2 == -1 ? 1 : array[operand2][i];
            if (type == AND) {
                array[res][i] &= array[operand1][i] & value;
            } else {
                array[res][i] &= !(array[operand1][i] & value);
            }
        }
    }

    stats += Stats(TIME[type], ENERGY[type], 1) * parallelism;
    return Stats(TIME[type], ENERGY[type], 1) * parallelism;
}

bool BaseXbar::equal(WORD x, WORD y) {
    if (word_len == 64) {
        return x == y;
    }
    return (x & ((1 << word_len) - 1)) == (y & ((1 << word_len) - 1));
}

void BaseXbar::clean_redundance() {
    vector<int> dirty_row = redundance[ROW]->find_all(DIRTY);
    for (int i = 0; i < (int) dirty_row.size(); ++i) {
        set(true, 0, array_size, dirty_row[i], 1, 1);
    }
    redundance[ROW]->set_status(CLEAN, dirty_row);

    vector<int> dirty_col = redundance[COL]->find_all(DIRTY);
    for (int i = 0; i < (int) dirty_col.size(); ++i) {
        set(true, 0, array_size, dirty_col[i], 1, 1, COL);
    }
    redundance[COL]->set_status(CLEAN, dirty_col);
}
