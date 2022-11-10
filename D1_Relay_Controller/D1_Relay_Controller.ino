
/* D1 Mini/ESP-12 (16-bit) E1.31 Relay Control
 * by Wolf I. Butler, Updated August, 2022
 */

#include <ESP8266WiFi.h> // a library to include
#include <E131.h>; // a library to include https://github.com/forkineye/E131

//Set your WiFi network information below...
const char* ssid = "CatLady"; // your network AP SSID
const char* password = "BigOrangeCat!"; // your network AP password

/* Set the universe and channel you want to use. Universe is E1.31 (DMX) universe.
 * As noted: You want to subtract 1 from you xLights channel number. This is because the E131 library 
 * starts channels at 0. instead of 1. So- if you use Channel 12 in xLights, set it to 11 below...
 */
const int universe = 1; // universe to listen to
const int channel = 14 ; // channel to use - these are 1 lower than normal (0 = 1)

/*  This script defaults to DHCP and Multicast.
 *  Be sure to read through ALL of the comments below if you want to use a fixed IP
 *  and/or switch to unicast. You must change the WiFi settings and E1.31 connect string!
 */

// Un-comment the following if not using DHCP, and edit accordingly.
// Important: Those are commas, not periods!!! This is just the way the IPAddress datatype works.
//const IPAddress ip(10,0,0,245);
//const IPAddress netmask(255,255,255,0);
//const IPAddress gateway(10,0,0,1);
//const IPAddress dns(8,8,8,8);

//See below to set Multicast or Unicast. Default is multicast!

int channel_val; // the current value of the channel we are using, don't change this.
long int num_ch_in_pkt; // number of channels in the recieved packet, don't change this.
int output_1 = 5; // names the ESP8266 GPIO output pin we are using (5 is D1 !!!), don't change this for Relay Shield.

E131 e131;
void setup() {
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // ***** If you want to use a fixed IP address instead of DHCP, un-comment the following...
  //WiFi.config(ip, dns, gateway, netmask);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    //Loop (forever) until WiFi is connected.
    delay(500);
    Serial.print(".");
  }
  
  //Set output pin and set it low (off).
  //If you want the relay to turned on at boot- set this to HIGH.
  pinMode(output_1, OUTPUT); // set pin as an output
  digitalWrite(output_1, LOW); // set pin to LOW

  // ***** Pick one. You can either use Unicast or Multicast. If you don't know which- pick Multicast. ****
//  e131.begin(E131_UNICAST, universe);   //Unicast (TCP)
  e131.begin(E131_MULTICAST, universe); //Multicast (UDP)
}

// ***** Don't change anything below...
//Any DMX value under 127 is "Off", above 127 is "On".
void loop() {
  num_ch_in_pkt = e131.parsePacket(); // parse packet
  if (num_ch_in_pkt) {                // if num_ch_in_pkt > 0
    channel_val = (e131.data[channel]);
    Serial.println(channel_val);
    if (channel_val > 127) {
      digitalWrite(output_1, HIGH); // SSR on
      Serial.println("Power On!");
    }
    else {
      digitalWrite(output_1, LOW); // SSR off
      Serial.println("Power Off!");
    }
  }
}
