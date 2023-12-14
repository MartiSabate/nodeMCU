//ESTE ES EL FICHERO CON EL POC PARA MOSTRAR LAS REDES WIFI EN UN SERVIDOR WEB, EVITANDO USAR WIFI MANAGER

#include <ESP8266WebServer.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <Adafruit_NeoPixel.h>
#include <LittleFS.h>
#include <DNSServer.h>
#include <TaskScheduler.h>





// Include local defined libraries

#include <ledsCalSync.h>
#include <focusRequests.h>

// GLOBAL VARIABLES

// CONFIGURATION VARIABLE TO BLOCK THE SUBMIT ACCESS WITH THE DEVICE MISSCONFIGURED

bool CONFIG = false;
// AP IP address global variable
IPAddress apIPAddress;

// Declare Web Server

ESP8266WebServer server(80);

// Start DNS Server

DNSServer dnsServer;

// Define task scheduler
// Create a scheduler object with the maximum number of tasks
Scheduler scheduler;




// Initialize LittleFS
void initFS() {

  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  else{
    Serial.println("LittleFS mounted successfully");
  }
}

// timer with minutes as imput 
void minuteTimer(int minutes){

  Serial.println(String(minutes) + " minutes timer started.");

  for(int i = 0; i<minutes; i++){
    int minutesLeft = minutes - i;
    // Print the current time every minute
    Serial.println(String(minutesLeft) + " minutes left.");

    delay (60000) ;
    
  }
}

// Print wifi status (connected or disconnected) as per line 50 https://github.com/dlitz/ArduinoCore-esp8266/blob/master/libraries/ESP8266WiFi/src/include/wl_definitions.h

void printApMode() {

  // 0 = station mode off
  // 2 = station mode on

  // Check and print WiFi mode
  int mode = WiFi.getMode();
  Serial.print("NodeMCU Mode > ");
  switch (mode) {
    case 0:
      Serial.print("OFF\n");
      break;
    case 2:
      Serial.print ("ON\n");
      break;
  }


}

// Stop the Access Point

void stopAccessPoint(){

 //Switch off all the leds
 setColor(strip.Color(0, 0, 0)); 


//Indicate in console the AP is getting shut down
printApMode();

Serial.print("Stopping Access Point...");


//10 second delay before stopping the device

int delaySeconds = 10;

for ( int i = 0; i<delaySeconds; i++){
  Serial.print(String(delaySeconds-i));
  delay(500);
  Serial.print(".");
  delay(500);
  
}

Serial.print("\n---\n");
// Stop the WiFi AP
WiFi.softAPdisconnect(true);  // Disconnect clients and stop the AP
WiFi.mode(WIFI_OFF);          // Switch off WiFi completely or use WIFI_STA if you need to connect to a station

printApMode();

Serial.println("Sleep the arduino indefinitely (until the reset button is pushed / pin 16 is activated)");

ESP.deepSleep(0);


}


void shutdown() {

String htmlCode = R"(
  <!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" type="image/x-icon" href="/brainFavicon" sizes="192x192">
  <link rel="apple-touch-icon" href="/brainFavicon">
  <title>WiFi Desactivado</title>
  <style>
      @font-face {
          font-family: 'Oswald';
          src: url('Oswald-Regular.ttf') format('truetype');
          font-weight: normal;
          font-style: normal;
      }

      body {
          display: flex;
          align-items: center;
          justify-content: center;
          height: 100vh;
          margin: 0;
          font-family: 'Oswald', 'Arial', sans-serif;
          background-color: #000;
          color: #fff;
          transform: scale(1);
      }

      .message {
          text-align: center;
          padding: 20px;
          border: 2px solid #FF0000;
          border-radius: 8px;
          max-width: 400px;
          width: 100%;
      }

      h1 {
          color: #FF0000;
      }

      p {
          color: #fff;
      }
  </style>
</head>
<body>
  <div class="message">
      <h1>WiFi Desactivado</h1>
      <p>La zona WiFi del dispositivo ha sido deshabilitada.</p>
      <p>Reinicia el dispositivo para habilitar la zona WiFi de nuevo.</p>
      <p>El dispositivo se apagará automáticamente una vez finalizados los Pomodoros programados.</p>
  </div>
</body>
</html>


  )";

 
  // Send HTML code
  server.send(200, "text/html", htmlCode);
  
  
  // Shut down the AP

  stopAccessPoint();




}


// Stop the system after 5 minutes if no pomodoro is configured
  // declare variables as global
  
  int minutes = 5;
  int remaining = 0;
void systemStop(){

  /*
  while (seconds <= 300){
  if (seconds == total){
    stopAccessPoint();
    seconds++;

  }
  if (seconds < total ){
    remaining = total - seconds;
    //Serial.println("Awaiting action - " + String(remaining) + " seconds left to shutdown AP");
    if (seconds % 60 == 0) {
      int minutes = remaining / 60;
      Serial.println(String(minutes) + " minutes remaining to shut down the AP");
    }
    seconds++;
    

    delay(1000);

  }
  */
  if (minutes == 0){
    Serial.println("Stopping the AP");
    stopAccessPoint();
    minutes--;
  }else{
    Serial.println(String(minutes) + " minutes left");
    minutes--;
  }
}

// Define task so a timer will run in the background to stop the device automatically

Task stopSystem(60000, 6, &systemStop);


// Pomodoro function declaration
// pomodoro(fColor, bColor, focusHours, focusMinutes, breakMinutes, cyclesInt);

void pomodoro(uint32_t focusColor, uint32_t breakColor, int focusHours = 1, int focusMins = 30, int breakMins = 0, int cyclesInt = 1){

// Activate Pomodoro timer

for (int i = 0; i < cyclesInt; i++) {

  // Increment the counter in 1 in the start of the serie for simplicity

  int o = i+1;

  // Print in console the serie iteration

  Serial.println("Serie " + String(o) + " de " + cyclesInt);


  // Set the pomodoro focus color

  setColor(focusColor);

  // Start the Pomodoro process

  //debug variable print in serial monitor

  Serial.println("focusHours value > " + String(focusHours));
  Serial.println("focusMins value > " + String(focusMins));


  // Start the focus timers

  Serial.println("Start hours timer");
  minuteTimer(focusHours*60);
  
  Serial.println("Start minutes timer");
  minuteTimer(focusMins);


  // Use the break always, except the last iteration

  if ( o != cyclesInt ){

    // Indicate the last iteration in the console

    Serial.println("Break time has started in iteration: " +  String(o));

    // Switch color to the break Color

    setColor(breakColor);

    // Break timer

    minuteTimer(breakMins);
  }

}

// Once the loop ends shut down the leds again

setColor(strip.Color(0, 0, 0)); // switch off all the leds


}



/* attempt 1 of send image
void sendFile(){

  server.send(200, "/brain.png", "http://192.168.4.1/brain.png");

}
*/

// attempt 2

void sendFile(String filename) {


  //Verify if the requested file exists
  String path = "/" + String(filename); //hardcode brain image path
  Serial.println("[i] Checking file path: " + path);
  // Check if the file exists
  if (!LittleFS.exists(path)) {
    Serial.println("[!] File not found");
    server.send(404, "text/plain", "File not found");
    return;
  }else{
    Serial.println("[i] File found in the filesystem");
  }


  // Open the file
  File file = LittleFS.open(path, "r");
  if (!file) {
    server.send(500, "text/plain", "Failed to open file");
    Serial.println("[!] Failed to open the file");
    return;
  }else{
    Serial.println("[i] File opened successfully");
  }

  Serial.println("File name is " + String(file.name()));

  // Get the file size
  size_t fileSize = file.size();

  // Send the file content in chunks
  const size_t bufferSize = 1024;
  char buffer[bufferSize];

  while (file.available() > 0) {
    size_t bytesRead = file.readBytes(buffer, std::min(bufferSize, fileSize));
    server.client().write(reinterpret_cast<uint8_t*>(buffer), bytesRead);
    fileSize -= bytesRead;
  }

  // Close the file
  file.close();

}

//send font function
void sendFont() {
  // Verify if the requested font file exists
  String fontPath = "/Oswald-Regular.ttf"; // Adjust the font file path
  Serial.println("Checking font file path: " + fontPath);

  // Check if the font file exists
  if (!LittleFS.exists(fontPath)) {
    Serial.println("Font file not found");
    server.send(404, "text/plain", "Font file not found");
    return;
  }

  // Open the font file
  File fontFile = LittleFS.open(fontPath, "r");
  if (!fontFile) {
    server.send(500, "text/plain", "Failed to open font file");
    return;
  }

  // Get the font file size
  size_t fontFileSize = fontFile.size();

  // Send the font file content in chunks
  const size_t bufferSize = 1024;
  char buffer[bufferSize];

  while (fontFile.available() > 0) {
    size_t bytesRead = fontFile.readBytes(buffer, std::min(bufferSize, fontFileSize));
    server.client().write(reinterpret_cast<uint8_t*>(buffer), bytesRead);
    fontFileSize -= bytesRead;
  }

  // Close the font file
  fontFile.close();
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
    Serial.printf("- failed to open file for writing. Error code: %d\r\n", file.getWriteError());
    
  }
  file.close();
}

// list all the files fron the directory passed as parameter

void listFiles(const char* directory) {
  Serial.println("Listing files in directory: " + String(directory));

  Dir dir = LittleFS.openDir(directory);

  while (dir.next()) {
    Serial.print("  FILE: ");
    Serial.println(dir.fileName());
  }

  Serial.println("File listing complete");
}


//Pomodoro focus configuration platform
void handleRoot() {


  String html = R"(
    <!DOCTYPE html>
    <html lang="es">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <link rel="icon" type="image/x-icon" href="/brainFavicon" sizes="192x192">
      <link rel="apple-touch-icon" href="/brainFavicon">
      <title>Pomodoro Focus</title>
      <style>
          @font-face {
              font-family: 'Oswald';
              src: url('/font') format('truetype'); /* Assuming the "/font" endpoint is correctly set up to serve the font */
              font-weight: normal;
              font-style: normal;
          }

          html, body {
              height: 100%;
              margin: 0;
          }
          body {
            font-family: 'Oswald', 'Arial', sans-serif;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
            margin: 0;
            background-color: #000; /* Black background color */
            color: #fff; /* Text color */
            transform: scale(1); /* Adjust the zoom level as needed */


          }

          .header-container {
              display: flex;
              align-items: center;
          }

          .header-container img {
              width: 50px; /* Adjust the width as needed */
              margin-right: 10px; /* Adjust the margin as needed */
          }

          h2 {
              text-align: center;
          }

          form {
              width: 300px;
              padding: 20px;
              border: 2px solid #4CAF50; /* Premium green border, customize as needed */
              border-radius: 10px;
              box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
              background-color: #111; /* Dark background color, customize as needed */
          }

          label {
              display: block;
              margin-bottom: 5px;
              color: #4CAF50; /* Premium green color, customize as needed */
          }

          input {
              width: calc(100% - 12px);
              margin-bottom: 10px;
              padding: 8px;
              box-sizing: border-box;
              background-color: #333; /* Darker background color, customize as needed */
              color: #fff; /* Text color */
              border: 1px solid #4CAF50; /* Premium green border, customize as needed */
              border-radius: 5px;
          }

          input[type="color"] {
              width: 100%;
              box-sizing: border-box;
              padding: 8px;
              margin-bottom: 10px;
          }

          input[type="submit"] {
              font-family: 'Oswald', 'Arial', sans-serif; /* Add this line to apply the font to the submit button */
              width: 100%;
              padding: 10px;
              background-color: #4CAF50; /* Premium green color, customize as needed */
              color: #fff; /* Text color */
              border: 2px solid #fff; /* White border, customize as needed */
              border-radius: 5px;
              cursor: pointer;
          }

          input[type="submit"]:hover {
              background-color: #45a049; /* Darker green on hover */
          }

          .wifi-shutdown-container {
              margin-top: 20px; /* Add some space between the forms */
              box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
              background-color: #111; /* Customize as needed */
          }

          .wifi-shutdown-container button {
              font-family: 'Oswald', 'Arial', sans-serif; /* Add this line to apply the font to the shutdown button */
              width: 100%;
              padding: 10px;
              background-color: #FF5733; /* Customize as needed */
              color: #fff; /* Text color */
              border: 2px solid #fff; /* White border, customize as needed */
              cursor: pointer;
          }

          .wifi-shutdown-container button:hover {
              background-color: #E04A2E; /* Darker orange on hover */
          }
          
      </style>
  </head>
  <body>
      <div class="header-container" style="user-select: none;">
          <img src="/brain" draggable="false">
          <h2>Configuración de tu Pomodoro</h2>
      </div>

      <form action="/submit" method="post">
          <label for="focusHours">Horas de Focus:</label>
          <input type="number" id="focusHours" name="focusHours" min="0" max="24" placeholder="Horas" value="0" required>
          <label for="focusMinutes">Minutos de Focus:</label>
          <input type="number" id="focusMinutes" name="focusMinutes" min="0" max="59" placeholder="Minutos" value="25" required>
          <label for="focusColor">Color de Foco:</label>
          <input type="color" id="focusColor" name="focusColor" value="#4CAF50" required>
          <label for="breakMinutes">Minutos de Descanso:</label>
          <input type="number" id="breakMinutes" name="breakMinutes" min="0" max="30" placeholder="Minutos para descanso" value="5" required>
          <label for="breakColor">Color de Descanso:</label>
          <input type="color" id="breakColor" name="breakColor" value="#FF5733" required>
          <label for="cycles">Número de Pomodoros a realizar:</label>
          <input type="number" id="cycles" name="cycles" min="1" max="100" value="1" required>
          <input type='hidden' name='config' value='true'>
          <input type="submit" value="Guardar Configuración">
      </form>
      <div class="wifi-shutdown-container">

          <form action="/shutdown">
              <button type="submit">Apagar zona WiFi</button>
      </div>
  </body>
  </html>
)";
server.send(200, "text/html", html);
/*
sendFile("brainFavicon.png");
sendFile("brain.png");
*/
sendFont();

// Set the config to true once the menu has been loaded at least 1 time 

CONFIG = true;

}


// Retrieve the configuration values from the root form


void handleSubmit() {
  
  // HTML Response content


  //IN CASE IT IS REQUIRED REDIRECT TO SHUTDOWN AFTER SUBMITTING THE FORM < <meta http-equiv="refresh" content="5;url=/shutdown">

  String htmlCode = R"(
  <!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Configuración exitosa</title>
  <link rel="icon" type="image/x-icon" href="/dashboard/brainFavicon.ico" sizes="57x57">
  <link rel="apple-touch-icon" href="/dashboard/brainFavicon.ico">
  <style>
    @font-face {
      font-family: 'Oswald';
      src: url('Oswald-Regular.ttf') format('truetype');
      font-weight: normal;
      font-style: normal;
    }

    html, body {
      height: 100%;
      margin: 0;
    }

    body {
      font-family: 'Oswald', 'Arial', sans-serif;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      height: 100vh;
      margin: 0;
      background-color: #000;
      color: #fff;
      transform: scale(1);
    }

    .header-container {
      display: flex;
      align-items: center;
      margin-bottom: 20px;
      padding: 10px; /* Optional: Add padding to the header-container */
      justify-content: center; /* Add this line to center the content horizontally */
    }

    .header-container img {
      width: 50px;
      margin-right: 10px;
    }

    h1 {
      color: #04d112;
      margin: 0; /* Add this line to center the h1 */
    }

    .message {
      border: 2px solid #04d112; /* Adapted border style from the first code */
      text-align: center;
      padding: 20px;
      border-radius: 8px;
      max-width: 400px;
      width: 100%;
    }

  </style>
</head>
<body>
  <div class="message">
    <div class="header-container">
      <div style="user-select: none;">
        <h1>Configuración Exitosa</h1>
      </div>
    </div>
    <p>La configuración se ha establecido correctamente</p>
    <p>En <strong>10 segundos</strong> se apagará la zona WiFi</p>
  </div>
</body>
</html>
  )";
  
  

  /* IN CASE I WANT TO STORE THE DATA IN THE MEMORY USE THIS
  // Handle form submission here
  String focusHours = server.arg("focusHours");
  writeFile(LittleFS, "/fHours.txt", focusHours.c_str());
  String focusMinutes = server.arg("focusMinutes");
  writeFile(LittleFS, "/fMinutes.txt", focusMinutes.c_str());
  String focusColor = server.arg("focusColor");
  writeFile(LittleFS, "/fColor.txt", focusColor.c_str());
  String breakMinutes = server.arg("breakMinutes");
  writeFile(LittleFS, "/bMinutes.txt", breakMinutes.c_str());
  String breakColor = server.arg("breakColor");
  */


  // VARIABLE DECLARATIONS

  // CONFIGURATION VARIABLE USED TO CHOOSE THE REPONSE SENT TO THE CLIENT

  String config = server.arg("config");

        
  // POMODORO CONFIG ARGS

  String focusHoursString = server.arg("focusHours");
  String focusMinutesString = server.arg("focusMinutes");
  String focusColorString = server.arg("focusColor");
  String breakMinutesString = server.arg("breakMinutes");
  String breakColorString = server.arg("breakColor");
  String cyclesString = server.arg("cycles");


  // Store the color values in uint32_t data type

  uint32_t fColor =  hexStringToColor(focusColorString);
  uint32_t bColor =  hexStringToColor(breakColorString);
  

  // Store de int values in int data type instead of string

  int focusHours = focusHoursString.toInt();
  int focusMinutes = focusMinutesString.toInt();
  int breakMinutes = breakMinutesString.toInt();
  int cyclesInt = cyclesString.toInt();

  // Declare all the time in seconds < TO REVIEW IF THESE 2 VARIABLES ARE NECESSARY

  //int focusSecs = focusHours * 60 * 60 + focusMinutes * 60 ;
  //int breakSecs = breakMinutes * 60 ;


  /*
  // coger datos en get -------> String red = server.arg("red")
  //Save values in a file
  writeFile(LittleFS, "/ssid.txt", red.c_str());
  */


  // config value debugging in serial monitor

  Serial.println("The variable config value is: " + String(config));
  Serial.println("The variable CONFIG value is: " + String(CONFIG));

  if (config == "true" && CONFIG){ //ONLY ACCESS TO SUBMIT IF THE DEVICE IS CONFIGURED

    Serial.println("Config if has been passed");

    // SETTING THE CONFIG VARIABLE TO FALSE AGAIN SO THE SERVER CANNOT BE SHUTDOWN TWICE

    CONFIG = false;


    Serial.println("Leds should start at " + focusColorString + " color...");

    setColor(fColor);

    server.send(200, "text/html", htmlCode);

    sendFont();

    


    // Disconnect the AP

    stopAccessPoint(); 

    
    // Start rolling the colors the Pomodoro

    pomodoro(fColor, bColor, focusHours, focusMinutes, breakMinutes, cyclesInt);

    }else{
      server.sendHeader("Location", "/", true);
      server.send(302, "text/plain", "Redirigiendo...");
    }
    
}

void getTest(){
      server.sendHeader("Location", "/", true);
      server.send(302, "text/plain", "Redirigiendo...");
      
}

void setup() {


  Serial.begin(115200);
  initFS();

  //stop the leds
  strip.begin();  // Inicializar la tira de LEDs
  strip.show();
  setColor(strip.Color(0, 0, 0)); // switch off all the leds
  printApMode();
  // Iniciar en modo de punto de acceso
  Serial.println("Starting AP...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP("BeamFocus-AP", "password"); // Nombre de la red y contraseña del AP
  printApMode();

  // list files in /
  listFiles("/");

  // Store the local AP IP address
  apIPAddress = WiFi.softAPIP();
  Serial.print("WiFi.localIP() value is > ");
  Serial.println(apIPAddress);


  // Add the task to the scheduler
  scheduler.addTask(stopSystem);

  // Enable the task
  stopSystem.enable();
  
  // Set up DNS server

  dnsServer.start(53, "local.openfocus", apIPAddress);
  Serial.println("DNS server started");

  // Configurar rutas del servidor web
  server.on("/", HTTP_GET, handleRoot);


  // Retrieve the configuration values in the POST URL
  server.on("/submit", HTTP_POST, handleSubmit);

  // Retrieve the configuration vlaues in the GET URL
  //server.on("/submit", HTTP_GET, getTest);


  // Stop the access point
  server.on("/shutdown", HTTP_GET, shutdown);
  
  // Send the png brain.png file through this function
  server.on("/brain", HTTP_GET, [](){ sendFile("brain.png"); });

  // Send the png brain.png file through this function
  server.on("/brainFavicon", HTTP_GET, [](){ sendFile("brainFavicon.ico"); });
  
  // Send the ttf (oswald font) file
  server.on("/font", HTTP_GET, sendFont);

  // Start web server
  server.begin();
  Serial.println("Servidor web iniciado");


  // Set up captive portal redirection
  server.onNotFound([]() {
    if (!CONFIG) {
      Serial.println("Client connected. Redirecting to captive portal.");
      server.sendHeader("Location", "/", true);
      server.send(302, "text/plain", "Redirecting to captive portal...");
    } else {
      server.send(404, "text/plain", "Not Found");
    }
  });



}


void loop() {



  // Listen for DNS Requests

  dnsServer.processNextRequest();

  // Listen for HTTP Requests

  server.handleClient();

  // Update the scheduler

  scheduler.execute();


 
}



// In case I have to connect the device to a netowrk

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
