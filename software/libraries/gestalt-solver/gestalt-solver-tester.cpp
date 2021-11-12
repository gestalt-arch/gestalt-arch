#include "gestalt-solver.cpp"

/*
        Usage: if (get_2d_dist(x1, y1, x2, y2) - distance > eps) result = 1;

        If the difference between the calculated distance and the desired distance
        doesn't fit within eps, the test fails.
*/ 
int main() {
    int result = 0;

    float eps = 1e-3;

    if (get_2d_dist(0, 1, 0, 0) - 1 > eps) result = 1;
    if (get_2d_dist(1, 1, 0, 0) - 1.414 > eps) result = 1;
    if (get_2d_dist(-69, 69, 69, -69) - 195.161 > eps) result = 1;
    if (get_2d_dist(-200.52, 9023.4, 1.1111, 999.8934) - 8026.040 > eps) result = 1;
    //if (get_2d_dist(-200.52, 9023.4, 1.1111, 999.8934) - 69 > eps) result = 1;

    return result;
}