#include <Arduino.h>
#include <math.h>
#include <vector>

const int64_t TIME = 3000000;
const double_t TIMEC = pow(TIME, 3);
const int32_t MAX_DISTANCE = 10000;
const int32_t PIN_STEP = 26;
const int32_t PIN_DIR = 27;
#define BREAK_TIME 1
double_t jumps = 0;

std::vector<int32_t> delays;

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
	if (x < TIME / 4)
	{
		result = 32 * pow(x, 3);
	}
	else if (TIME / 4 <= x < 3 * TIME / 4)
	{
		result = TIMEC - 12 * x * pow(TIME, 2) + 48 * pow(x, 2) * TIME - 32 * pow(x, 3);
	}
	else if (3 * TIME / 4 <= x < TIME)
	{
		result = -26 * TIMEC + 96 * x * pow(TIME, 2) - 96 * pow(x, 2) * TIME + 32 * pow(x, 3);
	}
	else
	{
		result = 0;
	}
	result /= 3 * TIMEC;
	result *= 5000;
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

	for (double_t current_time = 0; current_time < TIME; current_time++)
	{
		if (moveTest(current_time) >= jumps + 1)
		{
			jumps++;
			steptimes.push_back(current_time);
			current_time += 119;
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

	Serial.printf("%d %d", steptimes[9713], steptimes[9714]);
}

void loop()
{
	//Serial.printf("Steps: %d, Direction: %d\n", steps, direction);
	if (steps >= MAX_DISTANCE) // 10000 (10800)
	{
		delay(10000);
		toggleDirection(&direction);
		// steps = 0;
	}
}