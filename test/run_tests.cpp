#include <iostream>
#include <cstdint>

#include <opencv2/opencv.hpp>

#include "../src/new_latch.h"

int test_LATCH_frobenius_norm_squared();

int main()
{
    int n_failures = 0;
    int n_tests    = 0;
    std::cout << "Beginning tests..." << std::endl;

    /*Descriptor tests*/

    /*LATCH tests*/
    n_failures += test_LATCH_frobenius_norm_squared();
    ++n_tests;

    std::cout << "Tests complete." << std::endl;
    std::cout << n_tests - n_failures << " of "
        << n_tests << " passed." << std::endl;
    std::cout << n_failures << " of " << n_tests << " failed." << std::endl;
    return 0;
}

int test_LATCH_frobenius_norm_squared()
{
    int ct = 0;  // no. of test failures

    // LATCH object
    LATCH* latch = new LATCH();

    // cv::Mat object (0-48 in (row, col) order)
    cv::Mat im = cv::Mat::zeros(cv::Size(7,7), CV_64FC1);
    int stride = im.step;
    uint8_t *data = im.data;
    for (size_t i = 0; i < 7; ++i) {
        for (size_t j = 0; j < 7; ++j) {
            data[i*stride + j] = 7*i + j;
        }
    }

    // perform check and output results
    // (3, 3) is the center pixel
    if (!(latch->frobenius_norm_squared(im, 3, 3) == 38024)) {
        ++ct;
        std::cout << "LATCH::frobenius_norm_squared() failed" << std::endl;
        std::cout << "(0-48 in (row, col) order)" << std::endl;
    }

    return ct;
}
