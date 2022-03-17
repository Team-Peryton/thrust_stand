#include "Arduino.h"

const byte ledPin = 13;
const byte interruptPin = 39;
volatile byte state = LOW;
volatile long int up;
volatile long int change;

volatile unsigned long int a, b, c;
int temp[25], ch1[25], ch[16], i;
int x, y, z = 0;

void blink()
{
    state = !state;
    if (state)
    {
        up = micros();
    }
    else
    {
        change = micros() - up;
    }
}
void read_me()
{
    a = micros(); // store time value a when pin value falling
    c = a - b;    // calculating time inbetween two peaks
    b = a;        //
    temp[i] = c;     // storing 15 value in array
    i = i + 1;
    if (i == 25)
    {
        for (int j = 0; j < 25; j++)
        {
            ch1[j] = temp[j];
        }
        i = 0;
    }
}

void read_rc()
{
    for (z = 25; z > -1; z--)
    {
        if (ch1[z] > 10000)
        {
            y = z;
        }
    } // detecting separation space 10000us in that another array
    for (x = 1; x <= 15; x++)
    {
        ch[x] = (ch1[x + y]);
    }
}

void setup()
{
    Serial.begin(56700);
    pinMode(ledPin, OUTPUT);
    pinMode(interruptPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(interruptPin), read_me, FALLING);
}

void loop()
{   
    read_rc();
    Serial.print(ch[1]);Serial.print("\t");
    Serial.print(ch[2]);Serial.print("\t");
    Serial.print(ch[3]);Serial.print("\t");
    Serial.print(ch[4]);Serial.print("\t");
    Serial.print(ch[5]);Serial.print("\t");
    Serial.print(ch[6]);Serial.print("\t");
    Serial.print(ch[7]);Serial.print("\t");
    Serial.print(ch[8]);Serial.print("\t");
    Serial.print(ch[9]);Serial.print("\t");
    Serial.print(ch[10]);Serial.print("\t");
    Serial.print(ch[12]);Serial.print("\t");
    Serial.print(ch[13]);Serial.print("\t");
    Serial.print(ch[14]);Serial.print("\t");
    Serial.print(ch[15]);
    Serial.print("\n");
}