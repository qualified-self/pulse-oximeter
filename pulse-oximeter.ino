// build settings change between featherboard and arduino nano
#define __BUILD_FEATHER__ // to switch to Arduino #undef this line...
#undef __BUILD_NANO__ // ... and #define this one

#ifdef __BUILD_FEATHER__
  #include <ESP8266WiFi.h>
  #include <WiFiUdp.h>
  #include <OSCMessage.h>
#endif

#include <Plaquette.h>
#include <PqExtra.h>

#ifdef __BUILD_FEATHER__
  #include "wifisettings.h"
  
  WiFiUDP Udp;                                // A UDP instance to let us send and receive packets over UDP
  //const IPAddress outIp(192, 168, 1, 75);
  const IPAddress outIp(192, 168, 8, 205);
  const unsigned int outPort = 12345;          // remote port to receive OSC
  const unsigned int localPort = 54321;        // local port to listen for OSC packets (actually not used for sending)
#endif

#define INPUT_PIN A0 // change as needed
#define STDDEV_THRESHOLD 0.9 //.0
#define LED_PIN 2

// Sensor input.
AnalogIn in(INPUT_PIN);

// Normalizer to N(0, 1).
AdaptiveNormalizer normalizer(0, 1);
//Normalizer normalizer(0, 1);

// Detects rising signals.
//Thresholder peakDetector(STDDEV_THRESHOLD, THRESHOLD_HIGH);
Thresholder peakDetector(STDDEV_THRESHOLD, THRESHOLD_RISING, 0.0f);

// LED output.
DigitalOut led(LED_PIN);

// Serial output.
StreamOut sout;

float IBI = 0;
unsigned long lastbeat = 0;
unsigned long now = 0;

void begin() {
  Serial.begin(115200);

#ifdef __BUILD_FEATHER__  
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
#endif
}

void step() {
  // Process input.
  in >> normalizer >> peakDetector >> led;

  if (peakDetector) {
    now = millis();
    IBI = abs(now - lastbeat); //2 + (abs(now - lastbeat) / 1000); // IBI in seconds
    lastbeat = now;

#ifdef __BUILD_FEATHER__
    OSCMessage out("/beat");
    out.add(IBI);
    
    Udp.beginPacket(outIp, outPort);
    out.send(Udp);
    Udp.endPacket();
#endif
  } 

//  Serial.print(",");
//  Serial.print(IBI);
//  Serial.println();
//  delay(20);
}

