#ifndef DESCRIPTOR_H_
#define DESCRIPTOR_H_

#include <cstdint>

/*This class provides a simple way to store 512-bit values, by storing
  separately 8 64-bit values in an array.
  
  2018.08.30  --  L. Boyd*/

template <int kBitSize>
class Descriptor
{
public:
    static constexpr int kBitsPerWord = sizeof(uint64_t) * CHAR_BIT;
    static constexpr int kWords = kBitSize / kBitsPerWord;
    uint64_t word[kWords];

    // necessary for popcount64
    static const uint64_t m1  = 0x5555555555555555;
    static const uint64_t m2  = 0x3333333333333333;
    static const uint64_t m4  = 0x0f0f0f0f0f0f0f0f;
    static const uint64_t h01 = 0x0101010101010101;

public:
    /*Count population of ones in uint64_t*/
    uint64_t popcount64(uint64_t x)
    {
        x -= (x >> 1) & m1;
        x = (x & m2) + ((x >> 2) & m2);
        x = (x + (x >> 4)) & m4;
        return (x * h01) >> 56;
    }

    uint64_t HammingDistance(const Descriptor & des)
    {
        int pop = 0;
        for (int i = 0; i < kWords; ++i) {
            // count the ones in the XOR of the two descriptor strings
            // pop += __builtin_popcount(this->word[i]^des.word[i]);
            pop += popcount64(this->word[i]^des.word[i]);
        }
        return pop;
    }
};
typedef Descriptor<512> Descriptor512;

#endif
