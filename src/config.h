#define LED_red_pin 12
#define LED_yellow_pin 14
#define LED_green_pin 27

// Traffic light blink mode cycle in milliseconds
unsigned long blinkInterval = 1000;

bool use_wifi = false; // connect to wifi or create an access point

// for connecting to a network, use_wifi must be true
const char *ssid = "wifi";
const char *password = "pass";

// or set up an access point
const char *AP_ssid = "Traffic Lights";
// const char *AP_pass = "1234578"; //uncomment me to add a password