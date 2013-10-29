// GC_detect.ino
#include <GoldCode.h>
#include <RunEvery.h>

const int LIGHTPIN = A5;

const int NUM_CACHED_GCs = 10;
uint32_t cached_gcs[NUM_CACHED_GCs];
int cached_gc_seeds[NUM_CACHED_GCs] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 24};

int fb1[] = {5,2,3,4,5}; //First characteristic polynomial
int fb2[] = {3,3,5}; // second

// Reads the light sensor and sees which GC it matches by comparing it to each saved GC
// returns seed of matching GC
int readGC();

void setup() {
    Serial.begin(115200);
    Serial.println("Good morning");
    for (int i=0; i<NUM_CACHED_GCs; i++) {
        cached_gcs[i] = GoldCode::goldCode(fb1, fb2, cached_gc_seeds[i]);
    }
}

void loop() {
    int seedNum = readGC();
    if (seedNum != 0) {
        Serial.print(abs(seedNum));
        if (seedNum > 0) {
            Serial.println(": White");
        }
        else {
            Serial.println(": Green");
        }
        
    }
    //while(1);
}

int readGC() {
    unsigned int readings[31];
    unsigned int sum = 0;
    //Serial.println("Looping");
    for (int i=0; i<31;) {
        //runEveryMicros(250) {
        static unsigned long lastReadTime = micros();
        if (micros() - lastReadTime >= 250) {
            lastReadTime = micros();
            readings[i] = analogRead(LIGHTPIN);
            sum += readings[i];
            i++;
        }
    }

    uint32_t gc = 0;
    unsigned int avg = sum/31;
    
    for (int i=0; i < 31; i++) {
        gc |= (uint32_t)(readings[30-i] < avg) << i;
    }
    //GoldCode::printGC(gc);
    //Serial.println();
    
    for (int j=0; j<2; j++) {
        // first try non-inverted
        for (int i=0; i<NUM_CACHED_GCs; i++) {
            //Serial.print("With gold code ");
            //Serial.println(i);
            if (GoldCode::sameGC(gc, cached_gcs[i])) {
                //Serial.println("Readings:");
                //for (int j=0; j<31; j++) {
                //    Serial.println(readings[j]);
                //}
                //Serial.println();
                //Serial.print("avg: ");
                //Serial.println(avg);
                // If we're on the second loop, negate the value to signifigy that we inverted it
                return (j>0? -1 : 1) * cached_gc_seeds[i];
            }
        }
        // now invert and try again
        gc = ~gc;
    }
    
    return 0;
}
