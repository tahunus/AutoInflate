#include "graphics.h"

//GRAPHICS
void clearScreen() {
  u8g2.setDrawColor(0);       
  u8g2.drawBox(0, 0, 128, 64);
  u8g2.setDrawColor(1);
}

//VEST ANIMATION DURING PROFILE & HUG RUNS
static const unsigned char* image_VEST2_ANIMATIONS[] U8X8_PROGMEM = { //pointer array to the graphics variables
    image_VEST2_ANIMATION_1_bits,
    image_VEST2_ANIMATION_2_bits,
    image_VEST2_ANIMATION_3_bits
    //more animations...
};

//CONFIG MAIN PAGE
struct ConfigMainPage {
    const char* text;
    int txtX;
    const uint8_t* image; //using a pointer to avoid making a copy of a large variable
    int imgX, imgY, imgW, imgH;
};
ConfigMainPage configMainMenu[] = { //values for encoderInput 0..8
    {"BACK",      39, image_Pin_arrow_left_9x7_bits,  0, 57,  9,  7},  //used in all subConfig pages
    {"PROFILE",   39, image_profile_bits,             6,  9, 28, 29},
    {"HUG",       63, image_HUG_LARGE_bits,          24, 12, 32, 29},
    {"AIRSYS",    49, image_PUMP1_bits,               7,  9, 36, 28},
    {"MOTION",    49, image_TAP_bits,                 6,  4, 38, 36},
    {"CONFIG",    51, image_GEAR_bits,                6,  3, 41, 41},
    {"WIFI",      66, image_WIRELESS_bits,           10,  9, 48, 34},    
    {"-----TBD1", 11, nullptr,                        0,  0,  0,  0},
    {"-----TBD2", 11, nullptr,                        0,  0,  0,  0}  
    //more configurations...
};
int frameX = 17; //X position for first frame. Subsequent frames spaced @ frameW +2 to leave 1 pixel between frames
int frameY = 54;
int frameW = 10;
int frameH = 10;

//SELECT ARROW FROM MAIN PAGE
const byte arrowX[] = {55,75,94,112}; //selectArrow X position for STOP, HUG, PROFILE, CONFIG

//MENU PAGES
void mainPage() {
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

  //for runningAnimation
  if(airSys.pumpState) {
    u8g2.drawXBMP(8, 29, 25, 31, image_VEST2_ANIMATIONS[frameCount]); //frameCount always >= 0 && <= 2
    if(currentMillis - animationPreviousMillis >= animationFlipInterval) {
      animationPreviousMillis = currentMillis;
      frameCount++;
      if (frameCount > 2) frameCount = 0;
    }
  }

  u8g2.setDrawColor(0);
  int tempPowerLevel = map(powerLevel, 900, 1235, 23, 0); //CONVERT TO BARGRAPH
  u8g2.drawBox(119, 39, 7, tempPowerLevel); //BATTERY LEVEL INDICATOR

  //draw the selectArrow and flash it if airSys is running
  if (airSys.runType > 0)  { 
    u8g2.setDrawColor(!flash);
    u8g2.drawXBMP(arrowX[encoderInput], 19, 14, 7, image_selectArrow_bits);
  }
  else  {
    u8g2.setDrawColor(1);
    u8g2.drawXBMP(arrowX[encoderInput], 19, 14, 7, image_selectArrowOpen_bits);
  }
}

void configMainPage() {
  //draw the "BACK" arrow and the frames for all 8 config menu options
  u8g2.setDrawColor(1);
  u8g2.drawXBMP(0, 57, 9, 7, image_Pin_arrow_left_9x7_bits);
  for (int i = 0; i < 8; i++) 
    u8g2.drawFrame(frameX + (frameW +2) * i, frameY, frameW, frameH);

  //draw the text & image of the option selected by encoderInput
  const ConfigMainPage& cfg = configMainMenu[encoderInput];  //defined as reference (&) to avoid copying a big variable
  u8g2.setFont(u8g2_font_profont22_tr);
  u8g2.drawStr(cfg.txtX, 32, cfg.text);
  u8g2.drawXBMP(cfg.imgX, cfg.imgY, cfg.imgW, cfg.imgH, cfg.image);

  //flash the corresponding frame or the BACK arrow
  u8g2.setDrawColor(!flash);
  if (encoderInput > 0) 
    u8g2.drawBox(frameX + (frameW +2) * (encoderInput -1) +1, frameY +1, frameW -2 , frameH -2);
  else 
    u8g2.drawXBMP(cfg.imgX, cfg.imgY, cfg.imgW, cfg.imgH, cfg.image);
}

void configPage1() { //PROFILE
  //define X, Y & W for input field of PRESSURE, ON-TIME, OFF-TIME & CYCLE TIME
  const int txtX[4] = {9,35,57,94}; 
  const int txtY[4] = {21,10,56,23};
  const int txtW[4] = {18,24,24,30};

  if(SAVE){SAVE = false;}
  
  char buf[4];
  float tempPressure = (profileVar.highPressure * 0.0146) / 100;
  dtostrf(tempPressure, 3, 1, buf);
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.drawStr(txtX[0], txtY[0], buf);
  
  int onTimeMin = (profileVar.onTime / 60) % 60;
  int onTimeSec = profileVar.onTime % 60;
  char buf1[8];   //changed from 5 (need -> 4 (for onTimeMin) + 1 (for :) + 2 (for onTimeSec) + 1 (for \0) = 7 bytes)
  sprintf(buf1, "%d:%02d", onTimeMin, onTimeSec);
  u8g2.drawStr(txtX[1], txtY[1], buf1);

  int offTimeMin = (profileVar.offTime / 60) % 60;
  int offTimeSec = profileVar.offTime % 60;
  char buf2[8];    //changed from 5
  sprintf(buf2, "%d:%02d", offTimeMin, offTimeSec);
  u8g2.drawStr(txtX[2], txtY[2], buf2);

  int cycleTimeMin = (profileVar.cycleTime / 60) % 60;
  int cycleTimeSec = profileVar.cycleTime % 60;
  char buf4[8];  //changed from 6
  sprintf(buf4, "%02d:%02d", cycleTimeMin, cycleTimeSec);
  u8g2.drawStr(txtX[3], txtY[3], buf4); 

  u8g2.drawXBMP( 0, 57, 9, 7, image_Pin_arrow_left_9x7_bits);
  u8g2.drawXBMP( 26, 13, 60, 32, image_graph4_bits);
  u8g2.drawXBMP( 92, 27, 30, 19, image_cycle2_bits);
  u8g2.drawXBMP( 90, 52, 12, 12, image_help1_bits);
  u8g2.drawXBMP( 104, 53, 24, 11, image_KeySaveSelected_24x11_bits);
  
  if(!changeValue) {
    switch (encoderInput) {
      case 0: u8g2.setDrawColor(!flash);                                  //flash BACK arrow
              u8g2.drawXBMP( 0, 57, 9, 7, image_Pin_arrow_left_9x7_bits);  
              break;
      case 1: encoderInputTemp = (profileVar.highPressure * 0.0146) / 10; //PROFILE PRESSURE
              break;
      case 2: encoderInputTemp = profileVar.onTime;                       //PROFILE ON-TIME
              break;
      case 3: encoderInputTemp = profileVar.offTime;                      //PROFILE OFF-TIME
              break;
      case 4: encoderInputTemp = profileVar.cycleTime;                    //PROFILE CYCLE TIME
              break;
      case 5: SAVE = true; 
              if (!flash) u8g2.drawRBox(104, 53, 24, 11, 1);              //flash SAVE icon
              break;
      case 6: if (!flash) u8g2.drawRBox(90, 52, 12, 12, 3);               //flash HELP icon
    }
    if (encoderInput > 0 && encoderInput < 5 && !flash) //flash the frame around the selected text input
        u8g2.drawFrame(txtX[encoderInput-1] -2, txtY[encoderInput-1] -10, txtW[encoderInput-1], 12);
  }
  else {
    encoderConstrainValMin = 0;
    switch (element) {
      case 1: encoderConstrainValMax = MaxPressure; //PROFILE PRESSURE
              profileVar.highPressure = encoderInput * 689.4;
              break;
      case 2: encoderConstrainValMax = 599;         //PROFILE ON TIME
              profileVar.onTime = encoderInput; 
              break;
      case 3: encoderConstrainValMax = 599;         //PROFILE OFF TIME
              profileVar.offTime = encoderInput; 
              break;
      case 4: encoderConstrainValMax = 5999;        //PROFILE CYCLE TIME
              profileVar.cycleTime = encoderInput;
              break;
      case 5: break;                                //SAVE
      case 6: clearScreen();                        //HELP
              u8g2.drawStr(4, 30, "The profile icon is for");
              u8g2.drawStr(4, 42, "multiple inflation cycles.");
              u8g2.drawStr(4, 54, "Times can't be zero!");
              u8g2.drawXBMP( 55, 0, 16, 18, image_profileMINI_bits);
              encoderConstrainValMax = 1;  
    }
  }
}

void configPage2() { //HUG
  //define X, Y & W for input field of HUG PRESSURE, HUG TIME
  const int txtX[2] = {17,53}; 
  const int txtY[2] = {21,11};
  const int txtW[2] = {18,24};

  if(SAVE){SAVE = false;}

  u8g2.setDrawColor(1);
  u8g2.drawXBMP( 33, 13, 60, 32, image_graph5_bits);
  u8g2.drawXBMP( 0, 57, 9, 7, image_Pin_arrow_left_9x7_bits);
  u8g2.drawXBMP( 104, 53, 24, 11, image_KeySaveSelected_24x11_bits);
  u8g2.drawXBMP( 90, 52, 12, 12, image_help1_bits);

  int onTimeMin = (hugVar.hugTime / 60) % 60;
  int onTimeSec = hugVar.hugTime % 60;

  char buf[8];  //changed from 5
  sprintf(buf, "%d:%02d", onTimeMin, onTimeSec);
  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.drawStr(txtX[1], txtY[1], buf);

  char buf1[4];
  float tempPressure = (hugVar.hugPressure * 0.0146) / 100;
  dtostrf(tempPressure, 3, 1, buf1);
  u8g2.drawStr(txtX[0], txtY[0], buf1); 

  if(!changeValue) {
    switch (encoderInput) {
      case 0: u8g2.setDrawColor(!flash);                             //flash BACK arrow
              u8g2.drawXBMP( 0, 57, 9, 7, image_Pin_arrow_left_9x7_bits);  
              break;
      case 1: encoderInputTemp = (hugVar.hugPressure * 0.0146) / 10; //HUG PRESSURE
              break;
      case 2: encoderInputTemp = hugVar.hugTime;                     //HUG TIME
              break;
      case 3: SAVE = true; 
              if (!flash) u8g2.drawRBox(104, 53, 24, 11, 1);         //flash SAVE icon
              break;
      case 4: if (!flash) u8g2.drawRBox(90, 52, 12, 12, 3);          //flash HELP icon
    }
    if (encoderInput > 0 && encoderInput < 3 && !flash) //flash the frame around the selected text input
        u8g2.drawFrame(txtX[encoderInput-1] -2, txtY[encoderInput-1] -10, txtW[encoderInput-1], 12);
  }
  else {
    encoderConstrainValMin = 0;
    switch (element) {
      case 1: encoderConstrainValMax = MaxPressure; //HUG PRESSURE
              hugVar.hugPressure = encoderInput * 689.4;
              break;
      case 2: encoderConstrainValMax = 599;         // HUG TIME
              hugVar.hugTime = encoderInput; 
              break;                                //SAVE
      case 3: break;         
      case 4: clearScreen();                        //HELP
              u8g2.drawStr(6, 30, "The hug icon is for one");
              u8g2.drawStr(8, 42, "inflation cycle.");
              u8g2.drawStr(6, 54, "Time can't be zero!");
              u8g2.drawXBMP( 54, 0, 18, 18, image_HUG_bits);
              encoderConstrainValMax = 1;  
    }
  }
}

void configPage3()  { //AIRSYS
  //define X, Y, W & H for TXT messages of MAX PWM, MAX PRESSURE, RAMP THRESHOLD PRESSURE
  const int txtX[3] = {3,89,89}; 
  const int txtY[3] = {31,0,26};
  const int txtW[3] = {32,36,36};
  const int txtH[3] = {21,14,14};

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
  
  if(!changeValue) {
    switch (encoderInput) {
      case 0: u8g2.setDrawColor(!flash);                                   //flash BACK arrow
              u8g2.drawXBMP( 0, 57, 9, 7, image_Pin_arrow_left_9x7_bits);  
              break;
      case 1: encoderInputTemp = map(airSys.maxPumpPWM, 0, 1023, 0, 51);   //MAXIMUM PWM
              break;
      case 2: encoderInputTemp = (airSys.maxPressure * 0.0146) / 10;;      //MAXIMUM PRESSURE
              break;
      case 3: encoderInputTemp = (airSys.thresholdPressure * 0.0146) / 10; //RAMP THRESHOLD PRESSURE
              break;
      case 4: SAVE = true; 
              if (!flash) u8g2.drawRBox(104, 53, 24, 11, 1);               //flash SAVE icon
              break;
      case 5: u8g2.setDrawColor(2); 
              if (!flash) u8g2.drawRBox(90, 52, 12, 12, 3);                //flash HELP icon
              break;
    }
    if (encoderInput > 0 && encoderInput < 4 && !flash) { //flash the frame around the selected text input
        u8g2.setDrawColor(2);
        u8g2.drawBox(txtX[encoderInput-1], txtY[encoderInput-1], txtW[encoderInput-1], txtH[encoderInput-1]);
    }
  }
  else {
    encoderConstrainValMin = 0;
    switch (element) {
      case 1: encoderConstrainValMax = 51;          //MAXIMUM PWM
              airSys.maxPumpPWM = map(encoderInput, 0, 51, 0, 1023);
              break;
      case 2: encoderConstrainValMax = MaxPressure; //MAXIMUM PRESSURE
              airSys.maxPressure = encoderInput * 689.4; 
              break;                              
      case 3: encoderConstrainValMax = 5;           //RAMP THRESHOLD PRESSURE
              airSys.thresholdPressure = encoderInput * 689.4;
              break;   
      case 4: break;                                //SAVE - BLANK     
      case 5: clearScreen();                        //HELP
              u8g2.setFont(u8g2_font_helvB08_tr);
              u8g2.drawStr(0, 8, "PUMP MAX - Maximum");
              u8g2.drawStr(10, 17, "pump speed.");
              u8g2.drawStr(0, 29, "P.MAX - Maximum");
              u8g2.drawStr(10, 39, "system pressure.");
              u8g2.drawStr(0, 51, "THRESH - Pump ramping");
              u8g2.drawStr(10, 62, "threshold pressure.");   
              encoderConstrainValMax = 1;
              break;
    }
  }
}

void configPage4()//MOTION --- TBD
{}

void configPage5()  { //CONFIG
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

  //char buf1[5];
  int tempPWM = airSys.minPumpPWM;
  dtostrf(tempPWM, 3, 0, buf);
  u8g2.drawStr(49, 10, buf);

  //char buf2[5];
  int tempFreq = airSys.freq;
  dtostrf(tempFreq, 4, 0, buf);
  u8g2.drawStr(51, 22, buf);

  u8g2.setDrawColor(0);
  int tempPowerLevel = map(powerLevel, 900, 1235, 23, 0); //CONVERT TO BARGRAPH
  u8g2.drawBox(119, 4, 7, tempPowerLevel);//BATTERY LEVEL INDECATOR

  if(!changeValue)
    switch (encoderInput) {
      case 0: if(!flash) {                             //SAVE
                u8g2.setDrawColor(2);
                u8g2.drawBox(0, 53, 24, 11);
              }
              break;
      case 1: encoderInputTemp = airSys.minPumpPWM;    //PUMP MINIMUM PWM
              if(!flash) {
                u8g2.setDrawColor(1);
                u8g2.drawFrame(47, 0, 21, 12);
              }
              break;
      case 2: encoderInputTemp = (airSys.freq * 0.02); //PWM FREQUENCY
              if(!flash) {
                u8g2.setDrawColor(1);
                u8g2.drawFrame(50, 12, 26, 12);
              }
              break;
      case 3: if(!flash) {                             //ALL CLEAR
                u8g2.setDrawColor(2);
                u8g2.drawRBox(69, 52, 59, 11, 3);
              }
    }
  else
    switch (element) {
      case 1: encoderConstrainValMin = 200;          //PUMP MINIMUM PWM
              encoderConstrainValMax = 1023;
              airSys.minPumpPWM = encoderInput;
              break;
      case 2: encoderConstrainValMin = 2;            //PWM FREQUENCY
              encoderConstrainValMax = 199;
              airSys.freq = encoderInput * 50;
              break;
      case 3: preferences.begin("DATAStore", false); //CLEAR ALL
              preferences.clear();
              preferences.end();
              Serial.println("CLEARED");
              ESP.restart();    
    }
}

void displayData()  {//CONTINUOUS EXECUTE
  switch (pageNumber)  {
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
  }
}