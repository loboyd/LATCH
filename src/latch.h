#ifndef LATCH_H
#define LATCH_H

/*Simple, bare-bones implementation of the LATCH descriptor as described in
https://talhassner.github.io/home/projects/LATCH/LATCH.pdf

The patch triple were found here:
https://github.com/opencv/opencv_contrib/blob/master/modules/xfeatures2d/src/latch.cpp

2018.08.27  --  L. Boyd*/

#include <vector>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d.hpp>

#include "descriptor.h"

class LATCH
{
public:
    int patch_radius, window_radius; // use radius to ensure odd number (2r+1)
    static cv::Ptr<cv::FastFeatureDetector> detector;
    static std::vector<int> triples;
public:
    LATCH();
    LATCH(int _patch_size, int _window_size);

    /*Detect features using OpenCV built-in FAST detector*/
    std::vector<cv::KeyPoint>
    detect(cv::Mat& im);

    /*TODO*/
    /*Compute descriptors for a vector of keypoints
      Run LATCH::clean_keypoints to make sure all keypoints are
      within the correct bounds first*/
    // std::vector<keypoint>
    // std::vector<unsigned long long int>
    std::vector<descriptor>
    describe(cv::Mat &im, std::vector<cv::KeyPoint> &keypoints);

    /*Compute the Frobenius norm on a grayscale image patch*/
    double frobenius_norm(cv::Mat& patch);

    /*Compute LATCH boolean value for a single patch triple*/
    bool compare_patches(cv::Mat& p1, cv::Mat& p2, cv::Mat& anchor);
};

#endif // LATCH_H
