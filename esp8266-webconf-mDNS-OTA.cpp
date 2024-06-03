/**
   @file esp8266-webconf-mDNS-OTA.ino

   @author Pascal Gollor (http://www.pgollor.de/cms/), A.Lehmann 06/2024
   @data 2015-08-17

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

#include <ArduinoOTA.h>

#include "motor.h"
#include "config.h"
#include "web.h"


/**
   @brief mDNS and OTA Constants
   @{
*/
#define HOSTNAME "MARKISE-" ///< Hostename
#define APORT 8266 ///< OTA Port
/// @}

/**
   @brief Default WiFi connection information.
*/
const char* ap_default_ssid = "MARKISE-AP"; ///< Default SSID.
const char* ap_default_psk = "MarkisE"; ///< Default PSK.

/// Uncomment the next line for verbose output over UART.
//#define SERIAL_VERBOSE

/// Restart of the esp8266 will be triggert on this time
unsigned long g_restartTime = 0;

/**########################################################################
   @brief Arduino setup function.
*/
void setup()
{
  g_ssid = "";
  g_pass = "";

  initMotorPins();


  Serial.begin(115200);
  delay(100);

  #ifdef SERIAL_VERBOSE
    Serial.println("\r\n");
    Serial.print("Chip ID: 0x");
    Serial.println(ESP.getChipId(), HEX);
  #endif

  // Set Hostname.
  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);
  WiFi.hostname(hostname);

  // set hostname for Web.cpp
  setHostname(hostname);

  // Print hostname.
  Serial.println("Hostname: " + hostname);

  #ifdef SERIAL_VERBOSE
    Serial.println(WiFi.hostname());
  #endif

  // Initialize file system.
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount file system");
    return;
  }

  // Load wifi connection information.
  if (! loadConfig(&g_ssid, &g_pass))
  {
    g_ssid = "";
    g_pass = "";
    Serial.println("No WiFi connection information available.");
  }

  // Check WiFi connection
  // ... check mode
  if (WiFi.getMode() != WIFI_STA)
  {
    WiFi.mode(WIFI_STA);
    delay(10);
  }

  // ... Compare file config with sdk config.
  if (WiFi.SSID() != g_ssid || WiFi.psk() != g_pass)
  {
    Serial.println("WiFi config changed.");
    // ... Try to connect to WiFi station.
    WiFi.begin(g_ssid.c_str(), g_pass.c_str());
    // ... Pritn new SSID
    Serial.print("new SSID: ");
    Serial.println(WiFi.SSID());

    // ... Uncomment this for debugging output.
    //WiFi.printDiag(Serial);
  }
  else
  {
    // ... Begin with sdk config.
    WiFi.begin();
  }

  Serial.println("Wait for WiFi connection.");

  // ... Give ESP 10 seconds to connect to station.
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000)
  {
    Serial.write('.');
    //Serial.print(WiFi.status());
    delay(500);
  }
  Serial.println();

  // Check connection
  if (WiFi.status() == WL_CONNECTED)
  {
    // ... print IP Address
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("Can not connect to WiFi station. Go into AP mode.");

    // Go into AP mode.
    WiFi.mode(WIFI_AP);

    delay(10);

    WiFi.softAP(ap_default_ssid, ap_default_psk);

    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
  }

  // Set port
  ArduinoOTA.setPort(APORT);

  // set hostname
  ArduinoOTA.setHostname(hostname.c_str());

  // start OTA Server
  ArduinoOTA.begin();

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  // Initialize web server.
  // ... Add requests.
  g_server.on("/", handleRoot); /* Einstiegsseite */
  g_server.on("/config", handleConfig); /* Konfigurationsseite mit RESET*/
  g_server.on("/set", HTTP_GET, handleSet); /* Setze WIFI Passwort*/
  g_server.on("/restart", []() {
    g_server.send(200, "text/html", RESTART_HTML_ANSWER);
    g_restartTime = millis() + 100;
  } );
  g_server.serveStatic("/loading.gif", SPIFFS, "/loading.gif");

  // ... Start server.
  g_server.begin();

  motorStopTimer.attach(0.5, checkRunningMotors );
}

/**
   @brief Arduino loop function.
*/
void loop()
{
  // Handle OTA server.
  ArduinoOTA.handle();
  yield();

  // Handle Webserver requests.
  g_server.handleClient();

  // trigger restart?
  if (g_restartTime > 0 && millis() >= g_restartTime)
  {
    g_restartTime = 0;
    ESP.restart();
  }
}

