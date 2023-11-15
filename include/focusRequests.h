//Declaring file guards

#ifndef FOCUSE_REQUESTS_H
#define FOCUSE_REQUESTS_H

// including dependencies

#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>



// Retrieve the Access Token from the database with the root user < TEST FUNCTION

String getAccessToken(){
      std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    //client->setFingerprint(fingerprint);
    // Or, if you happy to ignore the SSL certificate, then use the following line instead:
    client->setInsecure();

    HTTPClient https;
    // Brute way to make the request < Instead it can be done nice with URI builder <  TODO
    Serial.print("[HTTPS] begin...\n");
    String aT = "";
    if (https.begin(*client, "https://elmejordominiodepruebasdelahistoriadelahumanidad.shop/focusWebTest/retrieveFromDatabase.php?local=9898011")) {  // HTTPS request to server

      


      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();
      
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);
          aT = payload;
          return payload;
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
        return aT;
      }
      

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
    return aT;
}


void getCurrentColor(){

  const char* host = "https://www.googleapis.com/calendar/v3/calendars/primary/events?maxResults=1&orderBy=startTime&singleEvents=true&timeMin=2023-11-14T00:00:00Z&timeMax=2023-11-14T23:59:59Z";
  //const int httpsPort = 443;
  HTTPClient http;

  //create a wifi client instance
  WiFiClient client;


  // Replace the URL with your actual API endpoint
  http.begin(client, host);

  // Replace the token with your actual access token
  http.addHeader("Authorization", "Bearer ya29.a0AfB_byCqPxTXGhN-AKZOG2iPEJAIkA_rpf4_SGXSVxuy085DhAFW2-8q7i5CLY6FuQEKseV0UiVqPibHs7_Px-5kaYAnoGn-2ChDqwOsN0YdVrfkCxvP5ZtFVNfxPiedwj6HEbshdothXgLqqXf4pFrek9aSQbvmssgaCgYKATUSARMSFQHGX2Mi_A4w6g1fUY1F4azHvKhRbA0170");
  http.addHeader("Accept", "application/json");

  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println(httpCode);
    Serial.println(payload);
  } else {
    Serial.println("HTTP request failed");
  }

  http.end();
}

#endif