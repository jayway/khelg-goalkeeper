#include <Arduino.h>
#include <ESP8266WiFi.h> // Include the Wi-Fi library
#include "Reporter.h"    //

#define PIN_STATUS D2       // Status led on PCB
#define IR_LED D3           // IR led
#define PIN_DETECT1 D4      // Infrared receiver ( IR photo module TSOP1738, TSOP34838)
#define PIN_FEEDBACK_LED D1 // Green led to indicate goal is detected

const char *ssid = "<WIFI SSID>";         // The SSID (name) of the Wi-Fi network you want to connect to
const char *password = "<WIFI PASSWORD>"; // The password of the Wi-Fi network
String team = "blue";                     // Change this to either red or blue depending on deployment target

const char *host = "api.foosball.link";
const char *fingerprint = "90 3C A1 1A 37 EA 31 C0 B1 8D BC 8E 1B 0D 60 12 43 C2 81 42";

//Create a new Reporter
Reporter reporter(host, "/", fingerprint);

// Minimum time in ms that the IR needs to be broken for the code to report
// is as a goal. This is to avoid goals being reported for flake variations.
const int minActiveTime = 2;

void blinkFeedback(int count)
{
  for (int i = 0; i < count; i++)
  {
    digitalWrite(PIN_FEEDBACK_LED, HIGH);
    delay(100);
    digitalWrite(PIN_FEEDBACK_LED, LOW);
    delay(100);
  }
}

void notifyGoal()
{
  digitalWrite(PIN_FEEDBACK_LED, HIGH);
  Serial.println("Registering goal");
  String postData = "{\"query\":\"mutation registerGoal($team: TeamColor!) {registerGoal(team: $team)}\",\"variables\":{\"team\":\"" + team + "\"}}";
  reporter.postMessageToServer(postData);
  digitalWrite(PIN_FEEDBACK_LED, LOW);
  delay(100);
  blinkFeedback(3);
}

void setup()
{
  Serial.begin(28800);
  delay(1000);
  Serial.println('\n');

  WiFi.begin(ssid, password); // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i);
    Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP()); // Send the IP address of the ESP8266 to the computer

  pinMode(PIN_STATUS, OUTPUT);
  pinMode(PIN_DETECT1, INPUT);
  pinMode(PIN_FEEDBACK_LED, OUTPUT);
  digitalWrite(PIN_FEEDBACK_LED, LOW);
  tone(IR_LED, 38000U);
  Serial.print("Setup done");
}

unsigned long highStart = 0;
bool hasNotified = true;

void loop()
{
  if (digitalRead(PIN_DETECT1) == HIGH)
  {
    if (highStart && !hasNotified)
    {
      unsigned long delta = millis() - highStart;
      if (delta > minActiveTime)
      {
        hasNotified = true;
        notifyGoal();
      }
    }
    else
    {
      highStart = millis();
    }
    digitalWrite(PIN_STATUS, LOW);
  }
  else
  {
    digitalWrite(PIN_STATUS, HIGH);
    highStart = 0;
    hasNotified = false;
  }
}
