#[no_mangle]
pub extern "C" fn print_hello_from_rust() {
    println!("Hello from Rust");
}

mod robot {
    pub mod pid {
        #[repr(C)]
        pub struct Pid {
            delta_time: f64,
            min_ctrl_value: f64,
            max_ctrl_value: f64,
            kp: f64,
            ki: f64,
            kd: f64,
            error: f64,
            error_sum: f64,
            previous_error: f64,
        }

        impl Pid {
            #[no_mangle]
            pub extern "C" fn new_pid(
                delta_time: f64,
                max_ctrl_value: f64,
                // min_ctrl_value: f64,
            ) -> Pid {
                // let min_ctrl_value: f64 = if min_ctrl_value != None {
                //     min_ctrl_value.expect("Not a valid double")
                // } else {
                //     -max_ctrl_value
                // };
                let min_ctrl_value = -max_ctrl_value;
                Pid {
                    delta_time,
                    min_ctrl_value,
                    max_ctrl_value,
                    kp: 0.0,
                    ki: 0.0,
                    kd: 0.0,
                    error: 0.0,
                    error_sum: 0.0,
                    previous_error: 0.0,
                }
            }

            // pub fn set_kp(&mut self, kp: f64) {
            //     self.kp = kp;
            // }
            #[no_mangle]
            pub extern "C" fn set_kp(&mut self, kp: f64) {
                self.kp = kp;
            }

            // pub fn set_ki(&mut self, ki: f64) {
            //     self.ki = ki;
            // }
            #[no_mangle]
            pub extern "C" fn set_ki(&mut self, ki: f64) {
                self.ki = ki;
            }

            pub fn set_kd(&mut self, kd: f64) {
                self.kd = kd;
            }
            pub fn get_delta_time(&self) -> f64 {
                self.delta_time
            }
            pub fn get_min_ctrl_value(&self) -> f64 {
                self.min_ctrl_value
            }
            pub fn get_max_ctrl_value(&self) -> f64 {
                self.max_ctrl_value
            }
            pub fn get_kp(&self) -> f64 {
                self.kp
            }
            pub fn get_ki(&self) -> f64 {
                self.ki
            }
            pub fn get_kd(&self) -> f64 {
                self.kd
            }
            pub fn get_error(&self) -> f64 {
                self.error
            }
            /// Returns the value if it is between the min_ctrl_value and the max_ctrl_value of the pid.
            /// If lower returns min_ctrl_value of the pid, if higher returns max_ctrl_value of the pid.
            /// Panics if the value is None.
            pub fn squash(&self, value: f64) -> f64 {
                let value = match Some(value) {
                    Some(_) if value < self.min_ctrl_value => self.min_ctrl_value,
                    Some(_) if value > self.max_ctrl_value => self.max_ctrl_value,
                    None => panic!("Value cannot be None"),
                    _ => value,
                };
                value
            }
            pub fn update(
                &mut self,
                set_value: f64,
                current_value: f64,
                ctrl_value: &mut f64,
                integration_threshold: f64,
            ) {
                self.error = set_value - current_value;
                if self.error.abs() < integration_threshold {
                    self.error_sum += self.error;
                    let kp_val: f64 = self.error;
                    let ki_val: f64 = self.error_sum * self.delta_time;
                    let kd_val: f64 = (self.previous_error - self.error) / self.delta_time;
                    self.previous_error = self.error;
                    let ctrl: f64 = self.kp * kp_val + self.ki * ki_val + self.kd * kd_val;
                    *ctrl_value = self.squash(ctrl);
                }
            }
        }
    }
    // pub mod h_bridge {
    //     pub struct HBridge {
    //         pin_pwm: i32,        // hbridge PWM pin
    //         pin_ina: i32,        // hbridge INB pin
    //         pin_inb: i32,        // hbridge INA pin
    //         freq_hz: i32,        // pwm frequency
    //         resolution_bit: i32, // pwm resolution
    //         pwm_ch: i32,         // pwm channel
    //         pwm_max: i32,        // maximum pwm value;
    //         max_ctrl_value: i32, // maximum ctrl value
    //     }
    //     impl HBridge {
    //         pub fn new() -> HBridge {
    //             todo!()
    //         }
    //         pub fn set_pwm() {
    //             todo!()
    //         }
    //         pub fn begin() {
    //             todo!()
    //         }
    //         pub fn break_motor() {
    //             todo!()
    //         }
    //     }
    // }
}

// use robot::{h_bridge::HBridge, pid::Pid};
// #[test]
// fn it_works() {
//     let mut pid: Pid = Pid::new(1.0, 1.0, None);
//     pid.set_kp(10.0);
//     assert_eq!(pid.get_kp(), 10.0);
//     assert_eq!(pid.squash(10.0), pid.get_max_ctrl_value());
// }
