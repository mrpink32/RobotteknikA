#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

enum
{
    CMD_PID,
    CMD_TOGGLE,
    CMD_ERR_REQ,
    CMD_POS_REQ,
    CMD_VEL_REQ,
    CMD_LED_STATE,
    CMD_MAX_POS_REQ,
    CMD_MAX_VEL_REQ,
    CMD_MAX_ERR_REQ,
    CMD_PREV_ERR,
    CMD_TARGET_POS,
    CMD_TARGET_VEL,
};

typedef struct
{
    const char *name;
    int32_t value;
} cmd_t;

static cmd_t cmds[] = {
    {"pid_", CMD_PID},
    {"err", CMD_ERR_REQ},
    {"toggle", CMD_TOGGLE},
    {"prev_err", CMD_PREV_ERR},
    {"led_state", CMD_LED_STATE},
    {"current_pos", CMD_POS_REQ},
    {"current_vel", CMD_VEL_REQ},
    {"max_pos", CMD_MAX_POS_REQ},
    {"max_vel", CMD_MAX_VEL_REQ},
    {"target_pos", CMD_TARGET_POS},
    {"target_vel", CMD_TARGET_VEL},
};

int32_t get_cmd_value(char *command)
{
    for (int i = 0; i < sizeof(cmds) / sizeof(cmd_t); i++)
    {
        cmd_t *cmd = &cmds[i];
        if (strncmp(command, cmd->name, strlen(cmd->name)) == 0)
        {
            return cmd->value;
        }
    }
    return -1;
}

const char *get_cmd_name(int32_t cmd_value)
{
    for (int i = 0; i < sizeof(cmds) / sizeof(cmd_t); i++)
    {
        cmd_t *cmd = &cmds[i];
        if (cmd->value == cmd_value)
        {
            return cmd->name;
        }
    }
    return NULL;
}

void handle_command(uint8_t client_num, char *command, size_t length)
{
    // char *command = (char *)payload;

    printf("[%u] Received text: %s\n", client_num, command);

    switch (get_cmd_value(command))
    {
    case CMD_TOGGLE:
        printf("sending %s to [%u]\n", command, client_num);
        break;
    case CMD_LED_STATE:
        printf("sending %s to [%u]\n", command, client_num);
        break;
    case CMD_PID:
        printf("sending %s to [%u]\n", command, client_num);
        break;
    case CMD_POS_REQ:
        printf("sending %s to [%u]\n", command, client_num);
        break;
    case CMD_VEL_REQ:
        printf("sending %s to [%u]\n", command, client_num);
        break;
    case CMD_ERR_REQ:
        printf("sending %s to [%u]\n", command, client_num);
        break;
    case CMD_MAX_POS_REQ:
        printf("sending %s to [%u]\n", command, client_num);
        break;
    case CMD_MAX_VEL_REQ:
        printf("sending %s to [%u]\n", command, client_num);
        break;
    case CMD_TARGET_POS:
        printf("sending %s to [%u]\n", command, client_num);
        break;
    case CMD_TARGET_VEL:
        printf("sending %s to [%u]\n", command, client_num);
        break;
    default:
        printf("[%u] Message not recognized\n", client_num);
    }
}

int main(int argc, char const *argv[])
{
    char *command = (char *)argv[1];
    handle_command(0, command, sizeof(command));
    return 0;
}
