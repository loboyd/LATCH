#include <iostream>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

#include "latch.h"

/*This file will ultimately contain some testing code.*/

int main()
{
    // read in images
    std::string imfile1("../images/right.jpg");
    std::string imfile2("../images/left.jpg");
    cv::Mat im1 = cv::imread(imfile1, cv::IMREAD_COLOR);
    cv::Mat im2 = cv::imread(imfile2, cv::IMREAD_COLOR);

    // check images actually have data
    if (right.empty() || left.empty()) {
        std::cout << "Could not open or find images" << std::endl;
        return -1;
    } else {
        // convert to grayscale
        cv::cvtColor(im1, im1, cv::COLOR_BGR2GRAY);
        cv::cvtColor(im2, im2, cv::COLOR_BGR2GRAY);
    }

    std::cout << "All done." << std::endl;
    return 0;
}
