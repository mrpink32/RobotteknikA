#include <Arduino.h>
#include <WIFI.h>

#define PINA 26
#define PINB 27
#define BREAKTIME 2

void setup()
{
	// hello world
	Serial.begin(115200);
	Serial.println("Hello World!");

	// setup pin
	pinMode(PINA, OUTPUT);
	pinMode(PINB, OUTPUT);

	// blink io pin
	digitalWrite(PINA, HIGH);
	delay(BREAKTIME);
	digitalWrite(PINA, LOW);
	delay(BREAKTIME);
}	

void loop()
{
	// // blink io pin
	// digitalWrite(PINA, HIGH);
	// delay(BREAKTIME);
	// digitalWrite(PINA, LOW);
	// delay(BREAKTIME);
}