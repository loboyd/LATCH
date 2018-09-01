#ifndef DESCRIPTOR_H_
#define DESCRIPTOR_H_

#include <vector>

/*This class provides a simple way to store 512-bit values, by storing
  separately 8 64-bit values in a vector.
  
  2018.08.30  --  L. Boyd*/

class descriptor
{
public:
    std::vector<unsigned long long int> values;
public:
    descriptor();
    void set_value(int position, unsigned long long int value);
};

#endif
