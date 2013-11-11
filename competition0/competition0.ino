// competition0.ino
#include <MudduinoBot.h>
#include <GoldCode.h>
#include <Servo.h>

MudduinoBot bot;

bool side;

void setup() {
    Serial.begin(115200);
    bot.begin();
    Serial.println("Good morning");

    int fb1[] = {5,2,3,4,5}; //First characteristic polynomial
    int fb2[] = {3,3,5}; // second
    int seeds[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 24};
    bot.cache_GCs(10, seeds, fb1, fb2);

    side = bot.getTeam();
    Serial.println(side);
}

bool wrong_color(int seedNum) {
    return (side && seedNum < 0) || (!side && seedNum > 0);
}

void loop() {
    static enum { BEELINE, BEELINE_FIND_CIRCLE, ON_CIRCLE} primary_state = BEELINE;
    static uint8_t secondary_state;
    static unsigned long switchTime = 0;
    static unsigned long timer = 0;
    unsigned long curTime = millis();
    unsigned long timeInState = curTime - switchTime;

    switch(primary_state) {
    case BEELINE:
        bot.flash_GC(6, !side); // flash 7
        bot.flash_GC(6, !side); // flash 7
        bot.flash_GC(6, !side); // flash 7
        bot.flash_GC(7, !side); // flash 8
        bot.flash_GC(7, !side); // flash 8
        bot.flash_GC(7, !side); // flash 8
        if (timeInState < 1000) {
            bot.move(255);
        }
        else if (timeInState < 1500) {
            bot.move(255 - (timeInState - 250)/4);
        }
        else {
            bot.move(150);
        }

        if (bot.getBumper() || timeInState > 3000) {
            primary_state = BEELINE_FIND_CIRCLE;
            switchTime = curTime;
            secondary_state = 0;
        }
        break;
    case BEELINE_FIND_CIRCLE:
        switch(secondary_state) {
        case 0:
            bot.move(-150, -50);
            if ((side == 1 && timeInState > 350) || (side == 0 && timeInState > 750)) secondary_state++;
            break;
        case 1:
            bot.move(150, -50);
            int rs = bot.getSideReflect();
            int rc = bot.getCenterReflect();
            if (rc > 800) { // on black
                bot.halt();
            }
            else if (rc > 700) { // on grey
                primary_state = ON_CIRCLE;
                switchTime = curTime;
                secondary_state = 0;
            }
        }
        break;
    case ON_CIRCLE:
        switch(secondary_state) {
        case 0:
        {
            // follow circle
            static int lastState = 0;
            int rs = bot.getSideReflect();
            int rc = bot.getCenterReflect();
            int state;
            static unsigned long lastStateChangeTime = 0;
            if (rs < 700) state = 0;
            else if (rc > 700) state = 1;
            else state = 2;
            if (state != lastState) {
                lastStateChangeTime = curTime;
            }
            if (curTime - lastStateChangeTime > 2000) { // We've been in one state for a second
                secondary_state = 100;
                lastStateChangeTime = curTime;
                timer = curTime;
            }
            switch (state) {
            case 0: // side is on white
                bot.move(120, -70);
                break;
            case 1: // center is on grey
                bot.move(120, 50);
                break;
            case 2:
                bot.move(120, -20);
                break;
            }
            bot.setServo(0);

            int seedNum = bot.readGC(MudduinoBot::LEFT);
            if (wrong_color(seedNum) && abs(seedNum) < 5 ) {
                bot.halt();
                secondary_state = 1;
                timer = curTime;
            }

            lastState = state;
            break;
        }
        case 100: // Probably stuck, back up
            bot.move(-200);
            if (curTime - timer > 500){
                secondary_state = 0;
                timer = curTime;
            }
            break;

        case 1:
            //bot.move(-200);
            timer = curTime;
            secondary_state = 2;
            break;
        case 2:
            //if (curTime - timer > 100) {
                secondary_state = 3;
            //}
            break;
        case 3:
            bot.setServo(180);
            if (curTime - timer > 1000) {
                timer = curTime;
                secondary_state = 0;
            }
            break;
        }
        break;
    }
}

