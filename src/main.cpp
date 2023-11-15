//ESTE ES EL FICHERO CON EL POC PARA MOSTRAR LAS REDES WIFI EN UN SERVIDOR WEB, EVITANDO USAR WIFI MANAGER

#include <ESP8266WebServer.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <LittleFS.h>


// Include local defined libraries

#include <ledsCalSync.h>
#include <focusRequests.h>

// GLOBAL VARIABLES


// Declare local variable chipId
String chipIdString = "";








// Function to send the requests < FUNCTION CODE 2343283247

void sendHttpRequest(String url, String authorizationHeader = "", String acceptHeader = "") {
  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {


    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    //client->setFingerprint(fingerprint);
    // Or, if you happy to ignore the SSL certificate, then use the following line instead:
    client->setInsecure();

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");

    if (https.begin(*client, url)) {
      //Setting the headers if necessary. In a future set the headers from an Array maybe < TODO
      if (authorizationHeader != "") {
        https.addHeader("Authorization", authorizationHeader);
      }
      if (acceptHeader != "") {
        https.addHeader("Accept", acceptHeader);
      }
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
          //Serial.println(payload);
          extractColorId(payload);
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      
      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }

}


//start web server
ESP8266WebServer server(80);

// Initialize LittleFS
void initFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  else{
    Serial.println("LittleFS mounted successfully");
  }
}







//littlefs function to read a file
String readFile(String filePath) {
  // Open the file in read mode
  File file = LittleFS.open(filePath, "r");
  String content = "";
  // If the file is available, read from it
  if (file) {
    Serial.println("Reading from file:");
    while (file.available()) {
      char c = file.read();
      Serial.write(c);
      content += c;
    }
    file.close();
  } else {
    Serial.println("Error: Unable to open the file.");
  }

  return content;
}
//LittleFS WriteFile function to write the content to the file from the path provided in the parameters
// Write file to LittleFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
  file.close();
}

//function to generate the local client ID
String generateClientId() {
  //generate the id
  uint32_t chipId = ESP.getChipId();
  //convert the id to an string
  String chipIdString = String(chipId);
  //convert it to a constant string to send it to the writefile function as it expects this data type
  const char *chipIdChar = chipIdString.c_str();
  //flush the file content so it doesn't solappe
  writeFile(LittleFS, "/localId.txt", "");
  //write the local id content to a file
  writeFile(LittleFS, "/localId.txt", chipIdChar);
  return "Local ID --> " + String(chipId);
}

//form to select the networks
void handleRoot() {
  String content = "<h1>OpenFocus - Redes Wi-Fi Disponibles:</h1>";
  int numNetworks = WiFi.scanNetworks();

  if (numNetworks == 0) {
    content += "<p>No se encontraron redes Wi-Fi.</p>";
  } else {
    String localId = readFile("/localId.txt");
    //Build form where networks are listed
    /*
    content += "<head>\n";
    content += "<link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.2.3/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-rbsA2VBKQhggwzxH7pPCaAqO46MgnOM80zW1RWuH61DGLwZJEdK2Kadq2F9CUG65\" crossorigin=\"anonymous\">";
    content += "<style>\n";
    content += "    body {\n";
    content += "        display: grid;\n";
    content += "        place-items: center;\n";
    content += "        height: 100vh;\n";
    content += "        border: solid;\n";
    content += "        border-color: black;\n";
    content += "    }\n";
    content += "\n";
    content += "    .enviar {\n";
    content += "        display: flex;\n";
    content += "        justify-content: center;\n";
    content += "    }\n";
    content += "\n";
    content += "    .form-container {\n";
    content += "        padding: 20px;\n";
    content += "        background-color: #fff;\n";
    content += "        box-shadow: 0px 0px 10px 0px rgba(0, 0, 0, 0.1);\n";
    content += "        border-radius: 10px;\n";
    content += "        position: fixed;\n";
    content += "        top: 50%;\n";
    content += "        left: 50%;\n";
    content += "        transform: translate(-50%, -50%);\n";
    content += "    }\n";
    content += "</style>\n";

    content += "</head>\n";
    */
    content += "<body>\n";
    content += " <div class=\"form-container\"";

    content += "<h1>Local ID --> " + localId;
    content += "<form action=\"/submit\" method=\"post\">";
    content += "<label for=\"opciones\">Selecciona una red wifi de la lista:</label>";
    //Creating network list, the value will be passed through the "opciones" content
    content += "<select id=\"opciones\" name=\"red\" class=\"form-select\" id=\"floatingSelect\" id=\"opcion_lista\">";
    for (int i = 0; i < numNetworks; ++i) {
      //Provide value to opcion local variable
      //String opcion = "opcion(" + String(i) + ")";
      content += "<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + "</option>";
    }
    content += "</select>";

    content += "<br><br>";

    content += "<label for=\"contrasena\">Introducir la contraseña</label>";
    content += "<input type=\"password\" id=\"contrasena\" name=\"contrasena\">";

    content += "<br><br>";

    content += "<div class=\"enviar\">";
    content += "<input type=\"submit\" class=\"btn btn-primary\" value=\"Conectarse a la red wifi\">";
    content += "</div>";
    content += "</form>";

 


  }

  server.send(200, "text/html", content);
}


//handle the post request from the networks form and print its content to the serial monitor
void handlePostRequest() {
  String red = server.arg("red");       // Get the value of the 'opcion' parameter from the POST request
  String contrasena = server.arg("contrasena"); // Get the value of the 'contrasena' parameter from the POST request

  Serial.print("Red: ");
  Serial.println(red);
  Serial.print("Contrasena: ");
  Serial.println(contrasena);
  // Add your code to handle the received message here
  

  //Save values in a file
  writeFile(LittleFS, "/ssid.txt", red.c_str());
  writeFile(LittleFS, "/pass.txt", contrasena.c_str());

  //connect to wifi
  WiFi.begin(red, contrasena);
  //indicate in serial monitor the wifi connection status/
    while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
  }
  //print a new line, then print WiFi connected and the IP address
  Serial.println("");
  Serial.println("WiFi connected");
  // Print the IP address
  Serial.println(WiFi.localIP());
  String localId = readFile("/localId.txt");

  //Declare the redirect website content
  String content = "<head>";
  content += "<meta http-equiv=\"refresh\" content=\"0;url=https://elmejordominiodepruebasdelahistoriadelahumanidad.shop/focusWebTest/login/test2/oauthTutorial?local=" + localId + "\">";
  content += "<title>Redirecting...</title>";
  content += "</head>";
  content += "<body>";
  content += "<p>If you are not redirected, <a href=\"https://elmejordominiodepruebasdelahistoriadelahumanidad.shop/focusWebTest/login/test2/oauthTutorial?local=" + localId + "\">click here</a>.</p";
  content += "</body>";
  server.send(301, "text/html", content); // Send a response to the client
}

//included for the littlefs google login request 
String processor(const String& var){
Serial.println("process variable " + var);
if(var == "TITLE"){
return "ESP - Template";
}
return String();
}

//new comment

//Display GoogleLogin 
/*
  void googleLogin(){
    //server.sendHeader("Location", "index.html",true);   //Redirect to our html web page
    //request->send(LittleFS, "/index.html", String(), false, processor);
    File file = LittleFS.open("/index.html", "r");
    if (file) {
      server.streamFile(file, "text/html");
      file.close();
    } else {
      server.send(404, "text/plain", "File not found");
    }
  }
*/

// Google login redirect button (with the localId)
void googleRedirect(){
String localId = readFile("/localId.txt");
String content = "";

//Declare the HTML content

content += "<!DOCTYPE html>\n";
content += "<html lang=\"en\">\n";
content += "<head>\n";
content += "    <meta charset=\"UTF-8\">\n";
content += "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
content += "    <title>Local ID Button</title>\n";
content += "</head>\n";
content += "<body>\n";
content += "    <button id=\"localIdButton\">Redirect with Local ID</button>\n";
content += "\n";
content += "    <script>\n";
content += "        // Get the local ID from your source (e.g., user input, database, etc.)\n";
content += "        let localId = \"" + localId + "\"; // Replace with the actual local ID\n";
content += "\n";
content += "        // Get the button element by its ID\n";
content += "        let button = document.getElementById(\"localIdButton\");\n";
content += "\n";
content += "        // Add click event listener to the button\n";
content += "        button.addEventListener(\"click\", function() {\n";
content += "            // Construct the URL with the local ID as a parameter\n";
content += "            let redirectUrl = `https://elmejordominiodepruebasdelahistoriadelahumanidad.shop/focusWebTest/login/test2/oauthTutorial?local=" + localId + "`;\n";
content += "\n" ;
content += "            // Redirect the user to the specified URL\n";
content += "            window.location.href = redirectUrl;\n";
content += "        });\n";
content += "    </script>\n";
content += "</body>\n";
content += "</html>\n";


// Send a response to the client

server.send(200, "text/html", content);
//Define parameters as variables


const char* acceptHeader1 = "application/json";
  
String accessToken = getAccessToken();
String serverUrl1 = "https://elmejordominiodepruebasdelahistoriadelahumanidad.shop/focusWebTest/writeToDatabase.php?local=122345555345&accessToken=" + accessToken;
String serverUrl2 = "https://elmejordominiodepruebasdelahistoriadelahumanidad.shop/focusWebTest/retrieveFromDatabase.php?local=9898011&accessToken=" + accessToken;
//String serverUrl3 = "https://www.googleapis.com/calendar/v3/calendars/primary/events?maxResults=1&orderBy=startTime&singleEvents=true&timeMin=2023-11-09T00:00:00Z&timeMax=2023-11-09T23:59:59Z";
String serverUrl3 = "https://www.googleapis.com/calendar/v3/calendars/primary/events?maxResults=1&orderBy=startTime&singleEvents=true&timeMin=2023-11-14T00:00:00Z&timeMax=2023-11-14T23:59:59Z";


while (1==1){
   //call to the website
  String accessToken = getAccessToken();
  String authorizationHeader1 = "Bearer " + accessToken;
  Serial.println("Request to " + serverUrl3);
  sendHttpRequest(serverUrl3, authorizationHeader1, acceptHeader1);
  
  Serial.println();
  Serial.println("Waiting 10 sec before the next round...");
  
  delay(10000);
}

}

void setup() {
  Serial.begin(115200);
  initFS();

  //stop the leds
  strip.begin();  // Inicializar la tira de LEDs
  strip.show();
  setColor(strip.Color(0, 0, 0)); // switch off all the leds

  //test client id
  //getCurrentColor();
  String clientId = generateClientId();
  Serial.println("Client ID: " + clientId);
  // Iniciar en modo de punto de acceso
  WiFi.mode(WIFI_AP);
  WiFi.softAP("BeamFocus-AP", "password"); // Nombre de la red y contraseña del AP

  // Configurar rutas del servidor web
  server.on("/", HTTP_GET, handleRoot);
  //server.on("/reroute", HTTP_POST, handleConnect); < This button is crucial to login
  server.on("/login", HTTP_GET, googleRedirect);

  //receive net info
  server.on("/submit", HTTP_POST, handlePostRequest); // Set up the POST request handler

  //send curl request to retrieve the color
  server.on("/curl", HTTP_GET, getCurrentColor);

  //login test with an uploaded html file
  //server.on("/login",googleLogin);
  // Iniciar el servidor web
  server.begin();
  Serial.println("Servidor web iniciado");
}

void loop() {
  server.handleClient();
  //call to the website
  /*
  Serial.println("Request to " + serverUrl3);
  sendHttpRequest(serverUrl3, authorizationHeader1, acceptHeader1);
  
  Serial.println();
  Serial.println("Waiting 10 sec before the next round...");
  delay(10000);
  */
}


void handleConnect() {
  String enteredSSID = server.arg("ssid");
  String enteredPassword = server.arg("password");

  if (enteredSSID.length() > 0 && enteredPassword.length() > 0) {
    WiFi.begin(enteredSSID.c_str(), enteredPassword.c_str());
    server.send(200, "text/html", "Conectando a la red WiFi. Reinicia el dispositivo para verificar la conexión.");
  } else {
    server.send(200, "text/html", "Por favor, ingresa las credenciales de la red.");
  }
}
