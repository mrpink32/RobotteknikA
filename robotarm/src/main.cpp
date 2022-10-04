#include <Arduino.h>
#include <WiFi.h>
#include <fstream>
#include <iostream>
#include <vector>

char ssid[] = "BottleFlipperNET";
char password[] = "mikkelfp";
const int PORT = 9000;
WiFiServer server(PORT);
TaskHandle_t Task1;

void html_server(void *pvParameters)
{
  for (;;)
  {
    WiFiClient client = server.available();

    if (client)
    {
      Serial.println("New client!");
      std::ifstream infile;
      infile.open("Index.html");
      std::vector<char> data;
      client.printf("HTTP/1.1 200 OK\r\nContent-Length: {%d}\r\n\r\n{%d}", data.size(), data);
      // while (client.connected())
      // {

      // }
      client.stop();
      infile.close();
      Serial.println("Client disconnected: " + String(client.connected()));
    }
    delay(1000);
  }
}

void setup()
{
  Serial.begin(112500);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password, 7);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  server.begin();
  Serial.println("Setup done!");
  xTaskCreatePinnedToCore(
      html_server, /* Task function. */
      "Task1",     /* name of task. */
      10000,       /* Stack size of task */
      NULL,        /* parameter of the task */
      1,           /* priority of the task */
      &Task1,      /* Task handle to keep track of created task */
      0);          /* pin task to core 1 */
}

void loop()
{
  // put your main code here, to run repeatedly:
}