/*
 * Main.cpp
 *
 */

#include <dirent.h>

#include "Farm.h"
#include "Map.h"
#include "Pipe.h"
#include "Primitive.h"
#include "Reduce.h"
#include "Skeleton.h"
using namespace std;

void image_convolution(int bit_width) {
    /*
     * initialize input data
     */
    const int image_size = 100;
    const int filter_size = 3;
    Data *image[image_size][image_size];
    for (int i = 0; i < image_size; ++i) {
        for (int j = 0; j < image_size; ++j) {
            image[i][j] = new Data();
        }
    }
    Data *filter[filter_size][filter_size];
    for (int i = 0; i < filter_size; ++i) {
        for (int j = 0; j < filter_size; ++j) {
            filter[i][j] = new Data();
        }
    }

    /*
     * represent the application with skeletons (bottom up)
     */
    Skeleton *convolution = new Map(bit_width);
    for (int i = 0; i <= image_size - filter_size; ++i) {
        for (int j = 0; j <= image_size - filter_size; ++j) {
            Skeleton *mac = new Pipe(bit_width);
            Skeleton *muls = new Map(bit_width);
            for (int k = 0; k < filter_size; ++k) {
                for (int l = 0; l < filter_size; ++l) {
                    muls->append(new Primitive(bit_width, MUL, image[i + k][j + l], filter[k][l]));
                }
            }
            mac->append(muls);
            mac->append(new Reduce(bit_width, new Primitive(bit_width, ADD, muls->outputs)));
            convolution->append(mac);
        }
    }

    /*
     * synthesize
     */
    convolution->synthesize();
    cout << convolution->cycle_compute << endl << convolution->bounding_box;

    /*
     * simulate
     */
    for (int i = 0; i < image_size; ++i) {
        for (int j = 0; j < image_size; ++j) {
            image[i][j]->set_value(1);
            image[i][j]->set_position(false);
        }
    }
    for (int i = 0; i < filter_size; ++i) {
        for (int j = 0; j < filter_size; ++j) {
            filter[i][j]->set_value(1);
            filter[i][j]->set_position(false);
        }
    }
    convolution->simulate();

    /*
     * clean
     */
    delete convolution;
    for (int i = 0; i < image_size; ++i) {
        for (int j = 0; j < image_size; ++j) {
            delete image[i][j];
        }
    }
    for (int i = 0; i < filter_size; ++i) {
        for (int j = 0; j < filter_size; ++j) {
            delete filter[i][j];
        }
    }
}

void sha_3(int bit_width) {
    /*
     * initialize input data
     */
    Data *A[5][5];
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            A[i][j] = new Data();
        }
    }
    Data *RC = new Data();

    /*
     * represent the application with skeletons (bottom up)
     */
    Skeleton *theta1 = new Map(bit_width);
    for (int i = 0; i < 5; ++i) {
        Skeleton *xors = new Pipe(bit_width);
        vector<Data*> outputs = xors->append(new Primitive(bit_width, XOR, A[i][0], A[i][1]));
        for (int j = 2; j < 5; ++j) {
            outputs = xors->append(new Primitive(bit_width, XOR, outputs[0], A[i][j]));
        }
        theta1->append(xors);
    }

    Skeleton *theta2 = new Map(bit_width);
    for (int i = 0; i < 5; ++i) {
        theta2->append(new Primitive(bit_width, XOR, (*theta1)[(i + 4) % 5], (*theta1)[(i + 1) % 5]));
    }

    Skeleton *theta3 = new Map(bit_width);
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            theta3->append(new Primitive(bit_width, XOR, A[i][j], (*theta2)[i]));
        }
    }

    Skeleton *chi = new Map(bit_width);
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            Skeleton *naxs = new Pipe(bit_width);
            vector<Data*> outputs = naxs->append(
                    new Primitive(bit_width, NOT, (*theta3)[(i + 1) % 5 * 5 + j]));
            outputs = naxs->append(new Primitive(bit_width, AND, outputs[0], (*theta3)[(i + 2) % 5 * 5 + j]));
            naxs->append(new Primitive(bit_width, XOR, outputs[0], (*theta3)[i * 5 + j]));
            chi->append(naxs);
        }
    }

    Skeleton *iota = new Primitive(bit_width, XOR, (*chi)[0], RC);

    Skeleton *sha_3 = new Pipe(24);
    sha_3->append(theta1);
    sha_3->append(theta2);
    sha_3->append(theta3);
    sha_3->append(chi);
    sha_3->append(iota);

    /*
     * synthesize
     */
    sha_3->synthesize();

    /*
     * simulate
     */
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            A[i][j]->set_value(1);
            A[i][j]->set_position(false);
        }
    }
    RC->set_value(1);
    RC->set_position(false);
    sha_3->simulate();

    /*
     * clean
     */
    delete sha_3;
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            delete A[i][j];
        }
    }
    delete RC;
}

void get_file_name(string path, vector<string> &files) {
    struct dirent *ptr;
    DIR *dir = opendir(path.c_str());
    while ((ptr = readdir(dir)) != NULL) {
        if (ptr->d_name[0] != '.') {  // skip . and ..
            files.push_back(ptr->d_name);
        }
    }
    closedir(dir);
}

int main() {
    image_convolution(8);
    //    sha_3(64);
    return 0;
}
