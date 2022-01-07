/* Dawn Simulator by Wolf I. Butler
 * 
 * Uses 10 WS2811 LEDs to simulate a gentle dawn, driven by a D1 Mini (ESP8266) Node MCU
 * This is an experiment! May need more LEDs and different color pattern.
 *
 * Hacked together from several "samples" found online.
 * 
 */

#include "ESP8266WiFi.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>

// Replace with your network credentials
const char* ssid = "WIFI_Network";
const char* password = "WIFI_Password";

//Set these to your needs
int gmtOffset = -5;       //Offset from GMT. This will need to be changed for DST
String startHour = "05";  //Hour to start the Dawn Simulation. Right now it only supports top-of-the-hour.

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedTime;
String currHour;

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    2

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 10

//Initialize LED Strip
//Using the recommended NEO_KHZ400 here, but the 800KHz option should also work.
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ400);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

void setup() {
  // Initialize Serial Monitor
  // This is really just used for debugging/timing. Not actually needed in production.
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

// Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(gmtOffset * 3600);
  
  strip.begin();            // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.clear();            // Turn OFF all pixels ASAP
  strip.setBrightness(50);  // Set initial BRIGHTNESS
}

// Default is true- display a quick "Dawn" demo on boot. You can disable it if you want...
boolean demoFlag = true;

// These set the delay between illuminating consecutive pixels.
// This provides a more-gradual change in the light level.
// You can tweak these if-desired, but they will change the duration of the whole process.
int demoDelay = 10;      //Demo pixel change milliseconds
int runDelay = 1500;    //Run pixel change miliseconds

// You shouldn't need to change anything below. These just set starting values.
// These are set in the Demo section below as-well, but need to be set outside of the main loop
// In case the Demo is bypassed by demoFlag = false above.

boolean runFlag = false;
int brightVal = 50;
int brightInc = 0;
int redVal = 1;
int redInc = 1;
int greenVal = 1;
int greenInc = 1;
int blueVal = 1;
int blueInc = 1;
int loopCount = 1;
char* prtString = "";

// In the loop below- you can "tweak" increment values and color levels.
// I set these to what works for me. Any changes will affect how long the "Dawn" lasts.
// As originally written- this runs for 65 minutes, and then max brightness is held for 30 minutes.

void loop() {

  while(demoFlag) {
    sprintf(prtString, "\nL-%d : B-%d : R-%d : G-%d : B-%d", loopCount, brightVal, redVal, greenVal, blueVal);
    Serial.print(prtString);
    colorWipe(strip.Color(redVal, greenVal, blueVal), brightVal, demoDelay);
    redVal = redVal + redInc;
    greenVal = greenVal + greenInc;
    blueVal = blueVal + blueInc;
    brightVal = brightVal + brightInc;
    //Ramp all colors up at first until they all hit100 (Pre-Dawn)
    //Increase Red and brightness. (Sun on horizon.)
    if(redVal == 100) {
      redInc = 5;
      greenInc = 0;
      blueInc = 0;
      brightInc = 1;
    }
    //Increase Green and speed up brightness increase once Red hits max. (Sunrise < 50%)
    if(redVal > 254) {
      redVal = 255;
      redInc = 0;
      greenInc = 1;
      brightInc = 5;
    }
    //Continue to increase green once max. overall brightness is reached. (Sunrise > 50%)
    if(brightVal > 254) {
      brightVal = 255;
      brightInc = 0;
      greenInc = 2;
    }
    //Finally- increase blue until we reach "White". (Sun above horizon.)
    if(greenVal > 254) {
      greenVal = 255;
      greenInc = 0;
      blueInc = 5;
    }
    //Once at maximum white brightness, hold and then reset.
    if(blueVal > 254) {
      delay(5000);
      colorWipe(strip.Color(0, 0, 0), 255, demoDelay*10);
      //Reset for actual main run...
      brightVal = 50;
      brightInc = 0;
      redVal = 1;
      redInc = 1;
      greenVal = 1;
      greenInc = 1;
      blueVal = 1;
      blueInc = 1;
      loopCount = 0;      
      demoFlag = false;
    }
    loopCount ++;
  }

  if(!runFlag) {
    timeClient.update();
    formattedTime = timeClient.getFormattedTime();
    currHour = formattedTime.substring(0, 2);
    Serial.print("\nTime: "+formattedTime+" Waiting for "+startHour+":00:00");
    if(currHour == startHour) {
      runFlag = true;
    } else {
      strip.clear(); // Insures all pixels stay off.
      delay(60000); //For our purposes- only need to check the time every minute.
    }
  }
  
  while(runFlag) {  
    timeClient.update();
    formattedTime = timeClient.getFormattedTime();
    sprintf(prtString, "\nL-%d : B-%d : R-%d : G-%d : B-%d \n", loopCount, brightVal, redVal, greenVal, blueVal);
    Serial.print(formattedTime+prtString);
    //This routine runes for 65 minutes (+30 minute hold time at the end). 
    colorWipe(strip.Color(redVal, greenVal, blueVal), brightVal, runDelay);
    redVal = redVal + redInc;
    greenVal = greenVal + greenInc;
    blueVal = blueVal + blueInc;
    brightVal = brightVal + brightInc;
    if(redVal == 100) {
      redInc = 5;
      greenInc = 0;
      blueInc = 0;
      brightInc = 1;
    }
    if(redVal > 254) {
      redVal = 255;
      redInc = 0;
      greenInc = 1;
      brightInc = 5;
    }
    if(brightVal > 254) {
      brightVal = 255;
      brightInc = 0;
      greenInc = 2;
    }
    if(greenVal > 254) {
      greenVal = 255;
      greenInc = 0;
      blueInc = 5;
    }
    if(blueVal > 254) {
      delay(1800000);   //Hold for 30 minutes, then reset.
      colorWipe(strip.Color(0, 0, 0), 255, runDelay);
      redVal = 1;
      redInc = 1;
      greenVal = 1;
      greenInc = 1;
      blueVal = 1;
      blueInc = 1;
      brightVal = 50;
      loopCount = 0;      
      runFlag = false;
    }
    loopCount ++;
  }  
}

void colorWipe(uint32_t color, int bright, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
  strip.setBrightness(bright);
}
