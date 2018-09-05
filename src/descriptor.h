#ifndef DESCRIPTOR_H_
#define DESCRIPTOR_H_

#include <cstdint>
#include <bits/stdc++.h>

/*This class provides a simple way to store 512-bit values, by storing
  separately 8 64-bit values in a vector.
  
  2018.08.30  --  L. Boyd*/

template <int kBitSize>
class Descriptor
{
public:
    static constexpr int kBitsPerWord = sizeof(uint64_t) * CHAR_BIT;
    static constexpr int kWords = kBitSize / kBitsPerWord;
    uint64_t word[kWords];
};
typedef Descriptor<512> Descriptor512;

#endif
