#include "dc_motor.h"

volatile double max_vel = 9600 / 10;
const double integration_threshold = 200;

DCmotor::DCmotor(ESP32Encoder enc, H_Bridge hb, Pid pid_vel, Pid pid_pos, float enc_res) : 
            enc(enc), hb(hb), pid_vel(pid_vel), pid_pos(pid_pos), enc_res(enc_res)
{
  this->pos_mode = true;
  this->ctrl_pos = ctrl_pos;
  this->ctrl_vel = ctrl_vel;
  this->req_pos = req_pos;
  this->req_vel = req_vel;
  this->current_pos = current_pos;
  this->current_vel = current_vel;
  this->prev_pos = current_pos;
  this->pid_pos.set_kp(25);
  this->pid_pos.set_ki(1);
  this->pid_pos.set_kd(1);
  this->pid_vel.set_kp(1);
  this->pid_vel.set_ki(0);
  this->pid_vel.set_kd(0);  
}

double DCmotor::get_req_pos()
{
  return req_pos;
}

double DCmotor::get_act_pos()
{
  return current_pos;
}

double DCmotor::get_ctrl_pos()
{
  return ctrl_pos;
}

double DCmotor::get_req_vel()
{
  return req_vel;
}

double DCmotor::get_act_vel()
{
  return current_vel;
}

double DCmotor::get_ctrl_vel()
{
  return ctrl_vel;
}

bool DCmotor::get_pos_mode()
{
  return pos_mode;
}

void DCmotor::init(pins_t pin, pwm_t pwm)
{
  this->enc.attachFullQuad(pin.enc_a, pin.enc_b);    // Attache pins for use as encoder pins
  this->enc.clearCount();
  this->hb.begin(pin.hb_pwm, pin.hb_a, pin.hb_b,
                pwm.freq, pwm.res, pwm.ch, pwm.ctrl_val);
}

void DCmotor::set_pos_mode(bool mode)
{
  pos_mode = mode;
}

void DCmotor::update_kx(char type, double value)
{
  if(pos_mode)
  {
    switch(type)
    {
    case 'p':
      log_i("position kp");
      pid_pos.set_kp(value);
      break;
    case 'i':
      pid_pos.set_ki(value);
      break;
    case 'd':
      pid_pos.set_kd(value);
      break;
    }
  }
  else
  {
    switch(type)
    {
    case 'p':
      log_i("velocity kp");
      pid_vel.set_kp(value);
      break;
    case 'i':
      pid_vel.set_ki(value);
      break;
    case 'd':
      pid_vel.set_kd(value);
      break;
    default:
      log_e("Error in DCmotor");
      break;
    }
  }
}

bool DCmotor::jobs_done()
{
  return abs(req_pos - current_pos) <= 10;
}

void DCmotor::update_pid()
{
  current_pos = enc.getCount();

  current_vel = (current_pos - prev_pos) / pid_pos.get_dt();

  if (pos_mode)
  {
    pid_pos.update(req_pos, current_pos, &ctrl_pos, integration_threshold);

    req_vel = constrain(ctrl_pos, -max_vel, max_vel);
  }

  pid_vel.update(req_vel, current_vel, &ctrl_vel, 100000);

  hb.set_pwm(ctrl_vel);

  prev_pos = current_pos;
}

void DCmotor::set_pos(double pos)
{
  req_pos = pos * enc_res;
  // wait_move();
}