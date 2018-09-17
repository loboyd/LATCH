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
    detect(const cv::Mat & im) const;

    /*Compute descriptors for a vector of keypoints using custom LATCH
      implementation.*/
    std::vector<Descriptor512>
    describe(const cv::Mat & im,
        const std::vector<cv::KeyPoint> & keypoints) const;

    /*Compute the square of the Frobenius distance between two grayscale
      image patches*/
    double frobenius_distance_squared(const cv::Mat & im,
        const int & row1,
        const int & col1,
        const int & row2,
        const int & col2) const;

    /*Compute LATCH boolean value for a single patch triple*/
    bool compare_patches(const cv::Mat & im,
        const int & patch1_row, const int & patch1_col,
        const int & patch2_row, const int & patch2_col,
        const int & anchor_row, const int & anchor_col) const;

    /*Find best matches from second vector of keypoints for each keypoint in
      first vector*/
    std::vector<size_t> match_keypoints_one_way(
        const std::vector<Descriptor512> & des1,
        const std::vector<Descriptor512> & des2) const;

    /*Returns indices for pairs of descriptors whose closest match is
      each other*/
    std::vector< std::pair<size_t, size_t> > match_keypoint_pairs(
        const std::vector<Descriptor512> & des1,
        const std::vector<Descriptor512> & des2) const;
        // const std::vector<size_t> & matches1,
        // const std::vector<size_t> & matches2);
};

#endif // LATCH_H
