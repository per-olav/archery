// For board "ESP32 Dev Module" or similar

#include <HardwareSerial.h>
#include <WiFi.h>

#define RX_PIN 16
#define TX_PIN 17

HardwareSerial serial2(2);

// Set web server port number to 80
WiFiServer server(80);

// String to store the HTTP request
String request;

const char* ssid     = "Archer Counter";
const char* password = "123456789";

#define DOCTYPE "<!DOCTYPE html>"
#define HEAD "<head>" \
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">" \
  "<link rel=\"icon\" href=\"data:,\">" \
  "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}" \
  ".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;" \
  "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}" \
  ".numbutton { background-color:#ff0000; border: none; color: white; padding: 4pt 4pt;" \
  "text-decoration: none; font-size: 16px; margin: 2px; cursor: pointer;}" \
  "</style>" \
  "</head>"

#define START_BUTTON "<a href=\"start\"><button class=\"button\">START</button></a>"
#define STOP_BUTTON "<a href=\"stop\"><button class=\"button\">STOP</button></a>"

#define ARROWS_BUTTONS "<a href=\"A1\"><button class=\"numbutton\">1</button></a>&nbsp &nbsp &nbsp" \
  "<a href=\"A2\"><button class=\"numbutton\">2</button></a>&nbsp &nbsp &nbsp" \
  "<a href=\"A3\"><button class=\"numbutton\">3</button></a>&nbsp &nbsp &nbsp" \
  "<a href=\"A4\"><button class=\"numbutton\">4</button></a>&nbsp &nbsp &nbsp" \
  "<a href=\"A5\"><button class=\"numbutton\">5</button></a>&nbsp &nbsp &nbsp" \
  "<a href=\"A6\"><button class=\"numbutton\">6</button></a>"

void setFixedIP() {
  IPAddress ip(10, 10, 10, 10);
  IPAddress nMask(255, 255, 255, 0);
  WiFi.softAPConfig(ip, ip, nMask);
  Serial.println(WiFi.softAPIP());
}

void wifiEventHandler(WiFiEvent_t event) {
  Serial.print("Wifi event: ");
  Serial.println(event);
  switch (event) {
    case SYSTEM_EVENT_AP_START:
      Serial.println("Access point starts");
      Serial.println("Set fixed IP address");
      setFixedIP();
      break;
  }
}

void setup() {
  Serial.begin(115200);
  serial2.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  // Delete old wifi config
  WiFi.disconnect(true);
  // Registering wifi event
  delay(1000);

  WiFi.onEvent(wifiEventHandler);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  server.begin();
}

void loop() {
  WiFiClient client = server.available(); // Wait for incoming client
  if (client) {
    request = "";
    if (fillrequest(client)) {
      Serial.println("First line of request: " + request); // Prints only until first new-line in request

      handleRequest(client, request);
    }
    client.stop();
  }
}

boolean fillrequest(WiFiClient client) {
  boolean requestOk = false;
  char c1 = '\0';
  char c2 = '\0';
  char c3 = '\0';
  char c4 = '\0';
  bool endOfLine = false;
  bool endOfRequest = false;
  // Get incoming data
  bool isInrequest = true;
  while (client.connected() && !requestOk) {
    if (client.available()) {
      c4 = c3;
      c3 = c2;
      c2 = c1;
      c1 = client.read();
      endOfLine = c1 == '\n';
      isInrequest = isInrequest && !endOfLine;
      if (isInrequest) {
        request += c1;
      }
      requestOk = c4 == '\r' && c3 == '\n' && c2 == '\r' && c1 == '\n';
    }
  }
  if (requestOk) {
    Serial.println("request end reached");
  } else {
    Serial.println("request not ended properly");
  }

  return requestOk;
}

void handleRequest(WiFiClient client, String request) {
  int p1 = request.indexOf(" /");
  int p2 = request.indexOf("HTTP");
  String requestUrlData = request.substring(p1 + 2, p2 - 1);
  Serial.print("Request data: ");
  Serial.println(requestUrlData);

  if (requestUrlData.equals("start")) {
    Serial.println("*** Start ***");
    sendStart();
  } else if (requestUrlData.equals("stop")) {
    Serial.println("*** Stop ***");
    sendStop();
  } else if (requestUrlData.startsWith("A")) {
    Serial.print("Set number of arrows to ");
    int numOfArrows = requestUrlData.charAt(1) - '0';
    numOfArrows = max(0, numOfArrows);
    numOfArrows = min(6, numOfArrows);
    Serial.println(numOfArrows);
    sendTime(numOfArrows * 40);
  }

  printResponse(client, requestUrlData);
}

void sendMessageCom(String message) {
  Serial.print("Sending message over serial com: ");
  Serial.println(message);
  serial2.print(message);
  serial2.print('\0');
}

void printResponse(WiFiClient client, String requestUrlData) {
  client.println(DOCTYPE);
  client.println("<html>");
  client.println(HEAD);
  client.println("<body>");
  client.println("<h1>Archery counter </h1>");
  client.println("<p>");
  client.println(ARROWS_BUTTONS);
  client.println("</p>");

  client.println("<p>&nbsp</p>");
  client.println("<p>&nbsp</p>");

  client.println("<p>");

  client.println(START_BUTTON);
  client.println(STOP_BUTTON);
  client.println("</p>");

  client.println("</body></html>");
}

void sendTime(int timeSeconds) {
  char msg[6];
  msg[0] = 'T';
  msg[1] = ' ';
  sprintf(&msg[2], "%d", timeSeconds);
  msg[5] = '\0';
  sendMessageCom(msg);
  delay(500);
}

void sendStart() {
  sendMessageCom("S 000");
}

void sendStop() {
  sendMessageCom("I"); // Interrupt
}

int min(int x, int y) {
  if (x < y) {
    return x;
  } else {
    return y;
  }
}

int max(int x, int y) {
  if (x > y) {
    return x;
  } else {
    return y;
  }
}
