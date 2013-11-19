#include <MudduinoBot.h>
//using namespace MudduinoBot;

void MudduinoBot::motorEn() {
    digitalWrite(len_pin, HIGH);
    digitalWrite(ren_pin, HIGH);
}
void MudduinoBot::initMotors() {
    digitalWrite(lplus_pin, LOW);
    digitalWrite(lminus_pin, LOW);
    digitalWrite(len_pin, LOW);
    digitalWrite(rplus_pin, LOW);
    digitalWrite(rminus_pin, LOW);
    digitalWrite(ren_pin, LOW);

    pinMode(lplus_pin, OUTPUT);
    pinMode(lminus_pin, OUTPUT);
    pinMode(len_pin, OUTPUT);
    pinMode(rplus_pin, OUTPUT);
    pinMode(rminus_pin, OUTPUT);
    pinMode(ren_pin, OUTPUT);
}
void MudduinoBot::halt() {
    motorEn();
    digitalWrite(lplus_pin, LOW);
    digitalWrite(lminus_pin, LOW);
    digitalWrite(rplus_pin, LOW);
    digitalWrite(rminus_pin, LOW);
}
void MudduinoBot::forward() {
    motorEn();
    digitalWrite(lplus_pin, HIGH);
    digitalWrite(lminus_pin, LOW);
    digitalWrite(rplus_pin, HIGH);
    digitalWrite(rminus_pin, LOW);
}
void MudduinoBot::backward() {
    motorEn();
    digitalWrite(lplus_pin, LOW);
    digitalWrite(lminus_pin, HIGH);
    digitalWrite(rplus_pin, LOW);
    digitalWrite(rminus_pin, HIGH);
}
void MudduinoBot::turnL() {
    motorEn();
    digitalWrite(lplus_pin, LOW);
    digitalWrite(lminus_pin, HIGH);
    digitalWrite(rplus_pin, HIGH);
    digitalWrite(rminus_pin, LOW);
}
void MudduinoBot::turnR() {
    motorEn();
    digitalWrite(lplus_pin, HIGH);
    digitalWrite(lminus_pin, LOW);
    digitalWrite(rplus_pin, LOW);
    digitalWrite(rminus_pin, HIGH);
}
void MudduinoBot::move(int fwd, int turn) {
    int leftPow = fwd + turn;
    int rightPow = fwd - turn;
    leftPow = constrain(leftPow, -255, 255);
    rightPow = constrain(rightPow, -255, 255);
    //Serial.print("Left: ");
    //Serial.print(leftPow);
    //Serial.print("\tRight: ");
    //Serial.print(rightPow);
    //Serial.println();
    analogWrite(len_pin, abs(leftPow));
    analogWrite(ren_pin, abs(rightPow));

    if (leftPow >= 0) {
        digitalWrite(lminus_pin, LOW);
        digitalWrite(lplus_pin, HIGH);
    }
    else {
        digitalWrite(lminus_pin, HIGH);
        digitalWrite(lplus_pin, LOW);
    }
    if (rightPow >= 0) {
        digitalWrite(rplus_pin, HIGH);
        digitalWrite(rminus_pin, LOW);
    }
    else {
        digitalWrite(rplus_pin, LOW);
        digitalWrite(rminus_pin, HIGH);
    }
}

void MudduinoBot::initServo() {
    serv.attach(serv_pin);
}
void MudduinoBot::setServo(int x) {
    serv.write(x);
}
void MudduinoBot::setServoRaw(int x) {
    serv.write(x);
}

void MudduinoBot::initBuzzer() {
}
void MudduinoBot::tone(unsigned int frequency, unsigned long duration) {
    ::tone(buzzer_pin, frequency, duration);
}
void MudduinoBot::noTone() {
    ::noTone(buzzer_pin);
}

void MudduinoBot::initSensors() {
    pinMode(dist_pin, INPUT);
    pinMode(light_fwd_pin, INPUT);
    pinMode(light_left_pin, INPUT);
    pinMode(light_right_pin, INPUT);
    pinMode(reflect_center_pin, INPUT);
    pinMode(reflect_side_pin, INPUT);
    pinMode(team_pin, INPUT);
    pinMode(bumper_pin, INPUT);
    digitalWrite(bumper_pin, HIGH); // enable pullup
}
int MudduinoBot::getDist() {
    return analogRead(dist_pin);
}
int MudduinoBot::getLight() {
    return analogRead(light_fwd_pin);
}
int MudduinoBot::getFwdLight() {
    return analogRead(light_fwd_pin);
}
int MudduinoBot::getRightLight() {
    return analogRead(light_right_pin);
}
int MudduinoBot::getLeftLight() {
    return analogRead(light_left_pin);
}
int MudduinoBot::getReflect() {
    return getCenterReflect();
}
int MudduinoBot::getCenterReflect() {
    return analogRead(reflect_center_pin);
}
int MudduinoBot::getSideReflect() {
    return analogRead(reflect_side_pin);
}
bool MudduinoBot::onWhite() {
    return getReflect() < 700;
}
bool MudduinoBot::getTeam() {
    return digitalRead(team_pin);
}

bool MudduinoBot::getBumper() {
    return digitalRead(bumper_pin);
}

void MudduinoBot::cache_GCs(int num_cached, int* seeds, int* fb1, int* fb2) {
    num_cached_GCs = num_cached;
    //cached_gcs = (uint32_t*) malloc(num_cached * sizeof (uint32_t));
    //cached_gc_seeds = (int*) malloc(num_cached * sizeof (int));
    cached_gcs = new uint32_t[num_cached];
    cached_gc_seeds = new int[num_cached];
    memcpy(cached_gc_seeds, seeds, sizeof(int)*num_cached_GCs);

    for (int i=0; i<num_cached; i++) {
        cached_gcs[i] = GoldCode::goldCode(fb1, fb2, cached_gc_seeds[i]);
    }
}

int MudduinoBot::readGC(lightSensor l, unsigned int* variance, unsigned int* readings) {
    unsigned int sum = 0;
    uint8_t lightPin;
    switch(l) {
        case LEFT: lightPin = light_left_pin; break;
        case RIGHT: lightPin = light_right_pin; break;
        case FORWARD: lightPin = light_fwd_pin; break;
        default: lightPin = -1; Serial.println("readGC called with unknown light sensor");
    }

    unsigned int buf[31];
    if (readings == NULL) readings = buf;
    for (int i=0; i<31;) {
        static unsigned long lastReadTime = micros();
        if (micros() - lastReadTime >= 250) {
            lastReadTime = micros();
            readings[i] = analogRead(lightPin);
            sum += readings[i];
            i++;
        }
    }

    unsigned int avg = sum/31;

    if (variance != NULL) {
        for (int i=0; i<31; i++) {
            int diff = readings[i] - avg;
            *variance += diff * diff;
        }
    }
    
    uint32_t gc = 0;
    for (int i=0; i < 31; i++) {
        // For each reading, convert it to binary by comparing to the average, then 
        //  OR it into the GC backwards, with the first element of the array
        //  going in the 30th position, etc
        gc |= (uint32_t)(readings[30-i] < avg) << i;
    }

    // Check the gold code against each one of the possibilities
    for (int i=0; i<num_cached_GCs; i++) {
        // sameGC returns the positive score if it's the same, or the negative score if it's inverted
        int same = GoldCode::sameGC(gc, cached_gcs[i]);
        if (same != 0) {
            return  (same > 0? +1 : -1) * cached_gc_seeds[i];
        }
    }
    // If nothing matched, return 0
    return 0;
}

bool MudduinoBot::readGC_async(uint32_t* GC_ret, lightSensor l, LightVals** vals) {
    unsigned long curTime = micros();
    if (vals != NULL) *vals = &lightVals[l];
    
    if (curTime - lightVals[l].lastReadTime < 250) {
        Serial.println('a');
        return false;
    }
    lightVals[l].lastReadTime = curTime;
    //Serial.println(curTime);

    uint8_t lightPin;
    switch(l) {
        case LEFT: lightPin = light_left_pin; break;
        case RIGHT: lightPin = light_right_pin; break;
        case FORWARD: lightPin = light_fwd_pin; break;
        default: lightPin = -1; Serial.println("readGC_async called with unknown light sensor"); return false;
    }
    lightVals[l].vals[lightVals[l].pos] = analogRead(lightPin);
    lightVals[l].pos++;
    
    if (lightVals[l].pos >= 32) {
        lightVals[l].pos = 0;
        
        unsigned int sum = 0;
        for (int i=0; i<31; i++) {
            sum += lightVals[l].vals[i];
        }
        unsigned int avg = sum/31;

        uint32_t gc = 0;
        
        for (int i=0; i < 31; i++) {
            // For each reading, convert it to binary by comparing to the average, then 
            //  OR it into the GC backwards, with the first element of the array
            //  going in the 30th position, etc
            gc |= (uint32_t)(lightVals[l].vals[30-i] < avg) << i;
        }

        // Check the gold code against each one of the possibilities
        for (int i=0; i<num_cached_GCs; i++) {
            // sameGC returns the positive score if it's the same, or the negative score if it's inverted
            int same = GoldCode::sameGC(gc, cached_gcs[i]);
            if (same != 0) {
                *GC_ret = (same > 0? +1 : -1) * cached_gc_seeds[i];
                return true;
            }
        }
        // If nothing matched, return 0
        *GC_ret = 0;
        return true;
    }
    return false;
}


void MudduinoBot::initLEDs() {
    pinMode(led_pin, OUTPUT);
}

void MudduinoBot::flash_GC(int which, bool inverted) {
    for (int i=0; i<31;) {
        static unsigned long lastWriteTime = 0;
        if (micros() - lastWriteTime >= 250) {
            lastWriteTime = micros();
            digitalWrite(led_pin, inverted ^ !!(cached_gcs[which] & (1UL << (30-i))));
            i++;
        }
    }
}

bool MudduinoBot::flash_GC_async(int which, bool inverted) {
    //unsigned long curTime = micros();
    //if (curTime - flashPos.lastTime < 250) return false;
    //flashPos.lastTime = curTime;

    digitalWrite(led_pin, inverted ^ !!(cached_gcs[which] & (1UL << (30 - flashPos.pos))));
    flashPos.pos++;
    if (flashPos.pos >= 32) {
        flashPos.pos = 0;
        return true;
    }
    return false;
}
