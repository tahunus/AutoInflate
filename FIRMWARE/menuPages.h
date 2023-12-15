void mainPage()
{
  u8g2.setDrawColor(1);
  u8g2.drawXBMP( 117, 35, 11, 29, image_BAT_bits);
  u8g2.drawXBMP( 0, 19, 41, 45, image_VEST2_bits);
  u8g2.drawRBox(0, 0, 42, 18, 4);
  u8g2.setDrawColor(0);
  u8g2.setFont(u8g2_font_profont22_tr);
  u8g2.setCursor(4, 16);
  u8g2.print(airSys.PRESSUREAveragepsi, 1);
  u8g2.setDrawColor(1);

  int tempTimerSecs = 0;
  int runningMin = 0;
  int runningSec = 0;
  
  tempTimerSecs = timerReadSeconds(timer1);
  tempTimerSecs = constrain(tempTimerSecs, 0, 5999);
  runningMin = (tempTimerSecs / 60) % 60;
  runningSec = tempTimerSecs % 60;
  
  char buf[6];
  sprintf(buf, "%02d:%02d", runningMin, runningSec);

  u8g2.setFont(u8g2_font_profont22_tr);
  u8g2.drawStr(50, 63, buf);
  u8g2.drawXBMP( 73, 0, 18, 18, image_HUG_bits);
  u8g2.drawXBMP( 93, 0, 16, 18, image_profileMINI_bits);
  u8g2.drawXBMP( 53, 0, 18, 18, image_STOP_bits);
  u8g2.drawXBMP( 110, 0, 18, 18, image_SETTINGS_bits);

  runningAnimation();

  u8g2.setDrawColor(0);
  int tempPowerLevel = map(powerLevel, 900, 1235, 23, 0); //CONVERT TO BARGRAPH
  u8g2.drawBox(119, 39, 7, tempPowerLevel); //BATTERY LEVEL INDECATOR

  //ACTIVE RUNNING TYPE
  if(airSys.runType == 1) //PROFILE
  {
    u8g2.setDrawColor(1);
    if(!flash)
    {
      u8g2.setDrawColor(1);
      u8g2.drawXBMP(94, 19, 14, 7, image_selectArrow_bits);
    }
    else
    {
      u8g2.setDrawColor(0);
      u8g2.drawBox(94, 19, 14, 7);
    }
  }
  else if(airSys.runType == 2) //HUG
  {
    u8g2.setDrawColor(1);
    if(!flash)
    {
      u8g2.setDrawColor(1);
      u8g2.drawXBMP(75, 19, 14, 7, image_selectArrow_bits);   
    }
    else
    {
      u8g2.setDrawColor(0);
      u8g2.drawBox(75, 19, 14, 7);
    }
  }

  if(encoderInput == 0) //STOP
    {
      u8g2.setDrawColor(1);
      u8g2.drawXBMP(55, 19, 14, 7, image_selectArrowOpen_bits);
    }
    else if(encoderInput == 1) //HUG
    {
      u8g2.setDrawColor(1);
      u8g2.drawXBMP(75, 19, 14, 7, image_selectArrowOpen_bits);
    }
    else if(encoderInput == 2) //PROFILE
    {
      u8g2.setDrawColor(1);
      u8g2.drawXBMP(94, 19, 14, 7, image_selectArrowOpen_bits);
    }
    else if(encoderInput == 3) //CONFIG
    {
      u8g2.setDrawColor(1);
      u8g2.drawXBMP(112, 19, 14, 7, image_selectArrowOpen_bits);
    }
}

void configMainPage()
{
  u8g2.setDrawColor(1);
  u8g2.drawXBMP( 0, 57, 9, 7, image_Pin_arrow_left_9x7_bits);
  u8g2.drawFrame(17, 54, 10, 10);
  u8g2.drawFrame(29, 54, 10, 10);
  u8g2.drawFrame(41, 54, 10, 10);
  u8g2.drawFrame(53, 54, 10, 10);
  u8g2.drawFrame(65, 54, 10, 10);
  u8g2.drawFrame(77, 54, 10, 10);
  u8g2.drawFrame(89, 54, 10, 10);
  u8g2.drawFrame(101, 54, 10, 10);

  if(encoderInput == 0) //BACK
  {
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_profont22_tr);
    u8g2.drawStr(39, 32, "BACK");

    if(!flash)
      {
        u8g2.setDrawColor(1);
        u8g2.drawXBMP( 0, 57, 9, 7, image_Pin_arrow_left_9x7_bits);
      }
      else
      {
        u8g2.setDrawColor(0);
        u8g2.drawBox(0, 57, 9, 7);
      }    
  }

  else if(encoderInput == 1) //PAGE 1 PROFILE
  {
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_profont22_tr);
    u8g2.drawStr(39, 32, "PROFILE");
    u8g2.drawXBMP( 6, 9, 28, 29, image_profile_bits);
    
    if(!flash)
    {
      u8g2.setDrawColor(1);
      u8g2.drawFrame(17, 54, 10, 10);
    }
    else
    {
      u8g2.drawBox(17, 54, 10, 10);
    }
  }

  else if(encoderInput == 2) //PAGE 2 HUG
  {
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_profont22_tr);
    u8g2.drawStr(63, 32, "HUG");
    u8g2.drawXBMP( 24, 12, 32, 29, image_HUG_LARGE_bits);

    if(!flash)
    {
      u8g2.setDrawColor(1);
      u8g2.drawFrame(29, 54, 10, 10);
    }
    else
    {
      u8g2.drawBox(29, 54, 10, 10);
    }
  }

  else if(encoderInput == 3) //PAGE 3 AIRSYS
  {
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_profont22_tr);
    u8g2.drawStr(49, 32, "AIRSYS");
    u8g2.drawXBMP( 7, 9, 36, 28, image_PUMP1_bits);

    if(!flash)
    {
      u8g2.setDrawColor(1);
      u8g2.drawFrame(41, 54, 10, 10);
    }
    else
    {
      u8g2.drawBox(41, 54, 10, 10);
    }
  }

  else if(encoderInput == 4) //PAGE 4 MOTION
  {
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_profont22_tr);
    u8g2.drawStr(49, 32, "MOTION");
    u8g2.drawXBMP( 6, 4, 38, 36, image_TAP_bits);
    
    if(!flash)
    {
      u8g2.setDrawColor(1);
      u8g2.drawFrame(53, 54, 10, 10);
    }
    else
    {
      u8g2.drawBox(53, 54, 10, 10);
    }
  }

  else if(encoderInput == 5) //PAGE 5 CONFIG
  {
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_profont22_tr);
    u8g2.drawStr(51, 32, "CONFIG");
    u8g2.drawXBMP( 6, 3, 41, 41, image_GEAR_bits);

    if(!flash)
    {
      u8g2.setDrawColor(1);
      u8g2.drawFrame(65, 54, 10, 10);
    }
    else
    {
      u8g2.drawBox(65, 54, 10, 10);
    }
  }

  else if(encoderInput == 6) //PAGE 6 WIFI
  {
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_profont22_tr);
    u8g2.drawStr(66, 32, "WIFI");
    u8g2.drawXBMP( 10, 9, 48, 34, image_WIRELESS_bits); 

    if(!flash)
    {
      u8g2.setDrawColor(1);
      u8g2.drawFrame(77, 54, 10, 10);
    }
    else
    {
      u8g2.drawBox(77, 54, 10, 10);
    }
  }

  else if(encoderInput == 7) //PAGE 7 NA
  {
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_profont22_tr);
    u8g2.drawStr(11, 32, "---------");

    if(!flash)
    {
      u8g2.setDrawColor(1);
      u8g2.drawFrame(89, 54, 10, 10);
    }
    else
    {
      u8g2.drawBox(89, 54, 10, 10);
    }
  }

  else if(encoderInput == 8) //PAGE 8 NA
  {
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_profont22_tr);
    u8g2.drawStr(11, 32, "---------");

    if(!flash)
    {
      u8g2.setDrawColor(1);
      u8g2.drawFrame(89, 54, 10, 10);
    }
    else
    {
      u8g2.drawBox(101, 54, 10, 10);
    }
  }
}

void configPage1() //PROFILE
{
  if(SAVE){SAVE = false;}
  
  char buf[4];
  float tempPressure = (profileVar.highPressure * 0.0146) / 100;
  dtostrf(tempPressure, 3, 1, buf);
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.drawStr(9, 21, buf);
  
  int onTimeMin = (profileVar.onTime / 60) % 60;
  int onTimeSec = profileVar.onTime % 60;
  char buf1[8];   //------------------------------------------------------changed from 5 (need -> 4 (for onTimeMin) + 1 (for :) + 2 (for onTimeSec) + 1 (for \0) = 7 bytes)
  sprintf(buf1, "%d:%02d", onTimeMin, onTimeSec);
  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.drawStr(35, 10, buf1);

  int offTimeMin = (profileVar.offTime / 60) % 60;
  int offTimeSec = profileVar.offTime % 60;
  char buf2[8];    //------------------------------------------------------changed from 5
  sprintf(buf2, "%d:%02d", offTimeMin, offTimeSec);
  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.drawStr(57, 56, buf2);

  int cycleTimeMin = (profileVar.cycleTime / 60) % 60;
  int cycleTimeSec = profileVar.cycleTime % 60;
  char buf4[8];  //--------------------------------------------------------changed from 6
  sprintf(buf4, "%02d:%02d", cycleTimeMin, cycleTimeSec);
  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.drawStr(94, 23, buf4); 

  u8g2.drawXBMP( 0, 57, 9, 7, image_Pin_arrow_left_9x7_bits);
  u8g2.drawXBMP( 26, 13, 60, 32, image_graph4_bits);
  u8g2.drawXBMP( 92, 27, 30, 19, image_cycle2_bits);
  u8g2.drawXBMP( 90, 52, 12, 12, image_help1_bits);
  u8g2.drawXBMP( 104, 53, 24, 11, image_KeySaveSelected_24x11_bits);
  
  

  if(!changeValue)
  {
    if(encoderInput == 0)//BACK
    {
      if(!flash)
        {
          u8g2.setDrawColor(1);
          u8g2.drawXBMP( 0, 57, 9, 7, image_Pin_arrow_left_9x7_bits);
        }
        else
        {
          u8g2.setDrawColor(0);
          u8g2.drawBox(0, 57, 9, 7);
        }     
    }
    else if(encoderInput == 1)//PROFILE PRESSURE
    {
      encoderInputTemp = (profileVar.highPressure * 0.0146) / 10;
      if(!flash)
        {
          u8g2.setDrawColor(1);
          u8g2.drawFrame(7, 11, 18, 12);
        }   
    }
    else if(encoderInput == 2)//PROFILE ON TIME
    {
      encoderInputTemp = profileVar.onTime;
      if(!flash)
        {
          u8g2.setDrawColor(1);
          u8g2.drawFrame(33, 0, 24, 12);
        }   
    }
    else if(encoderInput == 3)//PROFILE OFF TIME
    {
      encoderInputTemp = profileVar.offTime;
      if(!flash)
        {
          u8g2.setDrawColor(1);
          u8g2.drawFrame(55, 46, 24, 12);
        }   
    }
    else if(encoderInput == 4)//PROFILE CYCLE TIME
    {
      encoderInputTemp = profileVar.cycleTime;
      if(!flash)
        {
          u8g2.setDrawColor(1);
          u8g2.drawFrame(92, 13, 30, 12);
        }   
    }
    else if(encoderInput == 5)//SAVE
    {
      if(!flash)
        {
          u8g2.setDrawColor(2);
          u8g2.drawRBox(104, 53, 24, 11, 1);
        } 
        SAVE = true;  
    }
    else if(encoderInput == 6)//HELP
    {
      if(!flash)
      {
        u8g2.setDrawColor(2);
        u8g2.drawRBox(90, 52, 12, 12, 3);
      }
    }

  }
  else
  {
    if(element == 1)//PROFILE PRESSURE
    {
      encoderConstrainValMin = 0;
      encoderConstrainValMax = MaxPressure;
      profileVar.highPressure = encoderInput * 689.4;
    }
    else if(element == 2)//PROFILE ON TIME
    {
      encoderConstrainValMin = 0;
      encoderConstrainValMax = 599;
      profileVar.onTime = encoderInput;      
    }
    else if(element == 3)//PROFILE OFF TIME
    {
      encoderConstrainValMin = 0;
      encoderConstrainValMax = 599;
      profileVar.offTime = encoderInput; 
    }
    else if(element == 4)//PROFILE CYCLE TIME
    {
      encoderConstrainValMin = 0;
      encoderConstrainValMax = 5999;
      profileVar.cycleTime = encoderInput;
    }
    else if(element == 5)//SAVE
    {
       //SAVE
    }
    else if(element == 6)//HELP
    {
      u8g2.setDrawColor(0);
      u8g2.drawBox(0, 0, 128, 64);
      u8g2.setDrawColor(1);
      u8g2.setFont(u8g2_font_helvB08_tr);
      u8g2.drawStr(4, 30, "The profile icon is for");
      u8g2.drawStr(4, 42, "multiple inflation cycles.");
      u8g2.drawStr(4, 54, "Times can't be zero!");
      u8g2.drawXBMP( 55, 0, 16, 18, image_profileMINI_bits);
      encoderConstrainValMin = 0;
      encoderConstrainValMax = 1;   
    }
  }
}
void configPage2()//HUG
{
  if(SAVE){SAVE = false;}

  u8g2.setDrawColor(1);
  u8g2.drawXBMP( 33, 13, 60, 32, image_graph5_bits);
  u8g2.drawXBMP( 0, 57, 9, 7, image_Pin_arrow_left_9x7_bits);
  u8g2.drawXBMP( 104, 53, 24, 11, image_KeySaveSelected_24x11_bits);
  u8g2.drawXBMP( 90, 52, 12, 12, image_help1_bits);

  int onTimeMin = (hugVar.hugTime / 60) % 60;
  int onTimeSec = hugVar.hugTime % 60;

  char buf[8];  //--------------------------------------------------------changed from 5
  sprintf(buf, "%d:%02d", onTimeMin, onTimeSec);
  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.drawStr(53, 11, buf);

  char buf1[4];
  float tempPressure = (hugVar.hugPressure * 0.0146) / 100;
  dtostrf(tempPressure, 3, 1, buf1);
  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.drawStr(17, 21, buf1); 

  

  if(!changeValue)
    {
      if(encoderInput == 0)//BACK
      {
        if(!flash)
          {
            u8g2.setDrawColor(1);
            u8g2.drawXBMP( 0, 57, 9, 7, image_Pin_arrow_left_9x7_bits);
          }
          else
          {
            u8g2.setDrawColor(0);
            u8g2.drawBox(0, 57, 9, 7);
          }     
      }
      else if(encoderInput == 1)//HUG PRESSURE
      {
        if(!flash)
        {
          encoderInputTemp = (hugVar.hugPressure * 0.0146) / 10;
          u8g2.setDrawColor(1);
          u8g2.drawFrame(15, 11, 18, 12);
        }     
      }
      else if(encoderInput == 2)//HUG TIME
      {
        if(!flash)
        {
          encoderInputTemp = hugVar.hugTime;
          u8g2.setDrawColor(1);
          u8g2.drawFrame(51, 1, 24, 12);
        }
      }
      else if(encoderInput == 3)//SAVE
      {
        if(!flash)
          {
            u8g2.setDrawColor(2);
            u8g2.drawRBox(104, 53, 24, 11, 1);
          }
          SAVE = true;
      }
      else if(encoderInput == 4)//HELP
      {
        if(!flash)
        {
          u8g2.setDrawColor(2);
          u8g2.drawRBox(90, 52, 12, 12, 3);
        }
      }
    }
    else
    {
      if(element == 1)//HUG PRESSURE
      {
        encoderConstrainValMin = 0;
        encoderConstrainValMax = MaxPressure;
        hugVar.hugPressure = encoderInput * 689.4;
      }
      else if(element == 2)//HUG TIME
      {
        encoderConstrainValMin = 0;
        encoderConstrainValMax = 599;
        hugVar.hugTime = encoderInput;      
      }
      else if(element == 3)//SAVE
      {
         //BLANK
      }
      else if(element == 4)//HELP
      {
        u8g2.setDrawColor(0);
        u8g2.drawBox(0, 0, 128, 64);
        u8g2.setDrawColor(1);
        u8g2.setFont(u8g2_font_helvB08_tr);
        u8g2.drawStr(6, 30, "The hug icon is for one");
        u8g2.drawStr(8, 42, "inflation cycle.");
        u8g2.drawStr(6, 54, "Time can't be zero!");
        u8g2.drawXBMP( 54, 0, 18, 18, image_HUG_bits);
        encoderConstrainValMin = 0;
        encoderConstrainValMax = 1;
      }

    }
}

void configPage3()//AIRSYS
{
  if(SAVE){SAVE = false;}
  
  u8g2.setDrawColor(1);
  u8g2.drawXBMP( 67, 0, 17, 55, image_PRESSUREgauge1_bits);
  u8g2.drawXBMP( 41, 0, 17, 55, image_PUMPLEVEL_bits);
  u8g2.drawXBMP( 0, 57, 9, 7, image_Pin_arrow_left_9x7_bits);
  u8g2.drawXBMP( 1, 1, 36, 28, image_PUMP1_bits);
  u8g2.drawXBMP( 90, 52, 12, 12, image_help1_bits);
  u8g2.drawXBMP( 104, 53, 24, 11, image_KeySaveSelected_24x11_bits);

  u8g2.setDrawColor(0);
  byte temp = map(airSys.maxPumpPWM, 0, 1023, 51, 0);
  u8g2.drawBox(43, 2, 13, temp);
  

  u8g2.setDrawColor(2);
  temp = map(airSys.maxPressure, 0, (MaxPressure * 689.4), 51, 0);
  u8g2.drawBox(69, 2, 13, temp);
  
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_profont22_tr);

  char buf[4];
  float tempPressure = (airSys.maxPressure * 0.0146) / 100;
  dtostrf(tempPressure, 3, 1, buf);
  u8g2.drawStr(90, 14, buf);
  char buf1[4];
  tempPressure = (airSys.thresholdPressure * 0.0146) / 100;
  dtostrf(tempPressure, 3, 1, buf1);
  u8g2.drawStr(90, 40, buf1);

  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.drawStr(90, 23, "P.MAX");
  u8g2.drawStr(87, 49, "THRESH");
  u8g2.drawStr(4, 41, "PUMP");
  u8g2.drawStr(4, 51, "MAX");
  

  if(!changeValue)
    {
      if(encoderInput == 0)//BACK
      {
        if(!flash)
          {
            u8g2.setDrawColor(1);
            u8g2.drawXBMP( 0, 57, 9, 7, image_Pin_arrow_left_9x7_bits);
          }
          else
          {
            u8g2.setDrawColor(0);
            u8g2.drawBox(0, 57, 9, 7);
          }     
      }
      else if(encoderInput == 1)//MAXIMUM PWM
      {
        encoderInputTemp = map(airSys.maxPumpPWM, 0, 1023, 0, 51);
        if(!flash)
        {
          u8g2.setDrawColor(0);
          u8g2.drawBox(3, 31, 32, 21);
        }     
      }
      else if(encoderInput == 2)//MAXIMUM PRESSURE
      {
        encoderInputTemp = (airSys.maxPressure * 0.0146) / 10;
        if(!flash)
        {
          u8g2.setDrawColor(2);
          u8g2.drawBox(89, 0, 36, 14);
        }
      }
      else if(encoderInput == 3)//RAMP THRESHOLD PRESSURE
      {
        encoderInputTemp = (airSys.thresholdPressure * 0.0146) / 10;
        if(!flash)
        {
          u8g2.setDrawColor(2);
          u8g2.drawBox(89, 26, 36, 14);
        }
      }
      else if(encoderInput == 4)//SAVE
      {
        if(!flash)
          {
            u8g2.setDrawColor(2);
            u8g2.drawRBox(104, 53, 24, 11, 1);
          }
          SAVE = true;
      }
      else if(encoderInput == 5)//HELP
      {
        if(!flash)
        {
          u8g2.setDrawColor(2);
          u8g2.drawRBox(90, 52, 12, 12, 3);
        }
      }
    }
    else
    {
      if(element == 1)//MAXIMUM PWM
      {
        encoderConstrainValMin = 0;
        encoderConstrainValMax = 51;
        airSys.maxPumpPWM = map(encoderInput, 0, 51, 0, 1023);
      }
      else if(element == 2)//MAXIMUM PRESSURE
      {
        encoderConstrainValMin = 0;
        encoderConstrainValMax = MaxPressure;
        airSys.maxPressure = encoderInput * 689.4;
      }
      else if(element == 3)//RAMP THRESHOLD PRESSURE
      {
        encoderConstrainValMin = 0;
        encoderConstrainValMax = 5;
        airSys.thresholdPressure = encoderInput * 689.4;
      }
      else if(element == 4)//SAVE
      {
        //BLANK
      }
      else if(element == 5)//HELP
      {
        u8g2.setDrawColor(0);
        u8g2.drawBox(0, 0, 128, 64);
        u8g2.setDrawColor(1);
        u8g2.setFont(u8g2_font_helvB08_tr);
        u8g2.drawStr(0, 8, "PUMP MAX - Maximum");
        u8g2.drawStr(10, 17, "pump speed.");
        u8g2.drawStr(0, 29, "P.MAX - Maximum");
        u8g2.drawStr(10, 39, "system pressure.");
        u8g2.drawStr(0, 51, "THRESH - Pump ramping");
        u8g2.drawStr(10, 62, "threshold pressure.");   
        encoderConstrainValMin = 0;
        encoderConstrainValMax = 1;     
      }
    }
}

void configPage4()//MOTION
{
  
}
void configPage5()//CONFIG
{
  u8g2.setDrawColor(1);
  u8g2.drawXBMP( 117, 0, 11, 29, image_BAT_bits);
  u8g2.drawXBMP( 0, 53, 24, 11, image_KeySaveSelected_24x11_bits);
  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.drawStr(94, 8, "BAT");
  u8g2.drawStr(70, 62, "CLEAR ALL");
  u8g2.drawStr(0, 9, "PumpMin");
  u8g2.drawStr(0, 21, "PumpFreq");

  char buf[5];
  float tempVoltage = powerLevel * 0.00677;
  sprintf(buf, "%.1fV", tempVoltage);
  u8g2.drawStr(94, 18, buf);

  char buf1[5];
  int tempPWM = airSys.minPumpPWM;
  dtostrf(tempPWM, 3, 0, buf1);
  u8g2.drawStr(49, 10, buf1);

  char buf2[5];
  int tempFreq = airSys.freq;
  dtostrf(tempFreq, 4, 0, buf2);
  u8g2.drawStr(51, 22, buf2);

  u8g2.setDrawColor(0);
  int tempPowerLevel = map(powerLevel, 900, 1235, 23, 0); //CONVERT TO BARGRAPH
  u8g2.drawBox(119, 4, 7, tempPowerLevel);//BATTERY LEVEL INDECATOR

  if(!changeValue)
    {
      if(encoderInput == 0)//SAVE
      {
        if(!flash)
          {
            u8g2.setDrawColor(2);
            u8g2.drawBox(0, 53, 24, 11);
          }
      }
      else if(encoderInput == 1)//PUMP MINIMUM PWM
      {
        encoderInputTemp = airSys.minPumpPWM;
        if(!flash)
        {
          u8g2.setDrawColor(1);
          u8g2.drawFrame(47, 0, 21, 12);
        }     
      }
      else if(encoderInput == 2)//PWM FREQUENCY
      {
        
        encoderInputTemp = (airSys.freq * 0.02);
        if(!flash)
        {
          u8g2.setDrawColor(1);
          u8g2.drawFrame(50, 12, 26, 12);
        }
      }
      else if(encoderInput == 3)//ALL CLEAR
      {
        if(!flash)
          {
            u8g2.setDrawColor(2);
            u8g2.drawRBox(69, 52, 59, 11, 3);
          }
      }
    }
    else
    {
      if(element == 1)//PUMP MINIMUM PWM
      {
        encoderConstrainValMin = 200;
        encoderConstrainValMax = 1023;
        airSys.minPumpPWM = encoderInput;
      }
      else if(element == 2)//PWM FREQUENCY
      {
        encoderConstrainValMin = 2;
        encoderConstrainValMax = 199;
        airSys.freq = encoderInput * 50;
      }
      else if(element == 3)//CLEAR ALL
      {
        preferences.begin("DATAStore", false);
        preferences.clear();
        preferences.end();
        Serial.println("CLEARED");
        ESP.restart();     
      }
    }
}
