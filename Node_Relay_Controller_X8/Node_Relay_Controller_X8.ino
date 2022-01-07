/* Modification of old E1.31 single-channel relay controller
 * Supports 8 channels from the starting universe selected.
 * By Wolf Butler
 */

#include <ESP8266WiFi.h> // a library to include
#include <E131.h> // a library to include https://github.com/forkineye/E131

//Set your WiFi network information below...
//const char* ssid = "WIFI_Network"; // your network AP SSID
//const char* password = "WIFI_Password"; // your network AP password
const char* ssid = "WIFI_Network"; // your network AP SSID
const char* password = "WIFI_Password"; // your network AP password


// Comment the following out if using DHCP, otherwise- adjust as-needed.
// Important: Those are commas, not periods!!! This is just the way the IPAddress datatype works.
//const IPAddress ip(10,0,0,251);
//const IPAddress netmask(255,255,255,0);
//const IPAddress gateway( 10,0,0,1);
//const IPAddress dns(8,8,8,8);

//See below to set Multicast or Unicast. Default is unicast (TCP).

//Set the universe and START channel you want to use. Universe is E1.31 (DMX) universe.
//Generally you will just use 1-8, so just leave the default of 1.
const int universe = 1; // universe to listen to

//Generally you want consecutive channels starting at 0 (which will be 1 in the controller/control software).
//This MUST match the number of outputs below or bad things will happen.
int channels[] = { 20, 21, 22, 23, 24, 25, 26, 27 };

//Set the output pins here. These are the GPIO pins, and not necessearily "D" pins.
//For example- for the D1 Mini, pin GPIO 5 is "D1". We would enter pin 5 in this case.
//In short- "D" numbers don't correspond to GPIO numbers.

//Example: For D1, D2, D3, D4, D5, D6, D7, D8 on the D1 Mini, use..
//int outputs[] = { 5, 4, 0, 2, 14, 12, 13, 15 }

int outputs[] = { 5, 4, 0, 2, 14, 12, 13, 15 };

//Don't mess with these...
int channel_val; // the current value of the channel we are using, don't change this.
long int num_ch_in_pkt; // number of channels in the recieved packet, don't change this.
E131 e131;
int cval[8]; //Used for formatted output to serial terminal

void setup() {
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  //If you want to use DHCP instead of a fixed IP address (above), comment the following line out...
//  WiFi.config(ip, dns, gateway, netmask);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    //Loop (forever) until WiFi is connected.
    delay(500);
    Serial.print(".");
  }

  //Initialize the GPIOs and set the relays to off (default- Low)
  for ( int i = 0; i < 8; i++ ) {
    pinMode(outputs[i], OUTPUT);
    digitalWrite(outputs[i], LOW);
  }
 
  // **** Pick one. You can either use Unicast or Multicast. If you don't know which- pick Unicast. ****
//  e131.begin(E131_UNICAST, universe);   //Unicast (TCP)
  e131.begin(E131_MULTICAST, universe); //Multicast (UDP)

  Serial.println("Waiting for channel data...");
}

//Don't change anything below...
//Any DMX value under 127 is "Off", above 127 is "On".

void loop() {
    /* Parse a packet */
    uint16_t num_channels = e131.parsePacket();
    
    /* Process channel data if we have it */
    if (num_channels) {
      Serial.printf("\nUniverse %u / %u Channels | Packet#: %u / Errors: %u\n",
      e131.universe,              // The Universe for this packet
      num_channels,               // Number of channels in this packet
      e131.stats.num_packets,     // Packet counter
      e131.stats.packet_errors);   // Packet error counter
      for ( int i = 0; i < 8; i++ ) {
        channel_val = e131.data[channels[i]];
        cval[i] = channel_val;
        if (channel_val > 127) {
          digitalWrite(outputs[i], HIGH); // SSR on
        }
        else {
          digitalWrite(outputs[i], LOW); // SSR off
        }
      }
      Serial.printf("GPIO:Channel Values:\t%u:%u\t%u:%u\t%u:%u\t%u:%u\t%u:%u\t%u:%u\t%u:%u\t%u:%u\n",
      outputs[0], cval[0], outputs[1], cval[1], outputs[2], cval[2], outputs[3], cval[3],
      outputs[4], cval[4], outputs[5], cval[5], outputs[6], cval[6], outputs[7], cval[7]);
    }
}
