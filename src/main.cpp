//ESTE ES EL FICHERO CON EL POC PARA MOSTRAR LAS REDES WIFI EN UN SERVIDOR WEB, EVITANDO USAR WIFI MANAGER

#include <ESP8266WebServer.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <Adafruit_NeoPixel.h>
#include <LittleFS.h>



// Include local defined libraries

#include <ledsCalSync.h>
#include <focusRequests.h>

// GLOBAL VARIABLES

// CONFIGURATION VARIABLE TO BLOCK THE SUBMIT ACCESS WITH THE DEVICE MISSCONFIGURED

bool CONFIG = false;

// Start web server

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


void minuteTimer(int minutes){

  for(int i = 0; i<minutes; i++){

     delay (60000) ;
    
  }
}

// Stop the Access Point

void stopAccessPoint(){

 //Switch off all the leds
 setColor(strip.Color(0, 0, 0)); 


//Indicate in console the AP is getting shut down

Serial.println("Stopping Access Point...");

//5 second delay before stopping the device

for ( int i = 0; i<5; i++){

  Serial.print(5-i + ".");
  delay(1000);
  
}



}

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

  // Start the focus timers

  minuteTimer(focusHours*60);
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

void shutdown() {
 
  String htmlCode = R"(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>WiFi desactivado</title>
        <style>
            body {
                display: flex;
                align-items: center;
                justify-content: center;
                height: 100vh;
                margin: 0;
                font-family: Arial, sans-serif;
            }

            .message {
                text-align: center;
                padding: 20px;
                border: 2px solid #FF0000;
                border-radius: 8px;
                background-color: #FFE6E6;
                max-width: 400px;
                width: 100%;
            }

            h1 {
                color: #FF0000;
            }

            p {
                color: #333333;
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



void sendImage(){

  server.send(200, "/brain.png", "http://192.168.4.1/brain.png");

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



//form to select the networks
void handleRoot() {

  String html = R"(
    <!DOCTYPE html>
    <html lang="es">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Configuración de tu Pomodoro</title>
      <style>

          html, body {
              height: 100%;
              margin: 0;
          }
          body {
              font-family: 'Arial', sans-serif;
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
      <div class="header-container">
          <img src="http://192.168.4.1/brain">
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

// Set the config to true once the menu has been loaded at least 1 time 

CONFIG = true;

}


// Retrieve the configuration values from the root form


void handleSubmit() {
  
  // HTML Response content


  //IN CASE IT IS REQUIRED REDIRECT TO SHUTDOWN AFTER SUBMITTING THE FORM < <meta http-equiv="refresh" content="5;url=/shutdown">

  String htmlCode = R"(
  <!DOCTYPE html>
  <html lang="es">
  <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Configuración Exitosa</title>
      <style>
          body {
              display: flex;
              align-items: center;
              justify-content: center;
              height: 100vh;
              margin: 0;
          }

          .message {
              text-align: center;
              padding: 20px;
              border: 2px solid #4CAF50;
              border-radius: 8px;
              background-color: #E9F8F2;
          }
      </style>
  </head>
  <body>
      <div class="message">
          <h1>Pomodoros en proceso</h1>
          <p>La configuración se ha establecido correctamente.</p>
          <p>En<strong> 5 segundos </strong>se apagará la zona WiFi</p>
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


  if (config == "true" && CONFIG){ //ONLY ACCESS TO SUBMIT IF THE DEVICE IS CONFIGURED

    // SETTING THE CONFIG VARIABLE TO FALSE AGAIN SO THE SERVER CANNOT BE SHUTDOWN TWICE

    CONFIG = false;


    Serial.println("Leds should start at " + focusColorString + " color...");

    setColor(fColor);

    server.send(200, "text/html", htmlCode);
    


    // Disconnect the AP

    shutdown(); 

    
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

 
  // Iniciar en modo de punto de acceso
  WiFi.mode(WIFI_AP);
  WiFi.softAP("BeamFocus-AP", "password"); // Nombre de la red y contraseña del AP

  // Configurar rutas del servidor web
  server.on("/", HTTP_GET, handleRoot);


  // Retrieve the configuration values in the POST URL
  server.on("/submit", HTTP_POST, handleSubmit);

  // Retrieve the configuration vlaues in the GET URL
  server.on("/submit", HTTP_GET, getTest);


  // Stop the access point
  server.on("/shutdown", HTTP_GET, shutdown);

  // Send the png file
  server.on("/brain", HTTP_GET, sendImage);

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
  server.handleClient();
 
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
