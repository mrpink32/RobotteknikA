//
// Created by mikkel on 2023/03/17.
//
#include "robot.h"
#include <stdio.h>

int main()
{
    print_hello_from_rust();

    Pid pid = new_pid(0.1, 0.0);

    set_kp(&pid, 0.1);
    set_ki(&pid, 0.1);
    set_kd(&pid, 0.1);

    printf("%f\n", get_kp(&pid));
    printf("%f\n", get_ki(&pid));
    printf("%f\n", get_kd(&pid));

    return 0;
}