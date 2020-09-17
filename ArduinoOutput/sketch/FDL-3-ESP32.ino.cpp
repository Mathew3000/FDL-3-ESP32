#line 1 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FDL-3-ESP32.ino"
#line 1 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FDL-3-ESP32.ino"
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
  "Speed",
  "ROF",
  "Burst", 
  "MinSpin",
  "MaxSpin",   
  "FireMode",
  "Spindown",
  "Idle",
  "Load",
  "Save",
  "MinSpd",
  "MaxSpd",
  "BtnMode",
  "BrkAg",
  "ULock",
  "Bright",
  "Sound",
  "BatOff",
  "Info"    
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

#line 143 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FDL-3-ESP32.ino"
void setup();
#line 214 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FDL-3-ESP32.ino"
void loop();
#line 27 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\CycleHelpers.ino"
void cycleShutdown();
#line 32 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\CycleHelpers.ino"
void shutdownFwAndPush();
#line 37 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\CycleHelpers.ino"
void spinDownFW(bool resetCheckTimer);
#line 58 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\CycleHelpers.ino"
bool checkCount();
#line 73 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\CycleHelpers.ino"
void triggerLogic(bool &triggerState, byte &shotsCached);
#line 93 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\CycleHelpers.ino"
bool enterFireLoop(byte shotsCached);
#line 112 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\CycleHelpers.ino"
bool enterAutoLoop(byte shotsCached);
#line 131 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\CycleHelpers.ino"
bool inFullAuto();
#line 135 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\CycleHelpers.ino"
boolean triggerDown();
#line 139 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\CycleHelpers.ino"
boolean lockOn();
#line 143 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\CycleHelpers.ino"
int presetButtonDown();
#line 164 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\CycleHelpers.ino"
int pusherSwitchDown();
#line 176 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\CycleHelpers.ino"
int readESCPower();
#line 180 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\CycleHelpers.ino"
void setFwSpeed(int newSpeed);
#line 184 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\CycleHelpers.ino"
int getSpinupByMotorSpeed();
#line 197 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\CycleHelpers.ino"
double doubleMap(double x, double in_min, double in_max, double out_min, double out_max);
#line 201 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\CycleHelpers.ino"
byte getBurstCount();
#line 211 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\CycleHelpers.ino"
int getROF();
#line 9 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FireCycle.ino"
bool fireSequence();
#line 4 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Presets.ino"
void loadSettings();
#line 22 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Presets.ino"
void loadStaticSettings();
#line 27 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Presets.ino"
void writeStaticSettings();
#line 34 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Presets.ino"
bool staticSettingsChanged();
#line 46 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Presets.ino"
void loadCurrentSettings();
#line 53 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Presets.ino"
void writeCurrentSettings();
#line 74 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Presets.ino"
bool currentSettingsChanged();
#line 86 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Presets.ino"
void readPreset(byte presetIndex);
#line 91 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Presets.ino"
void writePreset(byte presetIndex);
#line 100 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Presets.ino"
void loadPreset(byte presetIndex);
#line 110 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Presets.ino"
void writeDefaultSettings();
#line 2 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\RenderHelpers.ino"
void renderVoltMeter();
#line 58 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\RenderHelpers.ino"
void batteryWarning();
#line 64 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\RenderHelpers.ino"
void batteryCritical();
#line 74 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\RenderHelpers.ino"
void renderLockIndicator();
#line 84 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\RenderHelpers.ino"
void renderInfoMenu();
#line 100 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\RenderHelpers.ino"
void renderUserLock();
#line 208 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\RenderHelpers.ino"
void renderKnobScrollMenu();
#line 253 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\RenderHelpers.ino"
void renderMenu(byte &menuIndex, const char label[], const char* menu[], byte arraySize);
#line 299 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\RenderHelpers.ino"
void renderGauge(int &gaugeValue, String label, int gaugeMin, int gaugeMax, int valueMin, int valueMax, int detPerMove);
#line 333 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\RenderHelpers.ino"
void renderSplash(String splashText);
#line 411 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\RenderHelpers.ino"
void renderPresetMenu();
#line 463 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\RenderHelpers.ino"
void renderPreset(byte preset);
#line 4 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Renderer.ino"
void renderScreen();
#line 129 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Renderer.ino"
void presetButtonAction(int presButton);
#line 3 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Utils.ino"
void toneAlt(int frequency, int duration);
#line 19 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Utils.ino"
void initBatteryCheck();
#line 28 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Utils.ino"
float getVoltLevel();
#line 55 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Utils.ino"
void startUpBeeps();
#line 63 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Utils.ino"
void clearSetRoutine();
#line 86 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Utils.ino"
void clearLockRoutine();
#line 143 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FDL-3-ESP32.ino"
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
  mainGauge = new MicroViewGauge(SSD1306_LCDWIDTH/2, OLED_HEADER + 24, 0, 100, WIDGETSTYLE1);
  voltMeter = new MicroViewSlider(56, 14, 106, 126, WIDGETSTYLE3);
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

#line 1 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\CycleHelpers.ino"
#define PSHFRNT 1
#define PSHREAR 2
#define PSHNONE 0

#define ANALOG_NONE 500
#define ANALOG_BT_1 4100
#define ANALOG_BT_2 3300
#define ANALOG_BT_3 2700
#define ANALOG_BT_ROT 2400

#define PRSTNONE 0
#define PRST1 1
#define PRST2 2
#define PRST3 3
#define PRSTROT 4

#define SAFETY 0
#define ABIDE 1
#define AUTO 2
#define CACHED 3

unsigned long logicLastCheck = 0;
unsigned long logicSwitchCheck = 0;
int clickCount = 0;
int hitCount = 0;

void cycleShutdown(){
  digitalWrite(pusherEnablePin, LOW);
  spinDownFW(true);
}

void shutdownFwAndPush(){
  digitalWrite(pusherEnablePin, LOW);
  setFwSpeed(NOTHROTTLE);
}

void spinDownFW(bool resetCheckTimer){
    if(resetCheckTimer){
      lastSpinCheck = 0;
    }
  
    if(currentSpeed > spindownTarget){
      if(currBlSettings.spinDown == 0){
        currentSpeed = spindownTarget;
      }
      else{
        unsigned long elapsedTime = lastSpinCheck == 0 ? 0 : millis() - lastSpinCheck;
        lastSpinCheck = millis();  
        int spindown = elapsedTime / currBlSettings.spinDown * 10;
        lastSpinCheck -= elapsedTime % currBlSettings.spinDown;        
        currentSpeed -= spindown;
        currentSpeed = max(spindownTarget, currentSpeed);
      }
    }
    setFwSpeed(currentSpeed);
}

bool checkCount(){  
  if(millis() - logicSwitchCheck > 1 && pusherSwitchDown() == PSHFRNT){
    logicSwitchCheck = millis();
    hitCount++;
  }

  if(hitCount > 3){    
    hitCount = 0;    
    return true;
  }
  else{
    return false;
  }
}

void triggerLogic(bool &triggerState, byte &shotsCached){  
  if(currBlSettings.fireMode == CACHED){ // in cache mode
      
    if(triggerState == false && triggerDown()){
      clickCount++;
    }
    if(triggerState == true && !triggerDown()){
      clickCount++;
    }

    if(clickCount > 50){
      clickCount = 0;
      triggerState = !triggerState;
      if(triggerState == true){
        shotsCached++;
      }
    }    
  }  
}

bool enterFireLoop(byte shotsCached){
  if(shotsCached > 0){  
    if(inFullAuto()){//full auto always abide by trigger except in auto mode for single shot
      return triggerDown() || (currBlSettings.fireMode != ABIDE && shotsCached >= 100);
    }  
    if(currBlSettings.fireMode == ABIDE){//trigger abide mode
      return triggerDown();
    }
    if(currBlSettings.fireMode == AUTO){//auto mode
      return true;
    }
    if(currBlSettings.fireMode == CACHED){//cached mode
      return true;
    }
  }
  
  return false;
}

bool enterAutoLoop(byte shotsCached){  
  if(shotsCached > 1){    
    if(inFullAuto()){//full auto always abide by trigger
      return triggerDown();
    }
    if(currBlSettings.fireMode == ABIDE){//trigger abide mode
      return triggerDown();
    }
    if(currBlSettings.fireMode == AUTO){//auto mode
      return true;
    }
    if(currBlSettings.fireMode == CACHED){//cached mode
      return true;
    }
  }
  
  return false;
}

bool inFullAuto(){
  return getBurstCount() == 100;
}

boolean triggerDown(){
  return digitalRead(triggerPin) == LOW;
}

boolean lockOn(){
  return digitalRead(lockPin) == LOW;
}

int presetButtonDown(){
  int readVal = analogRead(presetBtnPin);

  if(readVal < ANALOG_NONE){
    return PRSTNONE;
  }
  if(readVal < ANALOG_BT_ROT){
    return PRSTROT;
  }
  if(readVal < ANALOG_BT_3){
    return PRST3;
  }
  if(readVal < ANALOG_BT_2){
    return PRST2;
  }
  if(readVal < ANALOG_BT_1)
  {
    return PRST1;
  }
}

int pusherSwitchDown(){
  int readVal = analogRead(pusherSwitchPin);

  if(readVal < 400){
    return PSHNONE;
  }
  if(readVal < 900){
    return PSHFRNT;
  }
  return PSHREAR;
}

int readESCPower(){
  return map(currBlSettings.speedValue, 0, 100, MINTHROTTLE, MAXTHROTTLE);
}

void setFwSpeed(int newSpeed){
  flywheelESC.writeMicroseconds(newSpeed);
}

int getSpinupByMotorSpeed(){   
  double calcSpin = doubleMap(currBlSettings.speedValue, 0, 100, currBlSettings.minSpinup, currBlSettings.maxSpinup);
  if(currentSpeed < MINTHROTTLE){ return calcSpin; }
  if(currBlSettings.speedValue == 0){ return 0; }
  
  double minSync = 50;
  double targetThrottle = doubleMap(currBlSettings.speedValue, 0, 100, MINTHROTTLE, MAXTHROTTLE);  
  double throttleFactor =  (targetThrottle - currentSpeed) / (targetThrottle - MINTHROTTLE);
  calcSpin = calcSpin * throttleFactor - minSync;
  
  return max(0.0, calcSpin);
}

double doubleMap(double x, double in_min, double in_max, double out_min, double out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

byte getBurstCount(){
  byte arraySize = sizeof(burstMenu) / sizeof(size_t);
  if(currBlSettings.burstCount >= arraySize - 1){
    return 100;
  }
  else{
    return currBlSettings.burstCount + 1;
  }
}

int getROF(){
  return currBlSettings.rofValue;
}

#line 1 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FireCycle.ino"
const int frontDownLoopDelay = 3; //delay till pass front switch if start from front rare case
const int frontDownLoopDur = 50; //wait delay till front switch up error if not met
const int frontUpLoopDur = 200; //wait delay to hit front switch error if not hit
const int frontPassDelay = 5; //pseudo debounce delay at front switch
const int rearUpLoopDur = 80; //first wait for rear switch
const int rearUpLoopDur2 = 200; //backup wait for rear switch
const int pusherBlipDur = 8; //pusher blip in attempts to reach back switch

bool fireSequence(){

    if(currBlSettings.fireMode == SAFETY){
      toneAlt(3000, 50);
      delay(100);
      toneAlt(2000, 100);
      return true;
    }
    
    //start wheels 
    int spinupDelay = getSpinupByMotorSpeed();
    currentSpeed = readESCPower();
    setFwSpeed(currentSpeed);
    
    unsigned long loopLastCheck = 0;

    //get cached shot count  
    byte cacheShots = getBurstCount();
    bool triggerState = true;
    
    //spinup
    loopLastCheck = millis();
    while(millis() - loopLastCheck < spinupDelay ){
      triggerLogic(triggerState, cacheShots);
    }

    //if in abide and trigger up, shutdown
    if(!triggerDown() && currBlSettings.fireMode == ABIDE){
      cycleShutdown();
      idleRelease = millis() + currBlSettings.idleTime * 1000;
      spindownTarget = MINTHROTTLE;      
      return true;
    }

    //shots cached
    while(enterFireLoop(cacheShots)){

      //disable brake
      digitalWrite(pusherBrakePin, LOW); 
      delayMicroseconds(200);

      digitalWrite(pusherEnablePin, HIGH);      
      
      //fire cycle begin
      //while front down / left at front, run
      //rare state
      loopLastCheck = millis();
      while(pusherSwitchDown() == PSHFRNT){
        if(millis() - loopLastCheck < frontDownLoopDur){          
          triggerLogic(triggerState, cacheShots);
          delay(frontDownLoopDelay);
        }
        else{
          break;
        }          
      }
      
      //run till front hit
      loopLastCheck = millis();
      logicSwitchCheck = millis();
      hitCount = 0;

      while(!checkCount()){
        if(millis() - loopLastCheck < frontUpLoopDur){
          triggerLogic(triggerState, cacheShots);
        }
        else{
          //front not reached in time, shutdown and beep
          cycleShutdown();
          toneAlt(2000, 100);
          return true;
        }
      }
      
      if(enterAutoLoop(cacheShots)){ //more than one shot remaining

        //short delay to get past front switch
        loopLastCheck = millis();
        while(millis() - loopLastCheck < frontPassDelay){ }
      
        //get delay between shots
        int burstBreakDelay = map(currBlSettings.rofValue, 0, 100, 100, 0);

        //blip brakes
        if(burstBreakDelay > 5){
            digitalWrite(pusherEnablePin, LOW);
            delayMicroseconds(200);
            digitalWrite(pusherBrakePin, HIGH);

            loopLastCheck = millis();
            while(millis() - loopLastCheck < burstBreakDelay){ }
            
            digitalWrite(pusherBrakePin, LOW);
            delayMicroseconds(200);
        }

        digitalWrite(pusherEnablePin, HIGH);
        cacheShots--;
      }
      else{ //final shot
        cacheShots--;
        break;        
      }      
    }

    //cycle finish
    //shut down pusher
    digitalWrite(pusherEnablePin, LOW);         
        
    //blip brakes and coast
    loopLastCheck = millis();        
    delayMicroseconds(200);      
    digitalWrite(pusherBrakePin, HIGH);
    while(pusherSwitchDown() != PSHREAR && millis() - loopLastCheck < currStSettings.brkAgr){ }
    digitalWrite(pusherBrakePin, LOW);            
    delayMicroseconds(200);    
    
    //while rear not down, coast till hit
    //blip if fail, attempt once to correct
    loopLastCheck = millis();    
    while(pusherSwitchDown() != PSHREAR){
      if(millis() - loopLastCheck < rearUpLoopDur){ }
      else{        
        //back not hit attempt pusher blip to get it there
        digitalWrite(pusherEnablePin, HIGH);
        delay(pusherBlipDur);
        digitalWrite(pusherEnablePin, LOW);  
        
        //while rear not down, coast till hit
        loopLastCheck = millis();
        while(pusherSwitchDown() != PSHREAR){
          if(millis() - loopLastCheck < rearUpLoopDur2){ }
          else{
            //rear not found, beep and shutdown cycle
            toneAlt(3000, 80);
            delay(80);
            toneAlt(3000, 80);
            cycleShutdown();
            return true;
          }
        }
      }
    }   

    //hard stop at rear
    delayMicroseconds(200);    
    digitalWrite(pusherBrakePin, HIGH);
    
    //set brake release for 200ms from now
    brakeRelease = millis() + 200;    
    spindownTarget = MINTHROTTLE;
    cycleShutdown();

    //spindown until trigger released
    while(triggerDown() && !inFullAuto()){ 
      spinDownFW(false);
    };

    idleRelease = millis() + currBlSettings.idleTime * 1000 + 100;
}

#line 1 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Presets.ino"
byte offset = 0;
byte writes = 0;

void loadSettings(){
  byte readVersion = EEPROM.read(0);
  
  if(readVersion != versionNumber){
    //try again
    delay(200);
    readVersion = EEPROM.read(1);
    
    if(readVersion != versionNumber){    
      writeDefaultSettings();
    }
  }
  else{
    loadStaticSettings();
    loadCurrentSettings();
  }
}

void loadStaticSettings(){
  EEPROM.get(8, currStSettings);
  lastStSettings = currStSettings;
}

void writeStaticSettings(){
  if(!staticSettingsChanged()){ return; }
  
  EEPROM.put(8, currStSettings);
  lastStSettings = currStSettings;
}

bool staticSettingsChanged(){
  return (
    currStSettings.btnMode != lastStSettings.btnMode ||
    currStSettings.brightness != lastStSettings.brightness ||
    currStSettings.soundOn != lastStSettings.soundOn ||
    currStSettings.batOffset != lastStSettings.batOffset ||
    currStSettings.brkAgr != lastStSettings.brkAgr ||
    currStSettings.minSpeed != lastStSettings.minSpeed ||
    currStSettings.maxSpeed != lastStSettings.maxSpeed ||    
    currStSettings.usrLock != lastStSettings.usrLock );
}

void loadCurrentSettings(){
  offset = EEPROM.read(4);
  int settingsLoc = 102 + offset * 20;
  EEPROM.get(settingsLoc, currBlSettings);
  lastBlSettings = currBlSettings;
}

void writeCurrentSettings(){
  if(!currentSettingsChanged()){ return; }
  
  int writesLoc = 100 + offset * 20;
  EEPROM.get(writesLoc, writes);
  writes += 1;
  
  if(writes > 200){
    offset += 1;
    if(offset > 40){ offset = 0; }
    writesLoc = 100 + offset * 20;    
    writes = 0;
    EEPROM.put(4, offset);  
  }
  EEPROM.put(writesLoc, writes);
  
  int settingsLoc = 102 + offset * 20;
  EEPROM.put(settingsLoc, currBlSettings);
  lastBlSettings = currBlSettings;
}

bool currentSettingsChanged(){
  return (
    currBlSettings.speedValue != lastBlSettings.speedValue ||
    currBlSettings.rofValue != lastBlSettings.rofValue ||
    currBlSettings.burstCount != lastBlSettings.burstCount ||
    currBlSettings.minSpinup != lastBlSettings.minSpinup ||
    currBlSettings.maxSpinup != lastBlSettings.maxSpinup ||
    currBlSettings.fireMode != lastBlSettings.fireMode ||
    currBlSettings.spinDown != lastBlSettings.spinDown ||
    currBlSettings.idleTime != currBlSettings.idleTime);
}

void readPreset(byte presetIndex){
  int presetLoc = 24 + (presetIndex - 1) * 20;
  EEPROM.get(presetLoc, readBlSettings);
}

void writePreset(byte presetIndex){
  int presetLoc = 24 + (presetIndex - 1) * 20;
  EEPROM.put(presetLoc, currBlSettings); 
  
  toneAlt(2000, 50);
  delay(50);
  toneAlt(2000, 100);   
}

void loadPreset(byte presetIndex){
  int presetLoc = 24 + (presetIndex - 1) * 20;
  EEPROM.get(presetLoc, currBlSettings); 
  lastBlSettings = currBlSettings;

  toneAlt(4000, 50);
  delay(50);
  toneAlt(4000, 100);
}

void writeDefaultSettings(){  
  oled.clearDisplay();
  oled.setCursor(0,0);
  oled.print("Updating");
  oled.setCursor(0,14);
  oled.print("settings");
  oled.setCursor(0,28);
  float versionNum = (float)versionNumber / 100;
  oled.print(versionNum,2);
  oled.display();  
  
  //write defaults to all 
  EEPROM.put(0, versionNumber);
  EEPROM.put(1, versionNumber);
  offset = 0;
  EEPROM.put(4, offset);
  EEPROM.put(8, defStSettings);
  writes = 0;
  EEPROM.put(100, writes);
  EEPROM.put(102, defBlSettings); 
  EEPROM.put(24, defBlSettings);
  EEPROM.put(44, defBlSettings);
  EEPROM.put(64, defBlSettings); 
  currStSettings = defStSettings;
  lastStSettings = defStSettings;
  currBlSettings = defBlSettings;
  lastBlSettings = defBlSettings;  

  delay(1000);
  toneAlt(3000, 50);
  delay(50);
  toneAlt(2500, 100);
  delay(50);
  toneAlt(2000, 200);
  delay(100);
  
  oled.clearDisplay();
  oled.display();
}

#line 1 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\RenderHelpers.ino"

void renderVoltMeter(){  
  if(firstMenuRun){
    voltMeter->reDraw();
  }

  float voltLevel = getVoltLevel();
  voltLevel = 11.0;
  voltMeter->setValue(voltLevel * 10);

  if(speedLocked){
    renderLockIndicator();
  }
  else{
    oled.setCursor(41,0);
    if(voltLevel < 10){
      oled.print("0");
    }
    oled.print(voltLevel,1);    
  }
   
  if(voltLevel < 10.8){
    if(lastBatAlarm + 3000 < millis()){
      shutdownFwAndPush();
      batteryWarning();
      lastBatAlarm = millis();
    }
  }

  if(voltLevel < 10.7){
    if(lastBatAlarm + 1500 < millis()){
      shutdownFwAndPush();
      batteryWarning();
      lastBatAlarm = millis();
    }
  }
  
  if(voltLevel < 10.6){
    if(lastBatAlarm + 1000 < millis()){
      shutdownFwAndPush();
      batteryCritical();
      lastBatAlarm = millis();
    }
  }

  if(voltLevel < 10.5){
    while(true){
      shutdownFwAndPush();
      batteryCritical();
      delay(200);
    }
  }
  
  oled.setCursor(56,39);
  oled.print("B");    
}

void batteryWarning(){
  toneAlt(2400, 140);
  toneAlt(4000, 140);
  toneAlt(1200, 200);
}

void batteryCritical(){
  toneAlt(2400, 140);
  delay(100);
  toneAlt(4000, 140);
  delay(100);
  toneAlt(2400, 140);
  delay(100);
  toneAlt(4000, 140);
}

void renderLockIndicator(){
  int xLoc = 34;
  if(currStSettings.maxSpeed < 100){ xLoc = 40; }
  if(currStSettings.maxSpeed < 10){ xLoc = 46; }  
  oled.setCursor(xLoc,0);
  oled.print("SL");
  oled.print(currStSettings.maxSpeed);
  oled.setTextSize(SMALL_T);
}

void renderInfoMenu(){  
  oled.setTextSize(LARGE_T);
  oled.setCursor(0,OLED_HEADER);
  oled.print("Info");
  oled.setCursor(0,OLED_HEADER + (CH_LH + 2) * 1);
  oled.print("FDL-3");
  oled.setCursor(0,OLED_HEADER + (CH_LH + 2) * 2);
  oled.print("v");
  float versionNum = (float)versionNumber / 100;
  oled.print(versionNum,2);  
  oled.print("A");
  oled.display();
  firstMenuRun = false;
  oled.setTextSize(SMALL_T);    
}

void renderUserLock(){

  while(presetButtonDown() == PRSTROT){}
  
  oled.clearDisplay();
  oled.setTextSize(LARGE_T);  
  oled.setCursor(0,0);
  oled.print("User Lock");
  
  if(currStSettings.usrLock == 0){
    oled.setCursor(0,OLED_HEADER);
    oled.print("Set");
  }
  else{
    oled.setCursor(0,OLED_HEADER);
    oled.print("Enter");
  }
  oled.print(" Code");
  oled.display();

  if(currStSettings.usrLock == 0){
    toneAlt(2400, 80);
    toneAlt(1600, 80);
    toneAlt(2400, 140);
  }
  else{
    toneAlt(1600, 140);
    toneAlt(1200, 140);
    delay(200);
  }
  
  int usrLock = 0;
  
  for(int i = 0; i < 3; i++){
    int prstBut = 0;
    while(prstBut == 0){ prstBut = presetButtonDown(); }
    while(presetButtonDown() != 0) { };
    toneAlt(2400, 120);    
    if(prstBut == 4){ break; }

    if(i > 0 && i < 3){ usrLock *= 10; }
    usrLock += prstBut;

    if(usrLock > 0){
      oled.setCursor(0,OLED_HEADER + CH_LH + 4);  
      oled.print(usrLock);
      oled.display();
    }
  }
  delay(300);
  
  usrLock = usrLock < 100 ? 0 : usrLock;

  if(usrLock > 100){ //code fully entered
    if(currStSettings.usrLock == 0){
      currStSettings.usrLock = usrLock;
      writeStaticSettings();

      oled.clearDisplay();
      oled.setCursor(6,18);
      oled.print("Code Set");
      oled.setCursor(0,36);
      oled.print(currStSettings.usrLock);
      oled.display();      
      
      toneAlt(1600, 80);
      delay(50);
      toneAlt(2400, 140);
      delay(300);
    }
    else{
      if(currStSettings.usrLock == usrLock){
        currStSettings.usrLock = 0;
        writeStaticSettings();

        oled.clearDisplay();
        oled.setCursor(8,16);
        oled.print("Unlocked");
        oled.display();        
        
        toneAlt(2400, 80);
        delay(30);
        toneAlt(2400, 80);
        delay(30);
        toneAlt(3000, 80);
        delay(100);

        knobMenuIndex = 0;
        liveKnobScrollMode = false;
        firstMenuRun = true;
      }
    }
  }
  else{
    if(currStSettings.usrLock == 0){
      liveKnobScrollMode = true;      
    }
    toneAlt(1600, 80);
    delay(200);
  }

  oled.clearDisplay();
  oled.display();  
}

/////////////////
//main menu
/////////////////
void renderKnobScrollMenu(){
  encoderChange += myEnc.getCount();
  myEnc.setCount(0);

  byte arraySize = sizeof(knobMenu) / sizeof(size_t);
  if(abs(encoderChange) >= 8){
    if(!(knobMenuIndex == 0 && encoderChange < 0)){
      knobMenuIndex += encoderChange / 8;
      knobMenuIndex = constrain(knobMenuIndex, 0, arraySize - 1);
    }
    encoderChange = 0;    
    toneAlt(2000,10);
  }

  oled.setCursor(0,0);
  oled.setTextSize(LARGE_T);
  oled.print("Mode");

  int textLength = 0;
  const char* testPtr = knobMenu[knobMenuIndex];
  while(*(testPtr++) != '\0'){
    textLength++;
  }

  int txtWidth = CH_LW * (textLength + 1);
  int txtHeight = CH_LH;
  int txtLocX = constrain((OLED_WIDTH - txtWidth) / 2 - 1, 0, OLED_WIDTH / 2);
  int txtLocY = constrain((OLED_HEIGHT - txtHeight) / 2 - 1, 0, OLED_HEIGHT / 2);
  oled.fillRect(0, OLED_HEADER, OLED_WIDTH, OLED_HEIGHT - OLED_HEADER, BLACK);
  
  oled.setCursor(txtLocX,txtLocY);
  testPtr = knobMenu[knobMenuIndex];
  while(*testPtr != '\0'){
    oled.print(*testPtr);
    testPtr++;
  }
  
  oled.display();
  firstMenuRun = false;
  oled.setTextSize(SMALL_T);    
}

/////////////////
//menu type settings
/////////////////
void renderMenu(byte &menuIndex, const char label[], const char* menu[], byte arraySize){
  encoderChange += myEnc.getCount();
  myEnc.setCount(0);

  if(abs(encoderChange) >= 4){
    if(!(menuIndex == 0 && encoderChange < 0)){
      menuIndex += encoderChange / 4;
      menuIndex = constrain(menuIndex, 0, arraySize - 1);
    }
    encoderChange = 0;
  }

  oled.setTextSize(LARGE_T); 
  oled.setCursor(0,0);
  oled.print(label);

      

  int textLength = 0;
  const char* testPtr = menu[menuIndex];
  while(*(testPtr++) != '\0'){
    textLength++;
  }

  int txtWidth = CH_LW * (textLength + 1);
  int txtHeight = CH_LH;
  int txtLocX = constrain((OLED_WIDTH - txtWidth) / 2 - 1, 0, OLED_WIDTH / 2);
  int txtLocY = constrain((OLED_HEIGHT - txtHeight) / 2 - 1, 0, OLED_HEIGHT / 2);

  oled.fillRect(0, txtLocY, OLED_WIDTH, txtLocY + txtHeight, BLACK);  
  oled.setCursor(txtLocX, txtLocY);
  testPtr = menu[menuIndex];
  while(*testPtr != '\0'){
    oled.print(*testPtr);
    testPtr++;
  } 
  
  oled.display();
  firstMenuRun = false;
  oled.setTextSize(SMALL_T);    
}


/////////////////
//gauge type settings
/////////////////
void renderGauge(int &gaugeValue, String label, int gaugeMin, int gaugeMax, int valueMin, int valueMax, int detPerMove){
  mainGauge->setMinValue(gaugeMin);
  mainGauge->setMaxValue(gaugeMax);
  
  if(firstMenuRun){
    mainGauge->reDraw();
    firstMenuRun = false;
  }

  oled.setCursor(0,0);
  oled.setTextSize(LARGE_T);
  oled.print(label);
  oled.setTextSize(SMALL_T);
    
  encoderChange += myEnc.getCount();
  myEnc.setCount(0);

  if(abs(encoderChange) >= detPerMove || firstMenuRun){
    gaugeValue += encoderChange / detPerMove;
    gaugeValue = constrain(gaugeValue, valueMin, valueMax);
    encoderChange = 0;
  }
  // Deactivated Extra Delay
  /*oled.setCursor(OLED_WIDTH / 2 - 1.5 * CH_LW, OLED_HEIGHT - CH_LH);
  oled.fillRect(0, OLED_HEIGHT - CH_LH, OLED_WIDTH, CH_LH, BLACK);    
  oled.print(gaugeValue);
  if(gaugeValue < 10){ oled.print(" "); }
  if(gaugeValue < 100){ oled.print(" "); }*/

  mainGauge->setValue(gaugeValue);
  oled.display();
}


void renderSplash(String splashText){

  oled.setTextSize(LARGE_T);

  int spaceIndex = splashText.indexOf(' ');

  if(spaceIndex == -1){
    int txtWidth = CH_LW * (splashText.length() + 1) ;
    int txtLocX = constrain((OLED_WIDTH - txtWidth) / 2, 0, OLED_WIDTH / 2);
    int txtLocY = constrain((OLED_HEIGHT - CH_LH) / 2, 0, OLED_HEIGHT / 2);
  
    oled.setCursor(txtLocX,txtLocY);
  
    for(int i = 0; i < splashText.length(); i++){
      oled.print(splashText[i]);
      oled.display();
      delay(50);
    }
  }
  else{
    String sub1 = splashText.substring(0, spaceIndex);
    String sub2 = splashText.substring(spaceIndex + 1);

    int charWidth = CH_LW;
    int charHeight = CH_LH;

    if(sub1.length() * CH_LW > OLED_WIDTH){
      oled.setTextSize(SMALL_T);
      charWidth = CH_SW;
      charHeight = CH_SH;
    }
    else{
      oled.setTextSize(LARGE_T);
    }
    
    int txtWidth1 = charWidth * (sub1.length() + 1);        
    int txtLocX1 = constrain((OLED_WIDTH - txtWidth1) / 2, 0, OLED_WIDTH / 2);    
    int txtLocY1 = constrain((OLED_HEIGHT - charHeight) / 2, 0, OLED_HEIGHT / 2);
    txtLocY1 -= (charHeight/2 + 1);

    oled.setCursor(txtLocX1,txtLocY1);
  
    for(int i = 0; i < sub1.length(); i++){
      oled.print(sub1[i]);
      oled.display();
      delay(50);
    }
    charWidth = 5;
    if(sub2.length() * CH_LW > OLED_WIDTH){
      oled.setTextSize(SMALL_T);
      charWidth = CH_SW;
      charHeight = CH_SH;
    }
    else{
      oled.setTextSize(LARGE_T);
      charWidth = CH_LW;
      charHeight = CH_LH;
    }

    int txtWidth2 = charWidth * (sub2.length()+1);
    int txtLocX2 = constrain((OLED_WIDTH - txtWidth2) / 2, 0, OLED_WIDTH / 2);    
    int txtLocY2 = constrain((OLED_HEIGHT - charHeight) / 2, 0, OLED_HEIGHT / 2);
    txtLocY2 += charHeight/2 + 1;

    oled.setCursor(txtLocX2,txtLocY2);
  
    for(int i = 0; i < sub2.length(); i++){
      oled.print(sub2[i]);
      oled.display();
      delay(50);
    }
  }

  delay(600);  
  oled.clearDisplay();
  oled.display();  
}

void renderPresetMenu(){
  
  encoderChange += myEnc.getCount();
  myEnc.setCount(0);

  byte arraySize = sizeof(presetMenu) / sizeof(size_t);
  if(abs(encoderChange) >= 4){
    if(!(presetMenuIndex == 0 && encoderChange < 0)){
      presetMenuIndex += encoderChange / 4;
      presetMenuIndex = constrain(presetMenuIndex, 0, arraySize - 1);
    }
    encoderChange = 0;
  }

  oled.clearDisplay();
  if(presetMenuIndex > 0){
    renderPreset(presetMenuIndex);
  }
  else{
    firstMenuRun = true;
    if(speedLocked){ renderLockIndicator(); }  
    renderVoltMeter();
    
    oled.setCursor(0,OLED_HEADER);
    oled.print("Load");

    oled.setTextSize(LARGE_T);   
    
    int textLength = 0;
    const char* testPtr = presetMenu[presetMenuIndex];
    while(*(testPtr++) != '\0'){
      textLength++;
    }
    int txtWidth = CH_LW * (textLength + 1);
    int txtHeight = CH_LH;
    int txtLocX = constrain((OLED_WIDTH - txtWidth) / 2 - 1, 0, OLED_WIDTH / 2);
  
    oled.rectFill(0, 26, 55, 26 + txtHeight, BLACK, NORM);
    
    oled.setCursor(txtLocX, OLED_HEADER);    
    testPtr = presetMenu[presetMenuIndex];
    while(*testPtr != '\0'){
      oled.print(*testPtr);
      testPtr++;
    } 
  }
  
  oled.display();
  firstMenuRun = false;
  oled.setTextSize(SMALL_T);    
}

void renderPreset(byte preset){
  readPreset(preset);

  oled.clearDisplay();
  oled.setTextSize(SMALL_T);      
  oled.setCursor(0,OLED_HEADER);
  oled.print(firemodeMenu[readBlSettings.fireMode]);  
  oled.setCursor(0,OLED_HEADER + (CH_SH + 3) * 1);
  oled.print("S:");
  oled.print(readBlSettings.speedValue);  
  oled.setCursor(0,OLED_HEADER + (CH_SH + 3) * 2);
  oled.print("B:");
  oled.print(burstMenu[readBlSettings.burstCount]);  
  oled.setCursor(0,OLED_HEADER + (CH_SH + 3) * 3);
  oled.print("R:");
  oled.print(readBlSettings.rofValue);  

  oled.setCursor(OLED_WIDTH / 2,OLED_HEADER);
  oled.print("p:");
  oled.print(readBlSettings.minSpinup);
  oled.setCursor(OLED_WIDTH / 2,OLED_HEADER + (CH_SH + 3) * 1);
  oled.print("P:");
  oled.print(readBlSettings.maxSpinup);
  oled.setCursor(OLED_WIDTH / 2,OLED_HEADER + (CH_SH + 3) * 2);
  oled.print("D:");
  oled.print(readBlSettings.spinDown);
  oled.setCursor(OLED_WIDTH / 2,OLED_HEADER + (CH_SH + 3) * 3);
  oled.print("I:");
  oled.print(readBlSettings.idleTime);

  oled.rectFill(57, 39, 8, 9, WHITE , NORM);
  oled.setColor(BLACK);
  oled.setCursor(0,0);
  oled.setTextSize(LARGE_T);
  oled.print("Preset: ");
  oled.print(preset);
}

#line 1 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Renderer.ino"
/////////////
///RENDER////
/////////////
void renderScreen(){

  // PARTY
  //renderVoltMeter();

  if(liveKnobScrollMode){
    renderKnobScrollMenu();
  }
  
  else{
    switch(knobMenuIndex){
      case 0:
        renderGauge(currBlSettings.speedValue, "Speed", 0, 100, currStSettings.minSpeed, currStSettings.maxSpeed, 1);
        break;
      case 1:
        renderGauge(currBlSettings.rofValue, "ROF", 0, 100, 0, 100, 1);
        break;
      case 2:
        renderMenu(currBlSettings.burstCount, "Burst", burstMenu, sizeof(burstMenu) / sizeof(size_t));
        break;
      case 3:
        renderGauge(currBlSettings.minSpinup, "MinSpn", 150, 500, 150, 500, 1);
        break;
      case 4:
        renderGauge(currBlSettings.maxSpinup, "MaxSpn", 150, 500, 150, 500, 1);
        break;
      case 5:
        renderMenu(currBlSettings.fireMode, "FireMode", firemodeMenu, sizeof(firemodeMenu) / sizeof(size_t));
        break;  
      case 6:
        renderGauge(currBlSettings.spinDown, "SpnDwn", 6, 25, 6, 25, 8);
        break;
      case 7:
        renderGauge(currBlSettings.idleTime, "Idle", 0, 10, 0, 10, 8);
        break;    
      case 8:
        renderPresetMenu();
        break;
      case 9:
        renderMenu(presetMenuIndex, "Save", presetMenu, sizeof(presetMenu) / sizeof(size_t));
        break;
      case 10:
        renderGauge(currStSettings.minSpeed, "MinSpd", 0, 100, 0, currStSettings.maxSpeed, 1);
        break;
      case 11:
        if(!speedLocked){
          renderGauge(currStSettings.maxSpeed, "MaxSpd", 0, 100, currStSettings.minSpeed, 100, 1);
        }
        break; 
      case 12:
        renderMenu(currStSettings.btnMode, "Btn Mode", btnmodeMenu, sizeof(btnmodeMenu) / sizeof(size_t));
        break;
      case 13:
        renderGauge(currStSettings.brkAgr, "BrkAgr", 3, 25, 3, 25, 8);
        break;
      case 14:
        renderUserLock();
        break;
      case 15:
        renderGauge(currStSettings.brightness, "Bright", 0, 100, 0, 100, 1);
        break;
      case 16:
        renderMenu(currStSettings.soundOn, "Sound", soundMenu, sizeof(soundMenu) / sizeof(size_t));
        break;
      case 17:
        renderGauge(currStSettings.batOffset, "BatOff", -8, 8, -8, 8, 8);
        break;
      case 18:
        renderInfoMenu();
        break;      
      default:
        break;
    }
  }  
  
  //look for rot switch or preset press
  int presetButton = presetButtonDown();

  if(presetButton != PRSTNONE){
    if(presetButtonDown() == PRSTROT){ //rot button down
      if(!menuBtnWasDown){
                        
        if(liveKnobScrollMode){ //in main menu
          presetMenuIndex = 0;
        }
        else{ //in submenu
          if(knobMenu[knobMenuIndex] == "Load" && presetMenuIndex > 0){
            loadPreset(presetMenuIndex); 
          }
          if(knobMenu[knobMenuIndex] == "Save"  && presetMenuIndex > 0){
            writePreset(presetMenuIndex); 
          }
        }

        liveKnobScrollMode = !liveKnobScrollMode; //flip between main menu and setting
  
        if(speedLocked && knobMenu[knobMenuIndex] == "MaxSpd"){ //ignore flip if locked
          liveKnobScrollMode = true;
        }
        
        oled.clearDisplay();
        firstMenuRun = true;
        myEnc.setCount(0);
      }
      menuBtnWasDown = true;
    }
    else{ //preset button pressed   
      if(!menuBtnWasDown){ 
        toneAlt(1500, 10);
        delay(160);
        if(presetButtonDown() == presetButton){
          presetButtonAction(presetButton);
          menuBtnWasDown = true;
          oled.clearDisplay();
          firstMenuRun = true;
          myEnc.setCount(0);
        }
      }      
    }    
  }
  else{
    menuBtnWasDown = false;
  } 
}

void presetButtonAction(int presButton){
  if(currStSettings.btnMode == 0){//preset
    loadPreset(presButton);
  }
  if(currStSettings.btnMode == 1){//speed
    int valueCoef = 30;
    currBlSettings.speedValue = 100 - (presButton * valueCoef - valueCoef);
    toneAlt(2000, 100);
  }
  if(currStSettings.btnMode == 2){//rof
    int valueCoef = 25;
    currBlSettings.rofValue = 100 - (presButton * valueCoef - valueCoef);
    toneAlt(2000, 100);
  }
  if(currStSettings.btnMode == 3){//burst
    if(presButton == 3){
      currBlSettings.burstCount = 3;
    }
    else{
      currBlSettings.burstCount = presButton - 1;
    }
    toneAlt(2000, 100);
  }
}

#line 1 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Utils.ino"

//roll custom tone generator to avoid conflict with Timer2 and abstract pin
void toneAlt(int frequency, int duration){

  if(!currStSettings.soundOn){ return; }
  
  int freqDelay = 1000000 / frequency / 2;
  int loopCount = duration * 1000 / (freqDelay * 2);

  unsigned long offTime = millis() + duration;
  while(millis() < offTime){
    digitalWrite(buzzerPin, HIGH); 
    delayMicroseconds(freqDelay); 
    digitalWrite(buzzerPin, LOW); 
    delayMicroseconds(freqDelay);
  }
}

void initBatteryCheck(){
  //init battery average read values
  for(int x = 0; x < batteryCheckLength; x++){
    batteryCheck[x] = 12.0;
  }
  lastBatteryCheck = millis();
  batteryCheckSum = 12.0 * batteryCheckLength;
}

float getVoltLevel(){
  
 if(millis() > lastBatteryCheck + 300){ //check every 300ms
    const float vPow = 5.042;;
    const float r1 = 100000;
    const float r2 = 10000;

    float v = (analogRead(voltMeterPin) * vPow) / 1024.0;
    float v2 = v / (r2 / (r1 + r2));

    v2 *= 10;
    int v2Int = (int)v2;
    v2 = (float)v2Int / 10;

    batteryCheckSum -= batteryCheck[batteryCheckIndex];
    batteryCheck[batteryCheckIndex] = v2;
    batteryCheckSum += v2;
    batteryCheckIndex++;
    if(batteryCheckIndex >= batteryCheckLength){
      batteryCheckIndex = 0;
    }
    lastBatteryCheck = millis();    
  }
  
  return batteryCheckSum / batteryCheckLength + (float)currStSettings.batOffset / 10.0;
}

void startUpBeeps(){
  toneAlt(2000, 60);
  delay(60);
  toneAlt(2000, 60);
  delay(60);
  toneAlt(2000, 60);
}

void clearSetRoutine(){
  while(presetButtonDown() == 4){};
  delay(50);
  
  oled.setCursor(0,0);
  oled.print("Click knob");
  oled.setCursor(0,14);
  oled.print("to clear");
  oled.setCursor(0,28);
  oled.print("settings");
  oled.display();

  long currMills = millis();
  while(millis() < currMills + 2000){
    if(presetButtonDown() == 4){
      writeDefaultSettings();
      break;
    }
  }
  oled.clearDisplay();
  oled.display();
}

void clearLockRoutine(){
  while(lockOn() && presetButtonDown() == 3){ delay(50); };
  if(presetButtonDown() != 3){ return; }
  while(presetButtonDown() == 3){ delay(50); };
  while(presetButtonDown() == 0){ delay(50); };
  if(presetButtonDown() != 2){ return; }

  currStSettings.usrLock = 0;
  writeStaticSettings();

  oled.clearDisplay();  
  oled.setCursor(10,16);
  oled.print("Unlocked");
  oled.display();        
  
  toneAlt(2400, 80);
  delay(30);
  toneAlt(2400, 80);
  delay(30);
  toneAlt(3000, 80);
  delay(100);

  oled.clearDisplay(); 
  oled.display(); 
}

