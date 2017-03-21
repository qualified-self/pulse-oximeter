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
  const IPAddress outIp(192, 168, 8, 100);
  const unsigned int outPort = 12345;          // remote port to receive OSC
  const unsigned int localPort = 54321;        // local port to listen for OSC packets (actually not used for sending)
  IPAddress thisip;
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
Thresholder peakDetector(STDDEV_THRESHOLD, THRESHOLD_HIGH);
//Thresholder peakDetector(STDDEV_THRESHOLD, THRESHOLD_RISING, 0.0f);

// LED output.
DigitalOut led(LED_PIN);


bool beatDispatched = false;

float lastreading, thisreading;
float IBI = 0;
unsigned long lastbeat = 0;
unsigned long now = 0;

int sensor_id = -1;

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
  thisip = WiFi.localIP();
  Serial.println( thisip );

  // sensor ID is the last byte in the IP quad
  sensor_id = thisip[3];
#endif
}

void dispatch_beat() {
#ifdef __BUILD_FEATHER__
    OSCMessage out("/beat");
    out.add(sensor_id);
    out.add(IBI);
    
    Udp.beginPacket(outIp, outPort);
    out.send(Udp);
    Udp.endPacket();
#elif
    Serial.println("beat");
#endif
}

void step() {
  // process input.
  in >> normalizer;
  normalizer >> peakDetector; // >> led;
  thisreading = normalizer;

  Serial.print(thisreading);
  Serial.print(",");
  Serial.print(STDDEV_THRESHOLD);
  Serial.print(",");
 
  if (peakDetector) {
    now = millis();
    IBI = abs(now - lastbeat); //2 + (abs(now - lastbeat) / 1000); // IBI in seconds
    lastbeat = now;
    Serial.print(3.0);

    // send the beat osc message only once
    if(beatDispatched == false) {
      dispatch_beat();
      beatDispatched = true;
    }

  } else {
    Serial.print(.0);
    // make sure we dispatch the next beat
    beatDispatched = false;
  }

//  Serial.print(",");
//  Serial.print(IBI);

  Serial.println();
  delay(20);
}

