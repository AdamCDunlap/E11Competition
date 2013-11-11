#ifndef MUDDUINOBOT_H
#define MUDDUINOBOT_H

#include <Arduino.h>
#include <Servo.h>
#include <GoldCode.h>
#include <new.h>
#include <stdlib.h>

class MudduinoBot {
public:
    MudduinoBot(
        uint8_t lplus_in = 9, uint8_t lminus_in = 8, uint8_t len_in = 6,
        uint8_t rplus_in = 7, uint8_t rminus_in = 12, uint8_t ren_in = 11,
        uint8_t servo_in = 10,
        uint8_t dist_in = 14,
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
            bumper_pin(bumper_in)
    {}
    ~MudduinoBot() {
        //free(cached_gcs);
        //free(cached_gc_seeds);
        delete[] cached_gcs;
        delete[] cached_gc_seeds;
    }

    enum lightSensor { LEFT, RIGHT, FORWARD };

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

    int getLight(); // fwd, for compatibility
    int getFwdLight();
    int getLeftLight();
    int getRightLight();

    int getReflect(); // center, for compatibility
    int getCenterReflect();
    int getSideReflect();

    bool getBumper();

    bool onWhite();
    bool getTeam(); // Gets whether we're on the white or green team

    void cache_GCs(int num_cached, int* seeds, int* fb1, int* fb2);
    int readGC(lightSensor l = FORWARD);

    void initLEDs();
    void flash_GC(int which, bool inverted);

private:
    //const uint8_t lplus, lminus, len, rplus, rminus, ren, serv_pin, dist_pin, light_pin, reflect_pin, team_pin, buzzer_pin, led_pin;
    const uint8_t lplus_pin, lminus_pin, len_pin, rplus_pin, rminus_pin, ren_pin, serv_pin, dist_pin, light_fwd_pin, light_left_pin, light_right_pin, reflect_center_pin, reflect_side_pin, team_pin, buzzer_pin, led_pin, bumper_pin;
    Servo serv;

    int num_cached_GCs;
    int* cached_gc_seeds;
    uint32_t* cached_gcs;
};

#endif//MUDDUINOBOT_H
