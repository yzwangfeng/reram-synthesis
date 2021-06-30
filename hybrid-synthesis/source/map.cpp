/*
 * map.cpp
 *
 *  Created on: 2020/7/31
 *      Author: Feng Wang (yzwangfeng@pku.edu.cn)
 */

#include "map.h"
#include "primitive.h"

////////////////////////////////////////////////////////////////////////
///                         MAP SKELETON                             ///
////////////////////////////////////////////////////////////////////////

Map::Map(int bit_width_, int shift_len_)
        :
        Skeleton(MAP, bit_width_, shift_len_) {
}

Map::~Map() {
}

vector<Data*>& Map::append(Skeleton *s) {
    children.push_back(s);
    inputs.insert(inputs.end(), s->inputs.begin(), s->inputs.end());
    outputs.insert(outputs.end(), s->outputs.begin(), s->outputs.end());
    return outputs;
}

int Map::partition() {
    for (Skeleton *child : children) {
        if (child->st == PRIMITIVE) {
            if (((Primitive*) child)->pt == ADD || ((Primitive*) child)->pt == MUL) {
                child->hybrid = 2;
            } else {
                child->hybrid = 1;
            }
        } else {
            child->partition();
        }
        hybrid |= child->hybrid;
    }
    return hybrid;
}

void Map::reduce_stateful_logic() {
    for (Skeleton *child : children) {
        child->reduce_stateful_logic();
    }
    cycle_compute = children[0]->cycle_compute * 3;
}

void Map::assign_parallelism(int multiplier) {
    parallel_parent = multiplier;
    for (Skeleton *s : children) {
        s->assign_parallelism(children.size() * parallel_parent);
    }
}

void Map::allocate_bounding_box_digital() {
    for (Skeleton *child : children) {
        child->allocate_bounding_box_digital();
    }

//    bounding_box.xbar = children[0]->bounding_box.xbar;
//    int series = XBAR_LENGTH / children[0]->bounding_box.width;
//    parallel_own = XBAR_LENGTH / parallel_parent;
//    if (series * parallel_own < children.size()) {
//        bounding_box.width = children[0]->bounding_box.width * series;
//        bounding_box.height = children[0]->bounding_box.height * children.size() / series;
//    } else {
//        bounding_box.width = children[0]->bounding_box.width * children.size() / parallel_own;
//        bounding_box.height = children[0]->bounding_box.height * parallel_own;
//    }
//
//    for (int i = 1; i < children.size(); ++i) {
//        if (children[i]->bounding_box.xbar != bounding_box.xbar) {
//            /*
//             * TO DO: merge xbars
//             */
//            if (children[i - 1]->bounding_box.x + children[i - 1]->bounding_box.width
//                    + children[i]->bounding_box.width < bounding_box.width) {
//                children[i]->bounding_box.x = children[i - 1]->bounding_box.x
//                        + children[i - 1]->bounding_box.width;
//                children[i]->bounding_box.y = children[i - 1]->bounding_box.y;
//            } else {
//                children[i]->bounding_box.x = 0;
//                children[i]->bounding_box.y = children[i - 1]->bounding_box.y
//                        + children[i]->bounding_box.height;
//            }
//
//            delete children[i]->bounding_box.xbar;
//            children[i]->bounding_box.xbar = bounding_box.xbar;
//        }
//    }

    /*
     * for image convolution only
     */
    if (children[0]->st == PRIMITIVE) {
        bounding_box = Box(new Xbar(), -1, -1, children[0]->bounding_box.width * 3, 0,
                children[0]->bounding_box.height * 3);
    } else {
        bounding_box = Box(new Xbar(), -1, -1, children[0]->bounding_box.width * children.size() / 170, 0,
                children[0]->bounding_box.height * 170);
    }
}

void Map::reduce_mac() {

}

void Map::compare() {

}

void Map::allocate_bounding_box_analog() {
    /*
     * reduce、pipe与此相似
     */
    vector<Box*> analog_boxes;
    if (hybrid & 2 != 2) {  // 不能用模拟模式计算
        return;
    }
    for (Skeleton *child : children) {
        if (child->st != PRIMITIVE) {   // 只检查skeleton+primitive的组合
            child->allocate_bounding_box_analog();
        } else {
            /*
             * 枚举论文中五种可能的skeleton+primitive组合，得到初始bounding box大小
             */
            Box *analog_box = new Box();
            switch (((Primitive*) child)->pt) {
                case ADD:
                    analog_box->width = inputs.size();
                    analog_box->height = 2;
                    break;
                case MUL:
                    analog_box->width = analog_box->height = inputs.size();
                    break;
            }
            /*
             * 操作融合：检查pipe(mul, add)和pipe(add, add)，将其合并为一个bounding box，这里未给出代码
             */
            /*
             * 将超过阵列大小的bounding box拆分，这里未考虑宽度和高度均过大的情况
             */
            if (analog_box->width > XBAR_LENGTH) {  // 宽度过大，按列拆分即可
                while (analog_box->width > XBAR_LENGTH) {
                    analog_boxes.push_back(new Box(NULL, -1, -1, XBAR_LENGTH, XBAR_LENGTH, analog_box->height));
                    analog_box->width -= XBAR_LENGTH;
                }
                analog_boxes.push_back(new Box(NULL, -1, -1, analog_box->width, analog_box->width, analog_box->height));
            }
            if (analog_box->height > XBAR_LENGTH) { // 高度过大，按行拆分，最后额外增加一个阵列累加部分和，这里未考虑需要加法树的情况
                int adder_num = 1;
                while (analog_box->height > XBAR_LENGTH) {
                    analog_boxes.push_back(new Box(NULL, -1, -1, analog_box->width, analog_box->width, XBAR_LENGTH));
                    analog_box->height -= XBAR_LENGTH;
                    ++adder_num;
                }
                analog_boxes.push_back(new Box(NULL, -1, -1, analog_box->width, analog_box->width, analog_box->height));
                analog_boxes.push_back(new Box(NULL, -1, -1, analog_box->width, analog_box->width, adder_num));
            }
        }
    }
    /*
     * 根据优化目标选择串行映射或并行映射，将analog_boxes里较小的bounding box合并在一个阵列中，这里未给出实现
     */
}

void Map::simulate() {
    /*
     * TO DO: output statistics
     */
}
