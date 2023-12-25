/*
AUTOINFLATE V0.3.1 - 11/18/23
BY Tevian Busselle 

MIT License

Copyright (c) 2023 Tevian Busselle

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <Arduino.h>
#include <U8g2lib.h>
#include <MS5837.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>

#include "globalVariables.h"
#include "backEnd.h"
#include "frontEnd.h" 
#include "hwControl.h"

void setup(void) {
  Wire.begin(3, 4);
  u8g2.setI2CAddress(0x78); //0x78 (0x3C), 0x7A (0x3D)
  u8g2.begin();
  Serial.begin(115200); //SERIAL DEBUG IF NEEDED
  pixels.begin();
  
  pinMode(BATTERY, INPUT); //14k/2k divider
  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);
  pinMode(buttonPin, INPUT);
  pinMode(solenoidPin, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  pinMode(1, INPUT);   // Connect HX710 OUT to Arduino pin 2
  pinMode(2, OUTPUT);  // Connect HX710 SCK to Arduino pin 3

  sensor.setFluidDensity(997);  // Set the fluid density for pressure calculations
  sensor.init();
  sensor.read();
  
  analogReadResolution(12);//FOR READING BATTERY LEVEL
  
  preferences.begin("DATAStore", false);
  airSys.freq = preferences.getUInt("AIRS_freq", 5000);//Load default for PWM functions before starting.
  preferences.end();

  ledcSetup(pumpChannel, airSys.freq, resolution);
  ledcAttachPin(pumpPin, pumpChannel);

  timer1 = timerBegin(0, 80, false);//Timer#, Prescaler, (True count up, false count down).
  timerStop(timer1);
  timerAttachInterrupt(timer1, &onTimer1, true);
  timerRestart(timer1);

  timer2 = timerBegin(1, 80, false);//Timer#, Prescaler, (True count up, false count down).
  timerStop(timer2);
  timerAttachInterrupt(timer2, &onTimer2, true);
  timerRestart(timer2);
  
  for (int thisReading = 0; thisReading < numReadings; thisReading++)  { //Load zeros to average array.
      readings[thisReading] = 0;
  }
  
  for (int i = 0; i <= 50; i++)  {
    airSys.PRESSUREmbar = sensor.pressure();
    u8g2.clearBuffer();
    u8g2.setBitmapMode(1);
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_helvB08_tr);
    u8g2.drawStr(0, 8, "PRESSURE CALIBRATION");
    u8g2.drawStr(0, 25, "DISCONNECT AIR LINES");
    u8g2.drawStr(0, 38, "AND WAIT 5 SEC!");
    int boxWidth = 0;
    boxWidth = map(i, 0, 50, 127, 0);
    u8g2.setDrawColor(2);
    u8g2.drawBox(0, 43, boxWidth, 18);
    u8g2.sendBuffer();
    delay(100); 
    sensor.read();
  }
  airSys.pressureOffset = airSys.PRESSUREmbar * 100; //FUNCTION TO SET INITIAL PRESSURE OFFSET!
  
  attachInterrupt(digitalPinToInterrupt(encoderPinA), handleEncoderInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(buttonPin), [] {if (ButtonPressed+= (millis() - DebounceTimer) >= (delayTime )) DebounceTimer = millis();}, FALLING);
  
  //LOAD SAVED VALUES (pageNumber, RW)
  storedData(3, 0);
  storedData(4, 0);
  storedData(5, 0);
  storedData(7, 0);
}

void loop(void)  {
  currentMillis = millis();
  
  if (ButtonPressed > 0) { //ENCODER BUTTON PRESS DEBOUNCE
    handleButtonPress();
    ButtonPressed = 0;
  }
  
  if(currentMillis - selectorPreviousMillis >= selectorInterval) {  //GENERAL TIMED DELAY
    flash = !flash;
    powerLevel = constrain(analogRead(5), 0, 1300); //READ BATTERY VOLTAGE
    selectorPreviousMillis = currentMillis;
  }

  //MAIN DISPLAY LOOP
  u8g2.clearBuffer(); //BEGIN FRAME
  u8g2.setBitmapMode(1);
  displayData();
  u8g2.sendBuffer(); //END FRAME
  
  pixels.clear();
  pixels.setPixelColor(0, pixels.Color(0, 0, encoderInput));
  pixels.setPixelColor(1, pixels.Color(encoderInput, 0, 0));
  pixels.show();   

  getAveragePressure();
  airSYSLOOP();
}
