// build settings change between featherboard and arduino nano
#define __BUILD_FEATHER__ // to switch to Arduino #undef this line...
#undef __BUILD_NANO__ // ... and #define this one

#include <Metro.h>
#include "config.h"

#ifdef __BUILD_FEATHER__
  #include <ESP8266WiFi.h>
  #include <WiFiUdp.h>
  #include <OSCMessage.h>
#endif

#include <Plaquette.h>
#include <PqExtra.h>

#ifdef __BUILD_FEATHER__
  #include "wifisettings.h"
#endif


Metro alive = Metro(ALIVE_ACK_MS);

#define INPUT_PIN A0 // change as needed
#define STDDEV_THRESHOLD 0.9 //.0
#define LED_PIN 0 // built-in LED in featherboard is on pin 0

// Sensor input.
AnalogIn in(INPUT_PIN);

// Normalizer to N(0, 1).
AdaptiveNormalizer normalizer(0, 1);
//Normalizer normalizer(0, 1);

// Detects rising signals.
Thresholder peakDetector(STDDEV_THRESHOLD, THRESHOLD_HIGH);
//Thresholder peakDetector(STDDEV_THRESHOLD, THRESHOLD_RISING, 0.0f);

// LED output.
//DigitalOut led(LED_PIN);


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

  //led << 0;
#endif
}

void dispatch_beat() {
#ifdef __BUILD_FEATHER__
    OSCMessage out("/oximeter/beat");
    out.add(sensor_id);
    out.add(IBI);
    
    Udp.beginPacket(dest, txport);
    out.send(Udp);
    Udp.endPacket();
#endif
    //Serial.println("beat");
    beatDispatched = true;
}

void state_loop() {

  // send alive ACK message to show-control
  if(alive.check()) {
    _LOG("-> ACK");
    OSCMessage out("/oximeter/ack");
    out.add( sensor_id );
    Udp.beginPacket(dest, txport);
    out.send(Udp);
    Udp.endPacket();
    out.empty();
  }

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
    if(!beatDispatched) {
      dispatch_beat();
    }

  } else {
    Serial.print(.0);
    // make sure we dispatch the next beat
    beatDispatched = false;
  }

  state_loop();

//  Serial.print(",");
//  Serial.print(IBI);

  Serial.println();
  delay(20);
}

