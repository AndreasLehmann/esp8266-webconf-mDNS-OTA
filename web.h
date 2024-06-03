#include <Arduino.h>
#include <FS.h>
#include <ESP8266WebServer.h>

/// HTML answer on restart request.
#define RESTART_HTML_ANSWER "<!DOCTYPE html><html><head><meta http-equiv=\"refresh\" content=\"15; URL=http://" HOSTNAME ".local/\"></head><body>Restarting in 15 seconds.<br/><img src=\"/loading.gif\"></body></html>"


/// global WiFi SSID.
extern String g_ssid;
/// global WiFi PSK.
extern String g_pass;

/// Webserver handle on port 80.
extern ESP8266WebServer g_server;

void setHostname(String);
void handleRoot(void);
void handleConfig(void);
void handleSet(void);
void drawLoading(void);