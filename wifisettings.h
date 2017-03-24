//const char* ssid     = "your_wifi_ssid";
//const char* pass = "your_wifi_password";

// custom router
const char* ssid = "otherself";
const char* pass = "theselfandtheother";


WiFiUDP Udp;                                // A UDP instance to let us send and receive packets over UDP
const IPAddress dest(192, 168, 8, 100);
const unsigned int rxport = 54321;          // remote port to receive OSC
const unsigned int txport = 12345;        // local port to listen for OSC packets (actually not used for sending)
IPAddress thisip;

