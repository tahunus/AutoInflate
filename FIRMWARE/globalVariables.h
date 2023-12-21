byte MaxPressure = 30; //This number should remain low! Somewhere around 30 for 3PSI for safety!

//PINS
const int encoderPinA = 7;  // Encoder pin A
const int encoderPinB = 6;  // Encoder pin B
const int buttonPin = 0;    // Button pin
const int solenoidPin = 9;  // Solenoid
const int pumpPin = 10;     // Pump
const int RGB = 11;         // RGB LED
const int BATTERY = 5;      // Battery voltage

//HARDWARE
Preferences preferences;
MS5837 sensor;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
Adafruit_NeoPixel pixels(2, RGB, NEO_GRB + NEO_KHZ800);
const int pumpChannel = 0;
const int resolution = 10;
int powerLevel = 0;

//TIMERS
hw_timer_t *timer1 = NULL;
hw_timer_t *timer2 = NULL;

//ENCODER/BUTTON VARIABLES
bool lastEncoderState = LOW;
bool isButtonPressed = false;
int encoderInputTemp = 0;
int encoderInput = 0;
int encoderConstrainValMin = 0;
int encoderConstrainValMax = 3;
volatile unsigned long DebounceTimer;
volatile int ButtonPressed;
volatile unsigned int delayTime = 100;

//NAVIGATION VARIABLES
bool SAVE = false;
bool changeValue = false;
byte pageNumber = 1;
byte element = 0;
int pulseFeedback = 0;
uint32_t currentMillis = 0;
//Limit the encoder input for elements on a specific page. 
//BLANK, MainPage, ConfigPage, Profile, Hug, AirSYS, Motion, Config, WIFI, -----, -----
int PageElements[11] = {0, 3, 8, 6, 4, 5, 0, 3, 0, 0, 0}; 

//AVERAGING VARIABLES
const int numReadings = 5;//Number of readings
uint32_t readings[numReadings]; 
int readIndex = 0;
uint32_t total = 0;
uint32_t average = 0;

//PRESSURE INTERVALES
const unsigned long pressureReadInterval = 150;
unsigned long pressurePreviousMillis = 0;

//FLASH INTERVALES
const int selectorInterval = 200; //0.2 sec
uint32_t selectorPreviousMillis = 0;
bool flash = false;

//ANIMATION TIME
const int animationFlipInterval = 200; //0.2sec
uint32_t animationPreviousMillis = 0;
byte frameCount = 0;

//PROFILE VARIABLES
struct profileAttributes 
{
  byte runState;
  int highPressure;
  int onTime;
  int offTime;
  int cycleTime;
}; profileAttributes profileVar = {0,0,0,0,0};

//AIRSYS VARIABLES
struct airSystemAttributes
{
  int freq;
  int maxPumpPWM;
  int minPumpPWM;
  byte rampPump;
  byte pumpState; //0-OFF, 1-ON
  byte solenoidState; //0-OFF, 1-ON
  byte runType; //0-STOP, 1-PROFILE, 2-HUG
  int pumpPressure;
  int runTime;
  int maxPressure;
  int thresholdPressure;
  int PRESSUREmbar;
  int PRESSUREAveragembar;
  float PRESSUREAveragepsi;
  int pressureOffset;
}; airSystemAttributes airSys = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //-------------------------------added 15th initializer

//MOTION VARIABLES
struct motionAttributes
{
  byte tapsDetected;
  int xMotion;
  int yMotion;
  int zMotion;
}; motionAttributes motionVar = {0,0,0,0};

//HUG VARIABLES
struct hugAttributes
{
  int hugTime;
  int hugPressure;
}; hugAttributes hugVar = {0,0};
