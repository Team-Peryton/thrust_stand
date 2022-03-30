// modified from https://github.com/sparkfun/SparkFun_Qwiic_Scale_NAU7802_Arduino_Library/blob/master/examples/Example2_CompleteScale/Example2_CompleteScale.ino
#include "balance.h"
#include "esc.h"
#include <Wire.h>
#include <EEPROM.h>
#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h"
#include "ESP32Servo.h"

NAU7802 forceBalance;
BlheliEscTelemetry Esc;
unsigned long allTime = 0;
unsigned long current_time;
bool calibrate = false;
bool settingsDetected = false;
const bool scale_present = true;
const bool esc_present = true;
const bool servo_present = false;

Servo throttle;
ESP32PWM pwm;
int minUs = 1000;
int maxUs = 2000;
int pos;

void setup()
{
    Serial.begin(57600);

    if (servo_present){
        ESP32PWM::allocateTimer(1);
        throttle.setPeriodHertz(50);
        throttle.attach(34, minUs, maxUs);
        Serial.println("Servo setup completed");
    }

    if (scale_present){
        Wire.begin();
        Wire.setClock(400000); //Qwiic Scale is capable of running at 400kHz if desired

        if (forceBalance.begin() == false)
        {
            Serial.println("Scale not detected. Please check wiring. Freezing...");
            while (1);
        }
        Serial.println("Scale detected!");
    
        readSystemSettings(forceBalance, settingsDetected); //Load zeroOffset and calibrationFactor from EEPROM
        forceBalance.setSampleRate(NAU7802_SPS_320); //Increase to max sample rate
        forceBalance.calibrateAFE(); //Re-cal analog front end when we change gain, sample rate, or channel 
        forceBalance.setCalibrationFactor(111.08);
        forceBalance.setZeroOffset(63306);
        Serial.print("Zero offset: "); Serial.println(forceBalance.getZeroOffset());
        Serial.print("Calibration factor: "); Serial.println(forceBalance.getCalibrationFactor());

        Serial.println("Scales setup completed");
    }
    if (esc_present){
        Esc = BlheliEscTelemetry(14);

        Serial.println("ESC setup completed");
    }
}


void loop()
{
    current_time = micros() - allTime;

    if (servo_present){
        for (pos = 90; pos <= 180; pos += 1) { // sweep from 0 degrees to 180 degrees
		    // in steps of 1 degree
		    throttle.write(pos);
		    delayMicroseconds(10000);             // waits 20ms for the servo to reach the position
	    }
    }

    if (esc_present){
        Esc.update();
    }
    
    if (scale_present){

        if (calibrate == true)
        {
            if (forceBalance.available() == true)
            {
                long currentReading = forceBalance.getReading();
                float currentWeight = forceBalance.getWeight();

                Serial.print(currentReading);
                Serial.print(",");
                Serial.println(currentWeight, 2); //Print 2 decimal places

                if(settingsDetected == false)
                {
                    Serial.print("\tScale not calibrated. Press 'c'.");
                }
            }

            if (Serial.available())
            {
                byte incoming = Serial.read();

                if (incoming == 't') //Tare the scale
                    forceBalance.calculateZeroOffset();
                else if (incoming == 'c') //Calibrate
                {
                    calibrateScale(forceBalance);
                }
            }
        }      
    }

    float currentWeight = forceBalance.getWeight(true,1);
    float currentReading = forceBalance.getReading();

    Serial.print(current_time);
    Serial.print(",");
    Serial.print(Esc.eRPM);
    Serial.print(",");
    Serial.print(Esc.current);
    Serial.print(",");
    Serial.print(Esc.consumption);
    Serial.print(",");
    Serial.print(Esc.temperature);
    Serial.print(",");
    Serial.print(Esc.voltage);
    Serial.print(",");
    Serial.print(currentWeight); //Print 2 decimal places
    Serial.print(",");
    Serial.print(currentReading); //Print 2 decimal places
    Serial.print("\n");
}