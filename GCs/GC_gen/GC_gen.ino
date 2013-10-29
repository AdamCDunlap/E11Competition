#include <GoldCode.h>
#include <stdinout.h>

using namespace GoldCode;

void setup()
{
    Serial.begin(115200);
    int fb2[]={5,2,3,4,5}; //First characteristic polynomial
    int fb1[]={3,3,5}; // second
    const int numTests = 8;
    int seedList[numTests]={1,2,3,4,5,6,7,8};  //The 8 tests
    for(int i=0;i<numTests;i++)
    {
        uint32_t gc=goldCode(fb1,fb2,seedList[i]);
        printGC(gc);
        Serial.println();
    }

}

void loop()
{
}
