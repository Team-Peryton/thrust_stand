#include <Arduino.h>
#define ESCSERIAL Serial2

/**
 * Get telemetry from a Blheli ESC over the telemetry line
 */
class BlheliEscTelemetry {
    private:
        int r_byteIndex = 0;
        int l_byteIndex = 0;
        uint32_t tmp_m;
        byte TelemBuffer[100];
        uint8_t CRCTable[256];
        int num_motor_poles;
    
    public:
        int consumption = -1;
        int eRPM = -1;
        int RPM = -1;
        int temperature = -1;
        float voltage = -1;
        float current = -1;
        
        BlheliEscTelemetry(int motor_poles);
        uint8_t telem_getCRC(uint8_t message[], uint32_t length);
        void telem_buildCRCTable();
        uint8_t getCRCForByte(uint8_t val);
        void update();
};

/**
 * Constructor, generates CRC table and begins serial connection
 */
BlheliEscTelemetry::BlheliEscTelemetry(int motor_poles){
    telem_buildCRCTable();
    ESCSERIAL.begin(115200, SERIAL_8N1);
    num_motor_poles = motor_poles;
}

/**
 * check integrity of telemetry packet
 */
uint8_t BlheliEscTelemetry::telem_getCRC(uint8_t message[], uint32_t length)
{
    uint32_t i;
    uint32_t crc = 0;
    for (i = 0; i < length; i++)
        crc = CRCTable[crc ^ message[i]];
    return crc;
}

/**
 * 
 */
void BlheliEscTelemetry::telem_buildCRCTable()
{
    uint32_t i;
    // fill an array with CRC values of all 256 possible bytes
    for (i = 0; i < 256; i++)
        CRCTable[i] = getCRCForByte(i);
}

/**
 * 
 */
uint8_t BlheliEscTelemetry::getCRCForByte(uint8_t val)
{
    uint32_t j;
    for (j = 0; j < 8; j++)
        val = (val & 0x80) ? 0x7 ^ (val << 1) : (val << 1);
    return val;
}

/**
 * check the serial connection and store new values 
 */
void BlheliEscTelemetry::update()
{
    while (ESCSERIAL.available() > 0)
    {
        TelemBuffer[(l_byteIndex & 31)] = ESCSERIAL.read();
        l_byteIndex++;
    }
    if (l_byteIndex >= 10)
    {
        if (TelemBuffer[l_byteIndex - 1] == telem_getCRC(&(TelemBuffer[l_byteIndex - 10]), 9))
        {
            tmp_m = micros();
            consumption = TelemBuffer[l_byteIndex-5]<<8 | TelemBuffer[l_byteIndex-4];
            eRPM = 100*(TelemBuffer[l_byteIndex-3]<<8 | TelemBuffer[l_byteIndex-2]);
            RPM = 100*((TelemBuffer[l_byteIndex-3]<<8 | TelemBuffer[l_byteIndex-2])/num_motor_poles);
            temperature = TelemBuffer[l_byteIndex - 10];
            voltage = 1.0 * (TelemBuffer[l_byteIndex - 9] << 8 | TelemBuffer[l_byteIndex - 8]) / 100;
            current = 1.0 * (TelemBuffer[l_byteIndex - 7] << 8 | TelemBuffer[l_byteIndex - 6]) / 100;
            l_byteIndex = 0;
        }
    }
}