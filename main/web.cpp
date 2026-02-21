#include "config.h"
#include "globals.h"

const char* htmlPage =
"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
"<!DOCTYPE html><html><head><meta charset='UTF-8'>"
"<meta name='viewport' content='width=device-width, initial-scale=1'>"
"<style>"
".btn{padding:15px;margin:5px;width:100px;cursor:pointer;border:none;border-radius:5px;color:white;font-weight:bold;}"
".u{background:#555}.d{background:#555}.s{background:#f44336}"
".c_music{background:#E91E63;width:80%;display:block;margin:10px auto;}"
".c_exit{background:#D32F2F;width:80%;display:block;margin:10px auto;border:2px solid #fff;}"
"body{font-family:sans-serif;text-align:center;margin-top:20px;background:#f0f0f0}"
"</style>"
"</head>"
"<body>"
"<h1>Living Room Control</h1>"
"<h3>Blinds</h3>"
"<a href='/UP'><button class='btn u'>UP</button></a>"
"<a href='/STOP'><button class='btn s'>STOP</button></a>"
"<a href='/DOWN'><button class='btn d'>DOWN</button></a>"
"<hr>"
"<h3>Modes</h3>"
"<a href='/MUSIC'><button class='btn c_music'>MUSIC MODE</button></a>"
"<a href='/EXIT_MUSIC'><button class='btn c_exit'>EXIT MUSIC (RED)</button></a>"
"</body></html>";

void checkWebClient() {

  WiFiClient client = webServer.available();
  if (!client) return;

  String req = "";
  unsigned long start = millis();

  while (client.connected() && millis() - start <= 200) {

    if (client.available()) {

      char c = client.read();

      if (c == '\n') {

        if (req.startsWith("GET")) {

          if (req.indexOf("/UP") >= 0) blindTarget = 100;
          else if (req.indexOf("/DOWN") >= 0) blindTarget = 0;
          else if (req.indexOf("/STOP") >= 0) blindTarget = blindPos;
          else if (req.indexOf("/MUSIC") >= 0) {
            isMusicMode = true;
            lightPower = true;
          }
          else if (req.indexOf("/EXIT_MUSIC") >= 0) {
            isMusicMode = false;
            targetR = 255; targetG = 0; targetB = 0;
            setLEDs(255, 0, 0);
            lightPower = true;

            syncLightPower(true);
            syncLightColor(255, 0, 0);
          }

          client.print(htmlPage);
          break;
        }

        req = "";

      } else if (c != '\r') {
        req += c;
      }
    }
  }

  client.stop();
}