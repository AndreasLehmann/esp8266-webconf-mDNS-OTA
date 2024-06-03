
#include "web.h"
#include "config.h"
#include "motor.h"


/// global WiFi SSID.
String g_ssid = "";
/// global WiFi PSK.
String g_pass = "";

/// global WiFi PSK.
String hostname = "";

ESP8266WebServer g_server(80);


/** #########################################################################
   @brief setHostname
*/
void setHostname(String h)
{
  hostname = h;
}

/** #########################################################################
   @brief Handle http root request.

   URI:  http://<HOSTNAME>/?M2=R   schaltet Motor 2 in rechtslauf
*/
void handleRoot()
{
  String indexHTML;

  // Check for motor commands
  int motor = 0;
  if (g_server.argName(0) == "M1")
  {
    motor = 1;
  }
  else if (g_server.argName(0) == "M2")
  {
    motor = 2;
  }
  else if (g_server.argName(0) == "M3")
  {
    motor = 3;
  }

  // is there any command?
  if ( motor > 0 )
  {
    int dir = 0; // default=stop
    if (g_server.arg(0) == "L")
    {
      dir = 1;
    }
    else if (g_server.arg(0) == "R")
    {
      dir = 2;
    }
    setMotor (motor, dir);
  }

  File indexFile = SPIFFS.open("/index.html", "r");
  if (indexFile)
  {
    indexHTML = indexFile.readString();
    indexFile.close();

    char buffer[5];
    itoa(motorStatus[0], buffer, 10);
    indexHTML.replace("[M1_Status]", buffer);
    itoa(motorStatus[1], buffer, 10);
    indexHTML.replace("[M2_Status]", buffer);
    itoa(motorStatus[2], buffer, 10);
    indexHTML.replace("[M3_Status]", buffer);

  }
  else
  {
    indexHTML = "<!DOCTYPE html><html><head><title>File not found</title></head><body><h1>File not found.</h1></body></html>";
  }

  g_server.send (200, "text/html", indexHTML);
} // handleRoot


/** #########################################################################
   @brief Handle http config request.
*/
void handleConfig()
{
  String indexHTML;
  char buff[10];
  uint16_t s = millis() / 1000;
  uint16_t m = s / 60;
  uint8_t h = m / 60;

  File indexFile = SPIFFS.open("/config.html", "r");
  if (indexFile)
  {
    indexHTML = indexFile.readString();
    indexFile.close();

    snprintf(buff, 10, "%02d:%02d:%02d", h, m % 60, s % 60);

    // replace placeholder
    indexHTML.replace("[esp8266]", String(ESP.getChipId(), HEX));
    indexHTML.replace("[rssi]", String(WiFi.RSSI()));
    indexHTML.replace("[ssid]", g_ssid);
    indexHTML.replace("[pass]", g_pass);
    indexHTML.replace("[uptime]", buff);

    //Serial.println(g_indexHTML);
  }
  else
  {
    indexHTML = "<!DOCTYPE html><html><head><title>File not found</title></head><body><h1>File not found.</h1></body></html>";
  }

  g_server.send (200, "text/html", indexHTML);
} // handleConfig

/** #########################################################################################################
   @brief Handle set request from http server.

   URI: /set?ssid=[WiFi SSID],pass=[WiFi Pass]
*/
void handleSet()
{
  String response = "<!DOCTYPE html><html><head><meta http-equiv=\"refresh\" content=\"2; URL=http://";
  response += hostname;
  response += ".local\"></head><body>";

  // Some debug output
  Serial.print("uri: ");
  Serial.println(g_server.uri());

  Serial.print("method: ");
  Serial.println(g_server.method());

  Serial.print("args: ");
  Serial.println(g_server.args());

  // Check arguments
  if (g_server.args() < 2)
  {
    g_server.send (200, "text/plain", "Arguments fail.");
    return;
  }

  String ssid = "";
  String pass = "";

  // read ssid and psk
  for (uint8_t i = 0; i < g_server.args(); i++)
  {
    if (g_server.argName(i) == "ssid")
    {
      ssid = g_server.arg(i);
    }
    else if (g_server.argName(i) == "pass")
    {
      pass = g_server.arg(i);
    }
  }

  // check ssid and psk
  if (ssid != "" && pass != "")
  {
    // save ssid and psk to file
    if (saveConfig(&ssid, &pass))
    {
      // store SSID and PSK into global variables.
      g_ssid = ssid;
      g_pass = pass;

      response += "Successfull.";
    }
    else
    {
      response += "<h1>Fail save to config file.</h1>";
    }
  }
  else
  {
    response += "<h1>Wrong arguments.</h1>";
  }

  response += "</body></html>";
  g_server.send (200, "text/html", response);
} // handleSet


/**
   @brief Load loading.gif from filesytem and draw.
*/
void drawLoading()
{
  File gif = SPIFFS.open("/loading.gif", "r");
  if (gif)
  {
    g_server.send(200, "image/gif", gif.readString());
    gif.close();
  }
  else
  {
    g_server.send(404, "text/plain", "Gif not found.");
  }
} // drawLoading

