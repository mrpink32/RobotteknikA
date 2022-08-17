#include <Arduino.h>
#include <WIFI.h>

#define PINLED 23
#define BREAKTIME 100

void setup()
{
	// hello world
	Serial.begin(115200);
	Serial.println("Hello World!");

	// setup pin
	pinMode(PINLED, OUTPUT);
}

void loop()
{
	// blink io pin
	digitalWrite(PINLED, HIGH);
	delayMicroseconds(BREAKTIME);
	digitalWrite(PINLED, LOW);
	delayMicroseconds(BREAKTIME);
}