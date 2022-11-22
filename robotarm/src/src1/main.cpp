#include <ESP32Encoder.h>
#include "global.h"
#include "pid.h"
#include "hbridge.h"

TaskHandle_t PidTaskHandle;
TaskHandle_t MotionTaskHandle;
ESP32Encoder encoder;
Pid pid_vel(DT_S, PID_MAX_CTRL_VALUE);
Pid pid_pos(DT_S, PID_MAX_CTRL_VALUE);
H_Bridge hbridge;

const double integration_threshold = 200;

volatile double req_pos;
volatile double req_vel;
volatile int64_t current_pos;
volatile double current_vel;
volatile double max_vel = 9600 / 10;

double ctrl_pos;
double ctrl_vel;

bool mode_pos = true;

void pid_task(void *arg)
{
  int64_t prev_pos = current_pos;

  TickType_t xTimeIncrement = configTICK_RATE_HZ * pid_pos.get_dt();
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;)
  { // loop tager mindre end 18us * 2
    digitalWrite(PIN_PID_LOOP, HIGH);

    current_pos = encoder.getCount();
    current_vel = (current_pos - prev_pos) / DT_S;

    if (mode_pos)
    {
      pid_pos.update(req_pos, current_pos, &ctrl_pos, integration_threshold);

      req_vel = constrain(ctrl_pos, -max_vel, max_vel);
    }

    pid_vel.update(req_vel * 10, current_vel, &ctrl_vel, 100000);

    hbridge.set_pwm(ctrl_vel);

    prev_pos = current_pos;
    digitalWrite(PIN_PID_LOOP, LOW);
    vTaskDelayUntil(&xLastWakeTime, xTimeIncrement);
  }
}

void wait_move()
{
  while (abs(req_pos - current_pos) > 10)
  {
    vTaskDelay(1);
  }
}

void home()
{
  log_v("home initiated...");
  req_vel = 40000;
  mode_pos = false;

  while (digitalRead(PIN_LIMIT_SW))
  {
    vTaskDelay(1);
  }
  req_vel = -1000;
  while (!digitalRead(PIN_LIMIT_SW))
  {
    vTaskDelay(1);
  }
  req_vel = 0;

  mode_pos = true;
  req_pos = 0;
  encoder.setCount(-14100);
  wait_move();
  log_v("home complete.");
}

void set_pos(int32_t pos)
{
  req_pos = pos;
}

void motion_task(void *arg)
{
  home();

  vTaskDelay(5000);
  int32_t n = 0;
  //int32_t steps_pr_deg = 1920;

  //TickType_t xTimeIncrement = configTICK_RATE_HZ/10;
  //TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;)
  {
    log_v("motion_task...");

    //set_pos(steps_pr_deg*(90+n*90)); // 90 deg
    set_pos(n * 500 * 4 * 4.8); // 1 round
    wait_move();
    vTaskDelay(5000);
    n++;
    //vTaskDelayUntil( &xLastWakeTime, xTimeIncrement);
  }
}

void setup() // runs exclusive on core 1
{
  //disableCore0WDT();
  //disableCore1WDT();
  Serial.begin(115200);
  pinMode(PIN_PID_LOOP, OUTPUT);
  pinMode(PIN_LIMIT_SW, INPUT);

  ESP32Encoder::useInternalWeakPullResistors = UP; // Enable the weak pull up resistors
  encoder.attachFullQuad(PIN_ENC_A, PIN_ENC_B);    // Attache pins for use as encoder pins
  encoder.clearCount();

  hbridge.begin(PIN_HBRIDGE_PWM, PIN_HBRIDGE_INA, PIN_HBRIDGE_INB,
                PWM_FREQ_HZ, PWM_RES_BITS, PWM_CH, PID_MAX_CTRL_VALUE);

  pid_pos.set_kp(20.0); //12
  pid_pos.set_ki(5.0);
  pid_pos.set_kd(0.000);

  pid_vel.set_kp(0.05);
  pid_vel.set_ki(4);
  pid_vel.set_kd(0.000);

  log_v("starting pid task");
  xTaskCreatePinnedToCore(
      pid_task,       /* Function to implement the task */
      "pid_task",     /* Name of the task */
      3000,           /* Stack size in words */
      NULL,           /* Task input parameter */
      3,              /* Priority of the task from 0 to 25, higher number = higher priority */
      &PidTaskHandle, /* Task handle. */
      1);             /* Core where the task should run */

  log_v("starting motion task");
  xTaskCreatePinnedToCore(
      motion_task,
      "motion_task",
      3000, /* Stack size in words */
      NULL, /* Task input parameter */
      2,    /* Priority of the task from 0 to 25, higher number = higher priority */
      &MotionTaskHandle,
      0); /* Core where the task should run */
}

void loop() // runs exclusive on core 1
{
  Serial.printf(
      "req_pos: %.2f  curr_pos: %.2f  ctrl_pos: %.2f  set_vel: %.2f  curr_vel: %.2f ctrl_vel: %.2f\n\r",
      req_pos, (double)current_pos, ctrl_pos, req_vel, current_vel, ctrl_vel);

  /*
  int32_t pid_stack_min =  uxTaskGetStackHighWaterMark(PidTaskHandle);
  int32_t motion_stack_min = uxTaskGetStackHighWaterMark(MotionTaskHandle);  
  log_i("min stack avaliable [bytes], pid: %d,  motion: %d", pid_stack_min, motion_stack_min);
  */

  vTaskDelay(0.2 * configTICK_RATE_HZ);
}