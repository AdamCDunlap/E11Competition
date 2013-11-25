// competition0.ino
#include <MudduinoBot.h>
#include <GoldCode.h>
#include <Servo.h>
#include <RunEvery.h>
#include <stdinout.h>
#include <avr/eeprom.h>
#include <MemoryFree.h>
#include <TimerOne.h>

MudduinoBot bot;

bool side;

struct Thresholds {
    int CGW;
    int SGW;
    int CBG;
    int SBG;
} thrs = { 750, 650, 910, 853 };

int location=270;

const int servo_in = 10;
const int servo_out = 175;

bool sideOnWhite(int rs) { return rs < thrs.SGW; }
bool centerOnWhite(int rc) { return rc < thrs.CGW; }
bool centerOnGray(int rc) { return rc > thrs.CGW && rc < thrs.CBG; }
bool sideOnGray(int rs) { return rs > thrs.SGW && rs < thrs.SBG; }
bool sideOnBlack(int rs) { return rs > thrs.SBG; }
bool centerOnBlack(int rc) { return rc > thrs.CBG; }
void setEEPROMThreshs();
void targetGC(bool seeGC, unsigned long frontVariance, bool lookRight);
void lineFollowCircle(int state);

// This function is called every 250 microseconds by a timer interrupt and keeps the gold codes a'flashin
//ISR(TIMER2_COMPA_vect) {
//    static bool go = true;
//    if (go) {
//        static int i = 0;
//        if(bot.flash_GC_async(i < 3? 6 : 7, !side)) {
//            i = i >= 5? 0 : i+1;
//        }
//    }
//    go = !go;
//}

// Flash GCs of 7 and 8 continuously, 3 at a time
// Should be called by interrupt every 250 microseconds
void flashMoreGC() {
    static int i = 0;
    if(bot.flash_GC_async(i < 3? 6 : 7, !side)) {
        i = i >= 5? 0 : i+1;
    }
}

void setup() {
    Serial.begin(115200);
    bot.begin();
    pinMode(5, OUTPUT);

    //bot.move(200);

    //while(1);

    //bot.setServo(servo_in);
    //delay(1000);
    //bot.setServo(servo_out);
    //delay(1000);
    //bot.setServo(servo_in);
    //while(1);

    Serial.println("Good morning! I was compiled at " __TIME__ " on " __DATE__);
    bot.setServo(servo_in);

    int fb1[] = {5,2,3,4,5}; //First characteristic polynomial
    int fb2[] = {3,3,5}; // second
    int seeds[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 24};
    bot.cache_GCs(10, seeds, fb1, fb2);

    side = bot.getTeam();

    if (!side && bot.getBumper()) {
        setEEPROMThreshs();
        while(true) ;
    }
    else {
        eeprom_read_block(&thrs, (void*)0, sizeof(thrs));
        printf("Values are: CGW: %4d SGW: %4d CBG: %4d SBG: %4d.\n",
                thrs.CGW, thrs.SGW, thrs.CBG, thrs.SBG);
    }

    //Serial.print("freeMemory()=");
    //Serial.println(freeMemory());

	//// on the ATmega168, timer 0 is also used for fast hardware pwm
	//// (using phase-correct PWM would mean that timer 0 overflowed half as often
	//// resulting in different millis() behavior on the ATmega8 and ATmega168)
	//sbi(TCCR0A, WGM01);
	//sbi(TCCR0A, WGM00);

    //// Set prescale to 64
	//sbi(TCCR0B, CS01);
	//sbi(TCCR0B, CS00);

    //
	//// enable timer 0 overflow interrupt
	//sbi(TIMSK, TOIE0);

    //
	//// timers 1 and 2 are used for phase-correct hardware pwm
	//// this is better for motors as it ensures an even waveform
	//// note, however, that fast pwm mode can achieve a frequency of up
	//// 8 MHz (with a 16 MHz clock) at 50% duty cycle

	//TCCR1B = 0;

	//// set timer 1 prescale factor to 64
	//sbi(TCCR1B, CS11);
	//sbi(TCCR1B, CS10);

	//// put timer 1 in 8-bit phase correct pwm mode
	//sbi(TCCR1A, WGM10);

    //
	//// set timer 2 prescale factor to 64
	//sbi(TCCR2B, CS22);


	//// configure timer 2 for phase correct pwm (8-bit)
	//sbi(TCCR2A, WGM20);


    //cli(); // Disable interrupts
    //

    // Setup timer interrupt on timer2 to call the ISR every 125 microseconds
    //cli(); // disable interrupts
    //TCCR2A = 0;// set entire TCCR2A register to 0
    //TCCR2B = 0;// same for TCCR2B
    //TCNT2  = 0;//initialize counter value to 0
    //// set compare match register for 8khz increments
    //OCR2A = 124;// = (16*10^6) / (8000*8) - 1 (must be <256)
    //// turn on CTC mode
    //TCCR2A |= (1 << WGM21);
    //// Set CS22 bit for 64 prescaler
    //TCCR2B |= (1 << CS22);   
    //// enable timer compare interrupt
    //TIMSK2 |= (1 << OCIE2A);
    //sei(); // enable interrupts

    //for (int i=0; i<256; i++) {
    //    bot.move(i);
    //    delay(100);
    //}
    //for (int i=255; i>-256; i--) {
    //    bot.move(i);
    //    delay(100);
    //}
    //for (int i=-255; i<0; i++) {
    //    bot.move(i);
    //    delay(100);
    //}
    //
    //while(1);

    //bot.serv.detach();

    pinMode(10, OUTPUT);
    //digitalWrite(10, LOW);
    //Timer1.initialize(250);
    //Timer1.attachInterrupt(flashMoreGC);
}

bool wrong_color(int seedNum) {
    return (side && seedNum < 0) || (!side && seedNum > 0);
}

void loop() {
    static enum { BEELINE, BEELINE_FIND_CIRCLE, ON_CIRCLE, BLACK_LINE_FOLLOW, FIND_BOX} primary_state = BEELINE;
    static uint8_t secondary_state;
    static unsigned long switchTime = 0;
    static unsigned long timer = 0;
    static unsigned long servoTime = 0;
    static int target_gc;
    unsigned long curTime = millis();
    unsigned long timeInState = curTime - switchTime;
    int rs = bot.getSideReflect();
    int rc = bot.getCenterReflect();

    int seedNumLeft = bot.readGC(MudduinoBot::LEFT);
    int seedNumRight = bot.readGC(MudduinoBot::RIGHT);
    unsigned long frontVariance = -1;
    int seedNumFront = bot.readGC(MudduinoBot::FORWARD, &frontVariance);

    runEvery(100) {
        printf("Primary: %d Secondary: %4d rs: %4d rc: %4d Seeds: L:%2d R:%2d C:%2d frontVar: %10ld targ: %d\n",
            primary_state, secondary_state, rs, rc, seedNumLeft, seedNumRight, seedNumFront, frontVariance, target_gc);
    }
    digitalWrite(5, wrong_color(seedNumLeft) || wrong_color(seedNumRight) || wrong_color(seedNumLeft));

    //targetGC(seedNumFront == 5, 0, true);

    //if (sideOnGray(rs)) bot.tone(200);
    //else bot.noTone();
    

    switch(primary_state) {
    case BEELINE:
        //bot.setServo(servo_in);
        switch(secondary_state){
        case 0:
            bot.move(200, 20);
            if (timeInState>1000 || (seedNumLeft==1 && side==1) || (seedNumRight==2 && side==0)){
                secondary_state=1;
                timer=curTime;
            }
            if (bot.getBumper()){
                switchTime = curTime;
                primary_state=ON_CIRCLE;
                secondary_state=100;
                timer = curTime;
            }
            break;
        case 1:
            bot.move(120);
            if (curTime-timer>200){
                secondary_state=2;
            }

            if (bot.getBumper() || timeInState > 1500) {
                primary_state = BEELINE_FIND_CIRCLE;
                switchTime = curTime;
                secondary_state = 0;
            }
            break;
        case 2:
            targetGC(seedNumFront==24, frontVariance, side);
            
            if (bot.getBumper() || timeInState > 5000) {
                primary_state = BEELINE_FIND_CIRCLE;
                switchTime = curTime;
                secondary_state = 0;
            }
            break;
        }
        break;

    case BEELINE_FIND_CIRCLE:
        switch(secondary_state) {
        case 0:
            bot.move(-150, -100);
            if ((side == 1 && timeInState > 500) || (side == 0 && timeInState > 750)) secondary_state++;
            break;
        case 1:
            bot.move(150, 50);
            /*if (rc > thrs.CBG) { // on black
                bot.halt();
            }
            else*/ if (rc > thrs.CGW) { // on grey
                primary_state = ON_CIRCLE;
                switchTime = curTime;
                secondary_state = 0;
            }
            else if (curTime - switchTime > 1000) { // timeout
                primary_state = ON_CIRCLE;
                switchTime = curTime;
                timer = curTime;
                secondary_state = 100; // start out lost
            }
        }
        break;










    case ON_CIRCLE:
        //bot.noTone();
        switch(secondary_state) {
        case 0:
        {
            // follow circle
            static int lastState = 0;
            int state;
            static unsigned long lastStateChangeTime = 0;

            if (sideOnWhite(rs))        state = 0;
            else if (centerOnBlack(rc)) state = 1;
            else                        state = 2;
            //state = sideOnGray(rs);

            if (state != lastState) {
                lastStateChangeTime = curTime;
            }

            lineFollowCircle(state);

            switch(seedNumLeft){
                case 1:
                    location=45;
                    break;
                case 2:
                    location=135;
                    break;
                case 3:
                    location=225;
                    break;
                case 4:
                    location=315;
                    break;
                default:
                    break;
            }
           
            //if (curTime > 90000 && wrong_color(seedNumRight) && abs(seedNumRight) == 5) {
            //    primary_state=FIND_BOX;
            //    secondary_state = 0;
            //}
            
            if (curTime - lastStateChangeTime > 3000) { // We've been in one state for a while
                secondary_state = 100;
                lastStateChangeTime = curTime;
                timer = curTime;
            }
            if (bot.getBumper()) {
                secondary_state = 100;
                lastStateChangeTime = curTime;
                timer = curTime;
            }
                    
            if (wrong_color(seedNumLeft) && abs(seedNumLeft) < 5 && curTime - timer > 500) {
                bot.halt();
                secondary_state = 10;
                timer = curTime;
                //bot.serv.attach(10);
            }
            //else if (curTime - servoTime > 750 && bot.serv.attached()) {
            //    bot.serv.detach();
            //    digitalWrite(10, LOW);
            //    Timer1.initialize(250);
            //    Timer1.attachInterrupt(flashMoreGC);
            //}


            lastState = state;
            break;
        }
       // case 100: // Probably stuck, back up
       // 
        case 100:
            //bot.tone(880);
            bot.move(-200);
            if (curTime - timer > 150){
                timer = curTime;

                if (sideOnWhite(rs) && centerOnWhite(rc)) {
                    secondary_state = 101;
                }
                else if ((sideOnGray(rs)||sideOnBlack(rs)) && (centerOnBlack(rc) || centerOnGray(rc))) {
                    secondary_state = 102;
                }
                else {
                    secondary_state = 0;
                }
            }
            //if( (rs > thrs.SGW) && (rc > thrs.CGW)) { //START PATRICK MOD--adds turning to backing up if both see grey
            //    bot.move(-100, 100);
            //} //END PATRICK MOD
            //else {
            //    bot.move(-200);
            //}
            break;
        case 101:   //stuck on white
        {
            static int white_state = 0;
            static unsigned long noGCtime;
            static unsigned long turnTime;
            switch(white_state){
            case 0:
                bot.move(-120);
                if (curTime - timer > 400) {
                    white_state = 1;
                    timer = curTime;
                }
                break;
            case 1:
                bot.move(0, 90);
                if (abs(seedNumFront)<5 && seedNumFront != 0){
                    noGCtime=curTime;
                    white_state=2;
                }

                if (sideOnGray(rs)) {
                    white_state = 1;
                    secondary_state = 100;
                    switchTime = curTime;
                }

                if (curTime - timer > 3000) {
                    white_state = 0;
                    timer = curTime;
                }
            
                break;
            case 2:
                bot.move(120,0);
                if (abs(seedNumFront)!=0){
                    noGCtime=curTime;
                }
            //case 0:
            //    bot.move(-120);
            //    if(curTime - timer > 500) {
            //        white_state = 2;
            //        timer = curTime;
            //    }
            //    break;

            //case 1:
            //    targetGC(abs(seedNumFront) < 5, 0, false);
                if (bot.getBumper() && sideOnGray(rs)) {
                    white_state = 3;
                    timer = curTime;
                    //secondary_state = 0;
                    //switchTime = curTime;
                }
                if (curTime - timer > 3000) {
                    timer = curTime;
                    white_state = 0;
                }

                if((curTime-noGCtime)>500){
                    white_state=1;
                }
                break;
            case 3:
                bot.move(-70);
                if(curTime - timer > 75) {
                    white_state = 1;
                    secondary_state = 102;
                    timer = curTime;
                }
                break;
            //case 4:
            //    bot.move(-120, 0);
            //    if(curTime - timer > 500) {
            //        white_state = 1;
            //        timer = curTime;
            //    }
            //
            }
            break;
        }
        case 102: // stuck on gray
            bot.move(-10, 90);
            if (curTime - timer > 2000 || centerOnWhite(rc)) {
                timer = curTime;
                secondary_state = 103;
            }
            break;
        case 103:
            bot.move(0, -120);
            if (curTime - timer > 100) {
                secondary_state = 0;
                timer = curTime;
            }
            break;
            
        case 10:
            //bot.serv.attach(10);
            bot.setServo(servo_out);
            if (curTime - timer > 500) {
                bot.setServo(servo_in);
                timer = curTime;
                secondary_state = 0;
                servoTime = curTime;
            }
            break;
        }
        // If we're either on circle or lost AND either the front or the right
        //  sensors see an outside GC of the wrong color or 24.
        
        if ( secondary_state == 0 || secondary_state >= 100) {
            bool found = false;
            if ((wrong_color(seedNumFront) && abs(seedNumFront)>=5) || seedNumFront == 24) {
                target_gc = seedNumFront;
                found = true;
            }
            else if ((wrong_color(seedNumRight) && abs(seedNumRight)>=5) || seedNumRight == 24) {
                target_gc = seedNumRight;
                found = true;
            }

            if (found) {
                primary_state = BLACK_LINE_FOLLOW;
                secondary_state = 0;
                bot.tone(440, 2000);
            }
        }
        break;






    case BLACK_LINE_FOLLOW: case FIND_BOX:
    {
        static unsigned long lastSeenTime = 0;
        bool seeGC = seedNumFront == target_gc;

        switch(secondary_state) {
        case 0:
            lastSeenTime = curTime;
            secondary_state = 1;
        
            if (abs(target_gc) == 7 || abs(target_gc) == 8) {
                bot.serv.detach();
                digitalWrite(10, LOW);
                Timer1.initialize(250);
                Timer1.attachInterrupt(flashMoreGC);
            }
        case 1:
        {
            targetGC(seeGC, 0, true);
            if (seeGC) {
                lastSeenTime = curTime;
            }
        
            if( false //   bot.getBumper()
                || curTime - lastSeenTime > 3000
                || seedNumFront == -target_gc
                || seedNumRight == -target_gc
                || seedNumLeft  == -target_gc) {
                // We hit something or we lost the GC. Go to circle's lost
                //primary_state = ON_CIRCLE;
                //secondary_state = 100;
                //switchTime = curTime;
                secondary_state = 2;
                timer = curTime;
                if (abs(target_gc == 7) || abs(target_gc == 8)) {
                    // reenable the servo
                    bot.serv.attach(10);
                }
                bot.tone(880, 500);
            }
            break;
        }
        case 2:
            bot.move(-200);
            if (curTime - timer > 500) {
                timer = curTime;
                secondary_state = 3;
            }
            break;
        case 3:
            bot.move(0, 120);
            if (curTime - timer > 200) {
                timer = curTime;
                switchTime = curTime;
                primary_state = ON_CIRCLE;
                secondary_state = 0;
            }
            break;
        }
        break;
    }
    }
}

void setEEPROMThreshs() {
    // Start reading values into EEPROM
    // Play some tones so we know we're in this mode
    bot.tone(880);
    delay(200);
    bot.tone(480);
    delay(200);
    bot.tone(880);
    delay(1000);
    bot.noTone();
    while(bot.getBumper()) ; // Wait for bumper to be released
    
    Serial.println("Position over white and press bumper");
    while(!bot.getBumper()) ; // wait for press and release
    delay(100);
    while(bot.getBumper()) ;
    int centerwhiteval = bot.getCenterReflect();
    int sidewhiteval = bot.getSideReflect();
    bot.tone(200);
    delay(500);
    bot.noTone();
    centerwhiteval = (centerwhiteval + bot.getCenterReflect())/2;
    sidewhiteval = (sidewhiteval + bot.getSideReflect())/2;
    printf("Got values centerwhite = %4d sidewhite = %4d\n", centerwhiteval, sidewhiteval);
    
    bot.tone(880, 500);
    Serial.println("Position over gray and press bumper");
    while(!bot.getBumper()) ; // wait for press and release
    delay(100);
    while(bot.getBumper()) ;
    int centergrayval = bot.getCenterReflect();
    int sidegrayval = bot.getSideReflect();
    bot.tone(200);
    delay(500);
    bot.noTone();
    centergrayval = (centergrayval + bot.getCenterReflect())/2;
    sidegrayval = (sidegrayval + bot.getSideReflect())/2;
    printf("Got values centergray = %4d sidegray = %4d\n", centergrayval, sidegrayval);
    
    bot.tone(880, 500);
    Serial.println("Position over black and press bumper");
    while(!bot.getBumper()) ; // wait for press and release
    delay(100);
    while(bot.getBumper()) ;
    int centerblackval = bot.getCenterReflect();
    int sideblackval = bot.getSideReflect();
    bot.tone(200);
    delay(500);
    bot.noTone();
    centerblackval = (centerblackval + bot.getCenterReflect())/2;
    sideblackval = (sideblackval + bot.getSideReflect())/2;
    printf("Got values centerblack = %4d sideblack = %4d\n", centerblackval, sideblackval);

    thrs.CGW = (centergrayval + centerwhiteval)/2;
    thrs.SGW = (sidegrayval + sidewhiteval)/2;
    thrs.CBG = (centerblackval + centergrayval)/2;
    thrs.SBG = (sideblackval + sidegrayval)/2;

    printf("Values are: CGW: %4d SGW: %4d CBG: %4d SBG: %4d.\n"
           "Press bumper to confirm write, or reset to clear.\n",
            thrs.CGW, thrs.SGW, thrs.CBG, thrs.SBG);
    while(!bot.getBumper()) ;
    delay(100);
    while(bot.getBumper()) ;

    eeprom_write_block(&thrs, (void*)0, sizeof(thrs));

    // All done, play another tone
    bot.tone(880);
    delay(200);
    bot.tone(480);
    delay(200);
    bot.tone(880);
    delay(200);
    bot.noTone();
}

void targetGC(bool seeGC, unsigned long frontVariance, bool lookRight) {
    static unsigned long lastSeenTime = 0;
    static unsigned long foundTime = 0;
    static bool didSeeGC = false;
    unsigned long curTime = millis();
    int factor = 1;
    if (!lookRight) factor = -1;
    if (seeGC) {
        lastSeenTime = curTime;
    }
    if (!didSeeGC && seeGC) {
        foundTime = curTime;
    }
    didSeeGC = seeGC;
    if (curTime - lastSeenTime > 500) {
        // We've lost it, turn around
        bot.move(0, factor*90);
    }
    //else if (curTime - lastSeenTime > 750) {
    //    // We've lost it for a bit, turn back but keep moving forward
    //    bot.move(0, factor*90);
    //}
    else if (curTime - foundTime < 20) {
        bot.move(0, -factor*120);
    }
    else {
        // We've seen the GC recently, turn slightly and move forward quickly
        //bot.move(150, -factor*40);
        bot.move(150, -factor*20);
        //bot.move(150);
    }
}



void lineFollowCircle(int state) {
    switch (state) {
    case 0: // side is on white
        bot.move(100, -50);
        break;
    case 1: // we're near black line
        bot.move(100, -12);
        break;
    case 2:
        bot.move(100, 20);
        break;
    }
}
