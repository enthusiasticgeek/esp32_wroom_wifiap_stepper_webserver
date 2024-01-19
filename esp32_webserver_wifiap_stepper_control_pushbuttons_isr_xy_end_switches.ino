/*
Pratik M Tambe
References:
https://www.makerguides.com/esp32-and-tb6600-stepper-motor-driver/
*/
//This code is designed to control two stepper motors with a webserver on ESP32 -> X - axis and Y - axis (with end switches on both axes).

#include <Regexp.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "nca_atm_ap";
const char* password = "NcaAtmPassword";

//Change this webserver Wi-Fi AP mode IP as desired
const IPAddress customIP(192, 168, 4, 1);

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


//end switches (e.g. to stop the motor run when either mechanical or optical sensors detect the end of run e.g. on worm gear drive)
//X-axis
#define buttonPinBegin_X 36
#define buttonPinEnd_X 39

//Y-axis
#define buttonPinBegin_Y 32
#define buttonPinEnd_Y  33


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



//Buttons for optical encoder or mechanical switches

// variables will change:
int buttonStateBegin_X = 0;  // variable for reading the pushbutton status
// variables will change:
int buttonStateEnd_X = 0;  // variable for reading the pushbutton status

// variables will change:
int buttonStateBegin_Y = 0;  // variable for reading the pushbutton status
// variables will change:
int buttonStateEnd_Y = 0;  // variable for reading the pushbutton status

Button buttonBegin_X = { buttonPinBegin_X, 0, false };    //X begin
Button buttonEnd_X = { buttonPinEnd_X, 0, false };  //X end

Button buttonBegin_Y = { buttonPinBegin_Y, 0, false };    //X begin
Button buttonEnd_Y = { buttonPinEnd_Y, 0, false };  //X end


void IRAM_ATTR buttonISRBegin_X() {
  buttonBegin_X.numberKeyPresses++;
  buttonBegin_X.pressed = true;
}

void IRAM_ATTR buttonISREnd_X() {
  buttonEnd_X.numberKeyPresses++;
  buttonEnd_X.pressed = true;
}

void IRAM_ATTR buttonISRBegin_Y() {
  buttonBegin_Y.numberKeyPresses++;
  buttonBegin_Y.pressed = true;
}

void IRAM_ATTR buttonISREnd_Y() {
  buttonEnd_Y.numberKeyPresses++;
  buttonEnd_Y.pressed = true;
}


void resetCountersX() {
  buttonCW_X.numberKeyPresses = 0;
  buttonCCW_X.numberKeyPresses = 0;
  buttonBegin_X.numberKeyPresses = 0;
  buttonEnd_X.numberKeyPresses = 0;
}

void resetCountersY() {
  buttonCW_Y.numberKeyPresses = 0;
  buttonCCW_Y.numberKeyPresses = 0;
  buttonBegin_Y.numberKeyPresses = 0;
  buttonEnd_Y.numberKeyPresses = 0;
}

void resetCounters() {
  resetCountersX();
  resetCountersY();
  Serial.println("All the counters have been reset to 0");
}


WebServer server(80);

//Clockwise
void moveCCW_X() {
  if ((buttonBegin_X.pressed == true) || (buttonEnd_X.pressed == true)){
    return;
  }
  digitalWrite(DIR_X, LOW);
  digitalWrite(ENA_X, LOW);  //TB6600 that I tested needs this LOW. Other drivers may need HIGH
  digitalWrite(PUL_X, HIGH);
  delayMicroseconds(microsecs_X);
  digitalWrite(PUL_X, LOW);
  delayMicroseconds(microsecs_X);
}

//Counter Clockwise
void moveCW_X() {
  if ((buttonBegin_X.pressed == true) || (buttonEnd_X.pressed == true)){
    return;
  }
  digitalWrite(DIR_X, HIGH);
  digitalWrite(ENA_X, LOW);  //TB6600 that I tested needs this LOW. Other drivers may need HIGH
  digitalWrite(PUL_X, HIGH);
  delayMicroseconds(microsecs_X);
  digitalWrite(PUL_X, LOW);
  delayMicroseconds(microsecs_X);
}

// Counter Clockwise for Y-axis
void moveCW_Y() {
  if ((buttonBegin_Y.pressed == true) || (buttonEnd_Y.pressed == true)){
    return;
  }
  digitalWrite(DIR_Y, HIGH);
  digitalWrite(ENA_Y, LOW);  // TB6600 that I tested needs this LOW. Other drivers may need HIGH
  digitalWrite(PUL_Y, HIGH);
  delayMicroseconds(microsecs_Y);
  digitalWrite(PUL_Y, LOW);
  delayMicroseconds(microsecs_Y);
}

// Clockwise for Y-axis
void moveCCW_Y() {
  if ((buttonBegin_Y.pressed == true) || (buttonEnd_Y.pressed == true)){
    return;
  }
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
              "input[type=submit][name=reset] {background-color: lightgrey;}"
              "input[type=submit][name=gpio] {background-color: lightsalmon;}"
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
                                        "</table></form><br><hr>"
                                        "<form action='/button5'><table>"
                                        "<tr><th><h4 style='font-size:25px;color: black'>The number of steps_Y to traverse:</h4></th><td><input type='text' name='textbox3' style='font-size:25px;height:50px;width:200px' ></td>"
                                        "<td><input type='submit' name='button5' value='STEPS' style='font-size:25px;height:50px;width:200px' ></td></tr>"
                                        "<tr><td><h4 style='font-size:25px;color: green'> Steps [current value]: "
                + String(steps_Y) + "</h4></td></tr>"
                                    "</table></form><br>"
                                    "<form action='/button6'><table>"
                                    "<tr><th><h4 style='font-size:25px;color: black'>Delay between pulses (microsecs_Y):</h4></th><td><input type='text' name='textbox4' style='font-size:25px;height:50px;width:200px' ></td>"
                                    "<td><input type='submit' name='button6' value='MICROSECS' style='font-size:25px;height:50px;width:200px' ></td></tr>"
                                    "<tr><td><h4 style='font-size:25px;color: green'> Microsecs [current value]: "
                + String(microsecs_Y) + "</h4></td></tr>"
                                        "</table></form><br>"
                                        "<form action='/button7'><table>"
                                        "<tr><td><input type='submit' name='button7' value='Clockwise (Y)' style='font-size:25px;height:200px;width:300px' ></td></tr>"
                                        "</table></form><br><br><br>"
                                        "<form action='/button8'><table>"
                                        "<tr><td><input type='submit' name='button8' value='CounterClockwise (Y)' style='font-size:25px;height:200px;width:300px' ></td></tr>"
                                        "</table></form><br><hr><br>"
                                        "<form action='/reset'><table>"
                                        "<tr><td><input type='submit' name='reset' value='Reset Counters' style='font-size:25px;height:200px;width:300px' ></td></tr>"
                                        "</table></form><br><hr><br>"
                                        "<form action='/gpio'><table>"
                                        "<tr><td><input type='submit' name='gpio' value='Pinouts' style='font-size:25px;height:200px;width:300px' ></td></tr>"
                                        "</table></form><br>"
                                        "<form action='/reached_begin_x'><table>"
                                        "<tr><td><input type='submit' name='reached_begin_x' value='Reached begin X?' style='display:none;' ></td></tr>"
                                        "</table></form>"
                                        "<form action='/reached_end_x'><table>"
                                        "<tr><td><input type='submit' name='reached_end_x' value='Reached end X?' style='display:none;' ></td></tr>"
                                        "</table></form>"
                                        "<form action='/reached_begin_y'><table>"
                                        "<tr><td><input type='submit' name='reached_begin_y' value='Reached begin Y?' style='display:none;' ></td></tr>"
                                        "</table></form>"
                                        "<form action='/reached_end_y'><table>"
                                        "<tr><td><input type='submit' name='reached_end_y' value='Reached end Y?' style='display:none;' ></td></tr>"
                                        "</table></form><br>"
                                        "<form action='/button_begin_presses_x'><table>"
                                        "<tr><td><input type='submit' name='button_begin_presses_x' value='Presses begin X?' style='display:none;' ></td></tr>"
                                        "</table></form>"
                                        "<form action='/button_end_presses_x'><table>"
                                        "<tr><td><input type='submit' name='button_end_presses_x' value='Presses end X?' style='display:none;' ></td></tr>"
                                        "</table></form>"
                                        "<form action='/button_begin_presses_y'><table>"
                                        "<tr><td><input type='submit' name='button_begin_presses_y' value='Presses begin Y?' style='display:none;' ></td></tr>"
                                        "</table></form>"
                                        "<form action='/button_end_presses_y'><table>"
                                        "<tr><td><input type='submit' name='button_end_presses_y' value='Presses end Y?' style='display:none;' ></td></tr>"
                                        "</table></form><br>"
                                        "<form action='/button_cw_presses_x'><table>"
                                        "<tr><td><input type='submit' name='button_cw_presses_x' value='Presses CW X?' style='display:none;' ></td></tr>"
                                        "</table></form>"
                                        "<form action='/button_ccw_presses_x'><table>"
                                        "<tr><td><input type='submit' name='button_ccw_presses_x' value='Presses CCW X?' style='display:none;' ></td></tr>"
                                        "</table></form>"
                                        "<form action='/button_cw_presses_y'><table>"
                                        "<tr><td><input type='submit' name='button_cw_presses_y' value='Presses CW Y?' style='display:none;' ></td></tr>"
                                        "</table></form>"
                                        "<form action='/button_ccw_presses_y'><table>"
                                        "<tr><td><input type='submit' name='button_ccw_presses_y' value='Presses CCW Y?' style='display:none;' ></td></tr>"
                                        "</table></form><br><hr>"
                                        "<div class='center'>"
                                        "<h4>Copyright (c) 2023 Pratik M Tambe <enthusiasticgeek@gmail.com> [Software released under MIT License]</h4>"
                                        "</div>"
                                        "</body></html>");
}


void handleGPIO() {
  String htmlContent = 
              "<!DOCTYPE html>"
              "<html>"
              "<head>"
              "<title>ESP32 Arduino Webserver GPIO</title>"
              "<style>"
              "body {"
              "  font-family: 'Courier New', monospace;"
              "  text-align: center;"
              "}"
              ".table-container {"
              "  margin: 20px auto;"
              "}"
              ".pin {"
              "  width: 30px;"
              "  height: 30px;"
              "  border: 1px solid black;"
              "  margin: 5px;"
              "  display: flex;"
              "  align-items: center;"
              "  justify-content: center;"
              "  font-weight: bold;"
              "}"
              ".pin1 {"
              "  width: 30px;"
              "  height: 30px;"
              "  border: 1px solid black;"
              "  margin: 5px;"
              "  align-items: center;"
              "  justify-content: center;"
              "  font-weight: bold;"
              "}"
              ".black {"
              "  background-color: black;"
              "  color: white;"
              "  font-weight: bold;"
              "}"
              ".red {"
              "  background-color: red;"
              "  color: white;"
              "  font-weight: bold;"
              "}"
              ".darkred {"
              "  background-color: darkred;"
              "  color: white;"
              "  font-weight: bold;"
              "}"
              ".indianred {"
              "  background-color: indianred;"
              "  color: white;"
              "  font-weight: bold;"
              "}"
              ".darkgreen {"
              "  background-color: darkgreen;"
              "  color: white;"
              "  font-weight: bold;"
              "}"
              ".seagreen {"
              "  background-color: seagreen;"
              "  color: white;"
              "  font-weight: bold;"
              "}"
              ".limegreen {"
              "  background-color: limegreen;"
              "  color: white;"
              "  font-weight: bold;"
              "}"
              ".forestgreen {"
              "  background-color: forestgreen;"
              "  color: white;"
              "  font-weight: bold;"
              "}"
              ".darkblue {"
              "  background-color: darkblue;"
              "  color: white;"
              "  font-weight: bold;"
              "}"
              ".brown {"
              "  background-color: brown;"
              "  color: white;"
              "  font-weight: bold;"
              "}"
              ".chocolate {"
              "  background-color: chocolate;"
              "  color: white;"
              "  font-weight: bold;"
              "}"
              ".pink {"
              "  background-color: pink;"
              "  color: white;"
              "  font-weight: bold;"
              "}"
              ".blueviolet {"
              "  background-color: blueviolet;"
              "  color: white;"
              "  font-weight: bold;"
              "}"
              "table, th, td {"
              "  border:1px solid black;"
              "}"
              "</style>"
              "</head>"
              "<body>"
              ""
              "<h2>ESP32Wroom32D GPIO Pinout Diagram</h2>"
              ""
              "<table style=\"width:100%\">"
              "  <tr>"
              "    <td class='pin1'>FUN 1</td>"
              "    <td class ='pin1'>FUN 2</td>"
              "    <td class ='pin1'>FUN 3</td>"
              "    <td class ='pin1'>PIN</td>"
              "    <td class ='pin1'>ESP32 FUN</td>"
              "    <td class ='pin1'>ESP32 FUN</td>"
              "    <td class='pin1'>PIN</td>"
              "    <td class ='pin1'>FUN 1</td>"
              "    <td class ='pin1'>FUN 2</td>"
              "    <td class ='pin1'>FUN 3</td>"
              "  </tr>"
              "  <tr>"
              "    <td class='pin1'></td>"
              "    <td class ='pin1'></td>"
              "    <td class ='red'>3.3V</td>"
              "    <td class ='pin1'>1</td>"
              "    <td class ='pin1'></td>"
              "    <td class ='pin1'></td>"
              "    <td class='pin1'>38</td>"
              "    <td class ='black'>GND</td>"
              "    <td class ='pin1'></td>"
              "    <td class ='pin1'></td>"
              "  </tr>"
              "  <tr>"
              "    <td class='pin1'></td>"
              "    <td class ='darkred'>RESET</td>"
              "    <td class ='darkred'>EN</td>"
              "    <td class ='pin1'>2</td>"
              "    <td class ='pin1'></td>"
              "    <td class ='pin1'>PUL Y</td>"
              "    <td class='pin1'>37</td>"
              "    <td class ='darkgreen'>GIOP23</td>"
              "    <td class ='chocolate'>VSPI MOSI</td>"
              "    <td class ='pin1'></td>"
              "  </tr>"
              "  <tr>"
              "    <td class='pin1'></td>"
              "    <td class ='limegreen'>ADC0</td>"
              "    <td class ='darkgreen'>GIOP36</td>"
              "    <td class ='pin1'>3</td>"
              "    <td class ='pin1'>BEGIN X</td>"
              "    <td class ='pin1'>DIR Y</td>"
              "    <td class='pin1'>36</td>"
              "    <td class ='darkgreen'>GIOP22</td>"
              "    <td class ='brown'>I2C SCL</td>"
              "    <td class ='pin1'></td>"
              "  </tr>"
              "  <tr>"
              "    <td class='pin1'></td>"
              "    <td class ='limegreen'>ADC3</td>"
              "    <td class ='darkgreen'>GIOP39</td>"
              "    <td class ='pin1'>4</td>"
              "    <td class ='pin1'>END X</td>"
              "    <td class ='pin1'></td>"
              "    <td class='pin1'>35</td>"
              "    <td class ='darkgreen'>GIOP1</td>"
              "    <td class ='brown'>TX0</td>"
              "    <td class ='pin1'></td>"
              "  </tr>"
              "  <tr>"
              "    <td class='pin1'></td>"
              "    <td class ='limegreen'>ADC6</td>"
              "    <td class ='darkgreen'>GIOP34</td>"
              "    <td class ='pin1'>5</td>"
              "    <td class ='pin1'></td>"
              "    <td class ='pin1'></td>"
              "    <td class='pin1'>34</td>"
              "    <td class ='darkgreen'>GIOP3</td>"
              "    <td class ='brown'>RX0</td>"
              "    <td class ='pin1'></td>"
              "  </tr>"
              "  <tr>"
              "    <td class='pin1'></td>"
              "    <td class ='limegreen'>ADC7</td>"
              "    <td class ='darkgreen'>GIOP35</td>"
              "    <td class ='pin1'>6</td>"
              "    <td class ='pin1'></td>"
              "    <td class ='pin1'>ENA Y</td>"
              "    <td class='pin1'>33</td>"
              "    <td class ='darkgreen'>GIOP21</td>"
              "    <td class ='brown'>I2C SDA</td>"
              "    <td class ='pin1'></td>"
              "  </tr>"
              "  <tr>"
              "    <td class='blueviolet'>TOUCH9</td>"
              "    <td class ='limegreen'>ADC4</td>"
              "    <td class ='darkgreen'>GIOP32</td>"
              "    <td class ='pin1'>7</td>"
              "    <td class ='pin1'>BEGIN Y</td>"
              "    <td class ='pin1'></td>"
              "    <td class='pin1'>32</td>"
              "    <td class ='black'>GND</td>"
              "    <td class ='pin1'></td>"
              "    <td class ='pin1'></td>"
              "  </tr>"
              "  <tr>"
              "    <td class='blueviolet'>TOUCH8</td>"
              "    <td class ='limegreen'>ADC5</td>"
              "    <td class ='darkgreen'>GIOP33</td>"
              "    <td class ='pin1'>8</td>"
              "    <td class ='pin1'>END Y</td>"
              "    <td class ='pin1'></td>"
              "    <td class='pin1'>31</td>"
              "    <td class ='darkgreen'>GIOP19</td>"
              "    <td class ='chocolate'>VSPI MISO</td>"
              "    <td class ='pin1'></td>"
              "  </tr>"
              "  <tr>"
              "    <td class='chocolate'>DAC1</td>"
              "    <td class ='limegreen'>ADC18</td>"
              "    <td class ='darkgreen'>GIOP25</td>"
              "    <td class ='pin1'>9</td>"
              "    <td class ='pin1'>PUL X</td>"
              "    <td class ='pin1'></td>"
              "    <td class='pin1'>30</td>"
              "    <td class ='darkgreen'>GIOP18</td>"
              "    <td class ='chocolate'>VSPI SCK</td>"
              "    <td class ='pin1'></td>"
              "  </tr>"
              "  <tr>"
              "    <td class='chocolate'>DAC2</td>"
              "    <td class ='limegreen'>ADC19</td>"
              "    <td class ='darkgreen'>GIOP26</td>"
              "    <td class ='pin1'>10</td>"
              "    <td class ='pin1'>DIR X</td>"
              "    <td class ='pin1'></td>"
              "    <td class='pin1'>29</td>"
              "    <td class ='darkgreen'>GIOP5</td>"
              "    <td class ='chocolate'>VSPI SS</td>"
              "    <td class ='pin1'></td>"
              "  </tr>"
              "  <tr>"
              "    <td class='blueviolet'>TOUCH7</td>"
              "    <td class ='limegreen'>ADC17</td>"
              "    <td class ='darkgreen'>GIOP27</td>"
              "    <td class ='pin1'>11</td>"
              "    <td class ='pin1'> ENA X</td>"
              "    <td class ='pin1'></td>"
              "    <td class='pin1'>28</td>"
              "    <td class ='darkgreen'>GIOP17</td>"
              "    <td class ='brown'>TX2</td>"
              "    <td class ='pin1'></td>"
              "  </tr>"
              "  <tr>"
              "    <td class='blueviolet'>TOUCH6</td>"
              "    <td class ='limegreen'>ADC16</td>"
              "    <td class ='darkgreen'>GIOP14</td>"
              "    <td class ='pin1'>12</td>"
              "    <td class ='pin1'>BTN CCW X</td>"
              "    <td class ='pin1'></td>"
              "    <td class='pin1'>27</td>"
              "    <td class ='darkgreen'>GIOP16</td>"
              "    <td class ='brown'>RX2</td>"
              "    <td class ='pin1'></td>"
              "  </tr>"
              "  <tr>"
              "    <td class='blueviolet'>TOUCH5</td>"
              "    <td class ='limegreen'>ADC15</td>"
              "    <td class ='darkgreen'>GIOP12</td>"
              "    <td class ='pin1'>13</td>"
              "    <td class ='pin1'></td>"
              "    <td class ='pin1'>BTN CW Y</td>"
              "    <td class='pin1'>26</td>"
              "    <td class ='darkgreen'>GIOP4</td>"
              "    <td class ='limegreen'>ADC10</td>"
              "    <td class ='blueviolet'>TOUCH0</td>"
              "  </tr>"
              "  <tr>"
              "    <td class='pin1'></td>"
              "    <td class ='pin1'></td>"
              "    <td class ='black'>GND</td>"
              "    <td class ='pin1'>14</td>"
              "    <td class ='pin1'></td>"
              "    <td class ='pin1'></td>"
              "    <td class='pin1'>25</td>"
              "    <td class ='darkgreen'>GIOP0</td>"
              "    <td class ='limegreen'>ADC11</td>"
              "    <td class ='blueviolet'>TOUCH1</td>"
              "  </tr>"
              "  <tr>"
              "    <td class='blueviolet'>TOUCH4</td>"
              "    <td class ='limegreen'>ADC14</td>"
              "    <td class ='darkgreen'>GIOP13</td>"
              "    <td class ='pin1'>15</td>"
              "    <td class ='pin1'>BTN CW X</td>"
              "    <td class ='pin1'>BTN CCW Y</td>"
              "    <td class='pin1'>24</td>"
              "    <td class ='darkgreen'>GIOP2</td>"
              "    <td class ='limegreen'>ADC12</td>"
              "    <td class ='blueviolet'>TOUCH2</td>"
              "  </tr>"
              "  <tr>"
              "    <td class='brown'>RX1</td>"
              "    <td class ='seagreen'>FLASH D2</td>"
              "    <td class ='darkgreen'>GIOP9</td>"
              "    <td class ='pin1'>16</td>"
              "    <td class ='pin1'></td>"
              "    <td class ='pin1'></td>"
              "    <td class='pin1'>23</td>"
              "    <td class ='darkgreen'>GIOP15</td>"
              "    <td class ='limegreen'>ADC13</td>"
              "    <td class ='blueviolet'>TOUCH3</td>"
              "  </tr>"
              "  <tr>"
              "    <td class='brown'>TX1</td>"
              "    <td class ='seagreen'>FLASH D3</td>"
              "    <td class ='darkgreen'>GIOP10</td>"
              "    <td class ='pin1'>17</td>"
              "    <td class ='pin1'></td>"
              "    <td class ='pin1'></td>"
              "    <td class='pin1'>22</td>"
              "    <td class ='darkgreen'>GIOP8</td>"
              "    <td class ='seagreen'>FLASH D1</td>"
              "    <td class ='pin1'></td>"
              "  </tr>"
              "  <tr>"
              "    <td class='pin1'></td>"
              "    <td class ='seagreen'>FLASH CMD</td>"
              "    <td class ='darkgreen'>GIOP11</td>"
              "    <td class ='pin1'>18</td>"
              "    <td class ='pin1'></td>"
              "    <td class ='pin1'></td>"
              "    <td class='pin1'>21</td>"
              "    <td class ='darkgreen'>GIOP7</td>"
              "    <td class ='seagreen'>FLASH D0</td>"
              "    <td class ='pin1'></td>"
              "  </tr>"
              "  <tr>"
              "    <td class='pin1'></td>"
              "    <td class ='pin1'></td>"
              "    <td class ='indianred'>VIN 5V</td>"
              "    <td class ='pin1'>19</td>"
              "    <td class ='pin1'></td>"
              "    <td class ='pin1'></td>"
              "    <td class='pin1'>20</td>"
              "    <td class ='darkgreen'>GIOP6</td>"
              "    <td class ='seagreen'>FLASH CK</td>"
              "    <td class ='pin1'></td>"
              "  </tr>"
              "  <tr>"
                 "    <td colspan='10'><button onclick='window.location=\"/\"'>Return to Home</button></td>"
              "  </tr>"
              "</table>"
              ""
              "<p><b>ESP32Wroom32D based webserver to control two independent stepper motors.</b></p>"
              ""
              "</body>"
              "</html>";

  server.send(200, "text/html", htmlContent);
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
    steps_Y = textbox3_value.toInt();
    if (steps_Y == 0) {
      steps_Y = DEFAULT_STEPS;  //change to default
    }
    //TODO(Pratik) - restrict the steps_X
    //server.send(200, "text/plain", "Button 3 was pressed with value: " + String(value));
    Serial.println("\nButton 3 (steps_Y) was pressed with value: " + String(steps_Y));
  } else {
    //server.send(200, "text/plain", "Invalid input for text box 3");
    Serial.println("\nInvalid input for text box 3 (steps_Y): ");
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
    buttonCW_X.numberKeyPresses++;
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
    buttonCCW_X.numberKeyPresses++;
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
    buttonCW_Y.numberKeyPresses++;
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
    buttonCCW_Y.numberKeyPresses++;
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

void handleReset() {
  resetCounters();
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

//Check if motors X and Y axes reached begin or end position
void handleReachedBeginX() {
  String response = ( buttonBegin_X.pressed == true) ? "true" : "false";
  server.send(200, "text/plain", response);
}

void handleReachedEndX() {
  String response = ( buttonEnd_X.pressed == true) ? "true" : "false";
  server.send(200, "text/plain", response);
}

void handleReachedBeginY() {
  String response = ( buttonBegin_Y.pressed == true) ? "true" : "false";
  server.send(200, "text/plain", response);
}

void handleReachedEndY() {
  String response = ( buttonEnd_Y.pressed == true) ? "true" : "false";
  server.send(200, "text/plain", response);
}

//void handlePushButtonInterruptCW() {
//  pushButtonPressedCW = !digitalRead(pushbuttonPinCW_X); // Toggle pushButtonPressedCW variable
//}

//void handlePushButtonInterruptCCW() {
//  pushButtonPressedCCW = !digitalRead(pushbuttonPinCCW_X); // Toggle pushButtonPressedCCW variable
//}

// Begin/End switches - X and Y axes number of presses

void handleButtonBeginPressesX() {
  String response = String(buttonBegin_X.numberKeyPresses);
  server.send(200, "text/plain", response);
}

void handleButtonEndPressesX() {
  String response = String(buttonEnd_X.numberKeyPresses);
  server.send(200, "text/plain", response);
}

void handleButtonBeginPressesY() {
  String response = String(buttonBegin_Y.numberKeyPresses);
  server.send(200, "text/plain", response);
}

void handleButtonEndPressesY() {
  String response = String(buttonEnd_Y.numberKeyPresses);
  server.send(200, "text/plain", response);
}

// CW/CCW - X and Y axes number of presses

void handleButtonCWPressesX() {
  String response = String(buttonCW_X.numberKeyPresses);
  server.send(200, "text/plain", response);
}

void handleButtonCCWPressesX() {
  String response = String(buttonCCW_X.numberKeyPresses);
  server.send(200, "text/plain", response);
}

void handleButtonCWPressesY() {
  String response = String(buttonCW_Y.numberKeyPresses);
  server.send(200, "text/plain", response);
}

void handleButtonCCWPressesY() {
  String response = String(buttonCCW_Y.numberKeyPresses);
  server.send(200, "text/plain", response);
}

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

  // initialize end switches
  pinMode(buttonBegin_X.PIN, INPUT_PULLUP);
  attachInterrupt(buttonBegin_X.PIN, buttonISRBegin_X, FALLING);

  pinMode(buttonEnd_X.PIN, INPUT_PULLUP);
  attachInterrupt(buttonEnd_X.PIN, buttonISREnd_X, FALLING);

  pinMode(buttonBegin_Y.PIN, INPUT_PULLUP);
  attachInterrupt(buttonBegin_Y.PIN, buttonISRBegin_Y, FALLING);

  pinMode(buttonEnd_Y.PIN, INPUT_PULLUP);
  attachInterrupt(buttonEnd_Y.PIN, buttonISREnd_Y, FALLING);

  // Start WiFi in Access Point mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  // Set custom IP address for the AP mode
  WiFi.softAPConfig(customIP, IPAddress(0, 0, 0, 0), IPAddress(255, 255, 255, 0));

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

  server.on("/reset", handleReset);

  server.on("/reached_begin_x", handleReachedBeginX);
  server.on("/reached_end_x", handleReachedEndX);
  server.on("/reached_begin_y", handleReachedBeginY);
  server.on("/reached_end_y", handleReachedEndY);

  server.on("/button_begin_presses_x", handleButtonBeginPressesX);
  server.on("/button_end_presses_x", handleButtonEndPressesX);
  server.on("/button_begin_presses_y", handleButtonBeginPressesY);
  server.on("/button_end_presses_y", handleButtonEndPressesY);

  server.on("/button_cw_presses_x", handleButtonCWPressesX);
  server.on("/button_ccw_presses_x", handleButtonCCWPressesX);
  server.on("/button_cw_presses_y", handleButtonCWPressesY);
  server.on("/button_ccw_presses_y", handleButtonCCWPressesY);

  server.on("/gpio", handleGPIO);

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
        Serial.println("Motor Y enabled\n");
        digitalWrite(ENA_Y, HIGH);
      } else if ((inData == "en_y=0\r") || (inData == "EN_Y=0\r")) {
        Serial.println("Motor Y disabled\n");
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

  if ((buttonBegin_X.pressed) && (buttonEnd_X.pressed)) {
    Serial.printf("Begin and End push buttons should not be pressed simultaneously. Ignoring the commands!\n");
    buttonBegin_X.pressed = false;
    buttonEnd_X.pressed = false;
  } else if (buttonBegin_X.pressed) {
    Serial.printf("Button has been pressed %u times\n", buttonBegin_X.numberKeyPresses);
    // reached starting point
    Serial.println("Motor X reached starting point\n");
    //some_buttonBegin_X();
    buttonBegin_X.pressed = false;
  } else if (buttonEnd_X.pressed) {
    Serial.printf("Button has been pressed %u times\n", buttonEnd_X.numberKeyPresses);
    // reached ending point
    Serial.println("Motor X reached ending point\n");
    //some_buttonEnd_X();
    buttonEnd_X.pressed = false;
  }

  if ((buttonBegin_Y.pressed) && (buttonEnd_Y.pressed)) {
    Serial.printf("Begin and End push buttons should not be pressed simultaneously. Ignoring the commands!\n");
    buttonBegin_Y.pressed = false;
    buttonEnd_Y.pressed = false;
  } else if (buttonBegin_Y.pressed) {
    Serial.printf("Button has been pressed %u times\n", buttonBegin_Y.numberKeyPresses);
    // reached starting point
    Serial.println("Motor Y reached starting point\n");
    //some_buttonBegin_Y();
    buttonBegin_Y.pressed = false;
  } else if (buttonEnd_Y.pressed) {
    Serial.printf("Button has been pressed %u times\n", buttonEnd_Y.numberKeyPresses);
    // reached ending point
    Serial.println("Motor Y reached ending point\n");
    //some_buttonEnd_Y();
    buttonEnd_Y.pressed = false;
  }

  //call the webserver
  server.handleClient();
}


