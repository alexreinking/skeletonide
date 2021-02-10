#include "Halide.h"
using namespace Halide;

// The pipeline below is a single iteration of Zhang-Suen algorithm.
struct Skeletonide : public Generator<Skeletonide> {
    Input<Buffer<uint8_t>> input{"input", 2};
    Output<Buffer<uint8_t>> skel2{"skel2", 2};

    Func skel1{"skel1"};
    Func nbr{"nbr"};
    Func nbr_cnt{"nbr_cnt"};
    Func zero2one{"zero2one"};
    Func nbr2{"nbr2"};
    Func nbr_cnt2{"nbr_cnt2"};
    Func zero2one2{"zero2one2"};

    RDom j{0, 8};
    RDom t{0, 8};
    RDom j2{0, 8};
    RDom t2{0, 8};

    // offsets of neighbours of a given pixel (x, y)
    int n_x_idx[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    int n_y_idx[8] = {0, 1, 1, 1, 0, -1, -1, -1};

    Buffer<int> n_x_idx_buf{n_x_idx};
    Buffer<int> n_y_idx_buf{n_y_idx};

    Var x, y, k, x0, y0, xi, yi;

    void generate() {
        Expr W = input.width();
        Expr H = input.height();

        Func in_bounded = BoundaryConditions::repeat_edge(input);

        // fn to get the nth neighbours  of a pixel
        nbr(x, y, k) = in_bounded(x + n_x_idx_buf(k), y + n_y_idx_buf(k));

        // fn to count the number of non-zero neighbours
        nbr_cnt(x, y) = sum(select(nbr(x, y, j) > 0, 1, 0));

        // fn to count the number of transitions
        Expr zero2one_cond = nbr(x, y, t % 8) == 0 && nbr(x, y, (t + 1) % 8) == 255;
        zero2one(x, y) = sum(select(zero2one_cond, 1, 0));

        // step-1 of Zhang-Suen method
        Expr fst_cnd = (nbr_cnt(x, y) >= 2 && nbr_cnt(x, y) <= 6 && zero2one(x, y) == 1 &&
                        nbr(x, y, 0) * nbr(x, y, 2) * nbr(x, y, 4) == 0 &&
                        nbr(x, y, 0) * nbr(x, y, 2) * nbr(x, y, 6) == 0);

        skel1(x, y) = cast<uint8_t>(select(fst_cnd, 255, in_bounded(x, y)));

        // step-2 of Zhang-Suen method
        // operate on the array modified in step-1
        nbr2(x, y, k) = skel1(clamp(x + n_x_idx_buf(k), 0, W), clamp(y + n_y_idx_buf(k), 0, H));
        nbr_cnt2(x, y) = sum(select(nbr2(x, y, j2) > 0, 1, 0));

        Expr zero2one_cond2 = nbr2(x, y, t2 % 8) == 0 && nbr2(x, y, (t2 + 1) % 8) == 255;
        zero2one2(x, y) = sum(select(zero2one_cond2, 1, 0));

        Expr snd_cnd = (nbr_cnt2(x, y) >= 2 && nbr_cnt2(x, y) <= 6 && zero2one2(x, y) == 1 &&
                        nbr2(x, y, 0) * nbr2(x, y, 4) * nbr2(x, y, 6) == 0 &&
                        nbr2(x, y, 2) * nbr2(x, y, 4) * nbr2(x, y, 6) == 0);
        skel2(x, y) = cast<uint8_t>(select(snd_cnd, 255, skel1(x, y)));
    }

    void schedule() {
        if (get_target().has_gpu_feature()) {
            skel1.compute_root().gpu_tile(x, y, x0, y0, xi, yi, 8, 8);
            skel2.gpu_tile(x, y, x0, y0, xi, yi, 8, 8);
        } else {
            skel1.compute_root().vectorize(x, 8).parallel(y);
            skel2.compute_root().vectorize(x, 8).parallel(y);
        }
    }
};

HALIDE_REGISTER_GENERATOR(Skeletonide, skeletonide);
