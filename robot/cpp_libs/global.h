#pragma once
#include <Arduino.h>

// PINS SETUP
// Motor 1:
const int32_t PIN_PID_LOOP = 17;    // debug monitoring pid loop
const int32_t PIN_LIMIT_SW = 25;    // limit switch pin
const int32_t PIN_ENC_A = 26;       // encoder A ch
const int32_t PIN_ENC_B = 27;       // encoder B ch
const int32_t PIN_HBRIDGE_INA = 18; // hbridge INA pin
const int32_t PIN_HBRIDGE_INB = 19; // hbridge INB pin
const int32_t PIN_HBRIDGE_PWM = 23; // hbridhe PWM pin

// Motor 2:
const int32_t PIN_PID_LOOP = 17;    // debug monitoring pid loop
const int32_t PIN_LIMIT_SW = 25;    // limit switch pin
const int32_t PIN_ENC_A = 26;       // encoder A ch
const int32_t PIN_ENC_B = 27;       // encoder B ch
const int32_t PIN_HBRIDGE_INA = 18; // hbridge INA pin
const int32_t PIN_HBRIDGE_INB = 19; // hbridge INB pin
const int32_t PIN_HBRIDGE_PWM = 23; // hbridhe PWM pin

// Motor 3:
const int32_t PIN_PID_LOOP = 17;    // debug monitoring pid loop
const int32_t PIN_LIMIT_SW = 25;    // limit switch pin
const int32_t PIN_ENC_A = 26;       // encoder A ch
const int32_t PIN_ENC_B = 27;       // encoder B ch
const int32_t PIN_HBRIDGE_INA = 18; // hbridge INA pin
const int32_t PIN_HBRIDGE_INB = 19; // hbridge INB pin
const int32_t PIN_HBRIDGE_PWM = 23; // hbridhe PWM pin

// PWM SETUP
const int32_t PWM_CH = 0;          // 0 - 15 = 16 channels - remember to change
const int32_t PWM_FREQ_HZ = 19500; // 1Hz to 40MHz - observe restrictions
const int32_t PWM_RES_BITS = 12;   // 1 to 20 bits - observe restrictions

// PID SETUP
const double DT_S = 0.001; // loop time in seconds
const double PID_MAX_CTRL_VALUE = 4000;
