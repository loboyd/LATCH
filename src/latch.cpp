#include "latch.h"
#include <iostream>

LATCH::LATCH()
{
    patch_radius  =  3;
    window_radius = 24;
    detector = cv::FastFeatureDetector::create();
}

LATCH::LATCH(int _patch_radius, int _window_radius)
{
    patch_radius  = _patch_radius;
    window_radius = _window_radius;
    detector = cv::FastFeatureDetector::create();
}

std::vector<cv::KeyPoint>
LATCH::detect(const cv::Mat & im) const
{
    int nrows = im.rows;
    int ncols = im.cols;
    // total radius of detection
    int r = patch_radius + window_radius;
    
    // slice image so that keypoints are within LATCH bounds
    cv::Mat im_interior = im(cv::Range(r, nrows-r-1), cv::Range(r, ncols-r-1));

    std::vector<cv::KeyPoint> keypoints;
    // LATCH::detector->detect(im_interior, keypoints, cv::Mat());
    detector->detect(im_interior, keypoints, cv::Mat());

    // add back the rows and cols taken from the slicing
    for (unsigned int i = 0; i < keypoints.size(); ++i) {
        keypoints.at(i).pt.x += r;
        keypoints.at(i).pt.y += r;
    }
    
    return keypoints;
}

std::vector<Descriptor512>
LATCH::describe(const cv::Mat & im,
    const std::vector<cv::KeyPoint> & keypoints) const
{
    int row, col;
    uint64_t des_tmp;     // temp variable for chunk descriptor
    Descriptor512 des;    // single full descriptor
    std::vector<Descriptor512> descriptors;
    uint64_t compare_bit; // necessary to bitshift bool to >32 bits

    // loop over KeyPoints and build sum
    for (unsigned int i = 0; i < keypoints.size(); ++i) {
        row = keypoints.at(i).pt.y;
        col = keypoints.at(i).pt.x;

        // store 8 64-bit ints for a total of 512 bits
        for (int j = 0; j < 8; ++j) {
            des_tmp = 0;
            // loop over bits in a single descriptor word
            for (int k = 0; k < 64; ++k) {
                int p = 64*j + k;  // under j and k loops goes from 0 to 511

                // get patch translations
                int patch1_row = row + triples.at(6*p + 0);
                int patch1_col = col + triples.at(6*p + 1);
                int patch2_row = row + triples.at(6*p + 2);
                int patch2_col = col + triples.at(6*p + 3);
                int anchor_row = row + triples.at(6*p + 4);
                int anchor_col = col + triples.at(6*p + 5);

                // compare patches and add bit
                compare_bit = compare_patches(im,
                patch1_row, patch1_col,
                patch2_row, patch2_col,
                anchor_row, anchor_col);
                des_tmp |= (compare_bit << k);
            }
            des.word[j] = des_tmp;
        }
        descriptors.push_back(des);
    }
    return descriptors;
}

double LATCH::frobenius_distance_squared(const cv::Mat & im,
    const int & row1,
    const int & col1,
    const int & row2,
    const int & col2) const
{
    int stride = im.step;
    uint8_t *data = im.data;  // get underlying cv::Mat data for speed

    // iterate over both image patches in the same loop
    // add up the squares of the pixel differences
    double total = 0;
    for (int i = -patch_radius; i <= patch_radius; ++i) {
        for (int j = -patch_radius; j <= patch_radius; ++j) {
            // get translated indices into image data
            int ind1 = (row1+i)*stride + (col1+j);
            int ind2 = (row2+i)*stride + (col2+j);
            uint8_t val1 = data[ind1];
            uint8_t val2 = data[ind2];
            total += (val1-val2) * (val1-val2);
        }
    }

    return total;
}

bool LATCH::compare_patches(const cv::Mat & im,
    const int & patch1_row, const int & patch1_col,
    const int & patch2_row, const int & patch2_col,
    const int & anchor_row, const int & anchor_col) const
{
    double dist1 = frobenius_distance_squared(im, patch1_row,
        patch1_col, anchor_row, anchor_col);

    double dist2 = frobenius_distance_squared(im, patch2_row,
        patch2_col, anchor_row, anchor_col);

    return (dist2 < dist1);
}

std::vector<size_t> LATCH::match_keypoints_one_way(
    const std::vector<Descriptor512> & des1,
    const std::vector<Descriptor512> & des2) const
{
    uint64_t current_min = UINT64_MAX;  // minimum distance seen so far
    uint64_t distance;                  // distance for current keypoint
    Descriptor512 current_des;
    std::vector<size_t> matches(des1.size());

    // perform simple linear search over des2 for each descriptor in des1
    for (size_t i = 0; i < des1.size(); ++i) {
        current_des = des1.at(i);
        for (size_t j = 0; j < des2.size(); ++j) {
            distance = current_des.HammingDistance(des2.at(j));
            if (distance < current_min) {
                current_min = distance;
                matches.at(i) = j;
            }
        }
        current_min = UINT64_MAX;  // reset distance tracker
    }
    return matches;
}

std::vector< std::pair<size_t, size_t> > LATCH::match_keypoint_pairs(
    const std::vector<Descriptor512> & des1,
    const std::vector<Descriptor512> & des2) const
{
    std::vector<size_t> matches1 = match_keypoints_one_way(des1, des2);
    std::vector<size_t> matches2 = match_keypoints_one_way(des2, des1);
    int j;
    std::vector< std::pair<size_t, size_t> > pairs;
    for (size_t i = 0; i < matches1.size(); ++i) {
        j = matches1.at(i);
        if (matches2.at(j) == i) {
            pairs.emplace_back(i,j);
        }
    }
    return pairs;
}

std::vector<int> LATCH::triples = 
{
    13, -6, 19, 19, 23, -4,
    4, 16, 24, -11, 4, -21,
    22, -14, -2, -20, 23, 5,
    17, -10, 2, 10, 14, -18,
    -22, 2, -12, 12, -22, 21,
    11, 6, 7, 15, 3, -11,
    -7, 16, -10, -14, -3, 9,
    -5, 1, -16, 16, -9, -21,
    -19, 2, -2, -9, -22, 24,
    19, 12, -1, -19, 15, -9,
    7, -2, 22, -23, 13, 20,
    -3, 9, -17, -1, -5, -19,
    -3, -14, 5, -21, 10, 19,
    12, -9, 24, 20, 20, -20,
    -5, 18, 19, 11, -6, -16,
    22, 7, 1, -8, -10, 6,
    19, -4, 3, 8, -2, 19,
    -17, 10, -11, -12, -21, -17,
    24, -13, 18, -14, 14, -19,
    -24, -15, 15, -14, -23, -11,
    -6, 22, -1, -11, 6, -14,
    16, 18, 10, -23, 20, 4,
    23, 8, 4, 7, 17, -19,
    -2, -21, -11, -18, -3, 7,
    -23, 10, -11, 5, -16, 19,
    -24, 4, 15, -16, -19, -5,
    -19, -4, -1, 5, -20, 2,
    20, 12, 11, -24, 9, 22,
    9, 13, -6, -23, 10, 15,
    -22, -8, -4, -5, -15, 20,
    -6, -13, 1, 16, -6, 23,
    -18, -3, -8, -15, -18, 5,
    14, -12, 9, 13, 19, 12,
    -22, 16, -1, 19, 16, -12,
    1, 8, -1, -4, -3, 7,
    3, 15, 23, -23, 5, -9,
    2, -7, 14, -13, 6, 20,
    -18, 11, 16, -10, -12, 4,
    -15, 2, -9, 21, -21, 20,
    -3, 5, -22, 23, -7, -22,
    -17, 13, 24, -14, -24, -24,
    24, 15, 3, -22, -16, 7,
    -14, -20, 1, -7, -12, -2,
    19, 17, 0, 18, -12, -7,
    -12, 10, 8, 5, -21, -18,
    -15, 9, 13, 3, -18, -17,
    0, 5, 11, -22, 8, 18,
    21, 2, -22, -17, 15, 3,
    -22, -15, 18, 23, -23, 21,
    -24, 16, 10, -3, -8, -1,
    -19, 19, 22, 23, -14, -2,
    20, 15, -2, -19, 19, -15,
    -10, 12, 0, -9, -9, -16,
    13, -22, 16, 16, 0, -14,
    -8, -13, -1, 20, -5, -22,
    -7, -23, -4, 10, -1, 20,
    16, -1, -13, -16, 24, -18,
    -18, 12, 8, 19, -24, -14,
    -15, 24, 6, 2, -21, -22,
    20, -2, 8, 0, 17, -10,
    -19, 21, -7, 20, -14, 3,
    19, 17, 0, -16, -18, -19,
    -17, -19, 12, -23, -12, -8,
    9, 10, 9, -23, 21, -24,
    9, 19, -15, -18, 7, -19,
    5, 3, -3, -16, 4, -2,
    15, -10, -24, 16, 24, 11,
    17, 16, -9, 1, 18, -15,
    11, -5, 0, 24, -20, -12,
    -14, -19, 24, -16, -9, -6,
    22, -14, 2, -22, 16, 11,
    23, -1, 4, -10, 20, 22,
    -10, -9, 17, 13, -13, -13,
    -15, 13, 11, 9, -13, 9,
    22, 15, 2, 18, -12, -10,
    3, 23, 18, 15, 20, -24,
    7, -6, 16, 11, 8, 1,
    13, 16, 24, -20, 9, -4,
    -8, -3, 17, 24, -19, 17,
    11, 6, -5, 22, 14, -10,
    -5, -11, -15, -10, -22, 9,
    7, 18, -12, 8, 13, -24,
    9, 0, 2, 3, 7, 12,
    21, 14, 0, -8, -17, 2,
    22, 20, -5, 16, 19, -23,
    22, -18, -19, -3, 24, -15,
    18, 0, -11, 16, 17, 11,
    22, 15, -11, 7, 20, -9,
    -16, 10, 2, 1, -19, 20,
    -19, -4, 2, -3, -24, 17,
    -3, 21, 22, -12, -1, 3,
    -3, -20, -7, 23, -1, -9,
    -11, 3, -20, -5, -9, -8,
    19, -17, 21, 21, 21, -13,
    -10, 6, 2, -2, -17, -21,
    19, 24, 20, 6, 24, -11,
    10, -23, -1, -9, 8, -5,
    22, -20, -3, 24, 19, 5,
    -24, 6, 0, -13, -23, -15,
    10, 20, -22, -4, 9, -20,
    -24, 10, 5, -15, -24, -20,
    22, 6, 8, -7, 11, 22,
    -18, 7, -9, 19, -12, -5,
    -9, 21, -20, -17, -17, 22,
    -23, 6, -22, -12, -17, 7,
    -18, 3, 1, 24, -24, 20,
    -10, -9, 2, 15, 18, 18,
    16, -13, -18, 11, 9, -6,
    24, 24, -24, 22, 12, -12,
    20, 7, -21, 15, 22, -5,
    -9, -7, 23, -13, -17, -20,
    -9, -6, 23, 0, -22, 13,
    -15, -18, 1, -22, -17, 10,
    0, 4, 4, -8, 18, -8,
    -7, -6, -20, 18, -20, -3,
    -20, -14, 4, -9, -17, -8,
    -18, -7, 3, 8, -16, 5,
    7, -12, 10, 19, 20, 21,
    -22, 24, 4, 8, -22, 2,
    -19, -18, -18, 22, -2, 13,
    10, 9, -15, 15, 21, 16,
    16, 11, -24, -2, 24, 21,
    -7, -12, 1, 14, 9, 17,
    20, 17, 7, 7, 5, -24,
    -13, -8, 21, 18, -15, 11,
    -22, 8, 12, -8, -18, 23,
    14, 10, 6, -24, 17, -10,
    8, 13, 21, 17, 24, -3,
    -21, -24, 18, 11, -8, 5,
    -10, -23, -2, 23, -13, 5,
    11, 7, -1, -21, -10, -4,
    21, -22, -15, 6, 6, -4,
    16, -7, -7, -23, 19, 6,
    -1, 21, 23, -14, -2, -17,
    22, -13, -22, 4, 14, 3,
    -10, 3, 14, -11, -22, 8,
    11, 13, -24, 10, 24, 21,
    12, 2, 13, -16, 15, 1,
    -1, -4, 20, -22, -6, -19,
    -14, -20, 2, -11, -20, 24,
    -23, -10, 12, 1, -24, 2,
    -24, -23, -16, 13, -1, -11,
    -8, 6, 19, -13, -23, 23,
    -18, -24, 23, -16, -21, 16,
    -12, 19, -10, 6, -6, -16,
    0, -15, -13, 24, -2, 9,
    19, -4, 0, 21, 21, 16,
    -10, -24, -24, -20, -13, -5,
    24, 7, -13, 7, 18, 19,
    0, 22, -21, 20, 0, 18,
    23, 10, -13, -14, 16, 10,
    -10, -12, 8, 10, -13, 24,
    -22, -6, -17, 14, -6, 11,
    17, 17, -7, 17, 17, -12,
    22, -1, -2, -3, -24, 22,
    12, 0, 1, -11, 12, -16,
    -20, -6, -11, 17, -5, -19,
    18, 7, -8, 3, 23, -11,
    24, -7, -18, 24, 20, -1,
    -10, 4, -4, -22, -14, -8,
    15, -8, -16, 20, 17, 23,
    12, 15, 15, -19, 5, 4,
    -16, 21, 3, -3, -17, -15,
    -18, 14, -20, -22, -18, 12,
    21, 13, -18, 0, 12, -12,
    -20, 23, 15, -10, -14, -16,
    -24, 16, 12, -5, -16, 13,
    -11, -13, -4, -9, -2, -18,
    3, -12, -24, 0, -2, -3,
    -14, -14, 22, 9, -21, 17,
    18, 10, 2, 23, 15, 6,
    -8, -18, 15, 23, -11, 23,
    -24, 13, 4, 16, -24, -13,
    9, 0, 21, -23, 6, -24,
    -22, 13, 21, 19, -21, -10,
    -21, 19, 7, -2, -7, 1,
    2, -21, 8, 20, 11, -12,
    19, -19, -2, 24, 17, 1,
    -3, -7, 3, 17, -4, -13,
    -23, -5, -15, -14, -7, 11,
    -15, -23, 24, 22, -17, 18,
    5, -7, 11, -22, 18, -5,
    20, -11, -20, 0, 11, 4,
    18, 18, -9, 7, 19, 17,
    1, -17, 24, -24, 4, 3,
    -19, -23, 9, 23, -10, 9,
    7, -2, -13, 5, 16, -5,
    8, -13, -9, -23, 12, 13,
    6, -21, -1, 0, -4, 18,
    9, -17, -24, -22, 9, 17,
    -19, 2, 20, -14, -22, 23,
    22, 11, -9, -14, 8, -4,
    12, -22, -2, 13, 8, 21,
    9, -8, 14, 18, 5, -9,
    16, -13, -7, -7, 21, -12,
    13, -12, -10, 11, 7, 11,
    3, 8, 5, -6, 2, 14,
    24, -22, 8, 23, -7, -10,
    22, 11, 6, 20, -6, -9,
    10, -5, -2, -1, 12, 15,
    -14, 14, -23, 6, -13, -3,
    -9, 2, 22, -1, -24, -10,
    -17, 22, 6, -9, -12, -13,
    -12, 1, -4, 9, -14, -2,
    13, 2, 23, -2, 12, 5,
    16, -14, -4, -22, 18, 17,
    -13, -8, 7, -5, -21, -17,
    3, -1, -3, -2, -10, 19,
    18, 6, -14, 24, 20, 10,
    -7, -20, -23, 10, -4, -8,
    -20, -9, -10, 16, -14, -21,
    9, 17, -12, 21, 16, 24,
    19, -22, -11, -12, 24, -20,
    5, -15, 14, 12, 3, -13,
    -6, 20, 4, 22, 3, -20,
    -23, -12, 7, 12, -23, 16,
    22, 3, -4, 18, 22, 8,
    1, 1, 16, 9, -5, 18,
    21, -7, -5, -15, 24, -6,
    -13, 14, -12, 23, -17, 18,
    12, 18, -1, 0, 20, 7,
    14, -7, -15, -17, 18, -2,
    -18, -12, -11, 14, -15, -21,
    -2, -21, 4, -16, 10, -2,
    22, -23, -19, -17, 19, 16,
    8, 22, 7, -22, 22, -22,
    8, -17, -17, 0, 12, 22,
    13, 8, -6, 19, 19, -6,
    19, -4, -23, -10, 23, -13,
    -16, -19, -6, 18, -19, 23,
    5, -18, -4, -12, -18, -24,
    -10, 9, 6, 4, -16, -24,
    -7, -15, 0, 3, -1, 24,
    -5, 12, 10, -24, 24, 22,
    -13, 9, 1, 18, 15, 7,
    -18, -3, -11, -22, -18, -5,
    -9, 7, 14, 24, -6, -1,
    -1, -24, 22, 19, -1, 13,
    -19, 3, -15, -16, -12, 1,
    0, 12, 21, 21, 13, -22,
    -19, -9, 14, 12, -23, 17,
    13, -11, 22, 3, 24, -14,
    -19, 5, -1, 20, 18, 15,
    19, -19, -16, 24, 23, 7,
    -20, -13, 22, 21, -23, -3,
    -20, 19, 16, -2, -20, -19,
    18, 18, -12, -16, 14, -5,
    21, -16, -23, -5, 19, 8,
    -12, 12, -20, -5, -7, -6,
    -10, -24, -3, 18, -11, -5,
    8, 14, 2, -3, 9, 6,
    24, 2, -2, -6, 24, -12,
    7, -8, 0, 8, -3, 21,
    22, 1, 17, -12, 14, -23,
    19, -18, -1, 23, -21, -10,
    -6, -1, 14, -22, -9, -9,
    20, -24, -16, -11, 21, 19,
    -24, 9, -8, 17, -19, -7,
    -12, -3, 19, -24, -15, 0,
    -1, -21, -9, 22, -21, -4,
    23, 4, -3, 11, 9, 4,
    -10, 10, 10, 4, -8, 7,
    5, -15, 21, -23, 9, -12,
    -17, -21, -2, -15, -17, -15,
    21, 12, 9, 23, 1, -9,
    21, 20, 19, -6, 5, -1,
    -16, -21, 19, -3, -12, 15,
    14, 3, -2, 2, -20, -17,
    -3, -16, -15, -13, -21, 11,
    -18, 21, -5, -17, 5, 11,
    23, 7, -9, 17, 20, -6,
    11, -14, -21, 23, 19, -21,
    -9, -6, 23, -24, -16, 7,
    -22, 21, 7, 12, -19, -12,
    -3, 19, 23, 10, -3, -18,
    -2, 22, -8, -16, -5, 23,
    14, 21, 22, -19, 6, -9,
    -6, -24, -12, -13, -23, 9,
    4, -21, 14, -4, 23, 18,
    9, 7, -6, 6, 22, 0,
    14, -13, 8, 24, 16, -14,
    2, -20, 4, -13, 11, -11,
    14, -12, 23, 7, 10, 21,
    14, -4, 14, 22, 13, -14,
    11, -12, 21, 19, 20, -8,
    3, -20, 23, -13, 23, 23,
    4, 18, -2, 10, -11, 20,
    1, 21, 6, 15, 14, -3,
    16, 24, 8, -11, 18, 23,
    21, -3, 15, -23, 5, -5,
    7, 9, -12, -4, 14, 18,
    1, -24, 11, -9, -1, 10,
    -16, -10, -7, 22, -14, 5,
    -22, 18, -15, -24, -1, 8,
    -17, 16, 4, 0, -17, 24,
    -9, -8, 17, -3, -13, 21,
    24, 10, 12, 12, 3, -15,
    21, 6, -1, -5, 19, 19,
    -21, -15, 12, 14, -24, -15,
    -24, 10, 5, -11, -16, -6,
    16, -8, -5, -20, 15, -7,
    -4, 20, -5, 12, -9, 20,
    -18, 12, 10, -14, -14, 24,
    17, -12, -1, 13, 18, 19,
    -13, 22, 2, 9, -14, 19,
    13, -12, 5, 18, 4, -24,
    -23, -5, -1, -11, 19, 13,
    14, -11, -21, -8, 22, -22,
    -24, 21, -8, -21, 5, 14,
    11, -4, -9, -10, 16, 2,
    19, -12, -8, 14, 22, -23,
    -22, -13, 1, -4, -17, 4,
    -21, 10, 5, 3, -19, 18,
    -3, -18, 13, 15, 19, -23,
    -2, 12, 23, -19, -1, -10,
    15, -7, 0, -20, 7, 0,
    -17, -1, -5, 15, -16, -20,
    11, -21, 2, -15, 4, 2,
    -3, 5, 4, -2, -3, -14,
    13, 22, -15, 19, 9, -17,
    -4, 18, 21, 7, -2, 5,
    15, 22, 7, -23, 19, 14,
    11, 14, 24, -23, 11, 6,
    17, 21, -8, -13, 15, 11,
    -12, -23, 10, 8, -8, -11,
    12, -5, -16, -19, 18, -6,
    -20, -24, -1, -22, -24, -9,
    -17, -12, 9, 19, -16, 24,
    14, -9, -6, 7, 20, -23,
    7, 19, 24, 0, 9, 23,
    -23, 22, 11, 7, -24, 22,
    -21, 0, -8, 14, -20, 23,
    14, -8, -16, -15, 18, 11,
    2, -6, 24, 7, 6, 24,
    -14, 24, -4, 3, -21, 2,
    23, 10, 24, -24, 10, 10,
    11, 5, -2, 15, 12, 7,
    24, 11, -5, 6, 21, 12,
    12, 22, -1, 13, -15, -18,
    14, -23, 20, 1, 19, 23,
    -19, -22, 4, -2, -19, 20,
    8, 2, -9, 10, 23, 21,
    -11, -11, -1, 15, 9, 23,
    20, 1, 9, 9, 13, 21,
    9, -22, -5, -16, 5, -11,
    -17, -23, -7, 9, -24, 23,
    7, -9, 23, 2, 20, -16,
    15, -18, -22, 18, 16, 14,
    -13, -18, -23, -8, -22, 13,
    12, -9, 12, 20, 14, -12,
    -18, -5, -15, 3, -19, 8,
    -16, -13, 10, 10, -15, 17,
    -9, 5, -23, 16, -9, 3,
    -16, -19, 14, 21, -19, -22,
    -1, 5, 23, 13, 1, -24,
    -10, 19, -1, -23, -19, -23,
    4, -19, 8, 4, 7, 18,
    17, 12, 1, 7, -6, 18,
    11, -24, 8, 18, 16, -14,
    -22, 11, -11, -2, -20, 14,
    -5, 9, 9, -23, 16, 24,
    -12, -8, 14, -6, -11, 5,
    23, -6, -16, -5, 21, -15,
    21, -22, -24, -2, 13, -8,
    17, 19, 24, -4, 10, 6,
    -14, -21, -8, 13, -5, -1,
    -21, -12, 23, -24, -21, -17,
    12, 11, 21, 15, 13, 23,
    -9, 16, -23, -2, -6, 2,
    19, 4, 18, -24, 23, 6,
    8, -23, 15, -2, 7, 20,
    24, 10, 8, 24, 4, -3,
    -23, 5, 19, -3, -23, 23,
    -19, -20, 3, 15, -12, 6,
    -10, 23, 0, 3, 18, -22,
    12, 8, -24, 19, 22, 2,
    12, 0, -4, -24, 21, 16,
    -9, -3, 14, 14, -14, 4,
    18, 11, -9, -14, 21, -23,
    11, 22, 1, 4, 17, -3,
    13, -22, -17, 23, 11, 15,
    11, -14, 3, 9, -4, -12,
    -6, 16, 2, 5, 20, 6,
    10, -1, 7, 21, 12, 7,
    -21, 12, -14, -21, -23, 13,
    16, 24, 0, -10, -14, -16,
    -12, -6, 23, 8, -10, 9,
    14, -18, 2, 24, -9, -5,
    16, 17, 0, -1, 10, 21,
    7, 0, -12, -15, 13, -11,
    14, -20, -22, -13, 0, 1,
    -21, -15, 6, -23, -16, -20,
    -9, 24, 2, -17, -5, 4,
    -21, 18, 18, -22, -21, -6,
    8, -3, 5, 17, 18, 10,
    3, 0, 11, 22, -4, -12,
    -24, 10, 18, 20, -21, -24,
    -8, -19, 6, -24, 17, 7,
    1, 8, 19, 8, 13, -23,
    -21, -24, 21, 2, -21, -15,
    20, 17, 21, -3, 21, 18,
    -18, -10, 17, -18, -18, 10,
    5, -6, 19, 10, 11, 22,
    6, 24, 8, 13, 3, -8,
    -3, -12, -13, 4, -21, 23,
    -10, 5, -2, -22, 5, -9,
    20, -17, -24, 16, 5, -3,
    2, 5, 6, -24, 5, 21,
    -15, 22, 1, 7, -16, 0,
    -19, -21, -7, 10, 0, -23,
    -15, -6, -2, -18, -20, -8,
    -16, 19, 1, 15, 18, 4,
    5, 4, -21, -14, 4, 2,
    2, 19, 0, -8, 5, 7,
    -16, -18, 22, 2, -18, 22,
    -23, 2, 15, -21, -19, -10,
    -15, 12, -8, -14, -20, -11,
    -11, 3, 1, 20, -24, 20,
    2, 3, -1, 24, 17, 19,
    -22, 2, 9, -23, -20, -3,
    -11, -11, 11, -20, -13, -23,
    5, -1, 16, -7, 3, 9,
    23, -2, 14, 23, 13, -2,
    20, -12, 12, 18, 22, 1,
    16, -19, 11, 8, 7, 23,
    9, 9, 2, -20, 15, -23,
    10, 0, -10, 23, 9, 4,
    -24, -18, 3, 15, -16, -7,
    19, -17, -17, 1, 23, 11,
    22, -7, 0, 24, 0, -3,
    -24, -22, 15, 9, -8, -4,
    20, -14, -8, -14, 19, 9,
    24, -2, -8, -4, 24, 14,
    17, -1, 10, -23, 1, 15,
    9, -1, 0, -24, 13, -12,
    -5, 10, -18, -6, -23, 11,
    -20, 21, 18, 4, -9, -7,
    24, 15, 7, -3, -2, 11,
    -10, -24, 11, 2, -10, 13,
    17, -17, -14, -18, 21, -14,
    -9, -17, -4, -9, -10, -2,
    22, -21, 8, -11, 1, 23,
    -3, -15, -21, -20, -14, 19,
    3, -10, -11, 22, 3, -21,
    -23, -15, 0, 9, -19, 12,
    -24, -3, -5, 22, -23, 15,
    16, -9, -19, -18, 11, -1,
    -18, 6, 0, -24, 18, -23,
    15, -11, -24, 4, 16, 1,
    10, -21, 23, 1, 2, -10,
    18, -2, 1, 5, -7, -23,
    24, -16, -11, -22, 24, -19,
    19, 12, -23, 2, 12, 0,
    17, 9, 12, -12, 8, 11,
    -16, -16, 19, 0, -19, -21,
    15, 20, 20, -24, 9, 3,
    24, 1, 6, 21, 18, -19,
    -22, 21, -2, -14, 19, 22,
    0, -17, 18, -12, -2, 10,
    -21, -8, -9, -7, -18, -15,
    -19, -1, -7, -21, -23, -15,
    -23, -5, 13, 21, -18, 1,
    12, 6, 15, -12, 10, -20,
    16, -13, -20, -6, 14, 13,
    -9, -2, 11, -14, -13, -5,
    15, -4, 13, 17, 22, -3,
    19, -17, -2, 11, -23, 22,
    12, 16, 12, -4, 18, 9,
    0, 9, 11, -20, 11, 1,
    12, -11, 22, -9, 24, -23,
    -14, -13, -3, 5, 4, 12,
    14, 12, -14, 3, 15, 17,
    -11, -24, 18, -23, -5, 3,
    18, 9, 9, 20, 9, 3,
    -21, -10, 8, -1, -24, -23,
    13, 4, -3, -19, 19, 1,
    18, -18, 2, -21, 10, 13,
    -10, -17, 0, 12, 8, 19,
    21, 8, 2, -23, -19, 8,
    5, -4, -12, 18, 14, -12,
    19, -19, 14, 5, 9, 21,
    -21, -21, -8, 1, -1, 14,
    13, 6, 16, -24, 15, 14,
    -5, 21, -14, -8, -2, 11,
    -14, -21, -23, 19, -6, -6,
    10, -10, -23, -2, 16, 16,
    13, -14, 3, -15, 13, -23,
    -15, -13, 17, 12, -19, 19,
    -5, 18, -12, 10, -4, -16,
    -22, -15, -9, -18, -10, 16,
    -7, -5, 13, -18, -18, -23,
    -23, 22, -3, -24, 14, 20,
    12, 16, 21, -11, 19, 19,
    12, -18, -3, -17, 9, -14,
    -19, -11, 14, -13, -21, 23,
    8, -6, -18, 12, 17, 1,
    -4, -1, 4, 19, -12, -7,
    21, 3, -24, 21, 13, 8,
    17, 23, 2, 15, 21, -4,
    4, 16, -15, -20, 1, 6,
    16, -22, 6, 11, 18, -12,
    -24, -1, -18, 8, -13, -2,
    16, -6, -1, -7, -20, -20,
    9, -10, -15, 6, 17, 16,
    -19, 17, 19, 0, -18, -8,
    15, -23, 12, -6, 1, 11,
    21, -15, 6, 19, 10, -24,
    -16, 23, -1, -8, -17, -14,
    11, 2, -1, 7, 14, -2,
    11, 20, -1, -4, -3, -23,
    -19, 20, -11, -2, -20, -24,
    11, -12, 5, -21, -2, -13
};
