/* DMX_Projector_Power
 * IR control for popular Vankyo projectors 
 * Works with ESP, ESP32 devides like D1 Mini and ESP-32 DevKit.
 * by Wolf I. Butler, Updated January, 2021
 *
 */
#include <IRremoteESP8266.h>
#include <IRsend.h>
//#include <WiFi.h>           // Use this for ESP32. 
#include <ESP8266WiFi.h>    // Use this for ESP8266
#include <ESPAsyncE131.h>   // E1.31 Library https://github.com/forkineye/E131

IRsend irsend(4);  // IR LED Pin as per our circuit. Change this to the GPIO pin you are using.

//Set your WiFi network information below...
//const char* ssid = "Your_WIFI_SSID"; // your network AP SSID
//const char* password = "Your_WIFI_Password"; // your network AP password
const char* ssid = "WIFI_Network"; // your network AP SSID
const char* password = "WIFI_Password"; // your network AP password
//const char* ssid = "WIFI_Network"; // your network AP SSID
//const char* password = "WIFI_Password"; // your network AP password

//Set the E1.31 (DMX) Universe and channel you want to use.
const int UNIVERSE = 1;       // Universe to listen to.
const int UNIVERSE_COUNT = 1; // Universe count. Just leave at 1.

//Set the channel you want this to listen to...
const int CHANNEL = 25 ;      // E1.31 Channel to listen for.

//Projector state flag. Don't change.
bool isProjectorOn = false;

// ESPAsyncE131 instance with UNIVERSE_COUNT buffer slots
ESPAsyncE131 e131(UNIVERSE_COUNT);

void setup() {
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  irsend.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    //Loop (forever) until WiFi is connected.
    delay(500);
    Serial.print(".");
  }
  
  if (e131.begin(E131_MULTICAST, UNIVERSE, UNIVERSE_COUNT))   // Listen via Multicast
         Serial.println(F("Connected to WIFI..."));
     else 
         Serial.println(F("*** e131.begin failed ***"));

  //It takes several seconds after power on before the projector will respond to IR commands...
  delay(15000); //15 second delay. Enough time for projector to be ready for IR commands.

  //Attempt to ensure that the projector is in a known state (OFF) when started:
  Serial.println(F("Resetting Projector. Please wait..."));
  Serial.println(F("Powering Projector on..."));  
  irsend.sendNEC(0xFF15EA); //If off, turn on. If on, does nothing because of the following...  
  delay(2000);  //2 second delay
  irsend.sendNEC(0xFF59A6); //Down button. Cancels initial power command if projector was on.
  Serial.println(F("Waiting for Projector start-up..."));    
  delay(15000); //15 second delay. Enough time to warm up if it was just started.
  Serial.println(F("Turning Projector off..."));  
  irsend.sendNEC(0xFF15EA); //Power...
  delay(2000);  //2 second delay
  irsend.sendNEC(0xFF15EA); //Off.
  Serial.println(F("Online, listening for E1.31 data..."));
}

//Don't change anything below...
//Any DMX value under 127 is "Off", above 127 is "On".
//Because the power function is only a toggle- using isProjectorON flag to try to insure this works right.

void loop() {
  if (!e131.isEmpty()) {
    e131_packet_t packet;
    e131.pull(&packet);     // Pull packet from ring buffer  
    Serial.println(packet.property_values[CHANNEL]);
    if (packet.property_values[CHANNEL] > 127) {
      if (!isProjectorOn) {
        irsend.sendNEC(0xFF15EA);
        delay(2000);
        irsend.sendNEC(0xFF59A6); //Down button. Cancels initial power power-off dialog if projector was already on.  
        isProjectorOn = true;
        Serial.println("Projector On!");
      }
    }
    else {
      if (isProjectorOn) {
        irsend.sendNEC(0xFF15EA);
        delay(2000);
        irsend.sendNEC(0xFF15EA);
        isProjectorOn = false;
        Serial.println("Projector Off!");
      }
    }
  }
}
