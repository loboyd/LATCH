# LATCH
Implementation of LATCH descriptor (Levi/Hassner)

Detection is performed using the OpenCV built-in FAST detector, and the OpenCV KeyPoint class is used. Currently, only detection and description are possible, but matching will be included soon. A simple descriptor class is used to handle the 512-bit descriptors.

This is my own implementation of the LATCH descriptor by Levi and Hassner written from scratch in C++.

The LATCH paper is [here.](https://talhassner.github.io/home/projects/LATCH/LATCH.pdf)

I used the LATCH triple values from the OpenCV implementation [source code.](https://github.com/opencv/opencv_contrib/blob/master/modules/xfeatures2d/src/latch.cpp)

## Dependencies
OpenCV
