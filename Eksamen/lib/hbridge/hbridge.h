#pragma once
#include <Arduino.h>

class H_Bridge
{
public:
  H_Bridge(){};

  void set_pwm(int32_t ctrl_value);

  void begin(int32_t pin_pwm, int32_t pin_ina, int32_t pin_inb,
             int32_t freq_hz, int32_t resolution_bit,
             int32_t pwm_ch, int32_t max_ctrl_value);

  void break_motor();

private:
  int32_t pin_pwm;        // hbridge PWM pin
  int32_t pin_ina;        // hbridge INB pin
  int32_t pin_inb;        // hbridge INA pin
  int32_t freq_hz;        // pwm frequency
  int32_t resolution_bit; // pwm resolution
  int32_t pwm_ch;         // pwm channel
  int32_t pwm_max;        // maximum pwm value;
  int32_t max_ctrl_value; // maximum ctrl value
};
