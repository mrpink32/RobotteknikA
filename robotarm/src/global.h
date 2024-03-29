#pragma once
#include <Arduino.h>

// PINS SETUP
const int32_t PIN_PID_LOOP = 16;
const int32_t PIN_ENC_A = 26;
const int32_t PIN_ENC_B = 27;

const int32_t PIN_HBRIDGE_INA = 18; // hbridge INA pin
const int32_t PIN_HBRIDGE_INB = 19; // hbridge INB pin
const int32_t PIN_HBRIDGE_PWM = 23; // hbridhe PWM pin

const int32_t PIN_LIMIT_SW = 23;

// PWM SETUP
const int32_t PWM_CH = 0;                         // 0 - 7(15?)  = 8(16?) channels
const int32_t PWM_FREQ_HZ = 19500;                // 1Hz to 40MHz
const int32_t PWM_RES_BITS = 12;                  // 1 to 16(20) bits
const int32_t PWM_MIN = 0;                        // minimum pwm value
const int32_t PWM_MAX = pow(2, PWM_RES_BITS) - 1; // maximum pwm value

// PID SETUP
const double DT_S = 0.001; // loop time in seconds
const double MIN_CTRL_VALUE = -100;
const double MAX_CTRL_VALUE = 100;
const double PID_MAX_CTRL_VALUE = 4000;
