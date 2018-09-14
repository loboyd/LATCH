# LATCH
Implementation of the LATCH descriptor (Levi/Hassner)

This is my own implementation of the LATCH descriptor by Levi and Hassner written from scratch in C++.

Detection is performed using the OpenCV built-in FAST detector, and the OpenCV KeyPoint class is used. A simple brute-force matcher has been implemented where paired features must be each other's closest matcher under Hamming distance. A simple descriptor class is used to handle the 512-bit descriptors.

The LATCH paper is [here.](https://talhassner.github.io/home/projects/LATCH/LATCH.pdf)

I used the LATCH triple values from the OpenCV implementation [source code.](https://github.com/opencv/opencv_contrib/blob/master/modules/xfeatures2d/src/latch.cpp)

[csp256](https://github.com/csp256) has made many helpful suggestions for this project.

## Dependencies
OpenCV
