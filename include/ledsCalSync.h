//Declaring file guards

#ifndef LEDS_SYNC_H
#define LEDS_SYNC_H

// including dependencies

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>


// constants definition 

#define PIN            D4  // Pin de salida al que está conectada la tira de LEDs
#define NUM_LEDS       60  // Número de LEDs en tu tira

//Define the Google Calendar color ID conversion
struct ColorMapping {
    int colorId;
    uint32_t hexadecimal;
};

const ColorMapping colorMappings[] = {
    {1,  0x008080},
    {2,  0x00FF00},
    {3,  0xFF00FF},
    {4,  0x800000},
    {5,  0xFFFF00},
    {6,  0xFFB3AE},
    {7,  0x00FFFF},
    {8,  0x000000},
    {9,  0x000080},
    {10, 0x008000},
    {11, 0xFF0000}
};

// strip object definition

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);


// Fill the LED strip with a specific color from a hex code as an input

void setColor(uint32_t color) {
  for(int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);  // Stablish the LED color in the "i" position
  }
  strip.show();  // Update the LED strip to show the color
}


// Function to extract "colorId" from JSON response
void extractColorId(String jsonResponse) {


  // Create a JSON buffer
  StaticJsonDocument<2000> doc;

  // Deserialize the JSON data
  DeserializationError error = deserializeJson(doc, jsonResponse);

  // Check for parsing errors
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }


  /*
  // Extract the value of "colorId"
  const char* colorId = doc["items"][0]["colorId"];
  */
  const char* colorId = doc["items"][0]["colorId"];

  // Check if the "items" array exists and has at least one item
  if (doc.containsKey("items") && doc["items"].is<JsonArray>() && doc["items"].size() > 0) {
    // Check if the "colorId" key exists in the first item
    if (doc["items"][0].containsKey("colorId")) {
      // Extract the value of "colorId"

      // Print the extracted value
      Serial.print("Color ID: ");
      Serial.println(colorId);
      int color = atoi(colorId);
      uint32_t hexColorForID11 = colorMappings[color-1].hexadecimal; // Index 10 corresponds to color ID 11
      strip.begin();  // Inicializar la tira de LEDs
      strip.show();   // Apagar todos los LEDs al principio
      setColor(hexColorForID11);

    } else {
      Serial.print("Color ID: ");
      Serial.println("1");
    }
  } else {
    Serial.println("No items array in the JSON response.");
  }


}




#endif