#define LED_red_pin 16
#define LED_yellow_pin 17
#define LED_green_pin 18

// Traffic light blink mode cycle in milliseconds
unsigned long blinkInterval = 1000;

bool use_wifi = false; // connect to wifi or create an access point

// for connecting to a network, use_wifi must be true
const char *ssid = "36Batavia";
const char *password = "6472006991";

// or set up an access point
const char *AP_ssid = "Traffic Lights";
// const char *AP_pass = "1234578"; //uncomment me to add a password