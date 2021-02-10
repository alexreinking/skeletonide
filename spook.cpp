// A example to illustrate the usage of skeletonide.
// We load an in image and call the pipeline in a loop.
// TODO Check for completion flags

#include "HalideBuffer.h"
#include "halide_image_io.h"
#include "skeletonide.h"
#include <chrono>
#include <cstdio>

#define ITERS 100

using namespace Halide;
using namespace Halide::Tools;

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <input> <output>\n", argv[0]);
        return 1;
    }

    Runtime::Buffer<uint8_t> input = Tools::load_and_convert_image(argv[1]);
    Runtime::Buffer<uint8_t> output(input.width(), input.height());
    auto orig_input = input;

    auto t1 = std::chrono::high_resolution_clock::now();

    for (int j = 0; j < ITERS; j++) {
        for (int i = 0; i < 29; i++) {// TODO remove this hardcoded limit.
            skel(input, output);
            input = output;
        }
        input = orig_input;
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    printf("Mean time to skeletonize: %fms\n", dur / (float) ITERS);

    Tools::convert_and_save_image(output, argv[2]);
    printf("Image saved\n");

    return 0;
}
