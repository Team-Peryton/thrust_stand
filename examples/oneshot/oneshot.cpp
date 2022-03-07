/*
KISS ESC 24A Serial Example Code for Arduino.
made for a nanowii / promicro / arduino leonardo / atmega32µ4 arduino compatible boards as it uses tim 1 compare 1 and serial 1 RX.

connect the ESC's PWM signal to pin D9 and the telemetry pin of the ESC to the RX(I)/D0 pin

connect a signal source to pin D7 (servotester or RX or something else to give a throttle signal.. the oneshot 125 reading of the arduino will jitter a lot because of slow arduino ISR and SW reading)

use a serial monitor (arduino serial monitor) with 9600 baud to view the ESC's telemetry
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Wire.h>

// Global Variables
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int16_t ESC_telemetry[5]; // Temperature, Voltage, Current, used mAh, eRpM
static uint16_t requesttelemetry = 0;
static uint16_t regularThrottleSignal = 1000;

static uint8_t receivedBytes = 0;

// get the telemetry from the ESC
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void receiveTelemetry()
{
    static uint8_t SerialBuf[10];

    if (receivedBytes < 9)
    { // collect bytes
        while (Serial1.available())
        {
            SerialBuf[receivedBytes] = Serial1.read();
            receivedBytes++;
        }
        if (receivedBytes == 10)
        { // transmission complete

            uint8_t crc8 = get_crc8(SerialBuf, 9); // get the 8 bit CRC

            if (crc8 != SerialBuf[9])
                return; // transmission failure

            // compute the received values
            ESC_telemetry[0] = SerialBuf[0];                       // temperature
            ESC_telemetry[1] = (SerialBuf[1] << 8) | SerialBuf[2]; // voltage
            ESC_telemetry[2] = (SerialBuf[3] << 8) | SerialBuf[4]; // Current
            ESC_telemetry[3] = (SerialBuf[5] << 8) | SerialBuf[6]; // used mA/h
            ESC_telemetry[4] = (SerialBuf[7] << 8) | SerialBuf[8]; // eRpM *100
        }
    }
}

// 8-Bit CRC
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t update_crc8(uint8_t crc, uint8_t crc_seed)
{
    uint8_t crc_u, i;
    crc_u = crc;
    crc_u ^= crc_seed;
    for (i = 0; i < 8; i++)
        crc_u = (crc_u & 0x80) ? 0x7 ^ (crc_u << 1) : (crc_u << 1);
    return (crc_u);
}

uint8_t get_crc8(uint8_t *Buf, uint8_t BufLen)
{
    uint8_t crc = 0, i;
    for (i = 0; i < BufLen; i++)
        crc = update_crc8(Buf[i], crc);
    return (crc);
}

// read input PWM on D7
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ISR(INT6_vect)
{
    static uint16_t now, diff;
    static uint16_t last = 0;
    now = micros();
    if (!(PINE & (1 << 6)))
    {
        delayMicroseconds(5);
        if ((PINE & (1 << 6)))
            return;
        uint16_t newDiff = now - last;
        diff = (diff + diff + diff + newDiff) >> 2; // filter
        if (diff < 275 && diff > 100)
        { // oneshot125
            regularThrottleSignal = diff << 3;
        }
        else if (diff < 2250 && diff > 850)
        { // normal PWM
            regularThrottleSignal = diff;
        }
        else
            regularThrottleSignal = 1000;
    }
    else
    {
        delayMicroseconds(5);
        if (!(PINE & (1 << 6)))
            return;
        last = now;
    }
}

// Setup
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
    // init timer 1
    pinMode(9, OUTPUT);
    TCNT1 = 0;
    TCCR1C = 0;
    TIMSK1 = 0;
    TCCR1A = B10101010; // PWM_WaveformMode=14 -> Fast_PWM, TOP=ICRn, PWM_OutputMode=non-inverting
    TCCR1B = B00011001; // Prescaler=clk/1 / Imp=125.. 250us @11Bit oder Imp=1000.. 2000us @14Bit
    ICR1 = 0xFFFF;      // set TOP TIMER1 to max

    DDRE &= ~(1 << 6);    // pin 7 to input
    PORTE |= (1 << 6);    // enable pullups
    EIMSK |= (1 << INT6); // enable interuppt
    EICRB |= (1 << ISC60);

    Serial.begin(9600);    // open seria0 for serial monitor
    Serial1.begin(115200); // open Serial1 for ESC communication
}

// Main Loop
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
    static uint32_t loopTime = 0;
    static uint8_t telemetryCounter = 0;
    if (micros() - loopTime > 2000)
    { // 2000Hz looptime
        loopTime = micros();
        receiveTelemetry(); // look for incoming telemetry
        if (++telemetryCounter == 20)
        { // get telemetry with 25Hz
            telemetryCounter = 0;
            receivedBytes = 0; // reset bytes counter
            // request telemetry with a 30µs signal
            OCR1A = 30 << 4;
        }
        else
        {
            OCR1A = regularThrottleSignal << 1;
        }

        // print the telemetry
        if (telemetryCounter == 10)
        {
            Serial.println("Requested telemetry");
            Serial.print("Temperature (C): ");
            Serial.println(ESC_telemetry[0]);
            Serial.print("Voltage: (V) /100: ");
            Serial.println(ESC_telemetry[1]);
            Serial.print("Current (A) /100: ");
            Serial.println(ESC_telemetry[2]);
            Serial.print("used mA/h: ");
            Serial.println(ESC_telemetry[3]);
            Serial.print("eRpM *100: ");
            Serial.println(ESC_telemetry[4]);
            Serial.println(" ");
            Serial.println(" ");
        }
        else
        {
            // fire oneshot only when not sending the serial datas .. arduino serial library is too slow
            TCNT1 = 0xFFFF;
            OCR1A = 0;
        }
    }
}
