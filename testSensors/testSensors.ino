// testSensors.ino
#include <MudduinoBot.h>
#include <Servo.h>
#include <GoldCode.h>

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
    
    PRINT(bot.getDist());
    PRINT(bot.getLight());
    PRINT(bot.getFwdLight());
    PRINT(+  bot.getLeftLight());
    PRINT(bot.getRightLight());
    PRINT(bot.getReflect());
    PRINT(bot.getCenterReflect());
    PRINT(bot.getSideReflect());
    //PRINT(bot.readGC(MudduinoBot::FORWARD));
    //PRINT(bot.readGC(MudduinoBot::LEFT));
    //PRINT(bot.readGC(MudduinoBot::RIGHT));
    Serial.println();
}
