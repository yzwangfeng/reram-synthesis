#include "Skeleton.h"

struct Noc {
    vector<Skeleton*> skeleton;
    int area;
    int cover;
    double utils;

    Noc() {
        area = 0;
        cover = 0;
        utils = 0.0;
    }

    Noc(int Area) {
        skeleton.clear();
        area = Area;
        cover = 0;
        utils = 0.0;
    }

    void add_skel(Skeleton *skel) {
        cover += skel->bounding_box.width * skel->bounding_box.height;
        utils = (double) cover;  // other definition
        skeleton.push_back(skel);
    }

    void delete_skel(int k) {
        cover -= skeleton[k]->bounding_box.width * skeleton[k]->bounding_box.height;
        utils = (double) cover;  // other definition
        skeleton.erase(skeleton.begin() + k);
    }

    bool check_avil(Skeleton *skel, int x, int y) {
        if (x + skel->bounding_box.width > XBAR_LENGTH || y + skel->bounding_box.height > XBAR_LENGTH)
            return false;
        for (int i = skeleton.size() - 1; ~i; i--) {
            Skeleton *skel1 = skeleton[i];
            if (!(x + skel->bounding_box.width <= skel1->bounding_box.x
                    || x >= skel1->bounding_box.width + skel1->bounding_box.x
                    || y + skel->bounding_box.height <= skel1->bounding_box.y
                    || y >= skel1->bounding_box.height + skel1->bounding_box.y))
                return false;
        }
        return true;
    }
};
