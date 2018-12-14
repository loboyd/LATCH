#include <iostream>
#include <vector>
#include <cstdint>

#include <opencv2/opencv.hpp>

#include "latch.h"

int main()
{
    // set up video capture
    cv::VideoCapture cap("../video/camera.mp4");
    if (!cap.isOpened())
        return -1;

    // create LATCH
    LATCH *latch = new LATCH();
    std::cout << "LATCH created" << std::endl;

    // loop over frames in video stream
    for (;;) {
        cv::Mat frame, gray;
        cap >> frame;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        // get keypoints
        std::vector<cv::KeyPoint> keypoints = latch->detect(gray);
        std::cout << keypoints.size() << " keypoints found." << std::endl;

        cv::Scalar color(0,255,0);
        cv::drawKeypoints(gray,keypoints,gray,color);

        // for (size_t i = 0; i < keypoints.size(); ++i) {
        //     cv::KeyPoint kp = keypoints.at(i);
        //     cv::Point2i point(kp.pt.y, kp.pt.x);
        //     cv::circle(frame, point, 7, (0,255,0));
        // }

        cv::imshow("frame", gray);
        if (cv::waitKey(30) >= 0) break;
    }

    std::cout << "All done." << std::endl;
    return 0;
}
