#include "gestalt-solver.cpp"
#include <math.h>

/*
        Usage: if (get_2d_dist(x1, y1, x2, y2) - distance > eps) result = 1;

        If the difference between the calculated distance and the desired distance
        doesn't fit within eps, the test fails.
*/ 

int get_diff(float expected, float result) {
    float eps = 1e-3;
    if (fabs(expected - result) > eps) {
        printf("FAILED. EXPECTED: %f, ACTUAL: %f\n", expected, result);
        return 1;
    }
    return 0; 
}

int run_2d_dist_tests() {
    int failed = 0;
    failed |= get_diff(GestaltSolver::get_2d_dist(0, 1, 0, 0), 1);
    failed |= get_diff(GestaltSolver::get_2d_dist(1, 1, 0, 0), 1.414);
    failed |= get_diff(GestaltSolver::get_2d_dist(-69, 69, 69, -69), 195.161);
    failed |= get_diff(GestaltSolver::get_2d_dist(-200.52, 9023.4, 1.1111, 999.8934), 8026.040);
    failed |= get_diff(GestaltSolver::get_2d_dist(-200.52, 9023.4, 1.1111, 999.8934), 69);
    return failed;
}

int main() {
    int result = 0;
    result |= run_2d_dist_tests();
    return result;
}