// GetTwentyFour.ino
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
}

void loop() {
    static enum { BEELINE, INSIDE, OUTSIDE } primary_state = BEELINE;
    static unsigned long switchTime = 0;
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
        if (timeInState < 1500) {
            bot.move(255);
        }
        else if (timeInState < 3000) {
            bot.move(255 - (timeInState - 1500)/10);
        }
        else {
            // Seomthing weird's going on, stop
            bot.move(0);
        }
        break;
    }


    int seedNum = bot.readGC();
    if ((side && seedNum < 0) || (!side && seedNum > 0)) {
        bot.tone(440, 100);
    }
    
}
