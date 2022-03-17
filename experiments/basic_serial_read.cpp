#include <Arduino.h>
uint8_t telem[10];

void setup(){
    Serial.begin(56700);
    Serial2.begin(115200, SERIAL_8N1);
}

void loop(){
    long int start = micros();
    Serial2.readBytes(telem, 10);
    for(int i=0; i<10;i++){
        Serial.print(telem[i]);
    }
    Serial.println();
    Serial.println(micros() - start);
}