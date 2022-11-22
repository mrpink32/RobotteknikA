#include <Arduino.h>

#define PIN_PID_LOOP 17

TaskHandle_t  Task_A_Handle;
TaskHandle_t  Task_B_Handle;

void task_a(void *arg)
{
  for(;;)
  {
    // put your task_a code here, to run repeatedly


    vTaskDelay(10); // 10 ms if TICK_RATE = 1000 
  }
}

void task_b(void *arg)
{
  double loop_time_s = 0.01;
  TickType_t xTimeIncrement = configTICK_RATE_HZ * loop_time_s;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for(;;)
  {
    digitalWrite(PIN_PID_LOOP, HIGH);

    // put your task_b code here, to run repeatedly

    digitalWrite(PIN_PID_LOOP, LOW);
    vTaskDelayUntil(&xLastWakeTime, xTimeIncrement);
  }
}


void setup() {
  xTaskCreatePinnedToCore(
        task_a,         
       "task_a",        
        10000,               /* Stack size in words */
        NULL,                /* Task input parameter */
        2,                   /* Priority of the task from 0 to 25, higher number = higher priority */
        &Task_A_Handle,   
        0);             

  xTaskCreatePinnedToCore(
        task_b,         
       "task_b",        
        10000,               /* Stack size in words */
        NULL,                /* Task input parameter */
        2,                   /* Priority of the task from 0 to 25, higher number = higher priority */
        &Task_B_Handle,   
        0);             
}

void loop() {
 

  // put your main code here, to run repeatedly:
}