#include <ESP32Encoder.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <math.h>
#include <string>
#include <Arduino.h>
// #include <motor.h>
#include <global.h>
#include <pid.h>
#include <hbridge.h>

/* comment out for router connection */
#define SOFT_AP

/* Constants */
#ifdef SOFT_AP
const char *ssid = "grim";
const char *password = "grimgrim";
#else
const char *ssid = "grim2";
const char *password = "1234";
#endif

class Motor
{
private:
  ESP32Encoder encoder;
  H_Bridge hBridge;
  int64_t current_pos;
  int64_t current_vel;

  Motor(double delta_time_seconds, double PID_MAX_CTRL_VALUE);
  ~Motor();
};

Motor::Motor(double delta_time_seconds, double PID_MAX_CTRL_VALUE)
{
  Pid position_pid(delta_time_seconds, PID_MAX_CTRL_VALUE);
  Pid velocity_pid(delta_time_seconds, PID_MAX_CTRL_VALUE);
}

Motor::~Motor()
{
}

// Globals

const char *cmd_toggle = "toggle";
const char *cmd_led_state = "led_state";
const char *cmd_sli = "sli";
const char *cmd_pid = "pid_";
const char *cmd_pos = "req_pos";
const char *cmd_vel = "req_vel";

const int32_t wifi_channel = 9; // alle grupper skal have hver sin kanal
const int32_t dns_port = 53;
const int32_t http_port = 80;
const int32_t ws_port = 1337;

TaskHandle_t PidTaskHandle1;
TaskHandle_t PidTaskHandle2;
TaskHandle_t PidTaskHandle3;
TaskHandle_t PositionTaskHandle;
ESP32Encoder encoder1;
ESP32Encoder encoder2;
ESP32Encoder encoder3;
Pid pid_pos_1(DT_S, PID_MAX_CTRL_VALUE);
Pid pid_pos_2(DT_S, PID_MAX_CTRL_VALUE);
Pid pid_pos_3(DT_S, PID_MAX_CTRL_VALUE);
Pid pid_vel1(DT_S, PID_MAX_CTRL_VALUE);
Pid pid_vel2(DT_S, PID_MAX_CTRL_VALUE);
Pid pid_vel3(DT_S, PID_MAX_CTRL_VALUE);
H_Bridge hBridge1;
H_Bridge hBridge2;
H_Bridge hBridge3;

// Motor motor1(DT_S, PID_MAX_CTRL_VALUE);

const double integration_threshold = 200;

volatile double req_posx;
volatile double req_posy;
volatile double dest_posx;
volatile double dest_posy;
volatile double req_vel1;
volatile double req_vel2;
volatile double req_vel3;
volatile int64_t current_pos_1;
volatile int64_t current_pos_2;
volatile int64_t current_pos_3;
volatile double current_vel1;
volatile double current_vel2;
volatile double current_vel3;
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

AsyncWebServer Server(http_port);
WebSocketsServer WebSocket = WebSocketsServer(ws_port);
char MsgBuf[32];
int32_t LedState = 0;
int32_t SliderVal = 0;

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

void handle_slider(char *command, uint8_t client_num)
{
  char *value = strstr(command, ":");

  if (value == NULL || *value != ':')
  {
    log_e("[%u]: Bad command %s", client_num, command);
    return;
  }

  if (*(value + 1) == '?')
  {
    sprintf(MsgBuf, "%s:%d", cmd_sli, SliderVal);
    web_socket_send(MsgBuf, client_num, false);
  }
  else
  {
    errno = 0;
    char *e;
    int32_t result = strtol(value + 1, &e, 10);
    if (*e == '\0' && 0 == errno) // no error
    {
      SliderVal = result;
      log_i("[%u]: Slidervalue received %d", client_num, SliderVal);
    }
    else
    {
      log_e("[%u]: illegal Slidervalue received: %s", client_num, value + 1);
    }
    sprintf(MsgBuf, "%s:%d", cmd_sli, SliderVal);
    web_socket_send(MsgBuf, client_num, true);
  }
}

void set_pos(double posx, double posy)
{
  req_posx = posx;
  req_posy = posy;
}

void setDestination(double posx, double posy)
{
  dest_posx = posx;
  dest_posy = posy;
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
    sprintf(MsgBuf, "%s:%d", cmd_pos, encoder1.getCount());
    web_socket_send(MsgBuf, client_num, false);
  }
  else
  {
    char *e;
    std::string val = value;
    std::string command = command;
    std::string result1 = command.substr(val.size(), command.find(","));
    std::string result2 = command.substr(command.find(","), command.size());
    Serial.printf("%s, %s\n", result1, result2);
    if (*e == '\0' && 0 == errno) // no error
    {
      setDestination(0, 0);
      sprintf(MsgBuf, "%s:%f", cmd_pos, encoder1.getCount());
      web_socket_send(MsgBuf, client_num, true);
    }
    else
    {
      log_e("[%u]: illegal Slidervalue received: %s", client_num, value + 1);
    }
    Serial.println(encoder1.getCount());
    Serial.println(encoder2.getCount());
    // sprintf(MsgBuf, "%s:%d", cmd_pos, &req_pos);
    // web_socket_send(MsgBuf, client_num, true);
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
    sprintf(MsgBuf, "%s:%d", cmd_vel, max_vel);
    web_socket_send(MsgBuf, client_num, false);
  }
  else
  {
    errno = 0;
    char *e;
    double result = strtol(value + 1, &e, 10);
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
    pid_pos_1.set_kp(*parm_value);
    pid_pos_2.set_kp(*parm_value);
    break;
  case 'i':
    parm_value = &result;
    pid_pos_1.set_ki(*parm_value);
    pid_pos_2.set_ki(*parm_value);
    break;
  case 'd':
    parm_value = &result;
    pid_pos_1.set_kd(*parm_value);
    pid_pos_2.set_kd(*parm_value);
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

void handle_command(uint8_t client_num, uint8_t *payload, size_t length)
{
  char *command = (char *)payload;

  log_d("[%u] Received text: %s", client_num, command);

  if (strcmp(command, cmd_toggle) == 0)
    handle_toggle(client_num); // Toggle LED
  else if (strncmp(command, cmd_led_state, strlen(cmd_led_state)) == 0)
    handle_led_state(command, client_num); // Report the state of the LED
  else if (strncmp(command, cmd_sli, strlen(cmd_sli)) == 0)
    handle_slider(command, client_num); // slider
  else if (strncmp(command, cmd_pid, strlen(cmd_pid)) == 0)
    handle_kx(command, client_num); // pid params
  else if (strncmp(command, cmd_pos, strlen(cmd_pos)) == 0)
    handle_pos_req(command, client_num); // position request
  else if (strncmp(command, cmd_vel, strlen(cmd_vel)) == 0)
    handle_vel_req(command, client_num); // velocity request
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
  handleRequest(request, "/robot.css", "text/css");
}

// Callback: send style sheet
void onCSSRequestW3(AsyncWebServerRequest *request)
{
  handleRequest(request, "/w3.css", "text/css");
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
#ifdef SOFT_AP
  // Start access point
  WiFi.softAP(ssid, password, wifi_channel); // (alle grupper skal bruge en unik kanal)
  // Print our IP address

  log_i("AP running");
  log_i("My IP address: ");
  log_i("IP: %s", WiFi.softAPIP().toString().c_str());

#else
  // connect to local network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    log_i("Establishing connection to WiFi..");
  }
  log_i("Connected to network");
  log_i("IP: %s", WiFi.localIP().toString().c_str());
#endif

  // On HTTP request for root, provide index.html file
  Server.on("/", HTTP_GET, onIndexRequest);

  // On HTTP request for style sheet, provide robot.css
  Server.on("/robot.css", HTTP_GET, onCSSRequest);
  Server.on("/w3.css", HTTP_GET, onCSSRequestW3);

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
  int64_t prev_pos1 = current_pos_1;
  TickType_t xTimeIncrement = configTICK_RATE_HZ * pid_pos_1.get_dt();
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;)
  { // loop tager mindre end 18us * 2
    digitalWrite(PIN_PID_LOOP_1, HIGH);

    current_pos_1 = encoder1.getCount();
    current_vel1 = (current_pos_1 - prev_pos1) / DT_S;

    if (mode_pos)
    {
      pid_pos_1.update(req_posx, current_pos_1, &ctrl_pos_1, integration_threshold);

      req_vel1 = constrain(ctrl_pos_1, -max_vel, max_vel);
    }

    pid_vel1.update(req_vel1, current_vel1, &ctrl_vel1, integration_threshold);

    hBridge1.set_pwm(ctrl_vel1);

    prev_pos1 = current_pos_1;
    digitalWrite(PIN_PID_LOOP_1, LOW);
    vTaskDelayUntil(&xLastWakeTime, xTimeIncrement);
  }
}

void pid_task2(void *arg)
{
  int64_t prev_pos2 = current_pos_2;
  TickType_t xTimeIncrement = configTICK_RATE_HZ * pid_pos_2.get_dt();
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;)
  { // loop tager mindre end 18us * 2
    digitalWrite(PIN_PID_LOOP_2, HIGH);

    current_pos_2 = encoder2.getCount();
    current_vel2 = (current_pos_2 - prev_pos2) / DT_S;

    if (mode_pos)
    {
      pid_pos_2.update(req_posy, current_pos_2, &ctrl_pos_2, integration_threshold);

      req_vel2 = constrain(ctrl_pos_2, -max_vel, max_vel);
    }

    pid_vel2.update(req_vel2, current_vel2, &ctrl_vel2, integration_threshold);

    hBridge2.set_pwm(ctrl_vel2);

    prev_pos2 = current_pos_2;
    digitalWrite(PIN_PID_LOOP_2, LOW);
    vTaskDelayUntil(&xLastWakeTime, xTimeIncrement);
  }
}

void pid_task3(void *arg)
{
  int64_t prev_pos3 = current_pos_3;
  TickType_t xTimeIncrement = configTICK_RATE_HZ * pid_pos_3.get_dt();
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;)
  { // loop tager mindre end 18us * 2
    digitalWrite(PIN_PID_LOOP_3, HIGH);

    current_pos_3 = encoder3.getCount();
    current_vel3 = (current_pos_3 - prev_pos3) / DT_S;

    if (mode_pos)
    {
      pid_pos_3.update(req_posy, current_pos_3, &ctrl_pos_3, integration_threshold);

      req_vel3 = constrain(ctrl_pos_3, -max_vel, max_vel);
    }

    pid_vel2.update(req_vel3, current_vel3, &ctrl_vel3, integration_threshold);

    hBridge2.set_pwm(ctrl_vel3);

    prev_pos3 = current_pos_3;
    digitalWrite(PIN_PID_LOOP_3, LOW);
    vTaskDelayUntil(&xLastWakeTime, xTimeIncrement);
  }
}

void position_task(void *arg)
{
  for (;;)
  {
    int64_t prev_pos1 = encoder1.getCount();
    int64_t prev_pos2 = encoder2.getCount();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    double_t L1 = ((encoder1.getCount() - prev_pos1) / 1920) * (r / 50) * M_PI;
    double_t L2 = ((encoder2.getCount() - prev_pos2) / 1920) * (r / 50) * M_PI;
    if (L2 - L1 == 0)
    {
      device_x += L1 * cos(device_rotation);
      device_y += L1 * sin(device_rotation);
    }
    else
    {
      device_x += (cos(device_rotation + (L2 - L1) / b) - cos(device_rotation)) * b * (L1 + L2) / (2 * (L2 - L1));
      device_y += (sin(device_rotation + (L2 - L1) / b) - sin(device_rotation)) * b * (L1 + L2) / (2 * (L2 - L1));
    }
    device_rotation += (L2 - L1) / b;
    needed_rotation = atan2(dest_posx - device_x, dest_posy - device_y);
    double_t v = (device_rotation - needed_rotation) / (2 * M_PI);
    if (v >= 1)
    {
      v -= 1;
    }
    else if (v < 0)
    {
      v += 1;
    }
    Serial.printf("v: %f, L1: %f, L2: %f", v, L1, L2);
    if (min(v, 1 - v) < 0.05)
    {
      double_t l = sqrt(pow(device_x - dest_posx, 2) + pow(device_y - dest_posy, 2)) * (50 / r * M_PI) * 1920;
      set_pos(encoder1.getCount() + l, encoder2.getCount() + l);
    }
    else if (v < 0.5)
    {
      double_t l = v * b * (50 / r) * 1920;
      set_pos(encoder1.getCount() + l, encoder2.getCount() - l);
    }
    else
    {
      double_t l = (1 - v) * b * (50 / r) * 1920;
      set_pos(encoder1.getCount() + l, encoder2.getCount() - l);
    }
  }
}

void setup_tasks()
{
  log_i("starting pid task");
  xTaskCreatePinnedToCore(
      pid_task,        /* Function to implement the task */
      "pid_task",      /* Name of the task */
      5000,            /* Stack size in words */
      NULL,            /* Task input parameter */
      3,               /* Priority of the task from 0 to 25, higher number = higher priority */
      &PidTaskHandle1, /* Task handle. */
      1);              /* Core where the task should run */
  log_i("starting pid task");
  xTaskCreatePinnedToCore(
      pid_task2,       /* Function to implement the task */
      "pid_task2",     /* Name of the task */
      5000,            /* Stack size in words */
      NULL,            /* Task input parameter */
      3,               /* Priority of the task from 0 to 25, higher number = higher priority */
      &PidTaskHandle2, /* Task handle. */
      1);
  log_i("starting motion task");
  xTaskCreatePinnedToCore(
      position_task,       /* Function to implement the task */
      "position_task",     /* Name of the task */
      5000,                /* Stack size in words */
      NULL,                /* Task input parameter */
      3,                   /* Priority of the task from 0 to 25, higher number = higher priority */
      &PositionTaskHandle, /* Task handle. */
      0);
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
  // pinMode(PIN_LIMIT_SW, INPUT);

  // Motor motor1 = Motor(PIN_ENC_A, PIN_ENC_B);
  // Motor motor2 = Motor(PIN_ENC_A_2, PIN_ENC_B_2);

  ESP32Encoder::useInternalWeakPullResistors = UP;   // Enable the weak pull up resistors
  encoder1.attachFullQuad(PIN_ENC_A_1, PIN_ENC_B_1); // Attache pins for use as encoder pins
  encoder2.attachFullQuad(PIN_ENC_A_2, PIN_ENC_B_2); // Attache pins for use as encoder pins
  encoder1.clearCount();
  encoder2.clearCount();

  hBridge1.begin(PIN_HBRIDGE_PWM_1, PIN_HBRIDGE_INA_1, PIN_HBRIDGE_INB_1,
                 PWM_FREQ_HZ, PWM_RES_BITS, PWM_CH_1, PID_MAX_CTRL_VALUE);
  hBridge2.begin(PIN_HBRIDGE_PWM_2, PIN_HBRIDGE_INA_2, PIN_HBRIDGE_INB_2,
                 PWM_FREQ_HZ, PWM_RES_BITS, PWM_CH_2, PID_MAX_CTRL_VALUE);

  pid_pos_1.set_kp(10.0); // 12
  pid_pos_1.set_ki(2.0);
  pid_pos_1.set_kd(0.000);

  pid_pos_2.set_kp(10.0); // 12
  pid_pos_2.set_ki(2.0);
  pid_pos_2.set_kd(0.000);

  pid_vel1.set_kp(10);
  pid_vel1.set_ki(12);
  pid_vel1.set_kd(0);

  pid_vel2.set_kp(10);
  pid_vel2.set_ki(12);
  pid_vel2.set_kd(0);

  setup_spiffs();
  setup_network();
  setup_tasks();
}

void loop()
{
  // Look for and handle WebSocket data
  WebSocket.loop();
  Serial.printf(
      "req_pos_1: %.2f  req_pos_2: %.2f  \ncurr_pos_1: %.2f  curr_pos_2: %.2f  \nctrl_pos_1: %.2f  ctrl_pos_2: %.2f  \nreq_vel_1: %.2f  req_vel_2: %.2f  curr_vel_1: %.2f curr_vel_2: %.2f ctrl_vel_1: %.2f ctrl_vel_2: %.2f\n\r",
      req_posx, req_posy, (double)current_pos_1, (double)current_pos_2, ctrl_pos_1, ctrl_pos_2, req_vel1, req_vel2, current_vel1, current_vel2, ctrl_vel1, ctrl_vel2);
  Serial.printf("kp: %f, ki: %f, kd: %f, error: %f\n", pid_pos_1.get_kp(), pid_pos_1.get_ki(), pid_pos_1.get_kd(), pid_pos_1.get_error());
  Serial.printf("kp: %f, ki: %f, kd: %f, error: %f\n", pid_pos_2.get_kp(), pid_pos_2.get_ki(), pid_pos_2.get_kd(), pid_pos_2.get_error());
  Serial.printf("kp: %f, ki: %f, kd: %f, error: %f\n", pid_vel1.get_kp(), pid_vel1.get_ki(), pid_vel1.get_kd(), pid_vel1.get_error());
  Serial.printf("kp: %f, ki: %f, kd: %f, error: %f\n", pid_vel2.get_kp(), pid_vel2.get_ki(), pid_vel2.get_kd(), pid_vel2.get_error());
  Serial.printf("needed_rot: %f, device_rot: %f, device_x: %f, device_y: %f\n", needed_rotation, device_rotation, device_x, device_y);
  setDestination(100, 0);

  vTaskDelay(0.2 * configTICK_RATE_HZ);
}