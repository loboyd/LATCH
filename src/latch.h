#ifndef LATCH_H
#define LATCH_H

/*Simple, bare-bones implementation of the LATCH descriptor as described in
https://talhassner.github.io/home/projects/LATCH/LATCH.pdf

Matching is not supported yet, but that should be one of the next things to
implement. Also, smarter FAST detection would be nice. Maybe this is done
by using an adaptive threshold or maybe some nicer technique for keeping only
the ``best`` points.

The patch triple were found here:
https://github.com/opencv/opencv_contrib/blob/master/modules/xfeatures2d/src/latch.cpp

2018.08.27  --  L. Boyd*/

#include <vector>
#include <cstdint>

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

    /*Detect features using OpenCV built-in FAST detector. Eventually it
      would be nice to have the ability to specify the desired number of
      keypoints returned.*/
    std::vector<cv::KeyPoint>
    detect(cv::Mat& im);

    /*Compute descriptors for a vector of keypoints using custom LATCH
      implementation.*/
    std::vector<Descriptor512>
    describe(cv::Mat &im, std::vector<cv::KeyPoint> &keypoints);

    /*Compute the square of the Frobenius norm on a grayscale image patch*/
    double frobenius_norm_squared(cv::Mat& patch);

    /*Compute LATCH boolean value for a single patch triple*/
    bool compare_patches(cv::Mat& p1, cv::Mat& p2, cv::Mat& anchor);

    /*Find best matches from second vector of keypoints for each keypoint in
      first vector*/
    std::vector<size_t> match_keypoints_one_way(
        const std::vector<Descriptor512> & des1,
        const std::vector<Descriptor512> & des2);

    /*Returns indices for pairs of descriptors whose closest match is
      each other*/
    std::vector< std::pair<size_t, size_t> > match_keypoint_pairs(
        const std::vector<Descriptor512> & des1,
        const std::vector<Descriptor512> & des2);
        // const std::vector<size_t> & matches1,
        // const std::vector<size_t> & matches2);
};

#endif // LATCH_H
