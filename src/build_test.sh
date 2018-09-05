# g++ -Wall -std=c++11 -o test test.cpp latch.cpp descriptor.cpp `pkg-config opencv --libs --cflags`
g++ -Wall -std=c++11 -o test test.cpp latch.cpp `pkg-config opencv --libs --cflags`
