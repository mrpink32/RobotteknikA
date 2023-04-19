#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

const char *cmd_toggle = "toggle";
const char *cmd_ledstate = "led_state";
const char *cmd_pid = "pid_";
const char *cmd_pos = "pos";
const char *cmd_max_pos = "max_pos";
const char *cmd_vel = "vel";
const char *cmd_max_vel = "max_vel";
const char *cmd_err = "err";
const char *cmd_prev_err = "prev_err";
const char *cmd_target_pos = "target_pos";
const char *cmd_target_vel = "target_vel";

void handle_command(uint8_t client_num, char *command, size_t length)
{
    // char *command = (char *)payload;

    printf("[%u] Received text: %s\n", client_num, command);

    if (strcmp(command, cmd_toggle) == 0)
        printf("sending %s to [%u]\n", command, client_num);
    else if (strncmp(command, cmd_ledstate, strlen(cmd_ledstate)) == 0)
        printf("sending %s to [%u]\n", command, client_num);
    else if (strncmp(command, cmd_pid, strlen(cmd_pid)) == 0)
        printf("sending %s to [%u]\n", command, client_num);
    else if (strncmp(command, cmd_pos, strlen(cmd_pos)) == 0)
        printf("sending %s to [%u]\n", command, client_num);
    else if (strncmp(command, cmd_vel, strlen(cmd_vel)) == 0)
        printf("sending %s to [%u]\n", command, client_num);
    else if (strncmp(command, cmd_err, strlen(cmd_err)) == 0)
        printf("sending %s to [%u]\n", command, client_num);
    else if (strncmp(command, cmd_max_pos, strlen(cmd_max_pos)) == 0)
        printf("sending %s to [%u]\n", command, client_num);
    else if (strncmp(command, cmd_max_vel, strlen(cmd_max_vel)) == 0)
        printf("sending %s to [%u]\n", command, client_num);
    else if (strncmp(command, cmd_target_pos, strlen(cmd_target_pos)) == 0)
        printf("sending %s to [%u]\n", command, client_num);
    else if (strncmp(command, cmd_target_vel, strlen(cmd_target_vel)) == 0)
        printf("sending %s to [%u]\n", command, client_num);
    else
        printf("[%u] Message not recognized\n", client_num);

}

int main(int argc, char const *argv[])
{
    char *command = (char *)argv[1];
    handle_command(0, command, sizeof(command));
    return 0;
}



