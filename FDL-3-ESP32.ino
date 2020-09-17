#include <Arduino.h>
#include <analogWrite.h>
#include <ESP32PWM.h>
#include <ESP32Tone.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <EEPROM.h>
#include <ESP32Servo.h>
#include <ESP32Encoder.h>
#include <MicroViewWidget.h>


#define MENU_SPEED  "Speed"
#define MENU_ROF    "ROF"
#define MENU_BURST  "Burst"
#define MENU_MINSPIN  "MinSpin"
#define MENU_MAXSPIN  "MaxSpin"
#define MENU_FIREMODE "FireMode"
#define MENU_SPINDOWN "Spindown"
#define MENU_IDLE   "Idle"
#define MENU_LOAD   "Load"
#define MENU_SAVE   "Save"
#define MENU_MINSPD "MinSpd"
#define MENU_MAXSPD "MaxSpd"
#define MENU_BTNMODE  "BtnMode"
#define MENU_BRKAG  "BrkAg"
#define MENU_USERLOCK "UserLock"
#define MENU_BRIGHT "Bright"
#define MENU_SOUND  "Sound"
#define MENU_BATOFFSET  "BatOff"
#define MENU_INFO "Info"


const byte versionNumber = 101;
const String splashText = "FDL-3 ESP32"; //can be two lines split with space

Servo flywheelESC; 
ESP32Encoder myEnc; 
MicroViewWidget *mainGauge;
MicroViewWidget *voltMeter;
//PARTY

// OLED DEF
#define OLED_DC     26
#define OLED_CS     25
#define OLED_MOSI   13
#define OLED_SCK    14
#define OLED_RESET  27
#define OLED_WIDTH  128
#define OLED_HEIGHT 64
#define OLED_HEADER 16
#define SMALL_T     1
#define LARGE_T     2
#define CH_SH       7
#define CH_LH       14
#define CH_SW       5
#define CH_LW       10
Adafruit_SSD1306 oled(OLED_WIDTH,OLED_HEIGHT, OLED_MOSI, OLED_SCK, OLED_DC, OLED_RESET, OLED_CS);

#define NOTHROTTLE 1000
#define MINTHROTTLE 1285
#define MAXTHROTTLE 2000

//PINS
#define pusherBrakePin 33
#define pusherEnablePin 32
#define escPin 34
#define buzzerPin 17
#define triggerPin 16
#define pusherSwitchPin 4
#define lockPin 5
#define voltMeterPin 2
#define presetBtnPin 34
#define encoderPin1 19
#define encoderPin2 18

//MENUS
const char *knobMenu[] = 
{
  MENU_SPEED,
  MENU_ROF,
  MENU_BURST,
  MENU_MINSPIN,
  MENU_MAXSPIN,
  MENU_FIREMODE,
  MENU_SPINDOWN,
  MENU_IDLE,
  MENU_LOAD,
  MENU_SAVE,
  MENU_MINSPD,
  MENU_MAXSPD,
  MENU_BTNMODE, 
  MENU_BRKAG,
  MENU_USERLOCK,
  MENU_SOUND,
  MENU_BATOFFSET,
  MENU_INFO
};
byte knobMenuIndex = 0;

const char *burstMenu[] = {"1","2","3","F"};
const char *soundMenu[] = {"OFF","ON"};
const char *presetMenu[] = {"Back","1","2","3"};
const char *btnmodeMenu[] = {"PRESET","SPEED","ROF","BURST"};
const char *firemodeMenu[] = {"SAFE","TRIG","AUTO","CACHE"};
byte presetMenuIndex = 0;

//variables
bool liveKnobScrollMode = false;
bool menuBtnWasDown = false;
bool firstMenuRun = true;
bool speedLocked = false;
bool firstRun = true;
unsigned long lastSpinCheck = 0;
unsigned long lastSettingsSave = 0;
unsigned long lastBatAlarm = 0;
unsigned long brakeRelease = 0;
unsigned long idleRelease = 0;
long encoderChange = 0;
int currentSpeed = NOTHROTTLE;
int spindownTarget = NOTHROTTLE;

#define batteryCheckLength 6
float batteryCheck[batteryCheckLength];
unsigned long lastBatteryCheck = 0;
int batteryCheckIndex = 0;
float batteryCheckSum = 0.0;

struct StaticSettings {
  byte btnMode;
  int brightness;
  byte soundOn;
  int batOffset;
  int brkAgr;
  int usrLock;
  int minSpeed;
  int maxSpeed;
};
StaticSettings currStSettings = { 3, 100, true, 0, 16, 0, 0, 100 };
StaticSettings lastStSettings = { 3, 100, true, 0, 16, 0, 0, 100 };
StaticSettings defStSettings = { 3, 100, true, 0, 16, 0, 0, 100 };

struct BlastSettings {
  int speedValue;
  int rofValue;
  byte burstCount;
  int minSpinup;
  int maxSpinup;
  int spinDown; //units per ms for single value drop (higher = slower)
  int idleTime;
  byte fireMode;  
};
BlastSettings currBlSettings = { 50, 100, 0, 220, 220, 14, 0, 1 };
BlastSettings lastBlSettings = { 50, 100, 0, 220, 220, 14, 0, 1 };
BlastSettings readBlSettings = { 50, 100, 0, 220, 220, 14, 0, 1 };
BlastSettings defBlSettings = { 50, 100, 0, 220, 220, 14, 0, 1 };

void setup() {
  oled.begin();
  oled.clearDisplay();
  oled.display();
  oled.setTextSize(LARGE_T);   
  oled.setTextColor(SSD1306_WHITE); // Draw white text
  oled.setCursor(0, 0);     // Start at top-left corner
  oled.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Init EEPROM
  EEPROM.begin(1024);

  // Setup Encoder
  myEnc.attachHalfQuad(encoderPin1, encoderPin2);

  // Setup ESC
  flywheelESC.attach(escPin); 
  flywheelESC.writeMicroseconds(0);  

  // Setup Pin Modes
  pinMode(voltMeterPin, INPUT);
  pinMode(presetBtnPin, INPUT);
  pinMode(pusherSwitchPin, INPUT);
  pinMode(triggerPin, INPUT_PULLUP);
  pinMode(lockPin, INPUT_PULLUP);
  pinMode(pusherEnablePin, OUTPUT);
  pinMode(pusherBrakePin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  oled.clearDisplay();
  renderSplash(splashText);  
  loadSettings();
  
  if(presetButtonDown() == 4){
    int tmpLocVal = currStSettings.usrLock;
    clearSetRoutine();
    currStSettings.usrLock = tmpLocVal;
    writeStaticSettings();
  }
  
  if(lockOn() && presetButtonDown() == 3){
    clearLockRoutine();
  }

  //PARTY
  mainGauge = new MicroViewGauge(OLED_WIDTH/2, OLED_HEADER + 24, 0, 100, WIDGETSTYLE1);
  voltMeter = new MicroViewSlider(OLED_WIDTH - 15, OLED_HEADER, 106, 126, WIDGETSTYLE3 + WIDGETNOVALUE);
  mainGauge->reDraw();
  voltMeter->reDraw();

  initBatteryCheck(); 

  if(currStSettings.usrLock != 0){
    renderUserLock();
  }
  else{
    flywheelESC.writeMicroseconds(NOTHROTTLE);
    
    renderScreen();
    startUpBeeps();
    
    if(lockOn()){
      delay(500);
      toneAlt(3000, 150);
      delay(150);
      toneAlt(3000, 150);
      speedLocked = true;
    }
  }
}

void loop() {

  if(currStSettings.usrLock != 0){
    renderUserLock();
    return;
  }

  //cap speed at max
  if(currBlSettings.speedValue > currStSettings.maxSpeed){
    currBlSettings.speedValue = currStSettings.maxSpeed;
  }
  
  //check for trigger
  if(triggerDown() && millis() > 2800){
    fireSequence();
  }
  else{
    if(millis() > brakeRelease){
      digitalWrite(pusherBrakePin, LOW); 
      delayMicroseconds(200);
    }
    if(millis() > idleRelease){
      spindownTarget = NOTHROTTLE; 
    }
  }
   
  spinDownFW(false);  
  renderScreen();

  if(lastSettingsSave != 0 && millis() - lastSettingsSave < 2000){
     //don't save
  }
  else{
    if(currentSettingsChanged() || staticSettingsChanged()){
      writeCurrentSettings();
      writeStaticSettings();
      lastSettingsSave = millis();      
    }
  }
}
