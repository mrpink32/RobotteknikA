#include <math.h>

extern "C" void print_hello_from_rust();

struct Pid
{
    double delta_time;
    double min_ctrl_value;
    double max_ctrl_value;
    double kp;
    double ki;
    double kd;
    double error;
    double error_sum;
    double previous_error;
};

extern "C" Pid new_pid(double dt, double max_ctrl);

extern "C" void set_kp(Pid *pid, double kp);

extern "C" void set_ki(Pid *pid, double ki);

extern "C" void set_kd(Pid *pid, double kd);

extern "C" double_t get_delta_time(Pid *pid);

extern "C" double_t get_min_ctrl_value(Pid *pid);

extern "C" double_t get_max_ctrl_value(Pid *pid);

extern "C" double_t get_kp(Pid *pid);

extern "C" double_t get_ki(Pid *pid);

extern "C" double_t get_kd(Pid *pid);

extern "C" double_t get_error(Pid *pid);
