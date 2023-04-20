#pragma once
#include "pid.h"
#include "hbridge.h"
#include "ESP32Encoder.h"

class DCmotor
{
public:

struct pins_t
{
  int32_t hb_pwm;
  int32_t hb_a;
  int32_t hb_b;
  int32_t enc_a;
  int32_t enc_b;
};

struct pwm_t
{
  int32_t freq;
  int32_t res;
  int32_t ch;
  int32_t ctrl_val;
};

  DCmotor(ESP32Encoder enc, H_Bridge hb, Pid pid_vel, Pid pid_pos, float enc_res);

  double get_req_pos();
  double get_act_pos();
  double get_ctrl_pos();
  double get_req_vel();
  double get_act_vel();
  double get_ctrl_vel();
  bool get_pos_mode();
  bool jobs_done();
  void init(pins_t pin, pwm_t pwm);
  void set_pos_mode(bool mode);
  void update_kx(char type, double value);
  void update_pid();
  void set_pos(double pos);
  
private:
  H_Bridge hb;
  Pid pid_vel;
  Pid pid_pos;
  ESP32Encoder enc;
  float enc_res;
  pins_t pin;
  pwm_t pwm;

  bool pos_mode = true;
  char type;
  double value;
  double pos;

  double ctrl_pos;
  double ctrl_vel;

  double prev_pos;
  volatile double req_pos;
  volatile double req_vel;
  volatile double current_pos;
  volatile double current_vel;
};