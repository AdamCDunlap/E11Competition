#include "GoldCode.h"
#include <stdio.h>

int main() {
    int a = GoldCode::dotProduct(4UL, 4UL);
    int b = GoldCode::dotProduct(11223344UL, 11223344UL);
    int c = GoldCode::dotProduct(11220044UL, 11223344UL);
    printf("A: %d\nB: %d\nC: %d\n", a, b, c);

    int fb1[] = {5,2,3,4,5}; //First characteristic polynomial
    int fb2[] = {3,3,5}; // second
    uint32_t gc1 = GoldCode::goldCode(fb1, fb2, 1);
    uint32_t gc2 = GoldCode::goldCode(fb1, fb2, 2);
    uint32_t gc3 = GoldCode::goldCode(fb1, fb2, 3);
    uint32_t gc4 = GoldCode::goldCode(fb1, fb2, 4);
    uint32_t gc5 = GoldCode::goldCode(fb1, fb2, 5);
    uint32_t gc6 = GoldCode::goldCode(fb1, fb2, 6);
    uint32_t gc7 = GoldCode::goldCode(fb1, fb2, 7);
    uint32_t gc8 = GoldCode::goldCode(fb1, fb2, 8);
    uint32_t gc9 = ((gc1 & 1) << 30) | gc1 >> 1;

    char buf[31];
    GoldCode::printGC(gc1, buf);
    printf("%s\n", buf);
    GoldCode::printGC(gc2, buf);
    printf("%s\n", buf);
    GoldCode::printGC(gc3, buf);
    printf("%s\n", buf);
    GoldCode::printGC(gc4, buf);
    printf("%s\n", buf);
    GoldCode::printGC(gc5, buf);
    printf("%s\n", buf);
    GoldCode::printGC(gc6, buf);
    printf("%s\n", buf);
    GoldCode::printGC(gc7, buf);
    printf("%s\n", buf);
    GoldCode::printGC(gc8, buf);
    printf("%s\n", buf);

    //bool same = GoldCode::sameGC(0x12345678, 0x45678123);
    //bool same = GoldCode::sameGC(0x12345678, 0x12345678);
    printf("Dot product of gc1 and gc8 is %d\n", GoldCode::dotProduct(gc1, gc8));
    printf("Dot product of gc1 and gc9 is %d\n", GoldCode::dotProduct(gc1, gc9));
    printf("Dot product of gc1 and gc1 is %d\n", GoldCode::dotProduct(gc1, gc1));
    bool same = GoldCode::sameGC(gc1, gc8);
    printf("They %s\n", same? "the same" : "different");
}
