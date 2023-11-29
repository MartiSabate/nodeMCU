//Declaring file guards

#ifndef LEDS_SYNC_H
#define LEDS_SYNC_H

// including dependencies

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>


// constants definition 

#define PIN            D4  // Pin de salida al que está conectada la tira de LEDs
#define NUM_LEDS       60  // Número de LEDs en tu tira



// strip object definition

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);


// Fill the LED strip with a specific color from a hex code as an input

void setColor(uint32_t color) {
  for(int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);  // Stablish the LED color in the "i" position
  }
  strip.show();  // Update the LED strip to show the color
}



// Convert a String data type to uint32_t

uint32_t stringToUint32(String str) {
  // Convert String to const char* for strtoul function
  const char* charBuffer = str.c_str();

  // Use strtoul to convert the string to uint32_t
  uint32_t result = strtoul(charBuffer, NULL, 10);

  return result;
}

// Function to convert a hexadecimal color string to RGB
uint32_t hexStringToColor(const String& hexString) {
  // Skip the '#' character
  String hexSubstring = hexString.substring(1);
  
  // Convert the remaining hex digits to an integer
  uint32_t hexValue = strtoul(hexSubstring.c_str(), NULL, 16);

  
  // Extract red, green, and blue components
  uint8_t red = (hexValue >> 16) & 0xFF;
  uint8_t green = (hexValue >> 8) & 0xFF;
  uint8_t blue = hexValue & 0xFF;

  return strip.Color(red, green, blue);
}



#endif