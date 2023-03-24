#include <motor.h>

Motor::Motor(int32_t pin_pwm, int32_t pin_ina, int32_t pin_inb,
             int32_t freq_hz, int32_t resolution_bit,
             int32_t pwm_ch, int32_t max_ctrl_value,
             double delta_time_seconds)
{
    this->hbridge.begin(pin_pwm, pin_ina, pin_inb,
                        freq_hz, resolution_bit,
                        pwm_ch, max_ctrl_value);
    this->position_pid.setup(delta_time_seconds, max_ctrl_value);
    this->velocity_pid.setup(delta_time_seconds, max_ctrl_value);
}

Motor::~Motor()
{
}

void Motor::set_position(int32_t current_position)
{
    this->current_position = current_position;
}

void Motor::set_velocity(int32_t current_velocity)
{
    this->current_velocity = current_velocity;
}

int32_t Motor::get_position()
{
    return this->current_position;
}

int32_t Motor::get_velocity()
{
    return this->current_velocity;
}