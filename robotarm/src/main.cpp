/*
 * With inspiration from:
 *
 * https://shawnhymel.com/1882/how-to-create-a-web-server-with-websockets-using-an-esp32-in-arduino/
 *
 * Author: J. Sanggaard
 * Date: 10. september 2020
 *
 */
#include "pid.h"
#include "global.h"
#include "hbridge.h"
#include <ESP32Encoder.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>

/* comment out for router connection */
#define SOFT_AP

/* Constants */
#ifdef SOFT_AP
const char *ssid = "grimerer";
const char *password = "grimgrim";
#else
const char *ssid = "grim2";
const char *password = "1234";
#endif

// Globals

const char *cmd_toggle = "toggle";
const char *cmd_led_state = "led_state";
const char *cmd_sli = "sli";
const char *cmd_pid = "pid_";

const int32_t wifi_channel = 9; // alle grupper skal have hver sin kanal
const int32_t dns_port = 53;
const int32_t http_port = 80;
const int32_t ws_port = 1337;
const int32_t led_pin = 17;

TaskHandle_t PidTaskHandle;
TaskHandle_t MotionTaskHandle;
ESP32Encoder encoder;
Pid pid_vel(DT_S, PID_MAX_CTRL_VALUE);
Pid pid_pos(DT_S, PID_MAX_CTRL_VALUE);
H_Bridge hbridge;

const double integration_threshold = 200;

volatile double req_pos;
volatile double req_vel;
volatile int64_t current_pos;
volatile double current_vel;
volatile double max_vel = 9600 / 10;

double ctrl_pos;
double ctrl_vel;

bool mode_pos = true;

AsyncWebServer Server(http_port);
WebSocketsServer WebSocket = WebSocketsServer(ws_port);
H_Bridge hBridge;
char MsgBuf[32];
int32_t LedState = 0;
int32_t SliderVal = 0;
double KpVal = 3.1415;
double KiVal = 2.71;
double KdVal = 42.0;
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
	digitalWrite(led_pin, LedState);
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
			log_d("[%u]: Slidervalue received %d", client_num, SliderVal);
		}
		else
		{
			log_e("[%u]: illegal Slidervalue received: %s", client_num, value + 1);
		}
		sprintf(MsgBuf, "%s:%d", cmd_sli, SliderVal);
		web_socket_send(MsgBuf, client_num, true);
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

	switch (subtype)
	{
	case 'p':
		parm_value = &KpVal;
		pid_pos.set_kp(*parm_value);
		break;
	case 'i':
		parm_value = &KiVal;
		pid_pos.set_ki(*parm_value);
		break;
	case 'd':
		parm_value = &KdVal;
		pid_pos.set_kd(*parm_value);
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
	int64_t prev_pos = current_pos;

	TickType_t xTimeIncrement = configTICK_RATE_HZ * pid_pos.get_dt();
	TickType_t xLastWakeTime = xTaskGetTickCount();
	for (;;)
	{ // loop tager mindre end 18us * 2
		digitalWrite(PIN_PID_LOOP, HIGH);

		current_pos = encoder.getCount();
		current_vel = (current_pos - prev_pos) / DT_S;

		if (mode_pos)
		{
			pid_pos.update(req_pos, current_pos, &ctrl_pos, integration_threshold);

			req_vel = constrain(ctrl_pos, -max_vel, max_vel);
		}

		pid_vel.update(req_vel * 10, current_vel, &ctrl_vel, 100000);

		hbridge.set_pwm(ctrl_vel);

		prev_pos = current_pos;
		digitalWrite(PIN_PID_LOOP, LOW);
		vTaskDelayUntil(&xLastWakeTime, xTimeIncrement);
	}
}

void wait_move()
{
	while (abs(req_pos - current_pos) > 10)
	{
		vTaskDelay(1);
	}
}

void home()
{
	log_v("home initiated...");
	req_vel = 40000;
	mode_pos = false;

	while (digitalRead(PIN_LIMIT_SW))
	{
		vTaskDelay(1);
	}
	req_vel = -1000;
	while (!digitalRead(PIN_LIMIT_SW))
	{
		vTaskDelay(1);
	}
	req_vel = 0;

	mode_pos = true;
	req_pos = 0;
	encoder.setCount(-14100);
	wait_move();
	log_v("home complete.");
}

void set_pos(int32_t pos)
{
	req_pos = pos;
}

void motion_task(void *arg)
{
	home();
	int32_t n = 0;
	hBridge.begin(PIN_HBRIDGE_PWM, PIN_HBRIDGE_INA, PIN_HBRIDGE_INB,
				  PWM_FREQ_HZ, PWM_RES_BITS, PWM_CH, MAX_CTRL_VALUE);

	TickType_t xTimeIncrement = configTICK_RATE_HZ / 10;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	for (;;)
	{
		log_v("motion_task running...");
		int32_t pwm_val = SliderVal;
		if (LedState == 0)
		{
			pwm_val = -pwm_val;
		}
		hBridge.set_pwm(pwm_val);
		set_pos(n * 500 * 4 * 4.8); // 1 round
		wait_move();
		vTaskDelay(5000);
		n++;
		vTaskDelayUntil(&xLastWakeTime, xTimeIncrement);
	}
	/*
	vTaskDelay(5000);
	int32_t n = 0;
	//int32_t steps_pr_deg = 1920;

	//TickType_t xTimeIncrement = configTICK_RATE_HZ/10;
	//TickType_t xLastWakeTime = xTaskGetTickCount();
	for (;;)
	{
	  log_v("motion_task...");

	  //set_pos(steps_pr_deg*(90+n*90)); // 90 deg
	  set_pos(n * 500 * 4 * 4.8); // 1 round
	  wait_move();
	  vTaskDelay(5000);
	  n++;
	  //vTaskDelayUntil( &xLastWakeTime, xTimeIncrement);
	}
	*/
}

void setup_tasks()
{
	log_i("starting motion task");
	xTaskCreatePinnedToCore(
		motion_task,
		"motion_task",
		10000, /* Stack size in words */
		NULL,  /* Task input parameter */
		2,	   /* Priority of the task from 0 to 25, higher number = higher priority */
		&MotionTaskHandle,
		1); /* Core where the task should run */
}

void setup()
{
	// disableCore0WDT();
	// disableCore1WDT();

	// Init LED and turn off
	pinMode(led_pin, OUTPUT);
	digitalWrite(led_pin, LOW);

	// Start Serial port
	Serial.begin(115200);

	pinMode(PIN_PID_LOOP, OUTPUT);
	pinMode(PIN_LIMIT_SW, INPUT);

	setup_spiffs();
	setup_network();
	setup_tasks();

	ESP32Encoder::useInternalWeakPullResistors = UP; // Enable the weak pull up resistors
	encoder.attachFullQuad(PIN_ENC_A, PIN_ENC_B);	 // Attache pins for use as encoder pins
	encoder.clearCount();

	hbridge.begin(PIN_HBRIDGE_PWM, PIN_HBRIDGE_INA, PIN_HBRIDGE_INB,
				  PWM_FREQ_HZ, PWM_RES_BITS, PWM_CH, PID_MAX_CTRL_VALUE);

	pid_pos.set_kp(20.0); // 12
	pid_pos.set_ki(5.0);
	pid_pos.set_kd(0.000);

	pid_vel.set_kp(0);
	pid_vel.set_ki(4);
	pid_vel.set_kd(0);

	log_v("starting pid task");
	xTaskCreatePinnedToCore(
		pid_task,		/* Function to implement the task */
		"pid_task",		/* Name of the task */
		3000,			/* Stack size in words */
		NULL,			/* Task input parameter */
		3,				/* Priority of the task from 0 to 25, higher number = higher priority */
		&PidTaskHandle, /* Task handle. */
		1);				/* Core where the task should run */

	log_v("starting motion task");
	xTaskCreatePinnedToCore(
		motion_task,
		"motion_task",
		3000, /* Stack size in words */
		NULL, /* Task input parameter */
		2,	  /* Priority of the task from 0 to 25, higher number = higher priority */
		&MotionTaskHandle,
		0); /* Core where the task should run */
}

void loop()
{
	// Look for and handle WebSocket data
	WebSocket.loop();

	Serial.printf(
		"req_pos: %.2f  curr_pos: %.2f  ctrl_pos: %.2f  set_vel: %.2f  curr_vel: %.2f ctrl_vel: %.2f\n\r",
		req_pos, (double)current_pos, ctrl_pos, req_vel, current_vel, ctrl_vel);

	/*
	int32_t pid_stack_min =  uxTaskGetStackHighWaterMark(PidTaskHandle);
	int32_t motion_stack_min = uxTaskGetStackHighWaterMark(MotionTaskHandle);
	log_i("min stack avaliable [bytes], pid: %d,  motion: %d", pid_stack_min, motion_stack_min);
	*/

	vTaskDelay(0.2 * configTICK_RATE_HZ);
}
