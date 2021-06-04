/*
 * decoder.cpp
 *
 *  Created on: Nov 5, 2018
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

long instruction;

int decode() {

    bool wl_voltage[512], bl_voltage[512];
    int reg, bias;

    if ((instruction & 11) == 0) {  // MAGIC
        int wl = (instruction >> 53) & 0x1FF;
        int bl_in1 = (instruction >> 44) & 0x1FF;
        int bl_in2 = (instruction >> 35) & 0x1FF;
        int bl_out = (instruction >> 26) & 0x1FF;
        int parallelism = (instruction >> 18) & 0xFF;
        int direction = (instruction >> 17) & 0x1;
        if (direction == 0) {
            for (int i = wl; i < wl + parallelism; ++i) {
                wl_voltage[i] = 1;
            }
            bl_voltage[bl_in1] = bl_voltage[bl_in2] = bl_voltage[bl_out] = 1;
        } else {
            for (int i = wl; i < wl + parallelism; ++i) {
                bl_voltage[i] = 1;
            }
            wl_voltage[bl_in1] = wl_voltage[bl_in2] = wl_voltage[bl_out] = 1;
        }
    } else if ((instruction & 11) == 1) {   // READ
        int wl = (instruction >> 53) & 0x1FF;
        int bl = (instruction >> 44) & 0x1FF;
        int length = (instruction >> 38) & 0x3F;
        reg = (instruction >> 36) & 0x3;
        wl_voltage[wl] = 1;
        for (int i = bl; i < bl + length; ++i) {
            bl_voltage[i] = 1;
        }
    } else if ((instruction & 11) == 2) {    // WRITE
        int wl = (instruction >> 53) & 0x1FF;
        int bl = (instruction >> 44) & 0x1FF;
        int length = (instruction >> 38) & 0x3F;
        reg = (instruction >> 36) & 0x3;
        bias = (instruction >> 30) & 0x3;
        wl_voltage[wl] = 1;
        for (int i = bl; i < bl + length; ++i) {
            bl_voltage[i] = 1;
        }
    }
    return 0;
}
