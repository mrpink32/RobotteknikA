#pragma once
#include "pid.h"
#include "hbridge.h"

class Motor
{
private:
    int64_t current_position = 0;
    int64_t current_velocity = 0;
    // might be better to use integers
    int32_t target_position = 0;
    int32_t target_velocity = 0;

public:
    H_Bridge hbridge;
    Pid position_pid;
    Pid velocity_pid;

    Motor(int32_t pin_pwm, int32_t pin_ina, int32_t pin_inb,
          int32_t freq_hz, int32_t resolution_bit,
          int32_t pwm_ch, int32_t max_ctrl_value,
          double delta_time_seconds);

    ~Motor();

    void set_position(int32_t current_position);

    void set_velocity(int32_t current_velocity);

    int32_t get_position();

    int32_t get_velocity();

    int32_t get_target_position();

    int32_t set_target_position(int32_t target_position);

    int32_t get_target_velocity();
    
    int32_t set_target_velocity(int32_t target_velocity);


};
