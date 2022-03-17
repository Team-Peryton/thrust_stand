#include <Arduino.h>
#define MOTOR_POLES_DIV_2 14

int r_byteIndex = 0;
int l_byteIndex = 0;
uint32_t tmp_m;
byte TelemBuffer[100];
uint8_t CRCTable[256];


uint8_t getCRCForByte(uint8_t val)
{
    uint32_t j;
    for (j = 0; j < 8; j++)
        val = (val & 0x80) ? 0x7 ^ (val << 1) : (val << 1);
    return val;
}
void telem_buildCRCTable()
{
    uint32_t i;
    // fill an array with CRC values of all 256 possible bytes
    for (i = 0; i < 256; i++)
        CRCTable[i] = getCRCForByte(i);
}
uint8_t telem_getCRC(uint8_t message[], uint32_t length)
{
    uint32_t i;
    uint32_t crc = 0;
    for (i = 0; i < length; i++)
        crc = CRCTable[crc ^ message[i]];
    return crc;
}

void setup_esc(){
    telem_buildCRCTable();
    Serial2.begin(115200, SERIAL_8N1);
    Serial.println("Hello!");
}

void loop_esc(){
    Serial.println("loop");
    Serial.println(Serial2.available());
    while (Serial2.available() > 0)
    {
        TelemBuffer[(l_byteIndex & 31)] = Serial2.read();
        l_byteIndex++;
    }
    if (l_byteIndex >= 10)
    {
        if (TelemBuffer[l_byteIndex - 1] == telem_getCRC(&(TelemBuffer[l_byteIndex - 10]), 9))
        {
            tmp_m = micros();
            Serial.printf("LEFT Consump=%d mAh\n", (TelemBuffer[l_byteIndex-5]<<8 | TelemBuffer[l_byteIndex-4]));
            Serial.printf("LEFT eRPM=%d\n", 100*(TelemBuffer[l_byteIndex-3]<<8 | TelemBuffer[l_byteIndex-2]));
            Serial.printf("LEFT RPM=%d\n", 100*(TelemBuffer[l_byteIndex-3]<<8 | TelemBuffer[l_byteIndex-2])/MOTOR_POLES_DIV_2);
            Serial.printf("LEFT CRC=%.2X\n",TelemBuffer[l_byteIndex-1]);
            Serial.printf("LEFT New CRC=%.2X\n",telem_getCRC(&(TelemBuffer[l_byteIndex-10]), 9));
            Serial.printf("l-temp %d\n", TelemBuffer[l_byteIndex - 10]);
            Serial.printf("l-volt %3.2f\n", 1.0 * (TelemBuffer[l_byteIndex - 9] << 8 | TelemBuffer[l_byteIndex - 8]) / 100);
            Serial.printf("l-amps %3.2f\n", 1.0 * (TelemBuffer[l_byteIndex - 7] << 8 | TelemBuffer[l_byteIndex - 6]) / 100);
            l_byteIndex = 0;
        }
    }
}