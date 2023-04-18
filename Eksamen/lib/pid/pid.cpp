#include "pid.h"

void Pid::setup(double dt, int32_t max_ctrl_value)
{
    this->dt = dt;
    this->min_ctrl_value = -max_ctrl_value;
    this->max_ctrl_value = max_ctrl_value;
    this->error_sum = 0;
    this->previous_error = 0;
}

void Pid::set_kp(double kp)
{
    this->kp = kp;
}

void Pid::set_ki(double ki)
{
    this->ki = ki;
}

void Pid::set_kd(double kd)
{
    this->kd = kd;
}

int32_t Pid::set_max_ctrl_value(int32_t max_ctrl_value)
{
    this->max_ctrl_value = max_ctrl_value;
    return max_ctrl_value;
}

int32_t Pid::set_min_ctrl_value(int32_t min_ctrl_value)
{
    this->min_ctrl_value = min_ctrl_value;
    return min_ctrl_value;
}

double Pid::get_dt(void)
{
    return this->dt;
}

double Pid::get_kp(void)
{
    return this->kp;
}

double Pid::get_ki(void)
{
    return this->ki;
}

double Pid::get_kd(void)
{
    return this->kd;
}

int32_t Pid::get_max_ctrl_value(void)
{
    return this->max_ctrl_value;
}

int32_t Pid::get_min_ctrl_value(void)
{
    return this->min_ctrl_value;
}

double Pid::squash(double value)
{
    return (value < min_ctrl_value) ? min_ctrl_value : ((value > max_ctrl_value) ? max_ctrl_value : value);
}

void Pid::update(double set_value, double current_value, double *ctrl_value, double integration_threshold)
{
    double kp_val, ki_val, kd_val, ctrl;

    error = set_value - current_value;

    if (fabs(error) < integration_threshold)
        error_sum += error;

    kp_val = error;
    ki_val = error_sum * dt;
    kd_val = (previous_error - error) / dt;

    previous_error = error;
    ctrl = kp * kp_val + ki * ki_val + kd * kd_val;
    *ctrl_value = squash(ctrl);
}
