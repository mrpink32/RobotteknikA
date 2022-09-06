#include <Arduino.h>
#include <WIFI.h>

const int32_t PIN_STEP = 26;
const int32_t PIN_DIR = 27;
#define BREAK_TIME 1

int32_t steps = 0;
bool direction = true;

hw_timer_t *timer = NULL;
hw_timer_t *timer2 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timer2Mux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer2()
{
	// portENTER_CRITICAL_ISR(&timer2Mux);
	digitalWrite(PIN_STEP, LOW);
	steps++;
	// portEXIT_CRITICAL_ISR(&timer2Mux);
}

void IRAM_ATTR onTimer()
{
	portENTER_CRITICAL_ISR(&timerMux);
	digitalWrite(PIN_STEP, HIGH);
	timerWrite(timer2, 0);
	timerAlarmEnable(timer2);
	portEXIT_CRITICAL_ISR(&timerMux);
}

void toggleDirection(bool *direction)
{
	if (*direction)
	{
		*direction = false;
		digitalWrite(PIN_DIR, HIGH);
		delay(BREAK_TIME);
	}
	else
	{
		*direction = true;
		digitalWrite(PIN_DIR, LOW);
		delay(BREAK_TIME);
	}
}

void setup()
{
	// hello world
	Serial.begin(115200);
	Serial.println("Hello World!");

	// setup pin
	pinMode(PIN_STEP, OUTPUT);
	pinMode(PIN_DIR, OUTPUT);

	timer = timerBegin(0, 80, true); // 2 = 25ns, 80 = 1us
	timerAttachInterrupt(timer, &onTimer, true);
	timerAlarmWrite(timer, 120 /*1000*/, true);
	timerWrite(timer, 0);
	timerAlarmEnable(timer);

	timer2 = timerBegin(1, 80, true); // 2 = 25ns, 80 = 1us
	timerAttachInterrupt(timer2, &onTimer2, true);
	timerAlarmWrite(timer2, 8, false);
}

void loop()
{
	Serial.printf("Steps: %d, Dir: %d\n", steps, direction);
	if (steps >= 10000) // 10800
	{
		toggleDirection(&direction);
		steps = 0;
	}
}