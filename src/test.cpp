#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "latch.h"

int main(int argc, char** argv) {
    // get an image file
    std::string imfile("../images/right.jpg");
    if (argc > 2) {
        std::cout << "Usage: main <image_path>" << std::endl;
        return -1;
    } else if (argc > 1) {
        imfile = argv[1];
    }

    // read in image
    cv::Mat im, im_gray;
    im = cv::imread(imfile, cv::IMREAD_COLOR);
    if (im.empty()) {
        std::cout << "Could not open or find the image" << std::endl;
        return -1;
    } else {
        // convert to grayscale
        cv::cvtColor(im, im_gray, cv::COLOR_BGR2GRAY);
    }

    // create LATCH
    LATCH* my_latch = new LATCH();
    std::cout << "LATCH created" << std::endl;

    // detect keypoints
    std::vector<cv::KeyPoint> keypoints = my_latch->detect(im_gray);
    std::cout << keypoints.size() << " keypoints detected" << std::endl;

    // describe keypoints
    std::vector<Descriptor512> des = my_latch->describe(im_gray, keypoints);
    std::cout << des.size() << " descriptors computed" << std::endl;

    drawKeypoints(im_gray, keypoints, im_gray);

    // display image
    // cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);
    // cv::imshow("Display window", im_gray);

    // cv::waitKey(0);

    std::cout << "All done." << std::endl;

    return 0;
}

