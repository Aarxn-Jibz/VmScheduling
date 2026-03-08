#include <string.h>
void score(double * input, double * output) {
    double var0[2];
    if (input[1] <= 6.5) {
        memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
    } else {
        if (input[0] <= 13.0) {
            if (input[1] <= 10.5) {
                memcpy(var0, (double[]){0.1744186046511628, 0.8255813953488372}, 2 * sizeof(double));
            } else {
                memcpy(var0, (double[]){0.011142061281337047, 0.9888579387186629}, 2 * sizeof(double));
            }
        } else {
            if (input[1] <= 15.5) {
                memcpy(var0, (double[]){1.0, 0.0}, 2 * sizeof(double));
            } else {
                memcpy(var0, (double[]){0.13513513513513514, 0.8648648648648649}, 2 * sizeof(double));
            }
        }
    }
    memcpy(output, var0, 2 * sizeof(double));
}
