#include "new-motor-test.h"

typedef struct Pid
{
    double min_ctrl_value;
    double max_ctrl_value;

    double dt;
    double kp;
    double ki;
    double kd;
    double error;
    double error_sum;
    double previus_error;
} pid_t;

typedef struct HBridge
{
    int32_t pin_pwm;        // hbridge PWM pin
    int32_t pin_ina;        // hbridge INB pin
    int32_t pin_inb;        // hbridge INA pin
    int32_t freq_hz;        // pwm frequency
    int32_t resolution_bit; // pwm resolution
    int32_t pwm_ch;         // pwm channel
    int32_t pwm_max;        // maximum pwm value;
    int32_t max_ctrl_value; // maximum ctrl value
} hbridge_t;

NewMotorTest::NewMotorTest(double_t delta_time, /*double_t max_ctrl_value,*/ int32_t pwm_pin, int32_t ina_pin, int32_t inb_pin, int32_t freq_hz, int32_t resolution_bit, int32_t pwm_ch, int32_t pwm_max, int32_t max_ctrl_value)
{
    this->position_pid = create_pid(delta_time, max_ctrl_value);
    this->velocity_pid = create_pid(delta_time, max_ctrl_value);
    this->hbridge = create_hbridge(pwm_pin, ina_pin, inb_pin, freq_hz, resolution_bit, pwm_ch, max_ctrl_value);
    set_current_position(0);
    set_current_velocity(0);
    set_target_position(0);
    set_target_velocity(0);
    set_max_position(0);
    set_max_velocity(0);
}


NewMotorTest::~NewMotorTest()
{
    // delete this->position_pid;
    // delete this->velocity_pid;
    // delete this->hbridge;
}

pid_t NewMotorTest::create_pid(double_t delta_time, double_t max_ctrl_value)
{
    pid_t pid;
    pid.min_ctrl_value = -max_ctrl_value;
    pid.max_ctrl_value = max_ctrl_value;

    pid.dt = delta_time;
    pid.kp = 0;
    pid.ki = 0;
    pid.kd = 0;
    pid.error = 0;
    pid.error_sum = 0;
    pid.previus_error = 0;
    return pid;
}

hbridge_t NewMotorTest::create_hbridge(int32_t pwm_pin, int32_t ina_pin, int32_t inb_pin, int32_t freq_hz, int32_t resolution_bit, int32_t pwm_ch, int32_t max_ctrl_value)
{
    hbridge_t hbridge;
    hbridge.pin_pwm = pwm_pin;
    hbridge.pin_ina = ina_pin;
    hbridge.pin_inb = inb_pin;
    hbridge.freq_hz = freq_hz;
    hbridge.resolution_bit = resolution_bit;
    hbridge.pwm_ch = pwm_ch;
    hbridge.pwm_max = pow(2, resolution_bit) - 1;;
    hbridge.max_ctrl_value = max_ctrl_value;

    pinMode(pwm_pin, OUTPUT);
    pinMode(ina_pin, OUTPUT);
    pinMode(inb_pin, OUTPUT);

    ledcAttachPin(pwm_pin, pwm_ch);
    ledcSetup(pwm_ch, freq_hz, resolution_bit);
    return hbridge;
}

pid_t *NewMotorTest::pid_pointer(pid_types::pid_type_t pid_type) {
    pid_t *pid = NULL;
    switch (pid_type)
    {
    case pid_types::POSITION_PID:
    {
        *pid = this->position_pid;
        break;
    }
    case pid_types::VELOCITY_PID:
    {
        *pid = this->velocity_pid;
        break;
    }
    default:
        log_e("Unknown pid type");
        return;
    }
    return pid;
}

// setters
void NewMotorTest::set_current_position(int32_t position)
{
    this->current_position = position;
}

void NewMotorTest::set_current_velocity(int32_t velocity)
{
    this->current_velocity = velocity;
}

int32_t NewMotorTest::set_target_position(int32_t position)
{
    this->target_position = position;
}

int32_t NewMotorTest::set_target_velocity(int32_t velocity)
{
    this->target_velocity = velocity;
}

int32_t NewMotorTest::set_max_position(int32_t position)
{
    this->max_position = position;
}

int32_t NewMotorTest::set_max_velocity(int32_t velocity)
{
    this->max_velocity = velocity;
}

// getters
int32_t NewMotorTest::get_current_position()
{
    return this->current_position;
}

int32_t NewMotorTest::get_current_velocity()
{
    return this->current_velocity;
}

int32_t NewMotorTest::get_target_position()
{
    return this->target_position;
}

int32_t NewMotorTest::get_target_velocity()
{
    return this->target_velocity;
}

int32_t NewMotorTest::get_max_position()
{
    return this->max_position;
}

int32_t NewMotorTest::get_max_velocity()
{
    return this->max_velocity;
}


/**
 * Sets the proportional gain constant for the specified PID controller type.
 *
 * @param pid_type The type of PID controller to set the constant for.
 * @param kp The new proportional gain constant to set.
 * @return The value of the new constant that was set.
 */
double_t NewMotorTest::set_kp(pid_types::pid_type_t pid_type, double_t kp)
{
    pid_t *pid = pid_pointer(pid_type); // Get the PID controller for the given type
    double_t value = kp;                // Store the new value so it can be returned
    pid->kp = kp;                       // Set the proportional gain
    return value;
}


/**
 * Sets the value of the integral gain constant (ki) for the specified PID controller type.
 *
 * @param pid_type The type of the PID controller to set the ki value for.
 * @param ki The value to set the ki constant to.
 *
 * @return The value that was set for ki.
 */
double_t NewMotorTest::set_ki(pid_types::pid_type_t pid_type, double_t ki)
{
    pid_t *pid = pid_pointer(pid_type);
    double_t value = ki;
    pid->ki = ki;
    return value;
}


/**
 * Set the derivative gain of a PID controller.
 *
 * @param pid_type The type of PID controller to modify.
 * @param kd The new value of the derivative gain.
 * @return The new value of the derivative gain.
 */
double_t NewMotorTest::set_kd(pid_types::pid_type_t pid_type, double_t kd)
{
    pid_t *pid = pid_pointer(pid_type);
    double_t value = kd;
    pid->kd = kd;
    return value;
}


/**
 * Returns the delta time of the given PID controller type.
 *
 * @param pid_type The type of the PID controller.
 * @return The delta time of the PID controller.
 */
double_t NewMotorTest::get_delta_time(pid_types::pid_type_t pid_type) {
    pid_t *pid = pid_pointer(pid_type);
    double_t value = pid->dt;
    return value;
}


/**
 * Returns the proportional gain constant for the specified PID controller type.
 * 
 * @param pid_type The type of PID controller (e.g. PID_POSITION, PID_VELOCITY).
 * @return The proportional gain constant as a double.
 */
double_t NewMotorTest::get_kp(pid_types::pid_type_t pid_type) {
    pid_t *pid = pid_pointer(pid_type);
    double_t value = pid->kp;
    return value;
}

double_t NewMotorTest::get_ki(pid_types::pid_type_t pid_type) {
    pid_t *pid = pid_pointer(pid_type);
    double_t value = pid->ki;
    return value;
}

double_t NewMotorTest::get_kd(pid_types::pid_type_t pid_type) {
    pid_t *pid = pid_pointer(pid_type);
    double_t value = pid->kd;
    return value;
}

double_t NewMotorTest::get_error(pid_types::pid_type_t pid_type) {
    pid_t *pid = pid_pointer(pid_type);
    double_t value = pid->error;
    return value;
}

// other pid related methods
double_t NewMotorTest::squash(pid_types::pid_type_t pid_type, double_t value) {
    pid_t *pid = pid_pointer(pid_type);
    value = (value < pid->min_ctrl_value) ? pid->min_ctrl_value : ((value > pid->max_ctrl_value) ? pid->max_ctrl_value : value);
    return value;
}

void NewMotorTest::update(pid_types::pid_type_t pid_type, double_t set_value, double_t current_value, double_t *ctrl_value, double_t integration_threshold) {
    pid_t *pid = pid_pointer(pid_type);

    pid->error = set_value - current_value;

    if (fabs(pid->error) < integration_threshold)
        pid->error_sum += pid->error;

    double_t kp_val = pid->error;
    double_t ki_val = pid->error_sum * pid->dt;
    double_t kd_val = (pid->previus_error - pid->error) / pid->dt;

    pid->previus_error = pid->error;
    double_t ctrl = pid->kp * kp_val + pid->ki * ki_val + pid->kd * kd_val;
    *ctrl_value = squash(pid_type, ctrl);
}

// hbridge related methods
double map_double(double x, double in_min, double in_max, double out_min, double out_max)
{
    double divisor = (in_max - in_min);
    return (divisor == 0) ? 0 : (x - in_min) * (out_max - out_min) / divisor + out_min;
}

void NewMotorTest::set_pwm(int32_t ctrl_value) {
    hbridge_t hbridge = this->hbridge;
    if (ctrl_value < 0)
    {
        ctrl_value = -ctrl_value;
        digitalWrite(hbridge.pin_ina, LOW);
        digitalWrite(this->hbridge.pin_inb, HIGH);
    }
    else
    {
        digitalWrite(this->hbridge.pin_ina, HIGH);
        digitalWrite(this->hbridge.pin_inb, LOW);
    }

    double pwm_value = map_double(ctrl_value, 0, this->hbridge.max_ctrl_value, 0, this->hbridge.pwm_max);
    ledcWrite(this->hbridge.pwm_ch, pwm_value);
}

void NewMotorTest::break_motor() {
    digitalWrite(this->hbridge.pin_ina, HIGH);
    digitalWrite(this->hbridge.pin_inb, HIGH);
}
