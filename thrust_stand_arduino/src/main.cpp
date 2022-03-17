// modified from https://github.com/sparkfun/SparkFun_Qwiic_Scale_NAU7802_Arduino_Library/blob/master/examples/Example2_CompleteScale/Example2_CompleteScale.ino

#include <Wire.h>
#include <EEPROM.h>
#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h" 
#include "balance.h"
#include "esc.h"

NAU7802 forceBalance; //Create instance of the NAU7802 class
unsigned long allTime = 0;
unsigned long current_time;
bool calibrate = false;
bool settingsDetected = false;
const bool scale_present = false;
const bool esc_present = true;

void setup()
{
    Serial.begin(57600);

    if (scale_present) {
        Wire.begin();
        Wire.setClock(400000);

        if (forceBalance.begin() == false)
            Serial.println("Scale not detected. Please check wiring. Freezing..."); while (1);
    
        Serial.println("Scale detected!");
        readSystemSettings(forceBalance, settingsDetected); //Load zeroOffset and calibrationFactor from EEPROM
        forceBalance.setSampleRate(NAU7802_SPS_320); //Increase to max sample rate
        forceBalance.calibrateAFE(); //Re-cal analog front end when we change gain, sample rate, or channel 
        Serial.print("Zero offset: "); Serial.println(forceBalance.getZeroOffset());
        Serial.print("Calibration factor: "); Serial.println(forceBalance.getCalibrationFactor());
    }
    if (esc_present){
        setup_esc();
    }
}


void loop()
{
    current_time = micros() - allTime;
    Serial.print(current_time);

    if (esc_present){
        loop_esc();
        Serial.println();
    }
    
    if (scale_present){
        Serial.print(",");
        float currentWeight = forceBalance.getWeight(true,1);
        Serial.print(currentWeight); //Print 2 decimal places
        Serial.print(",");
        float currentReading = forceBalance.getReading();
        Serial.print(currentReading); //Print 2 decimal places
        Serial.print("\n");

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
}