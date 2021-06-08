/*
 * Generator.cpp
 *
 *  Created on: Dec 13, 2018
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#include <iostream>
#include <fstream>
using namespace std;

#define N 8

bool NOR(bool a, bool b = false) {
    return !(a | b);
}

void int2bool(int ia, bool ba[N]) {
    for (int i = 0; i < N; ++i) {
        ba[i] = ia & 1;
        ia >>= 1;
    }
}

int bool2int(bool ba[N]) {
    int ia = 0;
    for (int i = N - 1; i >= 0; --i) {
        ia = (ia << 1) + ba[i];
    }
    return ia;
}

void full_adder(bool a, bool b, bool ci, bool sum[2], int i, int j, ofstream& fout) {
    bool r1 = NOR(a, b);
    fout << "nor(r1" << i * 100 + j << ", ps" << i * 100 + j << ", res" << i * 10000 + j * 100 + j << ");\n";
    bool r2 = NOR(a, r1);
    fout << "nor(r2" << i * 100 + j << ", ps" << i * 100 + j << ", r1" << i * 100 + j << ");\n";
    bool r3 = NOR(b, r1);
    fout << "nor(r3" << i * 100 + j << ", res" << i * 10000 + j * 100 + j << ", r1" << i * 100 + j << ");\n";
    bool r4 = NOR(r2, r3);
    fout << "nor(r4" << i * 100 + j << ", r2" << i * 100 + j << ", r3" << i * 100 + j << ");\n";
    bool r5 = NOR(r4, ci);
    fout << "nor(r5" << i * 100 + j << ", r4" << i * 100 + j << ", c" << i * 100 + j << ");\n";
    bool r6 = NOR(r4, r5);
    fout << "nor(r6" << i * 100 + j << ", r4" << i * 100 + j << ", r5" << i * 100 + j << ");\n";
    bool r7 = NOR(r5, ci);
    fout << "nor(r7" << i * 100 + j << ", r5" << i * 100 + j << ", c" << i * 100 + j << ");\n";
    sum[0] = NOR(r6, r7);
    fout << "nor(s0" << i * 100 + j << ", r6" << i * 100 + j << ", r7" << i * 100 + j << ");\n";
    sum[1] = NOR(r1, r5);
    fout << "nor(s1" << i * 100 + j << ", r1" << i * 100 + j << ", r5" << i * 100 + j << ");\n";
}

void generate_addition() {
    char file[256];
    sprintf(file, "add_%d.v", N);
    ofstream fout(file, ios::out);
    fout << "module addl_" << N << "(";

    string name[8] = { "c", "r1", "r2", "r3", "r4", "r5", "r6", "r7" };
    fout << hex;
    for (int i = 0; i < N; ++i) {
        fout << " bx" << i << ", by" << i << ", bz" << i << ", ";
    }
    for (int i = 0; i <= N; ++i) {
        for (int k = 0; k < 8; ++k) {
            fout << name[k] << '_' << i;
            if (i != N || k != 7) {
                fout << ", ";
            }
        }
    }
    fout << ");\n";

    fout << "input";
    for (int i = 0; i < N; ++i) {
        fout << " bx" << i << ", by" << i;
        if (i != N - 1) {
            fout << ",";
        }
    }
    fout << ";\n";

    fout << "output";
    for (int i = 0; i < N; ++i) {
        fout << " bz" << i;
        if (i != N - 1) {
            fout << ",";
        }
    }
    fout << ";\n";

    fout << "wire ";
    for (int i = 0; i <= N; ++i) {
        for (int k = 0; k < 8; ++k) {
            fout << name[k] << '_' << i;
            if (i != N || k != 7) {
                fout << ", ";
            }
        }
    }
    fout << ";\n";

    fout << "assign c_0 = 0;\n";
    for (int i = 0; i < N; ++i) {
        fout << "nor(r1_" << i << ", bx" << i << ", by" << i << ");\n";
        fout << "nor(r2_" << i << ", bx" << i << ", r1_" << i << ");\n";
        fout << "nor(r3_" << i << ", by" << i << ", r1_" << i << ");\n";
        fout << "nor(r4_" << i << ", r2_" << i << ", r3_" << i << ");\n";
        fout << "nor(r5_" << i << ", r4_" << i << ", c_" << i << ");\n";
        fout << "nor(r6_" << i << ", r4_" << i << ", r5_" << i << ");\n";
        fout << "nor(r7_" << i << ", r5_" << i << ", c_" << i << ");\n";
        fout << "nor(bz" << i << ", r6_" << i << ", r7_" << i << ");\n";
        fout << "nor(c_" << i + 1 << ", r1_" << i << ", r5_" << i << ");\n";
    }

    fout << "endmodule" << endl;
}

void generate_multiplication() {
    char file[256];
    sprintf(file, "mul_%d.v", N);
    ofstream fout(file, ios::out);
    fout << "module mul_" << N << "(";

    string name[13] = { "nx", "ny", "ps", "s0", "s1", "c", "r1", "r2", "r3", "r4", "r5", "r6", "r7" };
    fout << hex;
    for (int i = 0; i <= N; ++i) {
        for (int j = 0; j <= N; ++j) {
            for (int k = 0; k < N; ++k) {
                fout << "res" << i * 10000 + j * 100 + k << ", ";
            }
        }
    }
    for (int i = 0; i < N; ++i) {
        fout << " bx" << i << ", by" << i << ", bz" << i << ", ";
        for (int j = 0; j < N; ++j) {
            for (int k = 0; k < 13; ++k) {
                fout << name[k] << i * 100 + j;
                if (i != N - 1 || j != N - 1 || k != 12) {
                    fout << ", ";
                }
            }
        }
    }
    fout << ");\n";

    fout << "input";
    for (int i = 0; i < N; ++i) {
        fout << " bx" << i << ", by" << i;
        if (i != N - 1) {
            fout << ",";
        }
    }
    fout << ";\n";

    fout << "output";
    for (int i = 0; i < N; ++i) {
        fout << " bz" << i;
        if (i != N - 1) {
            fout << ",";
        }
    }
    fout << ";\n";

    fout << "wire ";
    for (int i = 0; i <= N; ++i) {
        for (int j = 0; j <= N; ++j) {
            for (int k = 0; k < N; ++k) {
                fout << "res" << i * 10000 + j * 100 + k << ", ";
            }
        }
    }
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < 13; ++j) {
            for (int k = 0; k < N; ++k) {
                fout << name[j] << i * 100 + k;
                if (i != N - 1 || j != 12 || k != N - 1) {
                    fout << ", ";
                }
            }
        }
    }
    fout << ";\n";

    int ix = 0, iy = 0;
//    int iz = 0;
    bool bx[N], by[N], bz[N];

//    cin >> ix >> iy;
    int2bool(ix, bx);
    int2bool(iy, by);

    for (int i = 0; i < N; ++i) {
        bz[i] = NOR(NOR(bx[i]), NOR(by[0]));
        fout << "not(nx" << i << ", bx" << i << ");\n";
        fout << "not(ny" << i << ", by" << 0 << ");\n";
        fout << "nor(res" << 10100 + i << ", nx" << i << ", ny" << i << ");\n";
    }

    for (int i = 1; i < N; ++i) {
        bool ci = false;
        fout << "assign c" << i * 100 << " = 0;\n";
        for (int j = i; j < N; ++j) {
            bool partial_sum = NOR(NOR(bx[j - i]), NOR(by[i]));
            fout << "not(nx" << i * 100 + j << ", bx" << j - i << ");\n";
            fout << "not(ny" << i * 100 + j << ", by" << i << ");\n";
            fout << "nor(ps" << i * 100 + j << ", nx" << i * 100 + j << ", ny" << i * 100 + j << ");\n";

            bool sum[2];
            full_adder(partial_sum, bz[j], ci, sum, i, j, fout);

            bz[j] = NOR(NOR(sum[0]));
            for (int k = 0; k < N; ++k) {
                if (k != j) {
                    fout << "buf(res" << i * 10000 + (j + 1) * 100 + k << ", res" << i * 10000 + j * 100 + k
                            << ");\n";
                } else {
                    fout << "buf(res" << i * 10000 + (j + 1) * 100 + k << ", s0" << i * 100 + j << ");\n";
                }
            }

            ci = NOR(NOR(sum[1]));
            if (j != N - 1) {
                fout << "buf(c" << i * 100 + j + 1 << ", s1" << i * 100 + j << ");\n";
            }
        }
        for (int k = 0; k < N; ++k) {
            fout << "buf(res" << (i + 1) * 10000 + (i + 1) * 100 + k << ", res" << i * 10000 + N * 100 + k
                    << ");\n";
        }
    }

    for (int i = 0; i < N; ++i) {
        fout << "buf(bz" << i << ", res" << N * 10000 + N * 100 + i << ");\n";
    }

//    iz = bool2int(bz);
//    cout << iz << endl;

    fout << "endmodule" << endl;
}

int main() {
    generate_addition();
//    generate_multiplication();
}
