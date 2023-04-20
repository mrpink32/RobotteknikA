#pragma once
#include "ESP32Encoder.h"
#include <Arduino.h>
#include <stdint.h>
#include <math.h>

typedef struct Pid pid_t;

typedef struct HBridge hbridge_t;

// typeuef enum PidType moved to motor class, maybe return it later
namespace pid_types
{
    typedef enum PidType
    {
        POSITION_PID,
        VELOCITY_PID,
    } pid_type_t;
    
} // namespace pid_types


class NewMotorTest
{
    int32_t current_position;
    int32_t current_velocity;
    int32_t target_position;
    int32_t target_velocity;
    int32_t last_position;
    int32_t last_velocity;
    int32_t max_position;
    int32_t max_velocity;
    ESP32Encoder encoder;
    pid_t position_pid;
    pid_t velocity_pid;
    hbridge_t hbridge;

    void set_current_position(int32_t position);

    void set_current_velocity(int32_t velocity);
    pid_t *pid_pointer(pid_types::pid_type_t pid_type);

    // create pid
    pid_t create_pid(double_t dt, double_t max_ctrl_value);

    // create hbridge
    hbridge_t create_hbridge(int32_t pin_pwm, int32_t pin_ina, int32_t pin_inb, int32_t freq_hz, int32_t resolution_bit, int32_t pwm_ch, int32_t max_ctrl_value);

public:

    NewMotorTest(double_t delta_time, /*double_t max_ctrl_value,*/ int32_t pwm_pin, int32_t ina_pin, int32_t inb_pin, int32_t freq_hz, int32_t resolution_bit, int32_t pwm_ch, int32_t pwm_max, int32_t max_ctrl_value);

    ~NewMotorTest();

    int32_t get_current_position();

    int32_t get_current_velocity();

    int32_t set_target_position(int32_t target_position);

    int32_t get_target_position();

    int32_t set_target_velocity(int32_t target_velocity);

    int32_t get_target_velocity();

    int32_t set_max_position(int32_t max_position);

    int32_t get_max_position();

    int32_t set_max_velocity(int32_t max_velocity);

    int32_t get_max_velocity();

    // pid related methods
    double_t set_kp(pid_types::pid_type_t pid_type, double_t kp); // double_t set_position_kp(double_t kp); && double_t set_velocity_kp(double_t kp);
    double_t set_ki(pid_types::pid_type_t pid_type, double_t ki); // double_t set_position_ki(double_t ki); && double_t set_velocity_ki(double_t ki);
    double_t set_kd(pid_types::pid_type_t pid_type, double_t kd); // double_t set_position_kd(double_t kd); && double_t set_velocity_kd(double_t kd);

    double_t get_delta_time(pid_types::pid_type_t pid_type); // double_t get_position_delta_time(void); && double_t get_velocity_delta_time(void);

    double_t get_kp(pid_types::pid_type_t pid_type); // double_t get_position_kp(void); && double_t get_velocity_kp(void);
    double_t get_ki(pid_types::pid_type_t pid_type); // double_t get_position_ki(void); && double_t get_velocity_ki(void);
    double_t get_kd(pid_types::pid_type_t pid_type); // double_t get_position_kd(void); && double_t get_velocity_kd(void);
    
    double_t get_error(pid_types::pid_type_t pid_type); // double_t get_position_error(void); && double_t get_velocity_error(void);

    double_t squash(pid_types::pid_type_t pid_type, double_t value);
    void update(pid_types::pid_type_t pid_type, double_t set_value, double_t current_value, double_t *ctrl_value, double_t integration_threshold);

    // hbridge related methods
    void set_pwm(int32_t ctrl_value);

    void break_motor();
};
