# esp32_wroom_wifiap_stepper_webserver
This is a repository for controlling stepper e.g. NEMA17 or NEMA23 connected to ESP32 WROOM 32D using a webserver accessed via WiFi AP mode or via serial port interface. Known stepper drivers compatible with this project are TB6600 and DM542. There may also be other stepper drivers but they are currently untested for this project.

Note: ESP32 WROOM microcontroller offers soft WiFi AP mode.

Note: There are two *.ino files with similar code. A normal version without push buttons ISR and one with push buttons ISR. The user may program whichever they prefer of those two.

## Methods of controlling a stepper motor:

1. Method 1: Webserver (preferred).
2. Method 2: Serial Port interface (microusb serial port).
3. Method 3: (Optional) Push buttons control (only allows directional control ->  CCW and CW).

For compiling Arduino IDE, please refer https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/

The wiring connections are as follows: https://www.makerguides.com/esp32-and-tb6600-stepper-motor-driver/ 
One may replace TB6600 stepper driver with DM542 -> 2 phase stepper driver (See an important note on the electronics and the wiring below). A stepper like NEMA17 would be our target.

PUL+,PUL-,DIR+,DIR-,EN+,EN- connections need to match.

Similarly, A+,A-,B+,B-,GND,VDC connections also need to match.

VDC should fall within the specified voltage range per stepper motor specifications. 

(Optional) Additional direction control and traverse push buttons are gpio 13 and gpio 14. The pushbutton wiring on each gpio will be as follows: https://docs.arduino.cc/built-in-examples/digital/Button (connect to 3.3V instead of 5V) or https://randomnerdtutorials.com/esp32-digital-inputs-outputs-arduino/

Installing esp32 on arduino IDE v2 is as follows:
https://randomnerdtutorials.com/installing-esp32-arduino-ide-2-0/

Regex library installation: https://docs.arduino.cc/software/ide-v2/tutorials/ide-v2-installing-a-library

Type regex in search bar and click install.

## Usage for WebServer over WiFi AP Mode:

1. Connect microUSB cable to a laptop/PC and power on ESP32 WROOM 32D chip.
2. Scan the following SSID from cell phone/tablet 'nca_atm_ap'.
3. Connect to SSID and enter the password 'NcaAtmPassword'.
4. Open a web browser on a cell phone/tablet and navigate to IP address 192.168.4.1 in the address bar.
5. Enter the steps and set them with button click on 'submit'.
6. Enter the delay in microsecs between pulses and set them with button click on 'submit'.
7. Click on one of the buttons 'Clockwise(CW)' or 'CounterClockwise(CCW)' to rotate the stepper as necessary.


## Usage for Serial Control:

Baud is set to 115200n1

Port is set to /dev/ttyUSB\<x\> (Linux) or COM\<x\> (Windows)
  
 Valid commands are:
1. 'CW' (clockwise)
2. 'CCW' (counterclockwise)
3. 'EN=1' (enable)
4. 'EN=0' (disable)
5. 'steps=\<value\>' [default = 1] <---- steps to traverse/rotate
6. 'microsecs=\<value\>' [default = 50] <---- delay between successive pulses (50% duty cycle)

## !!!IMPORTANT!!! Wiring for DM542 Stepper Driver interfaced with ESP32 WROOM 32D

DM542 needs min 4.5VDC control signal voltage. Read the details here: https://www.laskakit.cz/user/related_files/dm542.pdf

ESP32 digital GPIO -> low value -> 0V and high value -> 3.3V. Read the details here: https://deepbluembedded.com/esp32-digital-inputs-outputs-arduino/

In order to interface PUL+,PUL-,DIR+,DIR-,EN+,EN- connections one needs to purchase 3.3V to 5V TTL converter. Some URLs (not exclusive) that I found where one may purchase this are as follows:

1. https://www.sparkfun.com/products/12009
2. https://www.amazon.com/3-3v-5v-logic-level-converter/s?k=3.3v+5v+logic+level+converter

TB6600 (unlike DM542) does not need this 3.3V to 5V TTL converter.
  
![alt text](https://github.com/enthusiasticgeek/esp32_wroom_wifiap_stepper_webserver/blob/main/Screenshot_20230503_082242.jpg "ESP32 ARDUINO WEBSERVER WIFI AP")

