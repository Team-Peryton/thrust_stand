#include <Arduino.h>

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