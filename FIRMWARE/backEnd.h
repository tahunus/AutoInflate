void STOP()  {
  airSys.solenoidState = 0;
  airSys.pumpState = 0;
  airSys.runType = 0;

  timerStop(timer1);
  timerRestart(timer1);
  timerStop(timer2);
  timerRestart(timer2);
  
  pulseFeedback = 2;
}

void airSYSLOOP() { 
    int tempPWM = 0;
    if(airSys.PRESSUREAveragembar < airSys.thresholdPressure)  {  //check for low pressure to start ramping up      
      tempPWM = map(airSys.PRESSUREAveragembar, 0, airSys.thresholdPressure, airSys.minPumpPWM, airSys.maxPumpPWM);  //map(value, fromLow, fromHigh, toLow, toHigh)
    }
    else {
      if(airSys.PRESSUREAveragembar >= airSys.thresholdPressure) {
        if(airSys.PRESSUREAveragembar >= airSys.pumpPressure)  {  //turn pump off if pressure was reached
          tempPWM = 0;
        }
        else {
          if((airSys.pumpPressure - airSys.PRESSUREAveragembar) >= airSys.thresholdPressure)  {  //run max until inside the ramp window
            tempPWM = airSys.maxPumpPWM;
          }
          else {
            if((airSys.pumpPressure - airSys.PRESSUREAveragembar) < airSys.thresholdPressure)  {  //ramp down to the max pressure and maintain
              int tempPressure = airSys.pumpPressure - airSys.PRESSUREAveragembar;
              tempPressure = constrain(tempPressure, 0, airSys.thresholdPressure);
              tempPWM = map(tempPressure, 0, airSys.thresholdPressure, airSys.minPumpPWM, airSys.maxPumpPWM);
            }
          }
        }
      }
    }
    
  if(airSys.pumpState && (airSys.pumpPressure != 0)) 
    ledcWrite(pumpChannel, tempPWM);
  else
    ledcWrite(pumpChannel, 0);

  if(airSys.solenoidState)
    digitalWrite(solenoidPin, HIGH);
  else
    digitalWrite(solenoidPin, LOW);

  if(pulseFeedback > 0)  {  //Feedback pulses from pump.
    delay(100);
    pixels.clear();
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.setPixelColor(1, pixels.Color(255, 0, 0));
    pixels.show();   
    for (int i = 1; i <= pulseFeedback; i++)  {
      ledcWrite(pumpChannel, 1023);
      delay(100);
      ledcWrite(pumpChannel, 0);
      delay(100);
    }
    pulseFeedback = 0;
  }
}

void getAveragePressure()  {
    if(currentMillis - pressurePreviousMillis >= pressureReadInterval) {
      sensor.read();
      //int tempPressure = (sensor.pressure() * 0.0145038) - pressureOffset;profileRUN
      int tempPressure = (sensor.pressure() * 100) - airSys.pressureOffset;
      if(tempPressure < 0)  
        tempPressure = 0;

      total = total - readings[readIndex];
      readings[readIndex] = tempPressure;
      total = total + readings[readIndex];
      readIndex = readIndex + 1;

      if (readIndex >= numReadings) 
        readIndex = 0;

      airSys.PRESSUREmbar = tempPressure;
      airSys.PRESSUREAveragembar = (total / numReadings);
      airSys.PRESSUREAveragepsi = ((airSys.PRESSUREAveragembar * 0.0146) / 100);
      pressurePreviousMillis = currentMillis;
    }
}

void profileRUN()  {
  airSys.runType = 1;
  if (profileVar.cycleTime != 0 && profileVar.onTime != 0 && 
      profileVar.cycleTime >= profileVar.onTime + 1)  {
    timerRestart(timer1);
    timerWrite(timer1,(profileVar.cycleTime * 1000000));
    timerAlarmWrite(timer1, 0, false);hugRun
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
  else {
    STOP();
  }
}

void hugRun()  {
  if(hugVar.hugTime != 0)  {
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
  else  {
    STOP();
  }
}

void storedData(byte PG, byte RW)  {  //pageNumber, read/write
  preferences.begin("DATAStore", false);
  switch (PG)   {
    case 3://PROFILE
      if(!RW)  {  //READ
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
      else  {  //WRITE
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
      if(!RW)  {  //READ
        hugVar.hugTime = preferences.getUInt("HUG_hugtime", 0);
        hugVar.hugPressure = preferences.getUInt("HUG_hugpres", 0);

        Serial.print("READ : ");
        Serial.print(hugVar.hugTime);
        Serial.print(" : ");
        Serial.println(hugVar.hugPressure);
      }
      else  {  //WRITE
        preferences.putUInt("HUG_hugtime", hugVar.hugTime);
        preferences.putUInt("HUG_hugpres", hugVar.hugPressure);

        Serial.print("WRITE : ");
        Serial.print(hugVar.hugTime);
        Serial.print(" : ");
        Serial.println(hugVar.hugPressure);
      }
      break;
    case 5://AIRSYS
      if(!RW)  {  //READ
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
      else  {  //WRITE
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
      //configPageActions();  //-------------------------------------------if function is TBD, should not call this??
      break;
    case 7://CONFIG
      if(!RW)   {
        airSys.minPumpPWM = preferences.getUInt("AIRS_minPWM", 200);//0-1023 DEFAULT LOW PWM
        airSys.freq = preferences.getUInt("AIRS_freq", 5000);//DEFAULT PWM FREQUENCY

        Serial.print("READ : ");
        Serial.print(airSys.minPumpPWM);
        Serial.print(" : ");
        Serial.println(airSys.freq);
      }
      else    {
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
      //configPageActions(); //WIFI //-------------------------------------------if function is TBD, should not call this??
      break;
    case 9:
      //configPageActions(); //----- //------------------------------------------if function is TBD, should not call this??
      break;
    case 10:
      //configPageActions(); //----- //------------------------------------------if function is TBD, should not call this??
      break;
    default:
      break;
  }
  preferences.end();
  pulseFeedback = 1;
}

void maxPressureCheck()  {
  if(profileVar.highPressure > airSys.maxPressure)  {
    profileVar.highPressure = airSys.maxPressure;
    pulseFeedback = 1;
  }
  if(hugVar.hugPressure > airSys.maxPressure)  {
    hugVar.hugPressure = airSys.maxPressure;
    pulseFeedback = 1;
  }
}

void mainPageActions() {
  element = encoderInput;
  switch (element) {
    case 0: STOP();                //STOP
            break;
    case 1: if(airSys.runType > 0) //HUG
              STOP(); 
            else 
              hugRun();
            break;
    case 2: if(airSys.runType > 0) //PROFILE
              STOP(); 
            else 
              profileRUN();
            break;
    case 3: pageNumber = 2;        //CONFIG
            encoderInput = 0; 
  }
}

void mainConfigPageActions()  {  //CONFIG PAGE
  element = encoderInput;
  if(element == 0)  
    pageNumber = 1;
  else 
    if(element > 0)  {  //----------------------------------------------------------can element be < 0 ??
      pageNumber = encoderInput +2;//SKIP FIRST TWO PAGES
      encoderInput = 0;
    }
}

void configPageActions()  {  //SUB CONFIG PAGES
  if(changeValue)  {
    changeValue = 0;
    encoderConstrainValMin = 0;
    encoderConstrainValMax = PageElements[pageNumber];
    encoderInput = element;
    maxPressureCheck();
  }
  else {
    if(encoderInput == 0)  {
      if(pageNumber == 7)  {  //Special condition for CONFIG page.
        storedData(pageNumber, 1);
        ESP.restart();        
      }      
      pageNumber = 2;//BACK TO CONFIG PAGE
      encoderInputTemp = 0;
      encoderInput = 0; 
    }
    else if(encoderInput != 0) {  //--------------------------------------redundant? can it be < 0?
      changeValue = 1;
      element = encoderInput;
      encoderInput = encoderInputTemp; //RECALL DATA
    }
  }
  if(SAVE)  {
    storedData(pageNumber, 1);
    element = 0;
    encoderInput = 0;
    changeValue = 0;
    SAVE = false;
    pageNumber = 2;//BACK TO CONFIG PAGE
  }
}

void encoderInteraction()  { //ONE TIME EXECUTE
  switch (pageNumber) {
    case 1:
      mainPageActions(); //MAIN PAGE   
      break;
    case 2:
      mainConfigPageActions(); //CONFIG PAGE
      break;
    case 3:   //PROFILE
    case 4:   //HUG
    case 5:   //AIRSYS
    case 6:   //MOTION
    case 7:   //CONFIG
    case 8:   //WIFI
    case 9:   //-----TBD1
    case 10:  //-----TBD2 (and cases 3 thru 9, above)
      configPageActions(); 
      break;
  }
  encoderConstrainValMin = 0;
  encoderConstrainValMax = PageElements[pageNumber];
}