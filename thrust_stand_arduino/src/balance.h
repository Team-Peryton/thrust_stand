#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h" 

void recordSystemSettings(NAU7802 &forceBalance);
void readSystemSettings(NAU7802 &forceBalance, bool settingsDetected);
void calibrateScale(NAU7802 &forceBalance);