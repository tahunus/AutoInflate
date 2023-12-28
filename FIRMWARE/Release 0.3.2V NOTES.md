Keeps the exact same functionality as 0.3.1V but collects functions into 5 logical groups:

1. AUTOINFLATE_0.3.2V.ino - main program with #include´s, setup() and loop()
2. globalVariables.h - self explained
3. graphics.h - variables with bitmaps for display images
4. hwControl.h - hardware control (i.e. timers, interrupts, etc)
5. frontEnd.h - user interface (i.e. menus, etc)
6. backEnd.h - operations and control (i.e. operate pump, run inflation profiles, store data, etc.)

For better reference, see matrix below. Rows have the functions called by the function in that row. Columns have the functions that call the function in that column. 

![image](https://github.com/tahunus/AutoInflate/assets/33431200/31f5f72f-98d7-4024-90e8-9343b2052f07)

Some functions were refactored:


## runningAnimation()
Was defined in line 1505 and called in line 545 inside mainPage().  Since it was used only in mainPage(), the code was refactored first declaring  a pointer array to the 3 image files defined in frontEnd.h, line 11 as
```c++
static const unsigned char* image_VEST2_ANIMATIONS[] U8X8_PROGMEM = { //pointer array to the graphics variables
    image_VEST2_ANIMATION_1_bits,
    image_VEST2_ANIMATION_2_bits,
    image_VEST2_ANIMATION_3_bits
    //more animations...
};
```
and the code inserted directly into mainPage() (now in line 76 of frontEnd.h)
```c++
  if(airSys.pumpState) {
    u8g2.drawXBMP(8, 29, 25, 31, image_VEST2_ANIMATIONS[frameCount]); //frameCount always >= 0 && <= 2
    if(currentMillis - animationPreviousMillis >= animationFlipInterval) {
      animationPreviousMillis = currentMillis;
      frameCount++;
      if (frameCount > 2) frameCount = 0;
    }
  }
```

## configMainPage()
Was defined in line 603 using 172 lines of code.  It was refactored using only 19 lines of code (now defined in line 101 of frontEnd.h) by first declaring a struct with the screen coordinates of all images and text:
```c++
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
```
and using _encoderInput_ as the index to retrieve data from the array:
```c++
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
```

## configPage1(), configPage2() & configPage3()
They were defined starting in line 777 and now reside in frontEnd.h starting in line 122.  The three functions received new local variables defining the screen coordinates for their text strings. For example, in configPage1():
```c++
  //define X, Y & W for input field of PRESSURE, ON-TIME, OFF-TIME & CYCLE TIME
  const int txtX[4] = {9,35,57,94}; 
  const int txtY[4] = {21,10,56,23};
  const int txtW[4] = {18,24,24,30};
```
and using _encoderInput_ as index to retrieve data from the array, plus some other tactics to reduce repetitive code. For example, see this selection using a switch statement in _configPage1()_ (vs nested IF/ELSE statements):
```c++
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
```
Total lines of code for the three functions were reduced from 407 to 242.

## buttonPressFunc(), encoderInteraction()
_buttonPressFunc()_ was defined in line 1374 and because it was only used by _encoderInteraction()_ (defined in line 1283), it was eliminated and its code refactored and included in the new _encoderInteraction()_ now defined in line 336 of backEnd.h with a reformatted switch statement that also saved 20 lines of code:
```c++
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
```
