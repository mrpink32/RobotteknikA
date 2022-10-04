#include <Arduino.h>
#include <math.h>
#include <vector>

const int32_t TIME = 1500;
const double_t TIMEC = pow(TIME, 3);
const int32_t MAX_STEPS = 10000;
const int32_t STEP_PIN = 26;
const int32_t DIR_PIN = 27;
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
	if (x < TIME / 4)
	{
		result = 32 * pow(x, 3);
	}
	else if (x < 3 * TIME / 4)
	{
		result = TIMEC - 12 * x * pow(TIME, 2) + 48 * pow(x, 2) * TIME - 32 * pow(x, 3);
	}
	else if (x < TIME)
	{
		result = -26 * TIMEC + 96 * x * pow(TIME, 2) - 96 * pow(x, 2) * TIME + 32 * pow(x, 3);
	}
	else
	{
		result = 6 * TIMEC;
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
	digitalWrite(STEP_PIN, LOW);
	steps++;
	timerAlarmWrite(timer, delays[steps] /*120*/, false);
	timerAlarmEnable(timer);

	// portEXIT_CRITICAL_ISR(&timer2Mux);
}

void IRAM_ATTR onTimer()
{
	portENTER_CRITICAL_ISR(&timerMux);
	digitalWrite(STEP_PIN, HIGH);
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
		digitalWrite(DIR_PIN, HIGH);
		delay(BREAK_TIME);
	}
	else
	{
		*direction = forward;
		digitalWrite(DIR_PIN, LOW);
		delay(BREAK_TIME);
	}
}

void setup()
{
	// hello world
	Serial.begin(115200);
	Serial.println("Hello World!");

	// setup pin
	pinMode(STEP_PIN, OUTPUT);
	pinMode(DIR_PIN, OUTPUT);

	std::vector<int32_t> step_times;

	for (double_t current_time = 0; current_time <= TIME; current_time += 0.001)
	{
		if (moveTest(current_time) >= jumps + 1)
		{
			jumps++;
			step_times.push_back(int32_t(current_time * 1000 + 0.1));
			current_time += 0.070;
			Serial.printf("%d: %f | %f\n", jumps, current_time, moveTest(current_time + 100));
		}
	}

	for (size_t i = 0; i <= MAX_STEPS - 3; i++)
	{
		delays.push_back(step_times[i + 1] - step_times[i]);
		Serial.printf("%d: %d\n", i, delays[i]);
	}

	// setup and start timer 1
	timer = timerBegin(0, 80, true); // 2 = 25ns, 80 = 1us
	timerAttachInterrupt(timer, &onTimer, true);
	timerAlarmWrite(timer, delays[steps] /*1000*/, false);
	timerWrite(timer, 0);
	timerAlarmEnable(timer);

	// setup and start timer 2
	timer2 = timerBegin(1, 80, true); // 2 = 25ns, 80 = 1us
	timerAttachInterrupt(timer2, &onTimer2, true);
	timerAlarmWrite(timer2, 8, false);
}

void loop()
{
	// Serial.printf("Steps: %d, Direction: %d\n", steps, direction);

	/* changes direction when the current amount of steps
	is larger or equal to the max amount of steps allowed */
	if (steps >= MAX_STEPS - 3)
	{
		toggleDirection(&direction);
		steps = 0;
	}
}