#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ESP8266WiFi.h> // a library to include
#include <ESPAsyncE131.h> // a library to include https://github.com/forkineye/E131

IRsend irsend(4);  // IR LED Pin as per our circuit.

//Set your WiFi network information below...
const char* ssid = "My_WIFI_SSID"; // your network AP SSID
const char* password = "My_WIFI_Password"; // your network AP password

//Set the universe and channel you want to use. Universe is E1.31 (DMX) universe.
//Channel will generally be 0, which is actually 1 (Red) in DMX. Use 1 for Green, and 2 for Blue.
const int UNIVERSE = 1; // universe to listen to
const int UNIVERSE_COUNT = 1; //Universe count
const int CHANNEL = 20 ; // channel to use

//Projector state. Don't change.
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
         Serial.println(F("Listening for data..."));
     else 
         Serial.println(F("*** e131.begin failed ***"));

  //Attempt to ensure that the projector is in a known state (OFF) when started:
  irsend.sendNEC(0xFF15EA); //If off, turn on. If on, does nothing because of the following delay...
  delay(15000); //15 second delay. Enough for projector to start-up if it was off.
  //Now turn off...
  irsend.sendNEC(0xFF15EA);
  delay(2000);  //2 second delay
  irsend.sendNEC(0xFF15EA);
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
