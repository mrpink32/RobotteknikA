#include <Arduino.h>
#include <math.h>
#include <vector>

const int32_t TIME = 1500;
const double_t TIMESQ = pow(TIME, 2);
const int32_t MAX_DISTANCE = 10000;
const int32_t PIN_STEP = 26;
const int32_t PIN_DIR = 27;
#define BREAK_TIME 1
int32_t jumps = 0;

std::vector<int16_t> delays;

int32_t steps = 0;
enum Direction
{
	forward,
	backward,
};
bool direction = forward;

double_t moveTest(double_t x)
{
	double_t result = 0;
	if (x < TIME / 2)
	{
		result = 2 * pow(x, 2);
	}
	else if (x < TIME)
	{
		result = -2 * pow(x, 2) + 4 * TIME * x - pow(TIME, 2);
	}
	else
	{
		result = TIMESQ;
	}
	result /= TIMESQ;
	result *= 10000;
	return result;
}

hw_timer_t *timer = NULL;
hw_timer_t *timer2 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timer2Mux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer2()
{
	// portENTER_CRITICAL_ISR(&timer2Mux);
	digitalWrite(PIN_STEP, LOW);
	steps++;
	timerAlarmWrite(timer, delays[steps] /*120*/, false);
	timerAlarmEnable(timer);
	
	// portEXIT_CRITICAL_ISR(&timer2Mux);
}

void IRAM_ATTR onTimer()
{
	portENTER_CRITICAL_ISR(&timerMux);
	digitalWrite(PIN_STEP, HIGH);
	timerWrite(timer2, 0);
	timerAlarmEnable(timer2);
	timerWrite(timer, 0);
	timerAlarmEnable(timer);
	portEXIT_CRITICAL_ISR(&timerMux);
}

void toggleDirection(bool *direction)
{
	if (!*direction)
	{
		*direction = backward;
		digitalWrite(PIN_DIR, HIGH);
		delay(BREAK_TIME);
	}
	else
	{
		*direction = forward;
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

	std::vector<int32_t> steptimes;

	for (double_t current_time = 0; current_time <= TIME; current_time += 0.001)
	{
		if (moveTest(current_time) >= jumps + 1)
		{
			jumps++;
			steptimes.push_back(int32_t(current_time * 1000 + 0.1));
			current_time += 0.070;
			//Serial.printf("%d: %f | %f\n", jumps, current_time, moveTest(current_time + 100));
		}
	}

	for (size_t i = 0; i <= MAX_DISTANCE - 2; i++)
	{
		delays.push_back(steptimes[i + 1] - steptimes[i]);
		Serial.printf("%d: %d\n", i, delays[i]);
	}

	// steps = 0;

	timer = timerBegin(0, 80, true); // 2 = 25ns, 80 = 1us
	timerAttachInterrupt(timer, &onTimer, true);
	timerAlarmWrite(timer, delays[steps] /*1000*/, false);
	timerWrite(timer, 0);
	timerAlarmEnable(timer);
	
	timer2 = timerBegin(1, 80, true); // 2 = 25ns, 80 = 1us
	timerAttachInterrupt(timer2, &onTimer2, true);
	timerAlarmWrite(timer2, 8, false);
}

void loop()
{
	//Serial.printf("Steps: %d, Direction: %d\n", steps, direction);
	if (steps >= MAX_DISTANCE-2) // 10000 (10800)
	{
		toggleDirection(&direction);
		steps = 0;
	}
}