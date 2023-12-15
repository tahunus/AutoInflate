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

byte MaxPressure = 30;  //This number should remain low! Somewhere around 30 for 3PSI for safety!

#include "globalVariables.h"
#include "graphics.h"
#include "sysFunctions.h"

void setup() {
  Wire.begin(3, 4);
  u8g2.setI2CAddress(0x78);  //0x78 (0x3C), 0x7A (0x3D)
  u8g2.begin();
  Serial.begin(9600);  //SERIAL DEBUG IF NEEDED
  pixels.begin();

  pinMode(BATTERY, INPUT);  //14k/2k divider
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

  analogReadResolution(12);  //FOR READING BATTERY LEVEL

  preferences.begin("DATAStore", false);
  airSys.freq = preferences.getUInt("AIRS_freq", 5000);  //Load default for PWM functions before starting.
  preferences.end();

  ledcSetup(pumpChannel, airSys.freq, resolution);
  ledcAttachPin(pumpPin, pumpChannel);

  timer1 = timerBegin(0, 80, false);  //Timer#, Prescaler, (True count up, false count down).
  timerStop(timer1);
  timerAttachInterrupt(timer1, &onTimer1, true);
  timerRestart(timer1);

  timer2 = timerBegin(1, 80, false);  //Timer#, Prescaler, (True count up, false count down).
  timerStop(timer2);
  timerAttachInterrupt(timer2, &onTimer2, true);
  timerRestart(timer2);

  for (int thisReading = 0; thisReading < numReadings; thisReading++)  //Load zeros to average array.
  {
    readings[thisReading] = 0;
  }

  for (int i = 0; i <= 50; i++) {
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
  airSys.pressureOffset = airSys.PRESSUREmbar * 100;  //FUNCTION TO SET INITIAL PRESSURE OFFSET!

  attachInterrupt(digitalPinToInterrupt(encoderPinA), handleEncoderInterrupt, CHANGE);
  attachInterrupt(
    digitalPinToInterrupt(buttonPin), [] {
      if (ButtonPressed += (millis() - DebounceTimer) >= (delayTime)) DebounceTimer = millis();
    },
    FALLING);

  //LOAD SAVED VALUES (pageNumber, RW)
  storedData(3, 0);
  storedData(4, 0);
  storedData(5, 0);
  storedData(7, 0);
}

void loop() {
  currentMillis = millis();

  if (ButtonPressed > 0)  //ENCODER BUTTON PRESS DEBOUNCE
  {
    handleButtonPress();
    ButtonPressed = 0;
  }

  if (currentMillis - selectorPreviousMillis >= selectorInterval)  //GENERAL TIMED DELAY
  {
    if (!flash) {
      flash = true;
    } else {
      flash = false;
    }
    powerLevel = constrain(analogRead(5), 0, 1300);  //READ BATTERY VOLTAGE
    selectorPreviousMillis = currentMillis;
  }

  //MAIN DISPLAY LOOP
  u8g2.clearBuffer();  //BEGIN FRAME
  u8g2.setBitmapMode(1);
  displayData();
  u8g2.sendBuffer();  //END FRAME

  pixels.clear();
  pixels.setPixelColor(0, pixels.Color(0, 0, encoderInput));
  pixels.setPixelColor(1, pixels.Color(encoderInput, 0, 0));
  pixels.show();

  getAveragePressure();
  airSYSLOOP();
}

void airSYSLOOP()
{ 
    int tempPWM = 0;
    if(airSys.PRESSUREAveragembar < airSys.thresholdPressure) //check for low pressure to start ramping up      
    {
      tempPWM = map(airSys.PRESSUREAveragembar, 0, airSys.thresholdPressure, airSys.minPumpPWM, airSys.maxPumpPWM);  //map(value, fromLow, fromHigh, toLow, toHigh)
    }
    else if(airSys.PRESSUREAveragembar >= airSys.thresholdPressure) 
    {
      if(airSys.PRESSUREAveragembar >= airSys.pumpPressure) //turn pump off if pressure was reached
      {
        tempPWM = 0;
      }
      else if((airSys.pumpPressure - airSys.PRESSUREAveragembar) >= airSys.thresholdPressure) //run max until inside the ramp window
      {
        tempPWM = airSys.maxPumpPWM;
      }
      else if((airSys.pumpPressure - airSys.PRESSUREAveragembar) < airSys.thresholdPressure) //ramp down to the max pressure and maintain
      {
        int tempPressure = airSys.pumpPressure - airSys.PRESSUREAveragembar;
        tempPressure = constrain(tempPressure, 0, airSys.thresholdPressure);
        tempPWM = map(tempPressure, 0, airSys.thresholdPressure, airSys.minPumpPWM, airSys.maxPumpPWM);
      }
    }
    
  if(airSys.pumpState && (airSys.pumpPressure != 0))
  {
    ledcWrite(pumpChannel, tempPWM);
  }
  else
  {
    ledcWrite(pumpChannel, 0);
  }

  if(airSys.solenoidState)
  {
    digitalWrite(solenoidPin, HIGH);
  }
  else
  {
    digitalWrite(solenoidPin, LOW);
  }

  if(pulseFeedback > 0) //Feedback pulses from pump.
  {
    delay(100);
    pixels.clear();
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.setPixelColor(1, pixels.Color(255, 0, 0));
    pixels.show();   
    for (int i = 1; i <= pulseFeedback; i++) 
    {
      ledcWrite(pumpChannel, 1023);
      delay(100);
      ledcWrite(pumpChannel, 0);
      delay(100);
    }
    pulseFeedback = 0;
  }
}

void getAveragePressure()
{
    if(currentMillis - pressurePreviousMillis >= pressureReadInterval) 
    {
      sensor.read();
      //int tempPressure = (sensor.pressure() * 0.0145038) - pressureOffset;
      int tempPressure = (sensor.pressure() * 100) - airSys.pressureOffset;
      if(tempPressure < 0)
      {
        tempPressure = 0;
      }

      total = total - readings[readIndex];
      readings[readIndex] = tempPressure;
      total = total + readings[readIndex];
      readIndex = readIndex + 1;

      if (readIndex >= numReadings) 
      {
        readIndex = 0;
      }

      airSys.PRESSUREmbar = tempPressure;
      airSys.PRESSUREAveragembar = (total / numReadings);
      airSys.PRESSUREAveragepsi = ((airSys.PRESSUREAveragembar * 0.0146) / 100);
      pressurePreviousMillis = currentMillis;
    }
}

#include "menuPages.h" //--------------------------------------------solve dependencies and move to #include section

void encoderInteraction()//ONE TIME EXECUTE
{
  buttonPressFunc(pageNumber);
  encoderConstrainValMin = 0;
  encoderConstrainValMax = PageElements[pageNumber];
}

void profileRUN()
{
  airSys.runType = 1;
  if((profileVar.cycleTime != 0) && (profileVar.onTime != 0))
  {
    if(profileVar.cycleTime >= (profileVar.onTime + 1))
    {
      timerRestart(timer1);
      timerWrite(timer1,(profileVar.cycleTime * 1000000));
      timerAlarmWrite(timer1, 0, false);
      timerAlarmEnable(timer1);
      timerStart(timer1);

      //profileVar.runState = 1;
      timerRestart(timer2);
      timerWrite(timer2,(profileVar.onTime * 1000000));
      timerAlarmWrite(timer2, 0, true);
      timerAlarmEnable(timer2);
      timerStart(timer2);
        
      airSys.pumpPressure = profileVar.highPressure;
      airSys.pumpState = 1;
      airSys.solenoidState = 1;
    }
    else
    {
      STOP();
    }
  }
  else
  {
    STOP();
  }
}

void hugRun()
{
  if(hugVar.hugTime != 0)
  {
    timerRestart(timer1);
    timerWrite(timer1,(hugVar.hugTime * 1000000));
    timerAlarmWrite(timer1, 0, false);
    timerAlarmEnable(timer1);
    timerStart(timer1);

    airSys.runType = 2;
    airSys.pumpPressure = hugVar.hugPressure;
    airSys.pumpState = 1;
    airSys.solenoidState = 1;
  }
  else
  {
    STOP();
  }
}
void displayData()//CONTINUOUS EXECUTE
{
  switch (pageNumber)
  {
    case 1:
      mainPage(); //MAIN PAGE
      break;
    case 2:
      configMainPage(); //CONFIG PAGE
      break;
    case 3:
      configPage1(); //PROFILE
      break;
    case 4:
      configPage2(); //HUG
      break;
    case 5:
      configPage3(); //AIRSYS
      break;
    case 6:
      configPage4(); //MOTION
      break;
    case 7:
      configPage5(); //CONFIG
      break;
    default:
      break;
  }
}
void buttonPressFunc(byte PG)//ONE TIME EXECUTE
{
//int storedValue = ((PG << 8) + EL);
switch (PG)
  {
    case 1:
      mainPageActions(); //MAIN PAGE   
      break;
    case 2:
      mainConfigPageActions(); //CONFIG PAGE
      break;
    case 3:
      configPageActions(); //PROFILE
      break;
    case 4:
      configPageActions(); //HUG
      break;
    case 5:
      configPageActions(); //AIRSYS
      break;
    case 6:
      configPageActions(); //MOTION
      break;
    case 7:
      configPageActions(); //CONFIG
      break;
    case 8:
      configPageActions(); //WIFI
      break;
    case 9:
      configPageActions(); //-----
      break;
    case 10:
      configPageActions(); //-----
      break;
    default:
      break;
  }
}

void mainPageActions()
{
  element = encoderInput;
  if(element == 0)//STOP
    {
      STOP();
    }
  else if(element == 1)//HUG
    {
      if(airSys.runType > 0)
      {
        STOP();
      }
      else
      {
        hugRun();
      }
    }
  else if(element == 2)//PROFILE
    {
      if(airSys.runType > 0)
      {
        STOP();
      }
      else
      {
        profileRUN();
      }
      
    }
  else if(element == 3)//CONFIG
    {
      pageNumber = 2;
      encoderInput = 0; 
    }

}
void mainConfigPageActions()//CONFIG PAGE
{
  element = encoderInput;
  if(element == 0)
  {
    pageNumber = 1;
  }
  else if(element > 0)
  {
    pageNumber = encoderInput +2;//SKIP FIRST TWO PAGES
    encoderInput = 0;
  }
}
void configPageActions()//SUB CONFIG PAGES
{
  if(changeValue)
  {
    changeValue = 0;
    encoderConstrainValMin = 0;
    encoderConstrainValMax = PageElements[pageNumber];
    encoderInput = element;
    maxPressureCheck();
  }
  else
  {
    if(encoderInput == 0)
    {
      if(pageNumber == 7)//Special condition for CONFIG page.
      {
        storedData(pageNumber, 1);
        ESP.restart();        
      }      
      pageNumber = 2;//BACK TO CONFIG PAGE
      encoderInputTemp = 0;
      encoderInput = 0; 
    }
    else if(encoderInput != 0)
    {    
      changeValue = 1;
      element = encoderInput;
      encoderInput = encoderInputTemp; //RECALL DATA
    }
  }
  if(SAVE)
  {
    storedData(pageNumber, 1);
    element = 0;
    encoderInput = 0;
    changeValue = 0;
    SAVE = false;
    pageNumber = 2;//BACK TO CONFIG PAGE
  }
}

void runningAnimation()
{
  if(airSys.pumpState)
  {
    if(frameCount == 0)
    {
      u8g2.drawXBMP( 8, 29, 25, 31, image_VEST2_ANIMATION_1_bits);
    }
    else if(frameCount == 1)
    {
      u8g2.drawXBMP( 8, 29, 25, 31, image_VEST2_ANIMATION_2_bits);
    }
    else if(frameCount == 2)
    {
      u8g2.drawXBMP( 8, 29, 25, 31, image_VEST2_ANIMATION_3_bits);
    }

    if(currentMillis - animationPreviousMillis >= animationFlipInterval)
    {
      if(frameCount == 0)
      {
        frameCount++;
      }
      else if (frameCount == 1)
      {
        frameCount++;
      }
      else if (frameCount == 2)
      {
        frameCount = 0;
      }
      animationPreviousMillis = currentMillis;
    }
  }
}


void STOP()
{
  airSys.solenoidState = 0;
  airSys.pumpState = 0;
  airSys.runType = 0;

  timerStop(timer1);
  timerRestart(timer1);

  timerStop(timer2);
  timerRestart(timer2);
  
  pulseFeedback = 2;
}

void encoderConstain()
{
  encoderInput = constrain(encoderInput, encoderConstrainValMin, encoderConstrainValMax);
}

void storedData(byte PG, byte RW)//pageNumber, read/write
{
  preferences.begin("DATAStore", false);
  switch (PG)
  {
    case 3://PROFILE
      if(!RW)//READ
      {
        profileVar.highPressure = preferences.getUInt("PRO_highpres", 0);
        profileVar.onTime = preferences.getUInt("PRO_ontime", 0);
        profileVar.offTime = preferences.getUInt("PRO_offtime", 0);
        profileVar.cycleTime = preferences.getUInt("PRO_cycletime", 0);

        Serial.print("READ : ");
        Serial.print(profileVar.highPressure);
        Serial.print(" : ");
        Serial.print(profileVar.onTime);
        Serial.print(" : ");
        Serial.print(profileVar.offTime);
        Serial.print(" : ");
        Serial.println(profileVar.cycleTime);
      }
      else//WRITE
      {
        preferences.putUInt("PRO_highpres", profileVar.highPressure);
        preferences.putUInt("PRO_ontime", profileVar.onTime);
        preferences.putUInt("PRO_offtime", profileVar.offTime);
        preferences.putUInt("PRO_cycletime", profileVar.cycleTime);

        Serial.print("WRITE : ");
        Serial.print(profileVar.highPressure);
        Serial.print(" : ");
        Serial.print(profileVar.onTime);
        Serial.print(" : ");
        Serial.print(profileVar.offTime);
        Serial.print(" : ");
        Serial.println(profileVar.cycleTime);
      }
      break;
    case 4://HUG
      if(!RW)//READ
      {
        hugVar.hugTime = preferences.getUInt("HUG_hugtime", 0);
        hugVar.hugPressure = preferences.getUInt("HUG_hugpres", 0);

        Serial.print("READ : ");
        Serial.print(hugVar.hugTime);
        Serial.print(" : ");
        Serial.println(hugVar.hugPressure);
        
      }
      else//WRITE
      {
        preferences.putUInt("HUG_hugtime", hugVar.hugTime);
        preferences.putUInt("HUG_hugpres", hugVar.hugPressure);

        Serial.print("WRITE : ");
        Serial.print(hugVar.hugTime);
        Serial.print(" : ");
        Serial.println(hugVar.hugPressure);
      }
      break;
    case 5://AIRSYS
      if(!RW)//READ
      {
        airSys.maxPumpPWM = preferences.getUInt("AIRS_maxPWM", 900);//0-1023
        airSys.rampPump = preferences.getChar("AIRS_ramp", 0);
        airSys.maxPressure = preferences.getUInt("AIRS_maxpres", 13788);//DEFAULT PRESSURE 2PSI=13788(mBAR/100)
        airSys.thresholdPressure = preferences.getUInt("AIRS_thres", 1378);//DEFAULT PRESSURE 0.2PSI=1378(mBAR/100)
        
        Serial.print("READ : ");
        Serial.print(airSys.maxPumpPWM);
        Serial.print(" : ");
        Serial.print(airSys.rampPump);
        Serial.print(" : ");
        Serial.print(airSys.maxPressure);
        Serial.print(" : ");
        Serial.println(airSys.thresholdPressure);
      }
      else//WRITE
      {
        preferences.putUInt("AIRS_maxPWM", airSys.maxPumpPWM);
        preferences.putChar("AIRS_ramp", airSys.rampPump);
        preferences.putUInt("AIRS_maxpres", airSys.maxPressure);
        preferences.putUInt("AIRS_thres", airSys.thresholdPressure);
      
        Serial.print("WRITE : ");
        Serial.print(airSys.maxPumpPWM);
        Serial.print(" : ");
        Serial.print(airSys.rampPump);
        Serial.print(" : ");
        Serial.print(airSys.maxPressure);
        Serial.print(" : ");
        Serial.println(airSys.thresholdPressure);
        
      }
      break;
    case 6://MOTION
      configPageActions(); 
      break;
    case 7://CONFIG
      if(!RW)
      {
        airSys.minPumpPWM = preferences.getUInt("AIRS_minPWM", 200);//0-1023 DEFAULT LOW PWM
        airSys.freq = preferences.getUInt("AIRS_freq", 5000);//DEFAULT PWM FREQUENCY

        Serial.print("READ : ");
        Serial.print(airSys.minPumpPWM);
        Serial.print(" : ");
        Serial.println(airSys.freq);
      }
      else
      {
        preferences.putUInt("AIRS_minPWM", airSys.minPumpPWM);
        preferences.putUInt("AIRS_freq", airSys.freq);

        Serial.print("READ : ");
        Serial.print(airSys.minPumpPWM);
        Serial.print(" : ");
        Serial.println(airSys.freq);

        ESP.restart(); 
      }
      break;
    case 8:
      configPageActions(); //WIFI
      break;
    case 9:
      configPageActions(); //-----
      break;
    case 10:
      configPageActions(); //-----
      break;
    default:
      break;
  }
  preferences.end();
  pulseFeedback = 1;
}

void maxPressureCheck()
{
  if(profileVar.highPressure > airSys.maxPressure)
  {
    profileVar.highPressure = airSys.maxPressure;
    pulseFeedback = 1;
  }
  if(hugVar.hugPressure > airSys.maxPressure)
  {
    hugVar.hugPressure = airSys.maxPressure;
    pulseFeedback = 1;
  }
}