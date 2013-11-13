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
int RSBGThreshold= 853;//side reflectance sensor black-grey threshold PATRICK MOD
int location=270;

const int servo_out = 180;
const int servo_in = 10;

bool sideOnWhite(int rs) {
    return rs < RSGWThreshold;
}
bool centerOnWhite(int rc) {
    return rc < RCGWThreshold;
}
bool centerOnGray(int rc) {
    return rc > RCGWThreshold && rc < RCBGThreshold;
}
bool sideOnGray(int rs) {
    return rs > RSGWThreshold && rs < RSBGThreshold;
}
bool sideOnBlack(int rs) {
    return rs > RSBGThreshold;
}
bool centerOnBlack(int rc) {
    return rc > RCBGThreshold;
}

void setup() {
    Serial.begin(115200);
    bot.begin();
    Serial.println("Good morning");
    bot.setServo(servo_in);

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
        Serial.print("\t rs: ");
        Serial.print(rs);
        Serial.print("\t rc: ");
        Serial.print(rc);
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
            bot.move(255);
        }
        else if (timeInState < 1500) {
            bot.move(255 - (timeInState - 250)/3);
        }
        else {
            bot.move(150);
        }

        if (bot.getBumper() || timeInState > 2000) {
            primary_state = BEELINE_FIND_CIRCLE;
            switchTime = curTime;
            secondary_state = 0;
        }
        bot.setServo(servo_in);
        break;
    case BEELINE_FIND_CIRCLE:
        switch(secondary_state) {
        case 0:
            bot.move(-150, -50);
            if ((side == 1 && timeInState > 250) || (side == 0 && timeInState > 750)) secondary_state++;
            break;
        case 1:
            bot.move(150, -50);
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










    case ON_CIRCLE:
        switch(secondary_state) {
        case 0:
        {
            // follow circle
            static int lastState = 0;
            int state;
            static unsigned long lastStateChangeTime = 0;

            if (sideOnWhite(rs))        state = 0;
            else if (centerOnGray(rc))  state = 1;
            else                        state = 2;

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
                bot.move(120, -50);
                break;
            case 1: // center is on grey
                bot.move(120, 10);
                break;
            case 2:
                bot.move(120, -16);
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
           
            int seedNumRight = bot.readGC(MudduinoBot::RIGHT);
           
            if (wrong_color(seedNumRight) && abs(seedNumRight)==6) {
                primary_state = BLACK_LINE_FOLLOW;
                secondary_state = 0;
            }
           
            if (curTime > 90000 && wrong_color(seedNumRight) && abs(seedNumRight) == 5) {
                primary_state=FIND_BOX;
                secondary_state = 0;
            }

            lastState = state;
            break;
        }
        case 100: // Probably stuck, back up
            //bot.tone(880);
            bot.move(-200);
            if (curTime - timer > 200){
                timer = curTime;

                if (sideOnWhite(rs) && centerOnWhite(rc)) {
                    secondary_state = 101;
                }
                else if (sideOnGray(rs) && centerOnGray(rc)) {
                    secondary_state = 102;
                }
                else {
                    secondary_state = 0;
                }
            }
            //if( (rs > RSGWThreshold) && (rc > RCGWThreshold)) { //START PATRICK MOD--adds turning to backing up if both see grey
            //    bot.move(-100, 100);
            //} //END PATRICK MOD
            //else {
            //    bot.move(-200);
            //}
            break;
        case 101:   //stuck on white
        {
            int seedNumFront = bot.readGC(MudduinoBot::FORWARD);
            static int white_state = 0;
            static unsigned long noGCtime;
            static unsigned long turnTime;
            switch(white_state){
                case 0:
                    bot.move(0, 90);
                    if (abs(seedNumFront)<5 && seedNumFront != 0){
                        noGCtime=curTime;
                        white_state=1;
                    }

                    if (sideOnGray(rs)) {
                        white_state = 0;
                        secondary_state = 0;
                        switchTime = curTime;
                    }

                    if (curTime - timer > 5000) {
                        secondary_state = 100;
                        timer = curTime;
                    }
                
                    break;
                case 1:
                    bot.move(120,0);
                    if (abs(seedNumFront)!=0){
                        noGCtime=curTime;
                    }

                    if (sideOnGray(rs)) {
                        white_state = 0;
                        secondary_state = 0;
                        switchTime = curTime;
                    }

                    if((curTime-noGCtime)>500){
                        white_state=0;
                    }
                    break;
                case 2:
                    bot.move(0,70);
                    if ((curTime - turnTime)>400){
                        secondary_state=0;
                        white_state = 0;
                    }
                    break;
               }
            break;
        }
        case 102: // stuck on gray
            bot.move(-50, 100);
            if (curTime - timer > 300) {
                secondary_state = 0;
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
            bot.setServo(servo_out);
            if (curTime - timer > 1000) {
                timer = curTime;
                secondary_state = 0;
            }
            break;
        }
        break;







    case BLACK_LINE_FOLLOW: case FIND_BOX:
        switch(secondary_state) {
        case 0:          //find the black line with gc5
        {
            bot.move(0,70);
            int seedNumFront = bot.readGC(MudduinoBot::FORWARD);
            if ((primary_state == BLACK_LINE_FOLLOW && seedNumFront == 5) || (primary_state == FIND_BOX && seedNumFront == 6) ) {
                secondary_state=1;
            }
        }
        case 1:  //drive until black line is found
        {
            bot.move(120,0);
            if (sideOnBlack(rs) || centerOnBlack(rc)){
                secondary_state=2;
            }
        }
        case 2:
        {
            // follow blackline
            static int lastState = 0;
            int rs = bot.getSideReflect();
            int rc = bot.getCenterReflect();
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
                bot.move(120, -70);
                break;
            case 1: // center is on white--turn right
                bot.move(120, 50);
                break;
            case 2:
                bot.move(120, 0);  //center on black, side on white--go straight
                break;
            }
            bot.setServo(servo_in);

            if (bot.getBumper()){
                int seedNumFront = bot.readGC(MudduinoBot::FORWARD);
                if ((side && seedNumFront == -6) || (!side && seedNumFront == 6 )) {
                    secondary_state = 3; // Diverge to black line code
                }
                if ((side && seedNumFront == -5) || (!side && seedNumFront == 5 )) {
                    secondary_state = 13; // Diverge to box code
                }
            }
           
            lastState = state;
            break;
        }
        case 3: // TODO: Come back!!
            break;

        // This is  for box
        case 13: //has hit the beacon--turning
        {
            bot.move(0,-900);
            int seedNumFront = bot.readGC(MudduinoBot::FORWARD);
            if (seedNumFront == 7) {
                secondary_state=14;
            }
            break;
        }
        case 14:  //has acquired gc7, drives forward
            bot.move(120,0);
            if(((rs<RSGWThreshold)&&(rs>200))||((rc<RCGWThreshold)&&(rc>200))){
                secondary_state=15;
              }
            break;
        case 15: //has passed yellow once, slows down
            bot.move(90,0);
            if(((rs<RSGWThreshold)&&(rs>200))||((rc<RCGWThreshold)&&(rc>200))){
                secondary_state=16;
               }
            break;
        case 16: //has passed yellow twice, stops
            bot.halt();
            break;

        case 100: // Probably stuck, back up
            bot.move(-200);
            if (curTime - timer > 500){
                secondary_state = 0;
                timer = curTime;
            }
            break;
        }
        break;  //END PATRICK MOD
   
    }
}
