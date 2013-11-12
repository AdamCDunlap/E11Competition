// competition0.ino
#include <MudduinoBot.h>
#include <GoldCode.h>
#include <Servo.h>
#include <RunEvery.h>

MudduinoBot bot;

bool side;

int RCGWThreshold= 750;//center reflectance sensor grey-white threshold PATRICK MOD
int RSGWThreshold= 650;//side reflectance sensor grey-white threshold PATRICK MOD
int RCBGThreshold= 910;//center reflectance sensor black-grey threshold PATRICK MOD
int RSBGThreshold= 845;//side reflectance sensor black-grey threshold PATRICK MOD
int location=270;

const int servo_out = 180;
const int servo_in = 10;

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
    static enum { BEELINE, BEELINE_FIND_CIRCLE, ON_CIRCLE, BLACK_LINE_FOLLOW, FIND_BOX} primary_state = BEELINE;
    static uint8_t secondary_state;
    static unsigned long switchTime = 0;
    static unsigned long timer = 0;
    unsigned long curTime = millis();
    unsigned long timeInState = curTime - switchTime;
    int rs = bot.getSideReflect();
    int rc = bot.getCenterReflect();

    runEvery(100) {
        Serial.print("Primary: ");
        Serial.print(primary_state);
        Serial.print("\tSecondary: ");
        Serial.print(secondary_state);
        Serial.println();
    }

    switch(primary_state) {
    case BEELINE:
        bot.flash_GC(6, !side); // flash 7
        bot.flash_GC(6, !side); // flash 7
        bot.flash_GC(6, !side); // flash 7
        bot.flash_GC(7, !side); // flash 8
        bot.flash_GC(7, !side); // flash 8
        bot.flash_GC(7, !side); // flash 8
        if (timeInState < 1000) {
//@@            bot.move(255);
        }
        else if (timeInState < 1500) {
//@@            bot.move(255 - (timeInState - 250)/4);
        }
        else {
//@@            bot.move(150);
        }

        if (bot.getBumper() || timeInState > 3000) {
            primary_state = BEELINE_FIND_CIRCLE;
            switchTime = curTime;
            secondary_state = 0;
        }
        bot.setServo(servo_in);
        break;
    case BEELINE_FIND_CIRCLE:
        switch(secondary_state) {
        case 0:
//@@            bot.move(-150, -50);
            if ((side == 1 && timeInState > 250) || (side == 0 && timeInState > 750)) secondary_state++;
            break;
        case 1:
//@@            bot.move(150, -50);
            if (rc > RCBGThreshold) { // on black
                bot.halt();
            }
            else if (rc > RCGWThreshold) { // on grey
                primary_state = ON_CIRCLE;
                switchTime = curTime;
                secondary_state = 0;
            }
        }
        break;
    case BLACK_LINE_FOLLOW:  ///PATRICK MOD START
        switch(secondary_state) {
        case 0:
        {
            // follow blackline
            static int lastState = 0;
            int state;
            static unsigned long lastStateChangeTime = 0;

            if (rs > RSGWThreshold) state = 0;  //using grey-white threshold for stronger contrast
            else if (rc < RCGWThreshold) state = 1;
            else state = 2;

            if (state != lastState) {
                lastStateChangeTime = curTime;
            }
            if (curTime - lastStateChangeTime > 2000) { // We've been in one state for a while
                secondary_state = 100;
                lastStateChangeTime = curTime;
                timer = curTime;
            }

            switch (state) {
            case 0: // side is on black--turn left
//@@                bot.move(120, -70);
                break;
            case 1: // center is on white--turn right
//@@                bot.move(120, 50);
                break;
            case 2:
//@@                bot.move(120, 0);  //center on black, side on white--go straight
                break;
            }
            bot.setServo(servo_in);
           
            lastState = state;
            break;
        }
        case 100: // Probably stuck, back up
//@@            bot.move(-200);
            if (curTime - timer > 1000){
                secondary_state = 0;
                timer = curTime;
            }
            break;
        }
        break;  //END PATRICK MOD
    case ON_CIRCLE:
        switch(secondary_state) {
        case 0:
        {
            // follow circle
            static int lastState = 0;
            int state;
            static unsigned long lastStateChangeTime = 0;

            //if (rs < RSGWThreshold) state = 0;
            //else if (rc > RCGWThreshold) state = 1;
            //else state = 2;
            if (rs < 700) state = 0;
            else if (rc > 700) state = 1;
            else state = 2;

            if (state != lastState) {
                lastStateChangeTime = curTime;
            }
            if (curTime - lastStateChangeTime > 2000) { // We've been in one state for a while
                secondary_state = 100;
                lastStateChangeTime = curTime;
                timer = curTime;
            }
            switch (state) {
            case 0: // side is on white
//@@                bot.move(120, -70);
                break;
            case 1: // center is on grey
//@@                bot.move(120, 50);
                break;
            case 2:
//@@                bot.move(120, -20);
                break;
            }
            bot.setServo(servo_in);

            int seedNumLeft = bot.readGC(MudduinoBot::LEFT);
           
            switch(seedNumLeft){   //START PATRICK MOD
                case 0:
                    location=45;
                    break;
                case 1:
                    location=135;
                    break;
                case 2:
                    location=225;
                    break;
                case 3:
                    location=315;
                    break;
            }  //END PATRICK MOD
            if (wrong_color(seedNumLeft) && abs(seedNumLeft) < 5 ) {
                bot.halt();
                secondary_state = 1;
                timer = curTime;
            }
           
            //int seedNumRight = bot.readGC(MudduinoBot::RIGHT);
           
            //if (wrong_color(seedNumRight) && abs(seedNumRight)==5) {
            //    //bot.tone(440);
            //}
            //else
            //{
            //    //bot.noTone();
            //}
           
            //if (curTime > 90000 && wrong_color(seedNumRight) && abs(seedNumRight) == 6) {
            //    primary_state=FIND_BOX;
            //}
            bot.noTone();

            lastState = state;
            break;
        }
        case 100: // Probably stuck, back up
            bot.tone(880);
            bot.move(-200, 0);
            if (curTime - timer > 500){
                secondary_state = 0;
                timer = curTime;
            }
            //if( (rs > RSGWThreshold) && (rc > RCGWThreshold)) { //START PATRICK MOD--adds turning to backing up if both see grey
//@@            //    bot.move(-100, 100);
            //} //END PATRICK MOD
            //else {
//@@            //    bot.move(-200);
            //}
            break;

        case 1:
//@@            //bot.move(-200);
            timer = curTime;
            secondary_state = 2;
            break;
        case 2:
            //if (curTime - timer > 100) {
                secondary_state = 3;
            //}
            break;
        case 3:
            bot.setServo(servo_out);
            if (curTime - timer > 1000) {
                timer = curTime;
                secondary_state = 0;
            }
            break;
        }
        break;
    case FIND_BOX:///PATRICK MOD START
        switch(secondary_state) {
        case 0:
        {
            // follow blackline
            static int lastState = 0;

            int state;
            static unsigned long lastStateChangeTime = 0;
            if (rs > RSGWThreshold) state = 0;  //using grey-white threshold for stronger contrast
            else if (rc < RCGWThreshold) state = 1;
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
            case 0: // side is on black--turn left
//@@                bot.move(120, -70);
                break;
            case 1: // center is on white--turn right
//@@                bot.move(120, 50);
                break;
            case 2:
//@@                bot.move(120, 0);  //center on black, side on white--go straight
                break;
            }
           
            int seedNum = bot.readGC(MudduinoBot::FORWARD);
            if (bot.getBumper() && !(wrong_color(seedNum)) && abs(seedNum)==6) {  //has hit and changed beacon
                secondary_state = 1;
            }
            lastState = state;
            break;
        }
        case 1: //has hit the beacon--turning
        {
//@@            bot.move(0,-70);
            int seedNum = bot.readGC(MudduinoBot::FORWARD);
            if (seedNum == 8) {
                secondary_state=2;
            }
            break;
        }
           
        case 2:  //has acquired gc7, drives forward
//@@            bot.move(120,0);
            if(((rs<RSGWThreshold)&&(rs>200))||((rc<RCGWThreshold)&&(rc>200))){
                secondary_state=3;
              }
            break;
        case 3: //has passed yellow once, slows down
//@@            bot.move(70,0);
            if(((rs<RSGWThreshold)&&(rs>200))||((rc<RCGWThreshold)&&(rc>200))){
                secondary_state=3;
               }
            break;
        case 4: //has passed yellow twice, stops
            bot.halt();
            break;
        case 100: // Probably stuck, back up
//@@            bot.move(-200);
            if (curTime - timer > 500){
                secondary_state = 0;
                timer = curTime;
            }
            break;
        }
       
        break;  //END PATRICK MOD
   
    }
}
