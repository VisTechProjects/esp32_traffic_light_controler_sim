#define VERSION "v0.1"

// pins for relay board
#define LED_red_pin 12
#define LED_yellow_pin 14
#define LED_green_pin 27

// Traffic light blink mode cycle in milliseconds
unsigned long blinkInterval = 1000;
bool blinkAllColors = true; // Default to blinking all colors drop down menu

unsigned long distance_max = 100; // Maximum distance in cm for the distance sensor
unsigned long distance_warning = 50; // Warning distance in cm
unsigned long distance_danger = 10; // Danger distance in cm

bool distance_sensor_enabled = true; // Default to false

bool use_wifi = true; // connect to wifi or create an access point

// for connecting to a network, use_wifi must be true
const char *ssid = "36Batavia";
const char *password = "6472006991";

// or set up an access point
const char *AP_ssid = "Traffic Lights";
// const char *AP_pass = "1234578"; //uncomment to add a password