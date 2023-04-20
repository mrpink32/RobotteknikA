#pragma once
#include <ESP32Encoder.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <math.h>
#include <string.h>
#include <Arduino.h>
#include <motor.h> // maybe switch to new version
// #include <new-motor-test.h> // <-- needs encoder related stuff
#include <global.h>
using namespace std;

// Constants
const char *SSID = "grim";
const char *PASSWORD = "grimgrim";

enum Commands
{
    CMD_PID,
    CMD_TOGGLE,
    CMD_ERR_REQ,
    CMD_POS_REQ,
    CMD_VEL_REQ,
    CMD_LED_STATE,
    CMD_MAX_POS,
    CMD_MAX_VEL,
    CMD_PREV_ERR,
    CMD_TARGET_POS,
    CMD_TARGET_VEL,
};

typedef struct
{
    const char *name;
    int32_t value;
} command_t;

static command_t commands[] = {
    {"pid_", CMD_PID},
    {"err", CMD_ERR_REQ},
    {"toggle", CMD_TOGGLE},
    {"prev_err", CMD_PREV_ERR},
    {"led_state", CMD_LED_STATE},
    {"current_pos", CMD_POS_REQ},
    {"current_vel", CMD_VEL_REQ},
    {"max_pos", CMD_MAX_POS},
    {"max_vel", CMD_MAX_VEL},
    {"target_pos", CMD_TARGET_POS},
    {"target_vel", CMD_TARGET_VEL},
};

const int32_t WIFI_CHANNEL = 9; // alle grupper skal have hver sin kanal
const int32_t DNS_PORT = 53;
const int32_t HTTP_PORT = 80;
const int32_t WS_PORT = 1337;

// Globals

AsyncWebServer Server(HTTP_PORT);
WebSocketsServer WebSocket = WebSocketsServer(WS_PORT);
TaskHandle_t MotionTaskHandle;

char MsgBuf[64];
int32_t LedState = 0;

TaskHandle_t PidTaskHandle;
// TaskHandle_t PidTaskHandle2;
// TaskHandle_t PidTaskHandle_3;
// TaskHandle_t PositionTaskHandle;
ESP32Encoder encoder1;
ESP32Encoder encoder2;
ESP32Encoder encoder3;
// Pid pid_pos_1(DT_S, PID_MAX_CTRL_VALUE);
// Pid pid_pos_2(DT_S, PID_MAX_CTRL_VALUE);
// Pid pid_pos_3(DT_S, PID_MAX_CTRL_VALUE);
// Pid pid_vel1(DT_S, PID_MAX_CTRL_VALUE);
// Pid pid_vel2(DT_S, PID_MAX_CTRL_VALUE);
// Pid pid_vel3(DT_S, PID_MAX_CTRL_VALUE);
// H_Bridge hBridge1;
// H_Bridge hBridge2;
// H_Bridge hBridge3;

Motor motor1(PIN_HBRIDGE_PWM_1, PIN_HBRIDGE_INA_1, PIN_HBRIDGE_INB_1,
             PWM_FREQ_HZ, PWM_RES_BITS, PWM_CH_1, MAX_CTRL_VALUE, DT_S);
Motor motor2(PIN_HBRIDGE_PWM_2, PIN_HBRIDGE_INA_2, PIN_HBRIDGE_INB_2,
             PWM_FREQ_HZ, PWM_RES_BITS, PWM_CH_2, MAX_CTRL_VALUE, DT_S);
Motor motor3(PIN_HBRIDGE_PWM_3, PIN_HBRIDGE_INA_3, PIN_HBRIDGE_INB_3,
             PWM_FREQ_HZ, PWM_RES_BITS, PWM_CH_3, MAX_CTRL_VALUE, DT_S);
Motor motors[3] = {motor1, motor2, motor3};

const double integration_threshold = 200;

volatile double req_posx;
volatile double req_posy;
volatile double dest_posx;
volatile double dest_posy;
// volatile int32_t current_pos_1;
// volatile int32_t current_pos_2;
// volatile int32_t current_pos_3;
// volatile double_t current_vel_1;
// volatile double_t current_vel_2;
// volatile double_t current_vel_3;
// volatile double max_vel = 300;
volatile double device_x = 0;
volatile double device_y = 0;
const double b = 24.5;
const double r = 5.05;

double ctrl_pos_1;
double ctrl_pos_2;
double ctrl_pos_3;
double ctrl_vel1;
double ctrl_vel2;
double ctrl_vel3;

bool mode_pos = true;

/***********************************************************
 * Functions
 */

/**
 * This function searches for a matching command name in the 'commands' array.
 * If a match is found, the corresponding command value is returned as an integer.
 * If no match is found, -1 is returned.
 *
 * @param command: char pointer representing the command name to search for
 * @return: integer value of the matching command, or -1 if no match is found
 */
int32_t get_command_value(char *command)
{
    for (int i = 0; i < sizeof(commands) / sizeof(command_t); i++)
    {
        command_t *cmd = &commands[i];
        if (strncmp(command, cmd->name, strlen(cmd->name)) == 0)
        {
            return cmd->value;
        }
    }
    return -1;
}

/**
 * This function takes an integer command value 'command_value' and searches for a matching command name in the 'commands' array.
 * If a match is found, the corresponding command name is returned as a const char pointer. If no match is found, NULL is returned.
 *
 * @param command_value: integer value of the command to search for
 * @return: const char pointer representing the command name, or NULL if no match is found
 */
const char *get_command_name(int32_t command_value)
{
    for (int i = 0; i < sizeof(commands) / sizeof(command_t); i++)
    {
        command_t *cmd = &commands[i];
        if (cmd->value == command_value)
        {
            return cmd->name;
        }
    }
    return NULL;
}

void web_socket_send(const char *buffer, uint8_t client_num, bool broadcast)
{
    if (broadcast)
    {
        log_d("Broadcasting: %s", buffer);
        WebSocket.broadcastTXT(buffer, strlen(buffer)); // all clients
    }
    else
    {
        log_d("Sending to [%u]: %s", client_num, buffer);
        WebSocket.sendTXT(client_num, buffer); // only one client
    }
}

void handle_toggle(int32_t client_num)
{
    LedState = LedState ? 0 : 1;
    log_i("Toggling LED to %u", LedState);
    digitalWrite(LED_PIN, LedState);
    sprintf(MsgBuf, "%d", LedState);
    web_socket_send(MsgBuf, client_num, true);
}

void handle_led_state(char *command, uint8_t client_num)
{
    char *value = strstr(command, ":");

    if (value == NULL || *value != ':')
    {
        log_e("[%u]: Bad command %s", client_num, command);
        return;
    }

    if (*(value + 1) == '?')
    {
        sprintf(MsgBuf, "%s:%d", get_command_name(CMD_LED_STATE), LedState);
        web_socket_send(MsgBuf, client_num, false);
    }
    else
    {
        errno = 0;
        char *e;
        int32_t result = strtol(value + 1, &e, 10);
        if (*e == '\0' && 0 == errno) // no error
        {
            LedState = result;
            log_d("[%u]: LedState received %d", client_num, LedState);
            sprintf(MsgBuf, "%s:%d", get_command_name(CMD_LED_STATE), LedState);
            web_socket_send(MsgBuf, client_num, true);
        }
        else
        {
            log_e("[%u]: illegal LedState received: %s", client_num, value + 1);
        }
    }
}

void handle_kx(char *command, uint8_t client_num)
{
    char *value = strstr(command, ":");

    if (value == NULL || *value != ':')
    {
        Serial.printf("[%u]: Bad command %s\n", client_num, command);
        log_e("[%u]: Bad command %s", client_num, command);
        return;
    }

    char subtype = *(value - 1);

    double *parm_value;
    errno = 0;
    char *e;
    double result = strtol(value + 1, &e, 10);
    switch (subtype)
    {
    case 'p':
        parm_value = &result;
        motor1.position_pid.set_kp(*parm_value);
        motor2.position_pid.set_kp(*parm_value);
        motor3.position_pid.set_kp(*parm_value);
        break;
    case 'i':
        parm_value = &result;
        motor1.position_pid.set_ki(*parm_value);
        motor2.position_pid.set_ki(*parm_value);
        motor3.position_pid.set_ki(*parm_value);
        break;
    case 'd':
        parm_value = &result;
        motor1.position_pid.set_kd(*parm_value);
        motor2.position_pid.set_kd(*parm_value);
        motor3.position_pid.set_kd(*parm_value);
        break;
    default:
        log_e("[%u]: Bad command %s", client_num, command);
        return;
        break;
    }

    if (*(value + 1) == '?')
    {
        errno = 0;
        char *e;
        double result = strtod(value + 1, &e);
        if (*e == '\0' && 0 == errno) // no error
        {
            *parm_value = result;
            log_d("[%u]: k%c value received %f", client_num, subtype, *parm_value);
            sprintf(MsgBuf, "%sk%c:%f", get_command_name(CMD_PID), subtype, *parm_value);
            web_socket_send(MsgBuf, client_num, false);
        }
        else
        {
            log_e("[%u]: illegal format of k%c value received: %s", client_num, subtype, value + 1);
        }
    }
    web_socket_send(MsgBuf, client_num, false);
    sprintf(MsgBuf, "%sk%c:%f", get_command_name(CMD_PID), subtype, *parm_value);
}

void handle_pos_req(char *command, uint8_t client_num)
{
    char *value = strstr(command, ":");

    if (value == NULL || *value != ':')
    {
        log_e("[%u]: Bad command %s", client_num, command);
        return;
    }
    else if (*(value + 1) != '?')
    {
        log_e("[%u]: setting position values is not supported: %s", client_num, command);
        return;
    }
    int32_t data_1 = motor1.get_position();
    int32_t data_2 = motor2.get_position();
    int32_t data_3 = motor3.get_position();
    sprintf(MsgBuf, "%s:%d,%d,%d,", get_command_name(CMD_POS_REQ), data_1, data_2, data_3);
    web_socket_send(MsgBuf, client_num, false);
}

void handle_vel_req(char *command, uint8_t client_num)
{
    char *value = strstr(command, ":");

    if (value == NULL || *value != ':')
    {
        log_e("[%u]: Bad command %s", client_num, command);
        return;
    }
    else if (*(value + 1) != '?')
    {
        log_e("[%u]: setting velocity values is not supported: %s", client_num, command);
        return;
    }
    int32_t data_1 = motor1.get_velocity();
    int32_t data_2 = motor2.get_velocity();
    int32_t data_3 = motor3.get_velocity();
    sprintf(MsgBuf, "%s:%d,%d,%d,", get_command_name(CMD_VEL_REQ), data_1, data_2, data_3);
    web_socket_send(MsgBuf, client_num, false);
}

void handle_err_req(char *command, uint8_t client_num)
{
    char *value = strstr(command, ":");

    if (value == NULL || *value != ':')
    {
        Serial.printf("[%u]: Bad command %s\n", client_num, command);
        log_e("[%u]: Bad command %s", client_num, command);
        return;
    }
    else if (*(value + 1) != '?')
    {
        log_e("[%u]: setting velocity values is not supported: %s", client_num, command);
        return;
    }
    double data_1 = motor1.position_pid.get_error();
    double data_2 = motor1.velocity_pid.get_error();
    double data_3 = motor2.position_pid.get_error();
    double data_4 = motor2.velocity_pid.get_error();
    double data_5 = motor3.position_pid.get_error();
    double data_6 = motor3.velocity_pid.get_error();
    sprintf(MsgBuf, "%s:%f,%f,%f,%f,%f,%f,", get_command_name(CMD_ERR_REQ), data_1, data_2, data_3, data_4, data_5, data_6);
    web_socket_send(MsgBuf, client_num, false);
}

void handle_max_pos(char *command, uint8_t client_num)
{
    char *value = strstr(command, ":");

    if (value == NULL || *value != ':')
    {
        log_e("[%u]: Bad command %s", client_num, command);
        return;
    }

    if (*(value + 1) == '?')
    {
        int32_t data_1 = motor1.position_pid.get_max_ctrl_value();
        int32_t data_2 = motor2.position_pid.get_max_ctrl_value();
        int32_t data_3 = motor3.position_pid.get_max_ctrl_value();
        sprintf(MsgBuf, "%s:%d,%d,%d,", get_command_name(CMD_MAX_POS), data_1, data_2, data_3);
        web_socket_send(MsgBuf, client_num, false);
    }

    errno = 0;
    char *e;
    char *data_1 = strtok(value + 1, ",");
    double result = strtod(value + 1, &e);
    if (*e == '\0' && errno == 0) // no error
    {
        int32_t data_1 = motor1.position_pid.set_max_ctrl_value(result);
        int32_t data_2 = motor2.position_pid.set_max_ctrl_value(result);
        int32_t data_3 = motor3.position_pid.set_max_ctrl_value(result);
        sprintf(MsgBuf, "%s:%d,%d,%d,", get_command_name(CMD_MAX_POS), data_1, data_2, data_3);
        web_socket_send(MsgBuf, client_num, false);
    }
    else
    {
        log_e("[%u]: setting position values is not supported: %s", client_num, command);
    }
}

void handle_max_vel(char *command, uint8_t client_num)
{
    char *value = strstr(command, ":");

    if (value == NULL || *value != ':')
    {
        log_e("[%u]: Bad command %s", client_num, command);
        return;
    }

    if (*(value + 1) == '?')
    {
        int32_t data_1 = motor1.velocity_pid.get_max_ctrl_value();
        int32_t data_2 = motor2.velocity_pid.get_max_ctrl_value();
        int32_t data_3 = motor3.velocity_pid.get_max_ctrl_value();
        sprintf(MsgBuf, "%s:%d,%d,%d,", get_command_name(CMD_MAX_VEL), data_1, data_2, data_3);
        web_socket_send(MsgBuf, client_num, false);
    }

    errno = 0;
    char *e;
    char *data_1 = strtok(value + 1, ",");
    double result = strtod(value + 1, &e);
    if (*e == '\0' && errno == 0) // no error
    {
        int32_t data_1 = motor1.velocity_pid.set_max_ctrl_value(result);
        int32_t data_2 = motor2.velocity_pid.set_max_ctrl_value(result);
        int32_t data_3 = motor3.velocity_pid.set_max_ctrl_value(result);
        sprintf(MsgBuf, "%s:%d,%d,%d,", get_command_name(CMD_MAX_VEL), data_1, data_2, data_3);
        web_socket_send(MsgBuf, client_num, false);
    }
    else
    {
        log_e("[%u]: setting velocity values is not supported: %s", client_num, command);
    }
}

void handle_target_pos(char *command, uint8_t client_num)
{
    char *value = strstr(command, ":");

    if (value == NULL || *value != ':')
    {
        log_e("[%u]: Bad command %s", client_num, command);
        return;
    }

    if (*(value + 1) == '?')
    {
        // placeholder values used util motor debugging has been done
        int32_t data_1 = motor1.get_target_position();
        int32_t data_2 = motor2.get_target_position();
        int32_t data_3 = motor3.get_target_position();
        sprintf(MsgBuf, "%s:%d,%d,%d,", get_command_name(CMD_TARGET_POS), data_1, data_2, data_3);
        web_socket_send(MsgBuf, client_num, false);
        return;
    }

    errno = 0;
    int32_t result = atoi(strtok(value + 1, ","));
    int32_t id = atoi(strtok(NULL, ","));
    log_d("token: %d | %s", result, id);

    if (errno != 0)
    {
        log_e("[%u]: Bad command %s", client_num, command);
        return;
    }

    switch (id)
    {
    case 0:
    {
        int32_t data = motor1.set_target_position(result);
        sprintf(MsgBuf, "%s:%d, %d,", get_command_name(CMD_TARGET_POS), id, data);
    }
    break;
    case 1:
    {
        int32_t data = motor2.set_target_position(result);
        sprintf(MsgBuf, "%s:%d, %d,", get_command_name(CMD_TARGET_POS), id, data);
    }
    break;
    case 2:
    {
        int32_t data = motor3.set_target_position(result);
        sprintf(MsgBuf, "%s:%d, %d,", get_command_name(CMD_TARGET_POS), id, data);
    }
    break;
    case 3:
    {
        int32_t data_1 = motor1.set_target_position(result);
        int32_t data_2 = motor2.set_target_position(result);
        int32_t data_3 = motor3.set_target_position(result);
        sprintf(MsgBuf, "%s:%d,%d,%d,%d,", get_command_name(CMD_TARGET_POS), id, data_1, data_2, data_3);
    }
    break;
    default:
        log_e("[%u]: Bad command %s", client_num, command);
        return;
    }
    web_socket_send(MsgBuf, client_num, false);
}

void handle_target_vel(char *command, uint8_t client_num)
{
    char *value = strstr(command, ":");

    if (value == NULL || *value != ':')
    {
        log_e("[%u]: Bad command %s", client_num, command);
        return;
    }

    if (*(value + 1) == '?')
    {
        // placeholder values used util motor debugging has been done
        int32_t data_1 = motor1.get_target_velocity();
        int32_t data_2 = motor2.get_target_velocity();
        int32_t data_3 = motor3.get_target_velocity();
        sprintf(MsgBuf, "%s:%d,%d,%d,", get_command_name(CMD_TARGET_VEL), data_1, data_2, data_3);
        web_socket_send(MsgBuf, client_num, false);
        return;
    }

    errno = 0;
    int32_t result = atoi(strtok(value + 1, ","));
    int32_t id = atoi(strtok(NULL, ","));
    log_d("token: %d | %s", result, id);

    if (errno != 0)
    {
        log_e("[%u]: Bad command %s", client_num, command);
        return;
    }

    switch (id)
    {
    case 0:
    {
        int32_t data = motor1.set_target_velocity(result);
        sprintf(MsgBuf, "%s:%d, %d,", get_command_name(CMD_TARGET_VEL), id, data);
    }
    break;
    case 1:
    {
        int32_t data = motor2.set_target_velocity(result);
        sprintf(MsgBuf, "%s:%d,", get_command_name(CMD_TARGET_VEL), data);
    }
    break;
    case 2:
    {
        int32_t data = motor3.set_target_velocity(result);
        sprintf(MsgBuf, "%s:%d,", get_command_name(CMD_TARGET_VEL), data);
    }
    break;
    case 3:
    {
        int32_t data_1 = motor1.set_target_velocity(result);
        int32_t data_2 = motor2.set_target_velocity(result);
        int32_t data_3 = motor3.set_target_velocity(result);
        sprintf(MsgBuf, "%s:%d,%d,%d,", get_command_name(CMD_TARGET_VEL), data_1, data_2, data_3);
    }
    break;
    default:
        log_e("[%u]: Bad command %s", client_num, command);
        return;
    }
    web_socket_send(MsgBuf, client_num, false);
}

/**
 * Handles a command received over a WebSocket connection.
 * @param client_num: The client number associated with the connection.
 * @param payload: The command payload received.
 * @param length: The length of the payload.
 */
void handle_command(uint8_t client_num, uint8_t *payload, size_t length)
{
    char *command = (char *)payload;

    log_d("[%u] Received text: %s", client_num, command);

    switch (get_command_value(command))
    {
    case CMD_TOGGLE:
        handle_toggle(client_num);
        break;
    case CMD_LED_STATE:
        handle_led_state(command, client_num);
        break;
    case CMD_PID:
        handle_kx(command, client_num);
        break;
    case CMD_POS_REQ:
        handle_pos_req(command, client_num);
        break;
    case CMD_VEL_REQ:
        handle_vel_req(command, client_num);
        break;
    case CMD_ERR_REQ:
        handle_err_req(command, client_num);
        break;
    case CMD_MAX_POS:
        handle_max_pos(command, client_num);
        // log_d("Implement handler for request: %s", command);
        break;
    case CMD_MAX_VEL:
        // log_d("Implement handler for request: %s", command);
        handle_max_vel(command, client_num);
        break;
    case CMD_TARGET_POS:
        // log_d("Implement handler for request: %s", command);
        handle_target_pos(command, client_num);
        break;
    case CMD_TARGET_VEL:
        // log_d("Implement handler for request: %s", command);
        handle_target_vel(command, client_num);
        break;
    default:
        log_e("[%u] Message not recognized", client_num);
        return;
    }

    WebSocket.connectedClients();
}

// Callback: receiving any WebSocket message
void onWebSocketEvent(uint8_t client_num,
                      WStype_t type,
                      uint8_t *payload,
                      size_t length)
{
    // Figure out the type of WebSocket event
    switch (type)
    {

    // Client has disconnected
    case WStype_DISCONNECTED:
        log_i("[%u] Disconnected!", client_num);
        break;

    // New client has connected
    case WStype_CONNECTED:
    {
        IPAddress ip = WebSocket.remoteIP(client_num);
        log_i("[%u] Connection from ", client_num);
        log_i("IP: %s", ip.toString().c_str());
    }
    break;

    // Handle text messages from client
    case WStype_TEXT:
        handle_command(client_num, payload, length);
        break;

    // For everything else: do nothing
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    default:
        break;
    }
}

void handleRequest(AsyncWebServerRequest *request,
                   const char *file_name,
                   const char *content_type)
{
    IPAddress remote_ip = request->client()->remoteIP();
    log_i("HTTP GET request of %s from %s", request->url().c_str(), remote_ip.toString().c_str());
    request->send(SPIFFS, file_name, content_type);
}

// Callback: send homepage
void onIndexRequest(AsyncWebServerRequest *request)
{
    handleRequest(request, "/index.html", "text/html");
}

// Callback: send style sheet
void onCSSRequest(AsyncWebServerRequest *request)
{
    handleRequest(request, "/style.css", "text/css");
}

// Callback: send javascript
void onJSRequest(AsyncWebServerRequest *request)
{
    handleRequest(request, "/main.js", "text/javascript");
}

// Callback: send 404 if requested file does not exist
void onPageNotFound(AsyncWebServerRequest *request)
{
    IPAddress remote_ip = request->client()->remoteIP();
    log_i("HTTP GET request of %s from %s", request->url().c_str(), remote_ip.toString().c_str());
    request->send(404, "text/plain", "Not found");
}

void setup_spiffs()
{
    // Make sure we can read the file system
    if (!SPIFFS.begin())
    {
        log_e("Error mounting SPIFFS");
        while (1)
            ;
    }
}

void setup_network()
{
    // Start access point
    WiFi.softAP(SSID, PASSWORD, WIFI_CHANNEL);
    log_i("AP running!");
    log_i("My IP address: ");
    log_i("IP: %s", WiFi.softAPIP().toString().c_str());
    log_i("SSID: %s", WiFi.softAPSSID().c_str());

    // On HTTP request for root, provide index.html file
    Server.on("/", HTTP_GET, onIndexRequest);

    // On HTTP request for style sheet, provide style.css
    Server.on("/style.css", HTTP_GET, onCSSRequest);

    // On HTTP request for javascript, provide main.js
    Server.on("/main.js", HTTP_GET, onJSRequest);

    // Handle requests for pages that do not exist
    Server.onNotFound(onPageNotFound);

    // Start web server
    Server.begin();

    // Start WebSocket server and assign callback
    WebSocket.begin();
    WebSocket.onEvent(onWebSocketEvent);
}

void pid_task(void *arg)
{
    TickType_t xTimeIncrement = configTICK_RATE_HZ * DT_S;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    motor1.set_position(encoder1.getCount());
    int64_t prev_pos_1 = motor1.get_position();
    motor2.set_position(encoder2.getCount());
    int64_t prev_pos_2 = motor2.get_position();
    motor3.set_position(encoder3.getCount());
    int64_t prev_pos_3 = motor3.get_position();
    for (;;)
    {                                  // loop tager mindre end 18us * 2
        motor1.set_target_velocity(0); // double_t req_vel_1 = 0;
        motor2.set_target_velocity(0); // double_t req_vel_2 = 0;
        motor3.set_target_velocity(0); // double_t req_vel_3 = 0;
        digitalWrite(PIN_PID_LOOP_2, HIGH);

        motor1.set_position(encoder1.getCount());
        motor1.set_velocity((motor1.get_position() - prev_pos_1) / DT_S);
        motor2.set_position(encoder2.getCount());
        motor2.set_velocity((motor2.get_position() - prev_pos_2) / DT_S);
        motor3.set_position(encoder3.getCount());
        motor3.set_velocity((motor3.get_position() - prev_pos_3) / DT_S);

        if (mode_pos)
        {
            motor1.position_pid.update(req_posy, motor1.get_position(), &ctrl_pos_1, integrati1n_threshold);
            motor1.set_target_velocity(constrain(ctrl_pos_1, -motor1.get_max_velocity(), motor1.get_max_velocity())); // req_vel_1 = constrain(ctrl_pos_1, -max_vel, max_vel);

            motor2.position_pid.update(req_posy, motor2.get_position(), &ctrl_pos_2, integration_threshold);
            motor2.set_target_velocity(constrain(ctrl_pos_2, -motor2.get_max_velocity(), motor2.get_max_velocity())); // req_vel_2 = constrain(ctrl_pos_2, -max_vel, max_vel);

            motor3.position_pid.update(req_posy, motor3.get_position(), &ctrl_pos_3, integration_threshold);
            motor3.set_target_velocity(constrain(ctrl_pos_3, -motor3.get_max_velocity(), motor3.get_max_velocity())); // req_vel_3 = constrain(ctrl_pos_3, -max_vel, max_vel);
        }

        motor1.velocity_pid.update(req_vel_1, motor1.get_velocity(), &ctrl_vel1, integration_threshold);
        motor1.hbridge.set_pwm(ctrl_vel1);
        prev_pos_1 = motor1.get_position();

        motor2.velocity_pid.update(motor2.get_target_velocity(), motor2.get_velocity(), &ctrl_vel2, integration_threshold);
        motor2.hbridge.set_pwm(ctrl_vel2);
        prev_pos_2 = motor2.get_position();

        motor3.velocity_pid.update(req_vel_3, motor3.get_velocity(), &ctrl_vel3, integration_threshold);
        motor3.hbridge.set_pwm(ctrl_vel3);
        prev_pos_3 = motor3.get_position();

        digitalWrite(PIN_PID_LOOP_2, LOW);
        vTaskDelayUntil(&xLastWakeTime, xTimeIncrement);
    }
}

// void motion_task(void *arg)
// {
//     HBridge.begin(PIN_MOTOR_CTRL, PIN_MOTOR_INA, PIN_MOTOR_INB,
//                   PWM_FREQ_HZ, PWM_RES_BITS, PWM_CH);

//     TickType_t xTimeIncrement = configTICK_RATE_HZ / 10;
//     TickType_t xLastWakeTime = xTaskGetTickCount();
//     for (;;)
//     {
//         log_v("motion_task running...");
//         int32_t pwm_val = SliderVal;
//         if (LedState == 0)
//         {
//             pwm_val = -pwm_val;
//         }
//         HBridge.set_pwm(pwm_val);
//         vTaskDelayUntil(&xLastWakeTime, xTimeIncrement);
//     }
// }

void setup_tasks()
{
    log_i("starting pid task");
    xTaskCreatePinnedToCore(
        pid_task,       /* Function to implement the task */
        "pid_task",     /* Name of the task */
        5000,           /* Stack size in words */
        NULL,           /* Task input parameter */
        3,              /* Priority of the task from 0 to 25, higher number = higher priority */
        &PidTaskHandle, /* Task handle. */
        1);
    // log_i("starting motion task");
    // xTaskCreatePinnedToCore(
    //     motion_task,       /* Function to implement the task */
    //     "motion_task",     /* Name of the task */
    //     5000,                /* Stack size in words */
    //     NULL,                /* Task input parameter */
    //     3,                   /* Priority of the task from 0 to 25, higher number = higher priority */
    //     &PositionTaskHandle, /* Task handle. */
    //     0);
}

void setup()
{
    // Init LED and turn off
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Start Serial port
    Serial.begin(115200);

    // pinMode(PIN_PID_LOOP_1, OUTPUT);
    pinMode(PIN_PID_LOOP_2, OUTPUT);
    // pinMode(PIN_PID_LOOP_3, OUTPUT);
    // pinMode(PIN_LIMIT_SW, INPUT);

    ESP32Encoder::useInternalWeakPullResistors = UP;   // Enable the weak pull up resistors
    encoder1.attachFullQuad(PIN_ENC_A_1, PIN_ENC_B_1); // Attache pins for use as encoder pins
    encoder2.attachFullQuad(PIN_ENC_A_2, PIN_ENC_B_2); // Attache pins for use as encoder pins
    encoder3.attachFullQuad(PIN_ENC_A_3, PIN_ENC_B_3); // Attache pins for use as encoder pins
    encoder1.clearCount();
    encoder2.clearCount();
    encoder3.clearCount();

    motor1.position_pid.set_kp(10.0); // 12
    motor1.position_pid.set_ki(2.0);
    motor1.position_pid.set_kd(0.0);
    motor1.velocity_pid.set_kp(10);
    motor1.velocity_pid.set_ki(12);
    motor1.velocity_pid.set_kd(0);

    motor2.position_pid.set_kp(10.0); // 12
    motor2.position_pid.set_ki(2.0);
    motor2.position_pid.set_kd(0.0);
    motor2.velocity_pid.set_kp(10);
    motor2.velocity_pid.set_ki(12);
    motor2.velocity_pid.set_kd(0);

    motor3.position_pid.set_kp(10.0);
    motor3.position_pid.set_ki(2.0);
    motor3.position_pid.set_kd(0.0);
    motor3.velocity_pid.set_kp(10);
    motor3.velocity_pid.set_ki(12);
    motor3.velocity_pid.set_kd(0);

    setup_spiffs();
    setup_network();
    setup_tasks();
}

void loop()
{
    // Look for and handle WebSocket data
    WebSocket.loop();
    // Serial.printf(
    //     "curr_pos_1: %.2f  curr_pos_2: %.2f  \nctrl_pos_1: %.2f  ctrl_pos_2: %.2f  \ncurrent_velocity_1: %.2f current_velocity_2: %.2f ctrl_vel_1: %.2f ctrl_vel_2: %.2f\n\r",
    //     (double_t)motor1.get_position(), (double_t)motor2.get_position(), ctrl_pos_1, ctrl_pos_2, motor1.get_velocity(), motor2.get_velocity(), ctrl_vel1, ctrl_vel2);
    // Serial.printf("kp: %f, ki: %f, kd: %f, error: %f\n", motor1.position_pid.get_kp(), motor1.position_pid.get_ki(), motor1.position_pid.get_kd(), motor1.position_pid.get_error());
    // Serial.printf("kp: %f, ki: %f, kd: %f, error: %f\n", motor1.velocity_pid.get_kp(), motor1.velocity_pid.get_ki(), motor1.velocity_pid.get_kd(), motor1.velocity_pid.get_error());
    // Serial.printf("kp: %f, ki: %f, kd: %f, error: %f\n", motor2.position_pid.get_kp(), motor2.position_pid.get_ki(), motor2.position_pid.get_kd(), motor2.position_pid.get_error());
    // Serial.printf("kp: %f, ki: %f, kd: %f, error: %f\n", motor2.velocity_pid.get_kp(), motor2.velocity_pid.get_ki(), motor2.velocity_pid.get_kd(), motor2.velocity_pid.get_error());

    vTaskDelay(0.2 * configTICK_RATE_HZ);
}
