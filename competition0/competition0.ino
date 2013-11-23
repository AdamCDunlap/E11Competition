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

    bot.serv.detach();

    pinMode(10, OUTPUT);
    digitalWrite(10, LOW);
    Timer1.initialize(250);
    Timer1.attachInterrupt(flashMoreGC);
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
    unsigned long curTime = millis();
    unsigned long timeInState = curTime - switchTime;
    int rs = bot.getSideReflect();
    int rc = bot.getCenterReflect();

    int seedNumLeft = bot.readGC(MudduinoBot::LEFT);
    int seedNumRight = bot.readGC(MudduinoBot::RIGHT);
    unsigned long frontVariance = -1;
    int seedNumFront = bot.readGC(MudduinoBot::FORWARD, &frontVariance);

    runEvery(100) {
        printf("Primary: %d Secondary: %4d rs: %4d rc: %4d Seeds: L:%2d R:%2d C:%2d frontVar: %10ld bump: %d cg: %d sg: %d\n",
            primary_state, secondary_state, rs, rc, seedNumLeft, seedNumRight, seedNumFront, frontVariance, bot.getBumper(), centerOnGray(rc), sideOnGray(rs));
    }

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
            }
            break;
        case 1:
            bot.move(120);
            if (curTime-timer>200){
                secondary_state=2;
            }

            if (bot.getBumper() || timeInState > 3000) {
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
            //if (state) {
            //    bot.move(80, 20);
            //}
            //else {
            //    bot.move(80, -50);
            //}
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
            //switch (state) {
            //case 0: // side is on white
            //    bot.move(120, -65);
            //    break;
            //case 1: // center is on grey
            //    bot.move(120, 45);
            //    break;
            //case 2:
            //    bot.move(120, -17);
            //    break;
            //}
            //bot.setServo(servo_in);

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
            if (wrong_color(seedNumLeft) && abs(seedNumLeft) < 5 && curTime - timer > 500) {
                bot.halt();
                secondary_state = 10;
                timer = curTime;
                bot.serv.attach(10);
            }
           
            //if (wrong_color(seedNumRight) && abs(seedNumRight)==6) {
            //    primary_state = BLACK_LINE_FOLLOW;
            //    secondary_state = 0;
            //}
           
            if (curTime > 90000 && wrong_color(seedNumRight) && abs(seedNumRight) == 5) {
                primary_state=FIND_BOX;
                secondary_state = 0;
            }
            
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

            if (curTime - servoTime > 750) {
                bot.serv.detach();
                servoTime = curTime + 99999999;
                digitalWrite(10, LOW);
                Timer1.initialize(250);
                Timer1.attachInterrupt(flashMoreGC);
            }


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
                    bot.move(0, 115);
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
                        white_state = 2;
                        timer = curTime;
                        //secondary_state = 0;
                        //switchTime = curTime;
                    }

                    if((curTime-noGCtime)>500){
                        white_state=0;
                    }
                    break;
                case 2:
                    bot.move(-70);
                    if(curTime - timer > 75) {
                        white_state = 0;
                        secondary_state = 102;
                        timer = curTime;
                    }
                    break;
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
            bot.move(0, -110);
            if (curTime - timer > 100) {
                secondary_state = 0;
                timer = curTime;
            }
            break;
            
        case 10:
            bot.setServo(servo_out);
            if (curTime - timer > 500) {
                timer = curTime;
                secondary_state = 0;
                servoTime = curTime;
            }
            break;
        }
        break;







    case BLACK_LINE_FOLLOW: case FIND_BOX:
        //bot.tone(220);
        switch(secondary_state) {
        case 0:          //find the black line with gc5
        {
            bot.move(0,100);
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
        case 2: case 6:
        {
            // follow blackline
            static int lastState = 0;
            int rs = bot.getSideReflect();
            int rc = bot.getCenterReflect();
            int state;
            static unsigned long lastStateChangeTime = 0;
            if (rs > thrs.SGW) state = 0;  //using grey-white threshold for stronger contrast
            else if (rc < thrs.CGW) state = 1;
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

            if (secondary_state == 2 && bot.getBumper()){
                if ((side && seedNumFront == -6) || (!side && seedNumFront == 6 )) {
                    secondary_state = 3; // Diverge to black line code
                    timer = curTime;
                }
                if ((side && seedNumFront == -5) || (!side && seedNumFront == 5 )) {
                    secondary_state = 13; // Diverge to box code
                }
            }
            if (secondary_state == 6 && centerOnGray(rc) && sideOnGray(rs)) {
                secondary_state = 7;
                timer = curTime;
            }              
           
            lastState = state;
            break;
        }
        case 3:
            bot.move(-100);
            if (curTime - timer >= 100) {
                secondary_state = 4;
                timer = curTime;
            }
            break;
        case 4:
            bot.move(0, 130);
            if (curTime-timer>=150){
              secondary_state =5;
              timer=curTime;
            }
            break;
        case 5:
            bot.move(0,100);
            if (centerOnBlack(rc)){
                secondary_state=6; // above
                timer=curTime;
            }
          break;
        case 7:
            bot.move(75, 120);
            if (sideOnWhite(rs)) {
              primary_state = ON_CIRCLE;
              secondary_state = 0;
              timer = curTime;
            }
            break;


        // This is  for box
        case 13: //has hit the beacon--turning
        {
            bot.move(0,-90);
            if (seedNumFront == 7) {
                secondary_state=14;
            }
            break;
        }
        case 14:  //has acquired gc7, drives forward
            bot.move(120,0);
            if(((rs<thrs.SGW)&&(rs>200))||((rc<thrs.CGW)&&(rc>200))){
                secondary_state=15;
              }
            break;
        case 15: //has passed yellow once, slows down
            bot.move(90,0);
            if(((rs<thrs.SGW)&&(rs>200))||((rc<thrs.CGW)&&(rc>200))){
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
    unsigned long curtime = micros();
    int factor = 1;
    if (!lookRight) factor = -1;
    if (seeGC) {
        lastSeenTime = curtime;
    }
    if (curtime - lastSeenTime > 1500) {
        // We've lost it; turn hard
        bot.move(0, factor*120);
    }
    else if (curtime - lastSeenTime > 500) {
        // We've lost it for a bit, turn back but keep moving forward
        bot.move(100, factor*60);
    }
    else {
        // We've seen the GC recently, turn slightly and move forward quickly
        bot.move(200, -factor*30);
    }
}
