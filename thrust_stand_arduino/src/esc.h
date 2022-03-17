#include <Arduino.h>

void telem_buildCRCTable();
uint8_t telem_getCRC(uint8_t message[], uint32_t length);
uint8_t getCRCForByte(uint8_t val);
void esc_setup();
void esc_loop();