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

extern "C" Pid new_pid(double dt, double max_ctrl, double test);

extern "C" void set_kp(Pid *pid, double kp);

extern "C" void set_ki(Pid *pid, double ki);