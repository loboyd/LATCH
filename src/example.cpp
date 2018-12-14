#include <iostream>
#include <string>
#include <vector>
#include <ctime>
// #include <cmath>

#include <opencv2/opencv.hpp>
// #include <opencv2/core/core.hpp>
// #include <opencv2/imgcodecs.hpp>
// #include <opencv2/highgui/highgui.hpp>
// #include <opencv2/features2d.hpp>

#include "latch.h"

int main(int argc, char** argv) {
    // get image files
    std::string right_imfile("../images/right.jpg");
    std::string  left_imfile("../images/left.jpg");
    // std::string right_imfile("../images/far.jpg");
    // std::string  left_imfile("../images/close.jpg");

    // read in images
    cv::Mat right, left, rgray, lgray;
    // these should probably just read in grayscale
    right = cv::imread(right_imfile, cv::IMREAD_COLOR);
    left  = cv::imread(left_imfile,  cv::IMREAD_COLOR);
    if (right.empty() || left.empty()) {
        std::cout << "Could not open or find the image" << std::endl;
        return -1;
    } else {
        // convert to grayscale
        cv::cvtColor(right, rgray, cv::COLOR_BGR2GRAY);
        cv::cvtColor(left,  lgray, cv::COLOR_BGR2GRAY);
    }

    // create LATCH
    LATCH* my_latch = new LATCH();
    std::cout << "LATCH created" << std::endl;

    // detect keypoints
    std::vector<cv::KeyPoint> rkeypoints = my_latch->detect(rgray);
    std::vector<cv::KeyPoint> lkeypoints = my_latch->detect(lgray);
    std::cout << rkeypoints.size() << " keypoints detected" << std::endl;

    // describe keypoints
    clock_t t1 = clock();
    std::vector<Descriptor512> rdes = my_latch->describe(rgray, rkeypoints);
    std::vector<Descriptor512> ldes = my_latch->describe(lgray, lkeypoints);
    clock_t t2 = clock();
    std::cout << "Descriptors computed in ";
    std::cout << (t2-t1)/CLOCKS_PER_SEC << " s." << std::endl;

    // match keypoints
    std::vector<std::pair<size_t, size_t>> pairs;
    pairs = my_latch->match_keypoint_pairs(rdes, ldes);
    std::cout << "Keypoints matched" << std::endl;
    std::cout << pairs.size() << " matches found." << std::endl;

    // draw keypoint matches
    cv::Mat matches;
    cv::hconcat(left,right,matches);

    // draw lines connecting matched keypoints
    int ncols = left.cols;
    for (size_t i = 0; i < pairs.size(); ++i) {
        cv::Point2i lpoint, rpoint;
        // coordinates of point in left frame
        lpoint.x = lkeypoints.at(pairs.at(i).second).pt.x;
        lpoint.y = lkeypoints.at(pairs.at(i).second).pt.y;
        
        // coordinates of point in right frame
        rpoint.x = rkeypoints.at(pairs.at(i).first).pt.x + ncols;
        rpoint.y = rkeypoints.at(pairs.at(i).first).pt.y;

        cv::line(matches, lpoint, rpoint, cv::Scalar(255,0,0));
    }

    // display image
    cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);
    cv::imshow("Display window", matches);

    cv::waitKey(0);

    std::cout << "All done." << std::endl;

    return 0;
}

