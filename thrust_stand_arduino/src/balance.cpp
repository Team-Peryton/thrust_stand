#include <EEPROM.h>
#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h" 

//EEPROM locations to store 4-byte variables
#define LOCATION_CALIBRATION_FACTOR 0 //Float, requires 4 bytes of EEPROM
#define LOCATION_ZERO_OFFSET 10 //Must be more than 4 away from previous spot. Long, requires 4 bytes of EEPROM

//Record the current system settings to EEPROM


void recordSystemSettings(NAU7802 &forceBalance)
{
    //Get various values from the library and commit them to NVM
    EEPROM.put(LOCATION_CALIBRATION_FACTOR, forceBalance.getCalibrationFactor());
    EEPROM.put(LOCATION_ZERO_OFFSET, forceBalance.getZeroOffset());
}

//Reads the current system settings from EEPROM
//If anything looks weird, reset setting to default value
void readSystemSettings(NAU7802 &forceBalance, bool &settingsDetected)
{
    float settingCalibrationFactor; //Value used to convert the load cell reading to lbs or kg
    unsigned long settingZeroOffset; //Zero value that is found when scale is tared

    //Look up the calibration factor
    EEPROM.get(LOCATION_CALIBRATION_FACTOR, settingCalibrationFactor);
    if (settingCalibrationFactor == 0xFFFFFFFF)
    {
        settingCalibrationFactor = 0; //Default to 0
        EEPROM.put(LOCATION_CALIBRATION_FACTOR, settingCalibrationFactor);
    }

    //Look up the zero tare point
    EEPROM.get(LOCATION_ZERO_OFFSET, settingZeroOffset);
    if (settingZeroOffset == 0xFFFFFFFF)
    {
        settingZeroOffset = 1000L; //Default to 1000 so we don't get inf
        EEPROM.put(LOCATION_ZERO_OFFSET, settingZeroOffset);
    }

    //Pass these values to the library
    forceBalance.setCalibrationFactor(settingCalibrationFactor);
    forceBalance.setZeroOffset(settingZeroOffset);

    settingsDetected = true; //Assume for the moment that there are good cal values
    if (settingCalibrationFactor < 0.1 || settingZeroOffset == 1000)
        settingsDetected = false; //Defaults detected. Prompt user to cal scale.
}

//Gives user the ability to set a known weight on the scale and calculate a calibration factor
void calibrateScale(NAU7802 &forceBalance)
{
    Serial.println();
    Serial.println();
    Serial.println(F("Scale calibration"));

    Serial.println(F("Setup scale with no weight on it. Press a key when ready."));
    while (Serial.available()) Serial.read(); //Clear anything in RX buffer
    while (Serial.available() == 0) delay(10); //Wait for user to press key

    forceBalance.calculateZeroOffset(64); //Zero or Tare the scale. Average over 64 readings.
    Serial.print(F("New zero offset: "));
    Serial.println(forceBalance.getZeroOffset());

    Serial.println(F("Place known weight on scale. Press a key when weight is in place and stable."));
    while (Serial.available()) Serial.read(); //Clear anything in RX buffer
    while (Serial.available() == 0) delay(10); //Wait for user to press key

    Serial.print(F("Please enter the weight, without units, currently sitting on the scale (for example '4.25'): "));
    while (Serial.available()) Serial.read(); //Clear anything in RX buffer
    while (Serial.available() == 0) delay(10); //Wait for user to press key

    //Read user input
    float weightOnScale = Serial.parseFloat();
    Serial.println();

    forceBalance.calculateCalibrationFactor(weightOnScale, 64); //Tell the library how much weight is currently on it
    Serial.print(F("New cal factor: "));
    Serial.println(forceBalance.getCalibrationFactor(), 2);

    Serial.print(F("New Scale Reading: "));
    Serial.println(forceBalance.getWeight(), 2);

    recordSystemSettings(forceBalance); //Commit these values to EEPROM
}
