#include <ESP32Encoder.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <math.h>
#include <string>
#include <Arduino.h>
#include <motor.h>
#include <global.h>
#include <svglib.h>
using namespace std;

// Constants

const char *SSID = "grim";
const char *PASSWORD = "grimgrim";

const char *cmd_toggle = "toggle";
const char *cmd_led_state = "led_state";
const char *cmd_pid = "pid_";
const char *cmd_pos = "req_pos";
const char *cmd_vel = "req_vel";
const char *cmd_err = "req_err";

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
// double KpVal = 3.1415;
// double KiVal = 2.71;
// double KdVal = 42.0;

TaskHandle_t PidTaskHandle1;
TaskHandle_t PidTaskHandle2;
TaskHandle_t PidTaskHandle_3;
TaskHandle_t PositionTaskHandle;
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
volatile double max_vel = 300;
volatile double device_x = 0;
volatile double device_y = 0;
volatile double device_rotation = 0;
volatile double needed_rotation = 0;
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
        sprintf(MsgBuf, "%s:%d", cmd_led_state, LedState);
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
        }
        else
        {
            log_e("[%u]: illegal LedState received: %s", client_num, value + 1);
        }
        sprintf(MsgBuf, "%s:%d", cmd_led_state, LedState);
        web_socket_send(MsgBuf, client_num, true);
    }
}

void handle_pos_req(char *command, uint8_t client_num)
{
    char *value = strstr(command, ":");

    if (value == NULL || *value != ':')
    {
        log_e("[%u]: Bad command %s", client_num, command);
        return;
    }

    if (*(value + 1) == '?')
    {
        int32_t data_1 = motor1.get_position();
        int32_t data_2 = motor2.get_position();
        int32_t data_3 = motor3.get_position();
        sprintf(MsgBuf, "%s:%d,%d,%d,", cmd_pos, data_1, data_2, data_3);
        web_socket_send(MsgBuf, client_num, false);
    }
    else
    {
        char *e;
        std::string val = value;
        Serial.printf("%s", val);
        std::string command = command;
        Serial.printf("%s", command);
        std::string result1 = command.substr(val.size(), command.find(","));
        std::string result2 = command.substr(command.find(","), command.size());
        Serial.printf("%s, %s\n", result1, result2);
        if (*e == '\0' && 0 == errno) // no error
        {

            int32_t data_1 = motor1.get_position();
            int32_t data_2 = motor2.get_position();
            int32_t data_3 = motor3.get_position();
            sprintf(MsgBuf, "%s:%d,%d,%d,", cmd_pos, data_1, data_2, data_3);
            // sprintf(MsgBuf, "%s:%d,%d,%d,", cmd_pos);
            web_socket_send(MsgBuf, client_num, true);
        }
        else
        {
            log_e("[%u]: illegal Slidervalue received: %s", client_num, value + 1);
        }
    }
}

void handle_vel_req(char *command, uint8_t client_num)
{
    char *value = strstr(command, ":");

    if (value == NULL || *value != ':')
    {
        log_e("[%u]: Bad command %s", client_num, command);
        return;
    }

    if (*(value + 1) == '?')
    {
        int32_t data_1 = motor1.get_velocity();
        int32_t data_2 = motor2.get_velocity();
        int32_t data_3 = motor3.get_velocity();
        sprintf(MsgBuf, "%s:%d,%d,%d,", cmd_vel, data_1, data_2, data_3);
        web_socket_send(MsgBuf, client_num, false);
    }
    else
    {
        errno = 0;
        char *e;
        int32_t result = strtol(value + 1, &e, 10);
        Serial.println(result);
        if (*e == '\0' && 0 == errno) // no error
        {
            max_vel = result;
            sprintf(MsgBuf, "%s:%f", cmd_vel, max_vel);
            web_socket_send(MsgBuf, client_num, true);
        }
        else
        {
            log_e("[%u]: illegal Slidervalue received: %s", client_num, value + 1);
        }
        // Serial.println("",current_vel);
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
        sprintf(MsgBuf, "%sk%c:%f", cmd_pid, subtype, *parm_value);
        web_socket_send(MsgBuf, client_num, false);
    }
    else
    {
        errno = 0;
        char *e;
        double result = strtod(value + 1, &e);
        if (*e == '\0' && 0 == errno) // no error
        {
            *parm_value = result;
            log_d("[%u]: k%c value received %f", client_num, subtype, *parm_value);
            sprintf(MsgBuf, "%sk%c:%f", cmd_pid, subtype, *parm_value);
            web_socket_send(MsgBuf, client_num, true);
        }
        else
        {
            log_e("[%u]: illegal format of k%c value received: %s", client_num, subtype, value + 1);
        }
    }
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

    if (*(value + 1) == '?')
    {
        double data_1 = motor1.position_pid.get_error();
        double data_2 = motor1.velocity_pid.get_error();
        double data_3 = motor2.position_pid.get_error();
        double data_4 = motor2.velocity_pid.get_error();
        double data_5 = motor3.position_pid.get_error();
        double data_6 = motor3.velocity_pid.get_error();
        sprintf(MsgBuf, "%s:%f,%f,%f,%f,%f,%f,", cmd_err, data_1, data_2, data_3, data_4, data_5, data_6);
        web_socket_send(MsgBuf, client_num, false);
    }
    else
    {
        log_e("[%u]: settimg error values is not supported: %s", client_num, value + 1);
    }
}

void handle_command(uint8_t client_num, uint8_t *payload, size_t length)
{
    char *command = (char *)payload;

    log_d("[%u] Received text: %s", client_num, command);
    // Serial.printf("%s/n", command);

    if (strcmp(command, cmd_toggle) == 0)
        handle_toggle(client_num); // Toggle LED
    else if (strncmp(command, cmd_led_state, strlen(cmd_led_state)) == 0)
        handle_led_state(command, client_num); // Report the state of the LED
    else if (strncmp(command, cmd_pid, strlen(cmd_pid)) == 0)
        handle_kx(command, client_num); // pid params
    else if (strncmp(command, cmd_pos, strlen(cmd_pos)) == 0)
        handle_pos_req(command, client_num); // position request
    else if (strncmp(command, cmd_vel, strlen(cmd_vel)) == 0)
        handle_vel_req(command, client_num); // velocity request
    else if (strncmp(command, cmd_err, strlen(cmd_err)) == 0)
        handle_err_req(command, client_num); // error request
    else
        log_e("[%u] Message not recognized", client_num);

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

double_t DotProduct(std::vector<double_t> vec_1, std::vector<double_t> vec_2)
{
    double_t result = 0;
    if (vec_1.size() != vec_2.size())
    {
        return result;
    }
    for (size_t i = 0; i < vec_1.size(); i++)
    {
        result += vec_1[i] * vec_2[i];
    }
    return result;
}

std::vector<std::vector<double_t>> MatrixProduct(std::vector<std::vector<double_t>> matrix, std::vector<double_t> vector)
{
    int32_t height = matrix.size();
    int32_t width = vector.size();
    std::vector<std::vector<double_t>> result = std::vector<std::vector<double_t>>();
    for (size_t i = 0; i < height; i++)
    {
        for (size_t j = 0; j < width; j++)
        {
            result[i][j] = DotProduct(matrix[i], vector);
        }
    }
    return result;
}

void pid_task_1(void *arg)
{
    motor1.set_position(encoder1.getCount());
    int64_t prev_pos = motor1.get_position();
    TickType_t xTimeIncrement = configTICK_RATE_HZ * DT_S;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for (;;)
    { // loop tager mindre end 18us * 2
        double_t req_vel = 0;
        digitalWrite(PIN_PID_LOOP_1, HIGH);

        motor1.set_position(encoder1.getCount());
        motor1.set_velocity((motor1.get_position() - prev_pos) / DT_S);

        if (mode_pos)
        {
            motor1.position_pid.update(req_posx, motor1.get_position(), &ctrl_pos_1, integration_threshold);

            req_vel = constrain(ctrl_pos_1, -max_vel, max_vel);
        }

        motor1.velocity_pid.update(req_vel, motor1.get_velocity(), &ctrl_vel1, integration_threshold);

        motor1.hbridge.set_pwm(ctrl_vel1);

        prev_pos = motor1.get_position();
        digitalWrite(PIN_PID_LOOP_1, LOW);
        vTaskDelayUntil(&xLastWakeTime, xTimeIncrement);
    }
}

void pid_task_2(void *arg)
{
    motor2.set_position(encoder2.getCount());
    int64_t prev_pos = motor2.get_position();
    TickType_t xTimeIncrement = configTICK_RATE_HZ * DT_S;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for (;;)
    { // loop tager mindre end 18us * 2
        double_t req_vel = 0;
        digitalWrite(PIN_PID_LOOP_2, HIGH);

        motor2.set_position(encoder2.getCount());
        motor2.set_velocity((motor2.get_position() - prev_pos) / DT_S);

        if (mode_pos)
        {
            motor2.position_pid.update(req_posy, motor2.get_position(), &ctrl_pos_2, integration_threshold);

            req_vel = constrain(ctrl_pos_2, -max_vel, max_vel);
        }

        motor2.velocity_pid.update(req_vel, motor2.get_velocity(), &ctrl_vel2, integration_threshold);

        motor2.hbridge.set_pwm(ctrl_vel2);

        prev_pos = motor2.get_position();
        digitalWrite(PIN_PID_LOOP_2, LOW);
        vTaskDelayUntil(&xLastWakeTime, xTimeIncrement);
    }
}

void pid_task_3(void *arg)
{
    TickType_t xTimeIncrement = configTICK_RATE_HZ * DT_S;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    motor3.set_position(encoder3.getCount());
    int64_t prev_pos = motor3.get_position();
    for (;;)
    { // loop tager mindre end 18us * 2
        double_t req_vel = 0;
        digitalWrite(PIN_PID_LOOP_3, HIGH);

        motor3.set_position(encoder3.getCount());
        motor3.set_velocity((motor3.get_position() - prev_pos) / DT_S);

        if (mode_pos)
        {
            motor3.position_pid.update(req_posy, motor3.get_position(), &ctrl_pos_3, integration_threshold);

            req_vel = constrain(ctrl_pos_3, -max_vel, max_vel);
        }

        motor3.velocity_pid.update(req_vel, motor3.get_velocity(), &ctrl_vel3, integration_threshold);

        motor3.hbridge.set_pwm(ctrl_vel3);

        prev_pos = motor3.get_position();
        digitalWrite(PIN_PID_LOOP_3, LOW);
        vTaskDelayUntil(&xLastWakeTime, xTimeIncrement);
    }
}

// void pid_task(void *arg)
// {
//     TickType_t xTimeIncrement = configTICK_RATE_HZ * DT_S;
//     TickType_t xLastWakeTime = xTaskGetTickCount();
//     motor3.set_position(encoder3.getCount());
//     int64_t prev_pos = motor3.get_position();
//     for (;;)
//     { // loop tager mindre end 18us * 2
//         double_t req_vel = 0;
//         digitalWrite(PIN_PID_LOOP_3, HIGH);

//         motor3.set_position(encoder3.getCount());
//         motor3.set_velocity((motor3.get_position() - prev_pos) / DT_S);

//         if (mode_pos)
//         {
//             motor3.position_pid.update(req_posy, motor3.get_position(), &ctrl_pos_3, integration_threshold);

//             req_vel = constrain(ctrl_pos_3, -max_vel, max_vel);
//         }

//         motor3.velocity_pid.update(req_vel, motor3.get_velocity(), &ctrl_vel3, integration_threshold);

//         motor3.hbridge.set_pwm(ctrl_vel3);

//         prev_pos = motor3.get_position();
//         digitalWrite(PIN_PID_LOOP_3, LOW);
//         vTaskDelayUntil(&xLastWakeTime, xTimeIncrement);
//     }
// }

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
        pid_task_1,      /* Function to implement the task */
        "pid_task",      /* Name of the task */
        5000,            /* Stack size in words */
        NULL,            /* Task input parameter */
        3,               /* Priority of the task from 0 to 25, higher number = higher priority */
        &PidTaskHandle1, /* Task handle. */
        1);              /* Core where the task should run */
    log_i("starting pid task");
    xTaskCreatePinnedToCore(
        pid_task_2,      /* Function to implement the task */
        "pid_task2",     /* Name of the task */
        5000,            /* Stack size in words */
        NULL,            /* Task input parameter */
        3,               /* Priority of the task from 0 to 25, higher number = higher priority */
        &PidTaskHandle2, /* Task handle. */
        1);
    log_i("starting pid task");
    xTaskCreatePinnedToCore(
        pid_task_3,       /* Function to implement the task */
        "pid_task_3",     /* Name of the task */
        5000,             /* Stack size in words */
        NULL,             /* Task input parameter */
        3,                /* Priority of the task from 0 to 25, higher number = higher priority */
        &PidTaskHandle_3, /* Task handle. */
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

    pinMode(PIN_PID_LOOP_1, OUTPUT);
    pinMode(PIN_PID_LOOP_2, OUTPUT);
    pinMode(PIN_PID_LOOP_3, OUTPUT);
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