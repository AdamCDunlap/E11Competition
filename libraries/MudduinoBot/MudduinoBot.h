#ifndef MUDDUINOBOT_H
#define MUDDUINOBOT_H

#include <Arduino.h>
#include <Servo.h>
#include <GoldCode.h>
#include <new.h>
#include <stdlib.h>

class MudduinoBot {
public:
    // Free pins: 0, 1, 5
    MudduinoBot(
        uint8_t lplus_in = 9, uint8_t lminus_in = 8, uint8_t len_in = 6,
        uint8_t rplus_in = 7, uint8_t rminus_in = 12, uint8_t ren_in = 11,
        uint8_t servo_in = 10,
        uint8_t dist_in = A0,
        uint8_t light_fwd_in = A5, uint8_t light_right_in = A3, uint8_t light_left_in = A2,
        uint8_t reflect_center_in = A4, uint8_t reflect_side_in = A1,
        uint8_t team_in = 3,
        uint8_t buzzer_in = 4,
        uint8_t led_in = 13,
        uint8_t bumper_in = 2) :
            lplus_pin(lplus_in), lminus_pin(lminus_in), len_pin(len_in),
            rplus_pin(rplus_in), rminus_pin(rminus_in), ren_pin(ren_in),
            serv_pin(servo_in),
            dist_pin(dist_in),
            light_fwd_pin(light_fwd_in), light_left_pin(light_left_in), light_right_pin(light_right_in),
            reflect_center_pin(reflect_center_in), reflect_side_pin(reflect_side_in),
            team_pin(team_in),
            buzzer_pin(buzzer_in),
            led_pin(led_in),
            bumper_pin(bumper_in),
            led_out(portOutputRegister(digitalPinToPort(led_pin))),
            led_bit(digitalPinToBitMask(led_pin))
            
    {
    }
    ~MudduinoBot() {
        //free(cached_gcs);
        //free(cached_gc_seeds);
        delete[] cached_gcs;
        delete[] cached_gc_seeds;
    }

    void begin() { initMotors(); initServo(); initSensors(); initBuzzer(); initLEDs(); }

    void motorEn();
    void initMotors();
    void halt();
    void forward();
    void backward();
    void turnL();
    void turnR();

    void move(int fwd, int turn=0);

    void initServo();
    void setServo(int x);
    void setServoRaw(int x);

    void initBuzzer();
    void tone(unsigned int frequency, unsigned long duration = 0);
    void noTone();

    void initSensors();
    int getDist();

    enum lightSensor { LEFT, RIGHT, FORWARD, NUM_LIGHT_SENSORS };

    int getLight(); // fwd, for compatibility
    int getFwdLight();
    int getLeftLight();
    int getRightLight();

    int getReflect(); // center, for compatibility
    int getCenterReflect();
    int getSideReflect();

    bool getBumper();

    bool onWhite();
    bool getTeam(); // Gets whether we're on the white or green team. White is true

    void cache_GCs(int num_cached, int* seeds, int* fb1, int* fb2);
    // readings lets the user pass in a length-31 array into which the raw
    // light sensor values will be put
    int readGC(lightSensor l = MudduinoBot::FORWARD, unsigned long* variance = NULL, unsigned int* readings = NULL);
    struct LightVals {
        uint8_t pos; // next position in the array we will store
        unsigned int vals[31]; // array of light sensor values
        unsigned long lastReadTime; // time in microseconds when we last read
    };
    // inputs: GC_ret, a place to store the gold code -- will not change if false is returned
    //         lightSensor, which sensor to read from
    //         vals, a place to store the pointer to where we are reading into
    // output: If it's done and the results are valid
    //  Note: Only works if being called very(very) fast --- each analogRead takes 100 uS so
    //  having one of these in a loop works great; two is iffy; and 3 is impossible
    //  Would not use.
    bool readGC_async(uint32_t* GC_ret, lightSensor l = MudduinoBot::FORWARD, LightVals** vals = NULL);

    void initLEDs();
    void flash_GC(int which, bool inverted);

    // flash_GC_async
    // MUST be called precisely every 250 microseconds.
    // returns true when done flashing a gold code
    bool flash_GC_async(int which, bool inverted);

    Servo serv;
private:
    const uint8_t lplus_pin, lminus_pin, len_pin, rplus_pin, rminus_pin, ren_pin, serv_pin, dist_pin, light_fwd_pin, light_left_pin, light_right_pin, reflect_center_pin, reflect_side_pin, team_pin, buzzer_pin, led_pin, bumper_pin;

    // Holds cached values for direct I/O for faster GC flashing
    volatile uint8_t* led_out;
    uint8_t led_bit;

    struct FlashPos {
        uint8_t pos;
        unsigned long lastTime;
    } flashPos;
    
    
    LightVals lightVals[MudduinoBot::NUM_LIGHT_SENSORS];
    int num_cached_GCs;
    int* cached_gc_seeds;
    uint32_t* cached_gcs;
};

#endif//MUDDUINOBOT_H
