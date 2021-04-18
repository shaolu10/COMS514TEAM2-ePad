/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com
*********/

// Load Wi-Fi library
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
//#include "TaskScheduler.h"
//#include <TaskScheduler.h>

// Replace with your network credentials
const char* ssid = "NETGEAR43";
const char* password = "yellowlotus909";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// GPIO where the DS18B20 is connected to
const int oneWireBus = 4;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

String powerState = "off";
String frontState = "off";
String backState = "off";
String leftState = "off";
String rightState = "off";

// Assign output variables to GPIO pins
const int power = 2;
const int front = 33;
const int back = 32;
const int left = 34;
const int right = 35;
int val = 0;
int changeable = 0;
int poweronpage = 0;
int poweroffpage = 0;
int mainpage = 0;
int temppage = 0;
int statuspage = 0;
int positionpage = 0;
float lasttemp=0;
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;


float readTemp() {
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  float temperatureF = sensors.getTempFByIndex(0);
  Serial.println(temperatureC);
  return temperatureC;
}

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(power, OUTPUT);
  digitalWrite(power, LOW);
  pinMode(front, OUTPUT);
  digitalWrite(front, LOW);
  sensors.begin();
  lasttemp=readTemp();

  //  Sch.init();
  //  Sch.addTask(checktemp,0,1000,1);
  //  xTaskCreate(checktemp,"temp",1000,NULL,1,NULL);
  xTaskCreatePinnedToCore(checktemp, "Task1", 10000, NULL, 1, NULL,  0);
  xTaskCreatePinnedToCore(webserver, "Task2", 10000, NULL, 1, NULL,  1);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void webserver(void * pvParameters) {
  for (;;) {
    WiFiClient client = server.available();   // Listen for incoming clients

    if (client) {                             // If a new client connects,
      currentTime = millis();
      previousTime = currentTime;
      Serial.println("New Client.");          // print a message out in the serial port
      String currentLine = "";                // make a String to hold incoming data from the client
      while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
        currentTime = millis();
        if (client.available()) {             // if there's bytes to read from the client,
          char c = client.read();             // read a byte, then
          Serial.write(c);                    // print it out the serial monitor
          header += c;
          if (c == '\n') {                    // if the byte is a newline character
            // if the current line is blank, you got two newline characters in a row.
            // that's the end of the client HTTP request, so send a response:
            if (currentLine.length() == 0) {
              // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
              // and a content-type so the client knows what's coming, then a blank line:
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();

              if (header.indexOf("GET /main") >= 0) {
                Serial.println("GPIO 26 on");
                powerState = "on";
                changeable = 1;
                //                frontState = "on";
                poweronpage = 0;
                poweroffpage = 0;
                mainpage = 1;
                temppage = 0;
                statuspage = 0;
                positionpage = 0;
              } else if (header.indexOf("GET /position") >= 0) {
                poweronpage = 0;
                poweroffpage = 0;
                mainpage = 0;
                temppage = 0;
                statuspage = 0;
                positionpage = 1;
              } else if (header.indexOf("GET /status") >= 0) {
                poweronpage = 0;
                poweroffpage = 0;
                mainpage = 0;
                temppage = 0;
                statuspage = 1;
                positionpage = 0;
              } else if (header.indexOf("GET /on") >= 0) {
                powerState = "off";
                frontState = "off";
                changeable = 0;
                poweronpage = 1;
                poweroffpage = 0;
                mainpage = 0;
                temppage = 0;
                statuspage = 0;
                positionpage = 0;
              } else if (header.indexOf("GET /off") >= 0) {
                poweronpage = 0;
                poweroffpage = 1;
                mainpage = 0;
                temppage = 0;
                statuspage = 0;
                positionpage = 0;
              } else if (header.indexOf("GET /temp") >= 0) {
                poweronpage = 0;
                poweroffpage = 0;
                mainpage = 0;
                temppage = 1;
                statuspage = 0;
                positionpage = 0;
              }
              if (powerState == "on") {
                digitalWrite(power, HIGH);
              } else {
                digitalWrite(power, LOW);
              }

              if (frontState == "on") {
                digitalWrite(front, HIGH);
              } else {
                digitalWrite(front, LOW);
              }

              if (poweronpage == 1) {
                client.println("<!DOCTYPE html><html>");
                client.println("<style>p{font-family: arial;font-weight: bold; font-size: 100px;}");
                client.println(".button{background-color: #2a609a;width: 80%;height: 200px;margin: 0 auto; color: white;outline: none;font-size: 50px;border: none;cursor: pointer;}");
                client.println(".block-display button{margin-bottom:50px;display:block;}");
                client.println("@media (max-width: 768px) {.inside-page-hero {text-align: center;}}</style><body>");
                client.println("<div class=\"block-display\"><p align=\"center\"> Welcome to ePad </p></br>");
                client.println("<a href=\"/main\"><button class=\"button\">Power On</button></a>");
              } else if (poweroffpage == 1) {
                client.println("<!DOCTYPE html><html>");
                client.println("<style>topnav{width: 135px;height: 50px;background-color: black;margin: 6px 0;border: none;color: white;padding: 15px 32px;text-align: center;text-decoration: none;display: inline-block;font-size: 16px;}");
                client.println("p{font-family: arial;font-weight: bold; font-size: 100px;}");
                client.println(".button{background-color: #2a609a;width: 80%;height: 200px;margin: 0 auto; color: white;outline: none;font-size: 50px;border: none;cursor: pointer;}");
                client.println(".button1{background-color: #2a609a;width: 30%;height: 100px;margin: 0 auto; color: white;outline: none;font-size: 50px;border: none;cursor: pointer;}");
                client.println(".block-display button{margin-bottom:50px;display:block;}");
                client.println("@media (max-width: 768px) {.inside-page-hero {text-align: center;}}</style><body>");
                client.println("<a href=\"/main\"><button class=\"button1\"><- Back</button></a>");
                client.println("<div class=\"block-display\"><p align=\"center\"> Welcome to ePad </p></br>");
                client.println("<a href=\"/on\"><button class=\"button\">Power Off</button></a>");
              } else if (mainpage == 1) {
                float curtemp = 0;
                unsigned long pt= millis();
                for (;;) {
                  unsigned long ct= millis();
                  curtemp = readTemp();
                  if (curtemp == -127 || curtemp == 85) {
                  } else {
                    lasttemp=curtemp;
                    break;
                  }
                  if(ct-pt>=timeoutTime){
                    curtemp=lasttemp;
                    break;
                  }
                }
                client.println("<!DOCTYPE html><html>");
                client.println("<head><meta http-equiv=\"refresh\" content=\"3\">");
                client.println("<style>p{font-family: arial;font-weight: bold; font-size: 100px;}");
                client.println(".button{background-color: #2a609a;width: 80%;height: 200px;margin: 0 auto; color: white;outline: none;font-size: 50px;border: none;cursor: pointer;}");
                client.println(".block-display button{margin-bottom:50px;display:block;}");
                client.println("@media (min-width: 768px) {button {width: 50%;white-space: nowrap; }</style></head><body>");
                client.println("<div class=\"block-display\"><p align=\"center\"> Main Menu: Settings </p></br>");
                client.println("<p align=\"center\">current temperature " + String(curtemp) + "&#176;C</p>");
                client.println("<button class=\"button\">Set Treatment Position </button>");
                client.println("<button class=\"button\">Set Temperature </button>");
                client.println("<button class=\"button\">Status </button>");
                client.println("<a href=\"/off\"><button class=\"button\">Power</button></a>");
                Serial.println(curtemp);
              } else if (temppage == 1) {
                client.println("<p><a href=\"/position\"><button class=\"button\">ON</button></a></p>");
              } else if (positionpage == 1) {
                client.println("<p><a href=\"/status\"><button class=\"button\">ON</button></a></p>");
              } else if (statuspage == 1) {
                client.println("<p><a href=\"/off\"><button class=\"button\">ON</button></a></p>");
              } else {
              }

              client.println("</body></html>");
              // The HTTP response ends with another blank line
              client.println();
              // Break out of the while loop
              break;
            } else { // if you got a newline, then clear currentLine
              currentLine = "";
            }
          } else if (c != '\r') {  // if you got anything else but a carriage return character,
            currentLine += c;      // add it to the end of the currentLine
          }
        }
      }
      // Clear the header variable
      header = "";
      // Close the connection
      client.stop();
      Serial.println("Client disconnected.");
      Serial.println("");
    }
  }
}
void checktemp(void * parameter) {
  for (;;) {
    float curtemp = 0;
    for (;;) {
      curtemp = readTemp();
      if (curtemp == -127 || curtemp == 85) {
      } else {
        break;
      }
    }
    Serial.println("current temp is ");
    Serial.println(curtemp);
    if (curtemp == -127 || curtemp == 85) {
    } else {
      if (changeable == 1) {
        if (curtemp > 28) {
          frontState = "off";
          digitalWrite(front, LOW);
        } else {
          frontState = "on";
          digitalWrite(front, HIGH);
        }
      }
    }
    delay(10000);
  }
}
void loop() {

}
