#define VERSION "0.1"

// pins for relay board
#define LED_red_pin 12
#define LED_yellow_pin 14
#define LED_green_pin 27

unsigned long blinkInterval = 1000;        // Traffic light blink mode interval in milliseconds
const unsigned long dangerHoldTime = 3000; // How long you must stay in the danger zone before flashing (3s)

#define DISTANCE_SENSOR_ENABLED // programticly enable distance sensor

// for connecting to wifi, uncomment both
#define WIFI_SSID "36Batavia"
#define WIFI_PASS "6472006991"

// or set up an access point, // use_wifi must be commented
#define AP_SSID "Traffic Lights"
// #define AP_PASS "1234578" //uncomment to add a password