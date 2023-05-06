/*
Pratik M Tambe
References:
https://www.makerguides.com/esp32-and-tb6600-stepper-motor-driver/
*/
#include <Regexp.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "nca_atm_ap";
const char* password = "NcaAtmPassword";

//Change the below defaults as necessary
#define DEFAULT_STEPS 1
#define DEFAULT_MICROSECS 50

//Initialize variables
#define BAUD_RATE   115200
//GPIO pins 13 and 14 are for CW and CCW push buttons https://docs.arduino.cc/built-in-examples/digital/Button
#define buttonPinCW 13
#define buttonPinCCW 14

//const int pushButtonPinCW = 2;
//volatile bool pushButtonPressedCW = false;

//const int pushButtonPinCCW = 3;
//volatile bool pushButtonPressedCCW = false;

int PUL = 25; //define Pulse pin
int DIR = 26; //define Direction pin
int ENA = 27; //define Enable Pin
int microsecs = DEFAULT_MICROSECS;
int steps = DEFAULT_STEPS;
// variables will change:
int buttonStateCW = 0;  // variable for reading the pushbutton status
// variables will change:
int buttonStateCCW = 0;  // variable for reading the pushbutton status

// Buffer to store incoming commands from serial port
String inData;

struct Button {
	const uint8_t PIN;
	uint32_t numberKeyPresses;
	volatile bool pressed;
};

Button buttonCW = {buttonPinCW, 0, false}; //clockwise
Button buttonCCW = {buttonPinCCW, 0, false};//counterclockwise

void IRAM_ATTR buttonISRCW() {
  buttonCW.numberKeyPresses++;
  buttonCW.pressed = true;
}

void IRAM_ATTR buttonISRCCW() {
  buttonCCW.numberKeyPresses++;
  buttonCCW.pressed = true;
}


WebServer server(80);



//Clockwise
void moveCW(){
    digitalWrite(DIR, LOW);
    digitalWrite(ENA, HIGH);
    digitalWrite(PUL, HIGH);
    delayMicroseconds(microsecs);
    digitalWrite(PUL, LOW);
    delayMicroseconds(microsecs);
}

//Counter Clockwise
void moveCCW(){
    digitalWrite(DIR, HIGH);
    digitalWrite(ENA, HIGH);
    digitalWrite(PUL, HIGH);
    delayMicroseconds(microsecs);
    digitalWrite(PUL, LOW);
    delayMicroseconds(microsecs);
}

bool isInteger(String str) {
  for (int i = 0; i < str.length(); i++) {
    if (!isdigit(str.charAt(i))) {
      return false;
    }
  }
  return true;
}

void handleRoot() {
  server.send(200, "text/html",
              "<html><head><style>"
              "body {background-color: #f0f0f0;}"
              "input[type=submit][name=button1] {background-color: lightyellow;}"
              "input[type=submit][name=button2] {background-color: lightyellow;}"
              "input[type=submit][name=button3] {background-color: lightgreen;}"
              "input[type=submit][name=button4] {background-color: lightblue;}"
              ".center {display: flex; justify-content: center; align-items: center; height: 100vh;}"
              "</style></head><body><h1>Arduino Simple Stepper Controller Web Server</h1>"
              "<form action='/button1'><table>"
              "<tr><th>The number of steps to traverse:</th><td><input type='text' name='textbox1'></td>"
              "<td><input type='submit' name='button1' value='Submit'></td></tr>"
              "<tr><td><h4> Steps [current value]: " + String(steps) + "</h4></td></tr>"
              "</table></form><br>"
              "<form action='/button2'><table>"
              "<tr><th>Delay between pulses (microsecs):</th><td><input type='text' name='textbox2'></td>"
              "<td><input type='submit' name='button2' value='Submit'></td></tr>"
              "<tr><td><h4> microsecs [current value]: " + String(microsecs) + "</h4></td></tr>"
              "</table></form><br>"
              "<form action='/button3'><table>"
              "<tr><td><input type='submit' name='button3' value='Clockwise (CW)'></td></tr>"
              "</table></form><br>"
              "<form action='/button4'><table>"
              "<tr><td><input type='submit' name='button4' value='CounterClockwise (CCW)'></td></tr>"
              "</table></form><br>"
              "<div class='center'>"
              "<h4>Copyright (c) 2023 Pratik M Tambe <enthusiasticgeek@gmail.com> [Software released under MIT License]</h4>"
              "</div>"
              "</body></html>");
}

void handleButton1() {
  String textbox1_value = server.arg("textbox1");
  if (isInteger(textbox1_value)) {
    steps = textbox1_value.toInt();
    if (steps == 0) {
      steps = DEFAULT_STEPS; //change to default
    }
    //TODO(Pratik) - restrict the steps
    //server.send(200, "text/plain", "Button 1 was pressed with value: " + String(value));
    Serial.println("\nButton 1 (steps) was pressed with value: " + String(steps));
  } else {
    //server.send(200, "text/plain", "Invalid input for text box 1");
    Serial.println("\nInvalid input for text box 1 (steps): ");
  }
  //handleRoot();
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void handleButton2() {
  String textbox2_value = server.arg("textbox2");
  if (isInteger(textbox2_value)) {
    microsecs = textbox2_value.toInt();
    if (microsecs == 0) {
      microsecs = DEFAULT_MICROSECS; //change to default
    }
    //TODO(Pratik) - restrict the microsecs
    //server.send(200, "text/plain", "Button 2 was pressed with value: " + String(value));
    Serial.println("\nButton 2 (delay microsecs) was pressed with value: " + String(microsecs));
  } else {
    //server.send(200, "text/plain", "Invalid input for text box 2");
    Serial.println("\nInvalid input for text box 2 (delay microsecs): ");
  }
  //handleRoot();
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void webButtonCW() {
    for (int i = 0; i < steps; i++) {
         moveCW();
         Serial.println(String(i));
    }
    Serial.println("Push button CW is pressed and CW move is complete.");
  /*
  if (pushButtonPressedCW) {
    //server.send(200, "text/plain", "Push button is pressed");
    Serial.print("Push button is pressed");
  } else {
    //server.send(200, "text/plain", "Push button is not pressed");
    Serial.print("Push button is not pressed");
  }*/
  //handleRoot();
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void webButtonCCW() {
    for (int i = 0; i < steps; i++) {
         moveCCW();
         Serial.println(String(i));
    }
    Serial.println("Push button CCW is pressed and CCW move is complete.");
  /*
  if (pushButtonPressedCW) {
    //server.send(200, "text/plain", "Push button is pressed");
    Serial.print("Push button is pressed");
  } else {
    //server.send(200, "text/plain", "Push button is not pressed");
    Serial.print("Push button is not pressed");
  }*/
  //handleRoot();
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}


//void handlePushButtonInterruptCW() {
//  pushButtonPressedCW = !digitalRead(pushButtonPinCW); // Toggle pushButtonPressedCW variable
//}

//void handlePushButtonInterruptCCW() {
//  pushButtonPressedCCW = !digitalRead(pushButtonPinCCW); // Toggle pushButtonPressedCCW variable
//}



// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(BAUD_RATE);  
  pinMode (PUL, OUTPUT);
  pinMode (DIR, OUTPUT);
  pinMode (ENA, OUTPUT);

  // initialize the pushbutton pins as an input:
	pinMode(buttonCW.PIN, INPUT_PULLUP);
	attachInterrupt(buttonCW.PIN, buttonISRCW, FALLING);

	pinMode(buttonCCW.PIN, INPUT_PULLUP);
	attachInterrupt(buttonCCW.PIN, buttonISRCCW, FALLING);
  
  // Start WiFi in Access Point mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println("WiFi AP mode started");
  Serial.println(WiFi.softAPIP());

  // Set up web server
  server.on("/", handleRoot);
  server.on("/button1", handleButton1);
  server.on("/button2", handleButton2);
  server.on("/button3", webButtonCW);
  server.on("/button4", webButtonCCW);
  server.begin();

  Serial.println("Web server started");

  // Set up push button switch
  //pinMode(pushButtonPinCW, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(pushButtonPinCW), handlePushButtonInterruptCW, CHANGE);

  // Set up push button switch
  //pinMode(pushButtonPinCCW, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(pushButtonPinCCW), handlePushButtonInterruptCCW, CHANGE);
  
}

//https://stackoverflow.com/questions/5697047/convert-serial-read-into-a-usable-string-using-arduino



// the loop function runs over and over again forever
void loop() {
    while (Serial.available() > 0)
    {
        char recieved = Serial.read();
        inData += recieved;     
        // Process message when return character is recieved
        if (recieved == '\r')
        {
            Serial.println("=== Type 'help' for detailed instructions! ===");
            Serial.print("\nArduino Received: ");
            Serial.print(inData);
            Serial.print("\n");

            MatchState ms;
            ms.Target ((char*)inData.c_str());
            char result = ms.Match ("(%a+)=(%d+)", 0);
            if (result == REGEXP_MATCHED)
            {
              // matching offsets in ms.capture
              char buf [100];  // large enough to hold expected string
              Serial.print ("Captures: ");
              Serial.println (ms.level);
              for (int j = 0; j < ms.level; j++) {
                //Serial.print ("Capture number: ");
                //Serial.println (j, DEC);
                //Serial.print ("Text: '");
                //Serial.print (ms.GetCapture (buf, j));
                //Serial.println ("'");
                //first is a string based on Regex capture group
                if ((j==0) && (ms.level==2)){
                    String captured = ms.GetCapture (buf, 0);
                    String value = ms.GetCapture (buf, 1);
                    if (captured  == "steps"){
                      steps = captured.toInt();
                      //TODO(Pratik) - check the bounds (need to be valid)
                      Serial.println("steps are now " + value);
                    } else if (captured == "microsecs"){
                      microsecs = captured.toInt();
                      //TODO(Pratik) - check the bounds (need to be valid)
                      Serial.println("microsecs are now " + value);
                    }
                }
              } 
            } else if (result == REGEXP_NOMATCH) {
              // no match
              //Serial.println("no match");
            }
            //TODO(Pratik) - Add more instructions as necessary    
            if ((inData == "cw\r") || (inData == "CW\r")){
                      Serial.println("Motor will turn clockwise\n");
                      for (int i = 0; i < steps; i++){
                          moveCW();
                      }
                      Serial.println("Motor finished turning clockwise\n");
            } else if ((inData == "ccw\r") || (inData == "CCW\r")){
                      Serial.println("Motor will turn counterclockwise\n");
                      for (int i = 0; i < steps; i++){
                          moveCCW();
                      }
                      Serial.println("Motor finished turning counterclockwise\n");
            } else if ((inData == "en=1\r") || (inData == "EN=1\r")){
                      Serial.println("Motor enabled\n");
                      digitalWrite(ENA, HIGH);
            } else if ((inData == "en=0\r") || (inData == "EN=0\r")){
                      Serial.println("Motor disabled\n");
                      digitalWrite(ENA, LOW);
            } else if ((inData.indexOf("steps:") >= 0)){
                      //int ind1 = inData.indexOf('steps');  //finds location of first ,
                      //int speed = inData.substring(0, ind1+6);  
                      Serial.println("steps\n");
            } else if(inData == "help\r"){ // DON'T forget to add "\n" at the end of the string.
                      Serial.println("\nValid commands are: \r\n 'CW' (clockwise),\r\n 'CCW' (counterclockwise),\r\n 'EN=1' (enable),\r\n 'EN=0'(disable),\r\n 'steps=<value>', \r\n 'microsecs=<value>'.");
            } else {
              //Any other remaining case?  
            }       
            inData = ""; // Clear recieved buffer
        }
    }

  // Check if manually someone presses push buttons.
  // Read the state of the pushbutton value.
  // Added a guard so that both clockwise and counterclockwise push buttons are not pressed simultaneously!
  if ((buttonCW.pressed)&&(buttonCCW.pressed)) {
     Serial.printf("CCW and CW push buttons should not be pressed simultaneously. Ignoring the commands!\n");
 		 buttonCW.pressed = false;		
     buttonCCW.pressed = false;

  } else if (buttonCW.pressed) {
		Serial.printf("Button has been pressed %u times\n", buttonCW.numberKeyPresses);
    // move CW:
    Serial.println("Motor will turn clockwise\n");
    moveCW();
		buttonCW.pressed = false;
	} else if (buttonCCW.pressed) {
		Serial.printf("Button has been pressed %u times\n", buttonCCW.numberKeyPresses);
    // move CCW:
    Serial.println("Motor will turn counterclockwise\n");
    moveCCW();
		buttonCCW.pressed = false;
	} 
      
  //call the webserver    
  server.handleClient();


}

