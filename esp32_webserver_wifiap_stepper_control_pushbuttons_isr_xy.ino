/*
Pratik M Tambe
References:
https://www.makerguides.com/esp32-and-tb6600-stepper-motor-driver/
*/
//This code is designed to control two stepper motors with a webserver on ESP32 -> X - axis and Y - axis.

#include <Regexp.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "nca_atm_ap";
const char* password = "NcaAtmPassword";

//Change the below defaults as necessary
#define DEFAULT_STEPS 1
#define DEFAULT_MICROSECS 50

//Initialize variables
#define BAUD_RATE 115200
//Stepper 1
//GPIO pins 13 and 14 are for CW and CCW push buttons https://docs.arduino.cc/built-in-examples/digital/Button
#define buttonPinCW_X 13
#define buttonPinCCW_X 14

//Stepper 2
//GPIO pins 15 and 16 are for CW and CCW push buttons https://docs.arduino.cc/built-in-examples/digital/Button
#define buttonPinCW_Y 4
#define buttonPinCCW_Y 2

//const int pushbuttonPinCW_X = 2;
//volatile bool pushButtonPressedCW = false;

//const int pushbuttonPinCCW_X = 3;
//volatile bool pushButtonPressedCCW = false;

int PUL_X = 25;  // Define PULSE pin for X-axis stepper
int DIR_X = 26;  // Define DIRECTION pin for X-axis stepper
int ENA_X = 27;  // Define ENABLE pin for X-axis stepper

int microsecs_X = DEFAULT_MICROSECS;
int steps_X = DEFAULT_STEPS;

// Initialize variables for Y-axis stepper motor
int PUL_Y = 23;  // Define PULSE pin for Y-axis stepper
int DIR_Y = 22;  // Define DIRECTION pin for Y-axis stepper
int ENA_Y = 21;  // Define ENABLE pin for Y-axis stepper

int microsecs_Y = DEFAULT_MICROSECS;
int steps_Y = DEFAULT_STEPS;


// variables will change:
int buttonStateCW_X = 0;  // variable for reading the pushbutton status
// variables will change:
int buttonStateCCW_X = 0;  // variable for reading the pushbutton status


// variables will change:
int buttonStateCW_Y = 0;  // variable for reading the pushbutton status
// variables will change:
int buttonStateCCW_Y = 0;  // variable for reading the pushbutton status

// Buffer to store incoming commands from serial port
String inData;

struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  volatile bool pressed;
};

Button buttonCW_X = { buttonPinCW_X, 0, false };    //clockwise
Button buttonCCW_X = { buttonPinCCW_X, 0, false };  //counterclockwise

Button buttonCW_Y = { buttonPinCW_Y, 0, false };    //clockwise
Button buttonCCW_Y = { buttonPinCCW_Y, 0, false };  //counterclockwise

void IRAM_ATTR buttonISRCW_X() {
  buttonCW_X.numberKeyPresses++;
  buttonCW_X.pressed = true;
}

void IRAM_ATTR buttonISRCCW_X() {
  buttonCCW_X.numberKeyPresses++;
  buttonCCW_X.pressed = true;
}


void IRAM_ATTR buttonISRCW_Y() {
  buttonCW_Y.numberKeyPresses++;
  buttonCW_Y.pressed = true;
}

void IRAM_ATTR buttonISRCCW_Y() {
  buttonCCW_Y.numberKeyPresses++;
  buttonCCW_Y.pressed = true;
}

WebServer server(80);

//Clockwise
void moveCCW_X() {
  digitalWrite(DIR_X, LOW);
  digitalWrite(ENA_X, LOW);  //TB6600 that I tested needs this LOW. Other drivers may need HIGH
  digitalWrite(PUL_X, HIGH);
  delayMicroseconds(microsecs_X);
  digitalWrite(PUL_X, LOW);
  delayMicroseconds(microsecs_X);
}

//Counter Clockwise
void moveCW_X() {
  digitalWrite(DIR_X, HIGH);
  digitalWrite(ENA_X, LOW);  //TB6600 that I tested needs this LOW. Other drivers may need HIGH
  digitalWrite(PUL_X, HIGH);
  delayMicroseconds(microsecs_X);
  digitalWrite(PUL_X, LOW);
  delayMicroseconds(microsecs_X);
}

// Counter Clockwise for Y-axis
void moveCW_Y() {
  digitalWrite(DIR_Y, HIGH);
  digitalWrite(ENA_Y, LOW);  // TB6600 that I tested needs this LOW. Other drivers may need HIGH
  digitalWrite(PUL_Y, HIGH);
  delayMicroseconds(microsecs_Y);
  digitalWrite(PUL_Y, LOW);
  delayMicroseconds(microsecs_Y);
}

// Clockwise for Y-axis
void moveCCW_Y() {
  digitalWrite(DIR_Y, LOW);
  digitalWrite(ENA_Y, LOW);  // TB6600 that I tested needs this LOW. Other drivers may need HIGH
  digitalWrite(PUL_Y, HIGH);
  delayMicroseconds(microsecs_Y);
  digitalWrite(PUL_Y, LOW);
  delayMicroseconds(microsecs_Y);
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
              "input[type=submit][name=button5] {background-color: lightyellow;}"
              "input[type=submit][name=button6] {background-color: lightyellow;}"
              "input[type=submit][name=button7] {background-color: lightgreen;}"
              "input[type=submit][name=button8] {background-color: lightblue;}"
              ".center {display: flex; justify-content: center; align-items: center; height: 100vh;}"
              "</style></head><body><h1>Arduino Simple Stepper Controller Web Server</h1>"
              "<form action='/button1'><table>"
              "<tr><th><h4 style='font-size:25px;color: black'>The number of steps_X to traverse:</h4></th><td><input type='text' name='textbox1' style='font-size:25px;height:50px;width:200px' ></td>"
              "<td><input type='submit' name='button1' value='STEPS' style='font-size:25px;height:50px;width:200px' ></td></tr>"
              "<tr><td><h4 style='font-size:25px;color: green'> Steps [current value]: "
                + String(steps_X) + "</h4></td></tr>"
                                    "</table></form><br>"
                                    "<form action='/button2'><table>"
                                    "<tr><th><h4 style='font-size:25px;color: black'>Delay between pulses (microsecs_X):</h4></th><td><input type='text' name='textbox2' style='font-size:25px;height:50px;width:200px' ></td>"
                                    "<td><input type='submit' name='button2' value='MICROSECS' style='font-size:25px;height:50px;width:200px' ></td></tr>"
                                    "<tr><td><h4 style='font-size:25px;color: green'> Microsecs [current value]: "
                + String(microsecs_X) + "</h4></td></tr>"
                                        "</table></form><br>"
                                        "<form action='/button3'><table>"
                                        "<tr><td><input type='submit' name='button3' value='Clockwise (X)' style='font-size:25px;height:200px;width:300px' ></td></tr>"
                                        "</table></form><br><br><br>"
                                        "<form action='/button4'><table>"
                                        "<tr><td><input type='submit' name='button4' value='CounterClockwise (X)' style='font-size:25px;height:200px;width:300px' ></td></tr>"
                                        "</table></form><br>"
                                        "<form action='/button5'><table>"
                                        "<tr><th><h4 style='font-size:25px;color: black'>The number of steps_Y to traverse:</h4></th><td><input type='text' name='textbox5' style='font-size:25px;height:50px;width:200px' ></td>"
                                        "<td><input type='submit' name='button5' value='STEPS' style='font-size:25px;height:50px;width:200px' ></td></tr>"
                                        "<tr><td><h4 style='font-size:25px;color: green'> Steps [current value]: "
                + String(steps_Y) + "</h4></td></tr>"
                                    "</table></form><br>"
                                    "<form action='/button6'><table>"
                                    "<tr><th><h4 style='font-size:25px;color: black'>Delay between pulses (microsecs_Y):</h4></th><td><input type='text' name='textbox6' style='font-size:25px;height:50px;width:200px' ></td>"
                                    "<td><input type='submit' name='button6' value='MICROSECS' style='font-size:25px;height:50px;width:200px' ></td></tr>"
                                    "<tr><td><h4 style='font-size:25px;color: green'> Microsecs [current value]: "
                + String(microsecs_Y) + "</h4></td></tr>"
                                        "</table></form><br>"
                                        "<form action='/button7'><table>"
                                        "<tr><td><input type='submit' name='button7' value='Clockwise (Y)' style='font-size:25px;height:200px;width:300px' ></td></tr>"
                                        "</table></form><br><br><br>"
                                        "<form action='/button8'><table>"
                                        "<tr><td><input type='submit' name='button8' value='CounterClockwise (Y)' style='font-size:25px;height:200px;width:300px' ></td></tr>"
                                        "</table></form><br>"
                                        "<div class='center'>"
                                        "<h4>Copyright (c) 2023 Pratik M Tambe <enthusiasticgeek@gmail.com> [Software released under MIT License]</h4>"
                                        "</div>"
                                        "</body></html>");
}

void handleButton1() {
  String textbox1_value = server.arg("textbox1");
  if (isInteger(textbox1_value)) {
    steps_X = textbox1_value.toInt();
    if (steps_X == 0) {
      steps_X = DEFAULT_STEPS;  //change to default
    }
    //TODO(Pratik) - restrict the steps_X
    //server.send(200, "text/plain", "Button 1 was pressed with value: " + String(value));
    Serial.println("\nButton 1 (steps_X) was pressed with value: " + String(steps_X));
  } else {
    //server.send(200, "text/plain", "Invalid input for text box 1");
    Serial.println("\nInvalid input for text box 1 (steps_X): ");
  }
  //handleRoot();
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void handleButton2() {
  String textbox2_value = server.arg("textbox2");
  if (isInteger(textbox2_value)) {
    microsecs_X = textbox2_value.toInt();
    if (microsecs_X == 0) {
      microsecs_X = DEFAULT_MICROSECS;  //change to default
    }
    //TODO(Pratik) - restrict the microsecs_X
    //server.send(200, "text/plain", "Button 2 was pressed with value: " + String(value));
    Serial.println("\nButton 2 (delay microsecs_X) was pressed with value: " + String(microsecs_X));
  } else {
    //server.send(200, "text/plain", "Invalid input for text box 2");
    Serial.println("\nInvalid input for text box 2 (delay microsecs_X): ");
  }
  //handleRoot();
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}


void handleButton3() {
  String textbox3_value = server.arg("textbox3");
  if (isInteger(textbox3_value)) {
    microsecs_Y = textbox3_value.toInt();
    if (microsecs_Y == 0) {
      microsecs_Y = DEFAULT_MICROSECS;  //change to default
    }
    //TODO(Pratik) - restrict the microsecs_Y
    //server.send(200, "text/plain", "Button 3 was pressed with value: " + String(value));
    Serial.println("\nButton 3 (delay microsecs_Y) was pressed with value: " + String(microsecs_Y));
  } else {
    //server.send(200, "text/plain", "Invalid input for text box 3");
    Serial.println("\nInvalid input for text box 3 (delay microsecs_Y): ");
  }
  //handleRoot();
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void handleButton4() {
  String textbox4_value = server.arg("textbox4");
  if (isInteger(textbox4_value)) {
    microsecs_Y = textbox4_value.toInt();
    if (microsecs_Y == 0) {
      microsecs_Y = DEFAULT_MICROSECS;  //change to default
    }
    //TODO(Pratik) - restrict the microsecs_Y
    //server.send(200, "text/plain", "Button 4 was pressed with value: " + String(value));
    Serial.println("\nButton 4 (delay microsecs_Y) was pressed with value: " + String(microsecs_Y));
  } else {
    //server.send(200, "text/plain", "Invalid input for text box 4");
    Serial.println("\nInvalid input for text box 4 (delay microsecs_Y): ");
  }
  //handleRoot();
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void webbuttonCW_X() {
  for (int i = 0; i < steps_X; i++) {
    moveCW_X();
    Serial.println(String(i));
  }
  Serial.println("[Motor X] Push button CW is pressed and CW move is complete.");
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

void webbuttonCCW_X() {
  for (int i = 0; i < steps_X; i++) {
    moveCCW_X();
    Serial.println(String(i));
  }
  Serial.println("[Motor X] Push button CCW is pressed and CCW move is complete.");
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


void webbuttonCW_Y() {
  for (int i = 0; i < steps_Y; i++) {
    moveCW_Y();
    Serial.println(String(i));
  }
  Serial.println("[Motor Y] Push button CW is pressed and CW move is complete.");
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

void webbuttonCCW_Y() {
  for (int i = 0; i < steps_Y; i++) {
    moveCCW_Y();
    Serial.println(String(i));
  }
  Serial.println("[Motor Y] Push button CCW is pressed and CCW move is complete.");
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
//  pushButtonPressedCW = !digitalRead(pushbuttonPinCW_X); // Toggle pushButtonPressedCW variable
//}

//void handlePushButtonInterruptCCW() {
//  pushButtonPressedCCW = !digitalRead(pushbuttonPinCCW_X); // Toggle pushButtonPressedCCW variable
//}



// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(BAUD_RATE);
  pinMode(PUL_X, OUTPUT);
  pinMode(DIR_X, OUTPUT);
  pinMode(ENA_X, OUTPUT);

  // initialize the pushbutton pins as an input:
  pinMode(buttonCW_X.PIN, INPUT_PULLUP);
  attachInterrupt(buttonCW_X.PIN, buttonISRCW_X, FALLING);

  pinMode(buttonCCW_X.PIN, INPUT_PULLUP);
  attachInterrupt(buttonCCW_X.PIN, buttonISRCCW_X, FALLING);

  pinMode(PUL_Y, OUTPUT);
  pinMode(DIR_Y, OUTPUT);
  pinMode(ENA_Y, OUTPUT);

  // initialize the pushbutton pins as an input:
  pinMode(buttonCW_Y.PIN, INPUT_PULLUP);
  attachInterrupt(buttonCW_Y.PIN, buttonISRCW_Y, FALLING);

  pinMode(buttonCCW_Y.PIN, INPUT_PULLUP);
  attachInterrupt(buttonCCW_Y.PIN, buttonISRCCW_Y, FALLING);

  // Start WiFi in Access Point mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println("WiFi AP mode started");
  Serial.println(WiFi.softAPIP());

  // Set up web server
  server.on("/", handleRoot);
  server.on("/button1", handleButton1);
  server.on("/button2", handleButton2);
  server.on("/button3", webbuttonCW_X);
  server.on("/button4", webbuttonCCW_X);

  server.on("/button5", handleButton3);
  server.on("/button6", handleButton4);
  server.on("/button7", webbuttonCW_Y);
  server.on("/button8", webbuttonCCW_Y);

  server.begin();

  Serial.println("Web server started");
}

//https://stackoverflow.com/questions/5697047/convert-serial-read-into-a-usable-string-using-arduino



// the loop function runs over and over again forever
void loop() {
  while (Serial.available() > 0) {
    char recieved = Serial.read();
    inData += recieved;
    // Process message when return character is recieved
    if (recieved == '\r') {
      Serial.println("=== Type 'help' for detailed instructions! ===");
      Serial.print("\nArduino Received: ");
      Serial.print(inData);
      Serial.print("\n");

      MatchState ms;
      ms.Target((char*)inData.c_str());
      char result = ms.Match("(%a+)=(%d+)", 0);
      if (result == REGEXP_MATCHED) {
        // matching offsets in ms.capture
        char buf[100];  // large enough to hold expected string
        Serial.print("Captures: ");
        Serial.println(ms.level);
        for (int j = 0; j < ms.level; j++) {
          //Serial.print ("Capture number: ");
          //Serial.println (j, DEC);
          //Serial.print ("Text: '");
          //Serial.print (ms.GetCapture (buf, j));
          //Serial.println ("'");
          //first is a string based on Regex capture group
          if ((j == 0) && (ms.level == 2)) {
            String captured = ms.GetCapture(buf, 0);
            String value = ms.GetCapture(buf, 1);
            if (captured == "steps_X") {
              steps_X = captured.toInt();
              //TODO(Pratik) - check the bounds (need to be valid)
              Serial.println("steps_X are now " + value);
            } else if (captured == "microsecs_X") {
              microsecs_X = captured.toInt();
              //TODO(Pratik) - check the bounds (need to be valid)
              Serial.println("microsecs_X are now " + value);
            } else if (captured == "steps_Y") {
              steps_Y = captured.toInt();
              //TODO(Pratik) - check the bounds (need to be valid)
              Serial.println("steps_Y are now " + value);
            } else if (captured == "microsecs_Y") {
              microsecs_Y = captured.toInt();
              //TODO(Pratik) - check the bounds (need to be valid)
              Serial.println("microsecs_Y are now " + value);
            }
          }
        }
      } else if (result == REGEXP_NOMATCH) {
        // no match
        //Serial.println("no match");
      }
      //TODO(Pratik) - Add more instructions as necessary
      if ((inData == "cw_x\r") || (inData == "CW_X\r")) {
        Serial.println("Motor X will turn clockwise\n");
        for (int i = 0; i < steps_X; i++) {
          moveCW_X();
        }
        Serial.println("Motor X finished turning clockwise\n");
      } else if ((inData == "ccw_x\r") || (inData == "CCW_X\r")) {
        Serial.println("Motor X will turn counterclockwise\n");
        for (int i = 0; i < steps_X; i++) {
          moveCCW_X();
        }
        Serial.println("Motor X finished turning counterclockwise\n");
      } else if ((inData == "cw_y\r") || (inData == "CW_Y\r")) {
        Serial.println("Motor Y will turn clockwise\n");
        for (int i = 0; i < steps_Y; i++) {
          moveCW_Y();
        }
        Serial.println("Motor Y finished turning clockwise\n");
      } else if ((inData == "ccw_y\r") || (inData == "CCW_Y\r")) {
        Serial.println("Motor Y will turn counterclockwise\n");
        for (int i = 0; i < steps_Y; i++) {
          moveCCW_Y();
        }
        Serial.println("Motor Y finished turning counterclockwise\n");
      } else if ((inData == "en_x=1\r") || (inData == "EN_X=1\r")) {
        Serial.println("Motor X enabled\n");
        digitalWrite(ENA_X, HIGH);
      } else if ((inData == "en_x=0\r") || (inData == "EN_X=0\r")) {
        Serial.println("Motor X disabled\n");
        digitalWrite(ENA_X, LOW);
      } else if ((inData.indexOf("steps_X:") >= 0)) {
        //int ind1 = inData.indexOf('steps_X');  //finds location of first ,
        //int speed = inData.substring(0, ind1+6);
        Serial.println("steps_X\n");
      } else if ((inData == "en_y=1\r") || (inData == "EN_Y=1\r")) {
        Serial.println("Motor X enabled\n");
        digitalWrite(ENA_Y, HIGH);
      } else if ((inData == "en_y=0\r") || (inData == "EN_Y=0\r")) {
        Serial.println("Motor X disabled\n");
        digitalWrite(ENA_Y, LOW);
      } else if ((inData.indexOf("steps_Y:") >= 0)) {
        //int ind1 = inData.indexOf('steps_Y');  //finds location of first ,
        //int speed = inData.substring(0, ind1+6);
        Serial.println("steps_Y\n");
      } else if (inData == "help\r") {  // DON'T forget to add "\n" at the end of the string.
        Serial.println("\nValid commands are: \r\n 'CW_X' or 'CW_Y' (clockwise),\r\n 'CCW_X' or 'CCW_Y' (counterclockwise),\r\n 'EN_X=1' or 'EN_Y=1' (enable),\r\n 'EN_X=0' or 'EN_Y=0' (disable),\r\n 'steps_X=<value>' or 'steps_Y=<value>', \r\n 'microsecs_X=<value>' or 'microsecs_Y=<value>'.");
      } else {
        //Any other remaining case?
      }
      inData = "";  // Clear recieved buffer
    }
  }

  // Check if manually someone presses push buttons.
  // Read the state of the pushbutton value.
  // Added a guard so that both clockwise and counterclockwise push buttons are not pressed simultaneously!
  if ((buttonCW_X.pressed) && (buttonCCW_X.pressed)) {
    Serial.printf("CCW and CW push buttons should not be pressed simultaneously. Ignoring the commands!\n");
    buttonCW_X.pressed = false;
    buttonCCW_X.pressed = false;

  } else if (buttonCW_X.pressed) {
    Serial.printf("Button has been pressed %u times\n", buttonCW_X.numberKeyPresses);
    // move CW:
    Serial.println("Motor will turn clockwise\n");
    moveCW_X();
    buttonCW_X.pressed = false;
  } else if (buttonCCW_X.pressed) {
    Serial.printf("Button has been pressed %u times\n", buttonCCW_X.numberKeyPresses);
    // move CCW:
    Serial.println("Motor will turn counterclockwise\n");
    moveCCW_X();
    buttonCCW_X.pressed = false;
  }
  if ((buttonCW_Y.pressed) && (buttonCCW_Y.pressed)) {
    Serial.printf("CCW and CW push buttons should not be pressed simultaneously. Ignoring the commands!\n");
    buttonCW_Y.pressed = false;
    buttonCCW_Y.pressed = false;
  } else if (buttonCW_Y.pressed) {
    Serial.printf("Button has been pressed %u times\n", buttonCW_Y.numberKeyPresses);
    // move CW:
    Serial.println("Motor will turn clockwise\n");
    moveCW_Y();
    buttonCW_Y.pressed = false;
  } else if (buttonCCW_Y.pressed) {
    Serial.printf("Button has been pressed %u times\n", buttonCCW_Y.numberKeyPresses);
    // move CCW:
    Serial.println("Motor will turn counterclockwise\n");
    moveCCW_Y();
    buttonCCW_Y.pressed = false;
  }

  //call the webserver
  server.handleClient();
}
