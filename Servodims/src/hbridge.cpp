#include "hbridge.h"

double map_double(double x, double in_min, double in_max, double out_min, double out_max)
{
    double divisor = (in_max - in_min);
    return (divisor == 0) ? 0 : (x - in_min) * (out_max - out_min) / divisor + out_min;
}

void H_Bridge::begin(int32_t pin_pwm, int32_t pin_ina, int32_t pin_inb,
                     int32_t freq_hz, int32_t resolution_bit,
                     int32_t pwm_ch, int32_t max_ctrl_value)
{
    this->pin_pwm = pin_pwm;
    this->pin_ina = pin_ina;
    this->pin_inb = pin_inb;
    this->freq_hz = freq_hz;
    this->resolution_bit = resolution_bit;
    this->pwm_ch = pwm_ch;
    this->pwm_max = pow(2, resolution_bit) - 1;
    this->max_ctrl_value = max_ctrl_value;

    pinMode(pin_pwm, OUTPUT);
    pinMode(pin_ina, OUTPUT);
    pinMode(pin_inb, OUTPUT);

    ledcAttachPin(pin_pwm, pwm_ch);
    ledcSetup(pwm_ch, freq_hz, resolution_bit);
}

void H_Bridge::set_pwm(int32_t ctrl_value)
{
    if (ctrl_value < 0)
    {
        ctrl_value = -ctrl_value;
        digitalWrite(pin_ina, LOW);
        digitalWrite(pin_inb, HIGH);
    }
    else
    {
        digitalWrite(pin_ina, HIGH);
        digitalWrite(pin_inb, LOW);
    }

    double pwm_value = map_double(ctrl_value, 0, max_ctrl_value, 0, pwm_max);
    ledcWrite(pwm_ch, pwm_value);
}

void H_Bridge::break_motor()
{
    digitalWrite(pin_ina, HIGH);
    digitalWrite(pin_inb, HIGH);
}