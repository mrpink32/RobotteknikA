#pragma once
#include <Arduino.h>

// const int32_t PIN_LIMIT_SW = 24;
const int32_t LED_PIN = 17;

// PINS SETUP
// Motor 1:
const int32_t PIN_PID_LOOP_1 = 16; // debug monitoring pid loop
// const int32_t PIN_LIMIT_SW_1 = 25;  // limit switch pin
const int32_t PIN_ENC_A_1 = 26;       // encoder A ch
const int32_t PIN_ENC_B_1 = 27;       // encoder B ch
const int32_t PIN_HBRIDGE_INA_1 = 4;  // hbridge INA pin
const int32_t PIN_HBRIDGE_INB_1 = 18; // hbridge INB pin
const int32_t PIN_HBRIDGE_PWM_1 = 19; // hbridhe PWM pin

// Motor 2:
const int32_t PIN_PID_LOOP_2 = 2; // debug monitoring pid loop
// const int32_t PIN_LIMIT_SW = 25;    // limit switch pin
const int32_t PIN_ENC_A_2 = 25;       // encoder A ch
const int32_t PIN_ENC_B_2 = 33;       // encoder B ch
const int32_t PIN_HBRIDGE_INA_2 = 21; // hbridge INA pin
const int32_t PIN_HBRIDGE_INB_2 = 22; // hbridge INB pin
const int32_t PIN_HBRIDGE_PWM_2 = 23; // hbridhe PWM pin

// Motor 3:
const int32_t PIN_PID_LOOP = 17;    // debug monitoring pid loop
const int32_t PIN_LIMIT_SW = 25;    // limit switch pin
const int32_t PIN_ENC_A = 26;       // encoder A ch
const int32_t PIN_ENC_B = 27;       // encoder B ch
const int32_t PIN_HBRIDGE_INA = 18; // hbridge INA pin
const int32_t PIN_HBRIDGE_INB = 19; // hbridge INB pin
const int32_t PIN_HBRIDGE_PWM = 23; // hbridhe PWM pin

// PWM SETUP
const int32_t PWM_CH_1 = 0;                       // 0 - 15 = 16 channels - remember to change
const int32_t PWM_CH_2 = 1;                       // 0 - 15 = 16 channels - remember to change
const int32_t PWM_CH_3 = 2;                       // 0 - 15 = 16 channels - remember to change
const int32_t PWM_FREQ_HZ = 19500;                // 1Hz to 40MHz - observe restrictions
const int32_t PWM_RES_BITS = 12;                  // 1 to 20 bits - observe restrictions
const int32_t PWM_MIN = 0;                        // minimum pwm value
const int32_t PWM_MAX = pow(2, PWM_RES_BITS) - 1; // maximum pwm value

// PID SETUP
const double DT_S = 0.01; // loop time in seconds
const double MIN_CTRL_VALUE = -100;
const double MAX_CTRL_VALUE = 100;
const double PID_MAX_CTRL_VALUE = 10000;
// const double PID_MAX_VEL_VALUE = 10000;
