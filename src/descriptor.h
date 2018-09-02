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
    
    /*This function is only used for updating an existing value. It's not
      very smart right now in that it will cause an error if you aren't
      careful to call it with a valid position value. A regular v.push_back()
      call should be used to set values.*/
    void set_value(int position, unsigned long long int value);
};

#endif
