#pragma once

#include <math.h>

class Pid
{
    double min_ctrl_value = 0;
    double max_ctrl_value = 0;

    double dt = 0;
    double kp = 0;
    double ki = 0;
    double kd = 0;
    double error = 0;
    double error_sum = 0;
    double previous_error = 0;

public:
    Pid(){};

    void setup(double delta_time, int32_t max_ctrl_value);
    void set_kp(double kp);
    void set_ki(double ki);
    void set_kd(double kd);

    double get_dt(void);

    double get_kp(void);
    double get_ki(void);
    double get_kd(void);
    double get_error(void) { return error; };

    double squash(double value);
    void update(double set_value, double current_value, double *ctrl_value, double integration_threshold);
};
