// GC_detect.ino
#include <GoldCode.h>

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
    for (int i=0; i<NUM_CACHED_GCs; i++) {
        cached_gcs[i] = GoldCode::goldCode(fb1, fb2, cached_gc_seeds[i]);
    }
}

void loop() {
    int seedNum = readGC();
    if (seedNum != 0) {
        //Serial.print(abs(seedNum));
        //if (seedNum > 0) {
        //    Serial.println(": White");
        //}
        //else {
        //    Serial.println(": Green");
        //}
        
    }
}

int readGC() {
    unsigned int readings[31];
    unsigned int sum = 0;
    for (int i=0; i<31;) {
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
        // For each reading, convert it to binary by comparing to the average, then 
        //  OR it into the GC backwards, with the first element of the array
        //  going in the 30th position, etc
        gc |= (uint32_t)(readings[30-i] < avg) << i;
    }

    // Check the gold code against each one of the possibilities
    for (int i=0; i<NUM_CACHED_GCs; i++) {
        // sameGC returns the positive score if it's the same, or the negative score if it's inverted
        int same = GoldCode::sameGC(gc, cached_gcs[i]);
        if (same != 0) {
            Serial.print("Detected Gold Code! ID is ");
            Serial.print(cached_gc_seeds[i]);
            Serial.print(", the correlation is ");
            Serial.print(abs(same));
            Serial.print(", and the color is ");
            Serial.print(same > 0? "White" : "Green");
            Serial.println();
            return  (same > 0? +1 : -1) * cached_gc_seeds[i];
        }
    }
    // If nothing matched, return 0
    return 0;
}
