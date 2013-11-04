#ifndef GOLDCODE_H
#define GOLDCODE_H
#include <stdint.h>

// By Patrick McKeen and Adam Dunlap

namespace GoldCode {
    // Generates a gold code
    // feedback1 and 2 are arrays where the first element is the length and
    //  the rest are the taps
    // seed2 is the seed for the second MLSRS in the gold code
    uint32_t goldCode(int feedback1[], int feedback2[], int seed2);
    
    // Computes the dot product of two gold codes as written
    //  returns +1 for every matching bit and -1 for every nonmatching bit
    int dotProduct(uint32_t gc1, uint32_t gc2);
    // Tries the dot product of each shifted version of two gold codes
    //  and returns the score if any shifted values have a dot product of
    //  greater than 25. Note that if a negative number is returned, one of the
    //  sequences is inverted
    int sameGC(uint32_t gc1, uint32_t gc2);

    // Writes an ASCII representation of the gold code to res..
    // res should be a char array with a length of at least 31
    void printGC(uint32_t gc, char* res);

}

#endif//GOLDCODE_H
