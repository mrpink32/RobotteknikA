#include <pid.h>
#include <hbridge.h>

class Motor : Pid
{
private:
public:
    Motor(/* args */);
    ~Motor();
};

Motor::Motor(double dt, double max_ctrl_value)
{
    Pid pos_pid(dt, max_ctrl_value);
}

Motor::~Motor()
{
}

class Pid
{
    double min_ctrl_value;
    double max_ctrl_value;

    double dt;
    double kp;
    double ki;
    double kd;
    double error;
    double error_sum;
    double previous_error;

public:
    Pid(double dt, double max_ctrl_value);

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
