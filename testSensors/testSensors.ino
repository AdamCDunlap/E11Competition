// testSensors.ino
#include <MudduinoBot.h>
#include <Servo.h>
#include <GoldCode.h>
#include <RunEvery.h>

MudduinoBot bot;
bool side;

#define PRINT(x) do { Serial.print(#x ": "); Serial.print(x); Serial.print('\t'); } while (0)

void setup() {
    Serial.begin(115200);
    bot.begin();

    int fb1[] = {5,2,3,4,5}; //First characteristic polynomial
    int fb2[] = {3,3,5}; // second
    int seeds[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 24};
    bot.cache_GCs(10, seeds, fb1, fb2);

    side = bot.getTeam();
}

void loop() {
    
    //PRINT(bot.getDist());
    //PRINT(bot.getFwdLight());
    //PRINT(+bot.getLeftLight());
    //PRINT(bot.getRightLight());
    //PRINT(bot.getCenterReflect());
    //PRINT(bot.getSideReflect());
    static uint32_t GC_fwd, GC_left, GC_right;
    bool done_fwd = bot.readGC_async(&GC_fwd, MudduinoBot::FORWARD);
    bool done_left = bot.readGC_async(&GC_left, MudduinoBot::LEFT);
    //bool done_right = bot.readGC_async(&GC_right, MudduinoBot::RIGHT);
    //runEvery(100) {
    if (done_fwd) {
        PRINT(micros());
        PRINT(done_fwd);
        //PRINT(done_left);
        //PRINT(done_right);
        PRINT(GC_fwd);
        PRINT(GC_left);
        PRINT(GC_right);
        Serial.println();
    }
}
