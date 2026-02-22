#include "config.h"
#include "globals.h"

// Helper to generate dynamic HTML with the current timer and mic status
void sendHtmlResponse(WiFiClient &client) {
  client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
  client.print("<!DOCTYPE html><html><head><meta charset='UTF-8'>");
  client.print("<meta name='viewport' content='width=device-width, initial-scale=1'>");
  client.print("<style>");
  client.print(".btn{padding:15px;margin:5px;width:100px;cursor:pointer;border:none;border-radius:5px;color:white;font-weight:bold;}");
  client.print(".u{background:#555}.d{background:#555}.s{background:#f44336}");
  client.print(".c_music{background:#E91E63;width:80%;display:block;margin:10px auto;}");
  client.print(".c_exit{background:#D32F2F;width:80%;display:block;margin:10px auto;border:2px solid #fff;}");
  client.print("body{font-family:sans-serif;text-align:center;margin-top:20px;background:#f0f0f0}");
  client.print(".timer{font-size:1.1em;color:#333;margin:10px;padding:15px;background:#fff;display:inline-block;border-radius:8px;border:1px solid #ccc;box-shadow: 0 2px 4px rgba(0,0,0,0.1);}");
  client.print(".status-line{margin-top: 8px; font-size: 0.9em;}");
  client.print("</style></head><body>");
  
  client.print("<h1>Living Room Control</h1>");

  // Status Section
  client.print("<div class='timer'>");
  if (isMusicMode) {
    client.print("Auto-Off: <b>Paused (Music)</b>");
  } else if (!lightPower) {
    client.print("Auto-Off: <b>Lights Off</b>");
  } else {
    client.print("Auto-Off in: <b>" + String(minutesRemaining) + " min</b>");
    client.print("<div class='status-line'>Mic Status: <b>" + lastDetectionStatus + "</b></div>");
  }
  client.print("</div>");

  client.print("<h3>Blinds</h3>");
  client.print("<a href='/UP'><button class='btn u'>UP</button></a>");
  client.print("<a href='/STOP'><button class='btn s'>STOP</button></a>");
  client.print("<a href='/DOWN'><button class='btn d'>DOWN</button></a>");
  client.print("<hr>");
  client.print("<h3>Modes</h3>");
  client.print("<a href='/MUSIC'><button class='btn c_music'>MUSIC MODE</button></a>");
  client.print("<a href='/EXIT_MUSIC'><button class='btn c_exit'>EXIT MUSIC (RED)</button></a>");
  client.print("</body></html>");
}

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
          // Blinds Logic
          if (req.indexOf("/UP") >= 0) blindTarget = 100;
          else if (req.indexOf("/DOWN") >= 0) blindTarget = 0;
          else if (req.indexOf("/STOP") >= 0) blindTarget = blindPos;
          
          // Music Logic
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

          sendHtmlResponse(client);
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