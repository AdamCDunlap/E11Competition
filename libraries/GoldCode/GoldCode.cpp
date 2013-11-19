#include "GoldCode.h"
#include <limits.h>

#ifndef abs
template<typename T> T abs(T x) {
    return x < 0? -x : x;
}
#endif

static uint32_t nextStep(uint32_t a, int feedbackList[]);
static uint32_t shiftRegister(uint32_t seed, int feedbackList[]);

// By Patrick McKeen and Adam Dunlap

uint32_t GoldCode::goldCode(int feedback1[], int feedback2[], int seed2) //returns the numerical value of the gold code
{
    uint32_t reg1=shiftRegister(1, feedback1);
    uint32_t reg2=shiftRegister(seed2, feedback2);
    return reg1^reg2; // XORs the 2 outputs of the LFSRs together
}

uint32_t shiftRegister(uint32_t seed, int feedbackList[]) //finds the shift register for the LFSR
{
    uint32_t reg = 0;
    uint32_t b = seed;
    for (int i=0; (b-seed) != 0 || i == 0 ; i++) {
        reg |= (b&1L) << (30-i); //takes the rightmost digit of each iteration and adds it to the binary string
        b = nextStep(b, feedbackList); //iterates to the next step
    }
    return reg;
}

uint32_t nextStep(uint32_t a, int feedbackList[]) //finds the next value outpu of the LFSR
{
    uint32_t r= a >> 1;//shifts one place to the right
    bool addOne = false;
    for (int i=1; i<feedbackList[0];i++)
    {
        bool n = ((1L << (5 - feedbackList[i])) & a) != 0; //tests if the feedback terminals are outputting 1s
        addOne^=n; //XORs the outputs of the feedback terminals together
    }
    return (addOne << 4 ) | r; //generates next iteration
}

int GoldCode::dotProduct(uint32_t gc1, uint32_t gc2) {
    //int score = 0;
    //for (int i=0; i<31; i++) {
    //    // If the ith bit is the same in each one, add 1
    //    // gc1 & (1UL << i) is the ith bit (although still kept in the same position)
    //    if ((gc1 & (1UL << i)) == (gc2 & (1UL << i))) score++;
    //    else                                          score--;
    //}
    //return score;

    // xor one GC with the NOT of the other, then make the top bit 0
    uint32_t v = (gc1 ^ ~gc2) & ~0x80000000;

    // now count the bits in v, with code
    // From http://graphics.stanford.edu/~seander/bithacks.html

    // count bits set in this (32-bit value)
    uint32_t c; // store the total here
    
    c = v - ((v >> 1) & 0x55555555);
    c = ((c >> 2) & 0x33333333) + (c & 0x33333333);
    c = ((c >> 4) + c) & 0x0F0F0F0F;
    c = ((c >> 8) + c) & 0x00FF00FF;
    c = ((c >> 16) + c) & 0x0000FFFF;
    return c*2 - 31;
}

int GoldCode::sameGC(uint32_t gc1, uint32_t gc2) {

    int score = dotProduct(gc1, gc2);
    if (abs(score) >= 22) return score;
    for (int i=0; i<31; i++) {
        gc2 = ((gc2 & 1) << 30) | gc2 >> 1;
        score = dotProduct(gc1, gc2);
        if (abs(score) >= 22) return score;
    }
    // If none of them have a good enough match, return 0
    return 0;
}

void GoldCode::printGC(uint32_t gc, char* res) {
    res[31] = '\0'; // null terminate
    for (int i=0; i<31; i++) {
        res[i] = (gc & 1) + '0';
        gc >>= 1;
    }
}
