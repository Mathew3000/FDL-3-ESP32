# 1 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FDL-3-ESP32.ino"
# 1 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FDL-3-ESP32.ino"
# 2 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FDL-3-ESP32.ino" 2
# 3 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FDL-3-ESP32.ino" 2
# 4 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FDL-3-ESP32.ino" 2
# 5 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FDL-3-ESP32.ino" 2
# 6 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FDL-3-ESP32.ino" 2
# 7 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FDL-3-ESP32.ino" 2
# 8 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FDL-3-ESP32.ino" 2

# 10 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FDL-3-ESP32.ino" 2
# 11 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FDL-3-ESP32.ino" 2
# 12 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FDL-3-ESP32.ino" 2
# 13 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FDL-3-ESP32.ino" 2





const byte versionNumber = 101;
const String splashText = "FDL-3 ESP32"; //can be two lines split with space

Servo flywheelESC;
ESP32Encoder myEnc;
MicroViewWidget *mainGauge;
MicroViewWidget *voltMeter;
//PARTY

// OLED DEF
#define OLED_DC 26
#define OLED_CS 25
#define OLED_MOSI 13
#define OLED_SCK 14
#define OLED_RESET 27
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_HEADER 16
#define SMALL_T 1
#define LARGE_T 2
#define CH_SH 7
#define CH_LH 14
#define CH_SW 5
#define CH_LW 10
Adafruit_SSD1306 oled(128,64, 13, 14, 26, 27, 25);

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
int currentSpeed = 1000;
int spindownTarget = 1000;

#define batteryCheckLength 6
float batteryCheck[6];
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
  oled.setTextSize(2);
  oled.setTextColor(1 /*|< Draw 'on' pixels*/); // Draw white text
  oled.setCursor(0, 0); // Start at top-left corner
  oled.cp437(true); // Use full 256 char 'Code Page 437' font

  // Init EEPROM
  EEPROM.begin(1024);

  // Setup Encoder
  myEnc.attachHalfQuad(19, 18);

  // Setup ESC
  flywheelESC.attach(34);
  flywheelESC.writeMicroseconds(0);

  // Setup Pin Modes
  pinMode(2, 0x01);
  pinMode(34, 0x01);
  pinMode(4, 0x01);
  pinMode(16, 0x05);
  pinMode(5, 0x05);
  pinMode(32, 0x02);
  pinMode(33, 0x02);
  pinMode(17, 0x02);

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
  mainGauge = new MicroViewGauge(128 /*|< DEPRECATED: width w/SSD1306_128_32 defined*//2, 16 + 24, 0, 100, 1);
  voltMeter = new MicroViewSlider(56, 14, 106, 126, 3);
  mainGauge->reDraw();
  voltMeter->reDraw();

  initBatteryCheck();

  if(currStSettings.usrLock != 0){
    renderUserLock();
  }
  else{
    flywheelESC.writeMicroseconds(1000);

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
      digitalWrite(33, 0x0);
      delayMicroseconds(200);
    }
    if(millis() > idleRelease){
      spindownTarget = 1000;
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
# 1 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\CycleHelpers.ino"
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
  digitalWrite(32, 0x0);
  spinDownFW(true);
}

void shutdownFwAndPush(){
  digitalWrite(32, 0x0);
  setFwSpeed(1000);
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
  if(millis() - logicSwitchCheck > 1 && pusherSwitchDown() == 1){
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
  if(currBlSettings.fireMode == 3){ // in cache mode

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
      return triggerDown() || (currBlSettings.fireMode != 1 && shotsCached >= 100);
    }
    if(currBlSettings.fireMode == 1){//trigger abide mode
      return triggerDown();
    }
    if(currBlSettings.fireMode == 2){//auto mode
      return true;
    }
    if(currBlSettings.fireMode == 3){//cached mode
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
    if(currBlSettings.fireMode == 1){//trigger abide mode
      return triggerDown();
    }
    if(currBlSettings.fireMode == 2){//auto mode
      return true;
    }
    if(currBlSettings.fireMode == 3){//cached mode
      return true;
    }
  }

  return false;
}

bool inFullAuto(){
  return getBurstCount() == 100;
}

boolean triggerDown(){
  return digitalRead(16) == 0x0;
}

boolean lockOn(){
  return digitalRead(5) == 0x0;
}

int presetButtonDown(){
  int readVal = analogRead(34);

  if(readVal < 500){
    return 0;
  }
  if(readVal < 2400){
    return 4;
  }
  if(readVal < 2700){
    return 3;
  }
  if(readVal < 3300){
    return 2;
  }
  if(readVal < 4100)
  {
    return 1;
  }
}

int pusherSwitchDown(){
  int readVal = analogRead(4);

  if(readVal < 400){
    return 0;
  }
  if(readVal < 900){
    return 1;
  }
  return 2;
}

int readESCPower(){
  return map(currBlSettings.speedValue, 0, 100, 1285, 2000);
}

void setFwSpeed(int newSpeed){
  flywheelESC.writeMicroseconds(newSpeed);
}

int getSpinupByMotorSpeed(){
  double calcSpin = doubleMap(currBlSettings.speedValue, 0, 100, currBlSettings.minSpinup, currBlSettings.maxSpinup);
  if(currentSpeed < 1285){ return calcSpin; }
  if(currBlSettings.speedValue == 0){ return 0; }

  double minSync = 50;
  double targetThrottle = doubleMap(currBlSettings.speedValue, 0, 100, 1285, 2000);
  double throttleFactor = (targetThrottle - currentSpeed) / (targetThrottle - 1285);
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
# 1 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\FireCycle.ino"
const int frontDownLoopDelay = 3; //delay till pass front switch if start from front rare case
const int frontDownLoopDur = 50; //wait delay till front switch up error if not met
const int frontUpLoopDur = 200; //wait delay to hit front switch error if not hit
const int frontPassDelay = 5; //pseudo debounce delay at front switch
const int rearUpLoopDur = 80; //first wait for rear switch
const int rearUpLoopDur2 = 200; //backup wait for rear switch
const int pusherBlipDur = 8; //pusher blip in attempts to reach back switch

bool fireSequence(){

    if(currBlSettings.fireMode == 0){
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
    if(!triggerDown() && currBlSettings.fireMode == 1){
      cycleShutdown();
      idleRelease = millis() + currBlSettings.idleTime * 1000;
      spindownTarget = 1285;
      return true;
    }

    //shots cached
    while(enterFireLoop(cacheShots)){

      //disable brake
      digitalWrite(33, 0x0);
      delayMicroseconds(200);

      digitalWrite(32, 0x1);

      //fire cycle begin
      //while front down / left at front, run
      //rare state
      loopLastCheck = millis();
      while(pusherSwitchDown() == 1){
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
            digitalWrite(32, 0x0);
            delayMicroseconds(200);
            digitalWrite(33, 0x1);

            loopLastCheck = millis();
            while(millis() - loopLastCheck < burstBreakDelay){ }

            digitalWrite(33, 0x0);
            delayMicroseconds(200);
        }

        digitalWrite(32, 0x1);
        cacheShots--;
      }
      else{ //final shot
        cacheShots--;
        break;
      }
    }

    //cycle finish
    //shut down pusher
    digitalWrite(32, 0x0);

    //blip brakes and coast
    loopLastCheck = millis();
    delayMicroseconds(200);
    digitalWrite(33, 0x1);
    while(pusherSwitchDown() != 2 && millis() - loopLastCheck < currStSettings.brkAgr){ }
    digitalWrite(33, 0x0);
    delayMicroseconds(200);

    //while rear not down, coast till hit
    //blip if fail, attempt once to correct
    loopLastCheck = millis();
    while(pusherSwitchDown() != 2){
      if(millis() - loopLastCheck < rearUpLoopDur){ }
      else{
        //back not hit attempt pusher blip to get it there
        digitalWrite(32, 0x1);
        delay(pusherBlipDur);
        digitalWrite(32, 0x0);

        //while rear not down, coast till hit
        loopLastCheck = millis();
        while(pusherSwitchDown() != 2){
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
    digitalWrite(33, 0x1);

    //set brake release for 200ms from now
    brakeRelease = millis() + 200;
    spindownTarget = 1285;
    cycleShutdown();

    //spindown until trigger released
    while(triggerDown() && !inFullAuto()){
      spinDownFW(false);
    };

    idleRelease = millis() + currBlSettings.idleTime * 1000 + 100;
}
# 1 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Presets.ino"
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
# 1 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\RenderHelpers.ino"

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
  oled.setTextSize(1);
}

void renderInfoMenu(){
  oled.setTextSize(2);
  oled.setCursor(0,16);
  oled.print("Info");
  oled.setCursor(0,16 + (14 + 2) * 1);
  oled.print("FDL-3");
  oled.setCursor(0,16 + (14 + 2) * 2);
  oled.print("v");
  float versionNum = (float)versionNumber / 100;
  oled.print(versionNum,2);
  oled.print("A");
  oled.display();
  firstMenuRun = false;
  oled.setTextSize(1);
}

void renderUserLock(){

  while(presetButtonDown() == 4){}

  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setCursor(0,0);
  oled.print("User Lock");

  if(currStSettings.usrLock == 0){
    oled.setCursor(0,16);
    oled.print("Set");
  }
  else{
    oled.setCursor(0,16);
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
      oled.setCursor(0,16 + 14 + 4);
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
      knobMenuIndex = ((knobMenuIndex)<(0)?(0):((knobMenuIndex)>(arraySize - 1)?(arraySize - 1):(knobMenuIndex)));
    }
    encoderChange = 0;
    toneAlt(2000,10);
  }

  oled.setCursor(0,0);
  oled.setTextSize(2);
  oled.print("Mode");

  int textLength = 0;
  const char* testPtr = knobMenu[knobMenuIndex];
  while(*(testPtr++) != '\0'){
    textLength++;
  }

  int txtWidth = 10 * (textLength + 1);
  int txtHeight = 14;
  int txtLocX = (((128 - txtWidth) / 2 - 1)<(0)?(0):(((128 - txtWidth) / 2 - 1)>(128 / 2)?(128 / 2):((128 - txtWidth) / 2 - 1)));
  int txtLocY = (((64 - txtHeight) / 2 - 1)<(0)?(0):(((64 - txtHeight) / 2 - 1)>(64 / 2)?(64 / 2):((64 - txtHeight) / 2 - 1)));
  oled.fillRect(0, 16, 128, 64 - 16, 0 /*|< Draw 'off' pixels*/ /*|< Draw 'off' pixels*/);

  oled.setCursor(txtLocX,txtLocY);
  testPtr = knobMenu[knobMenuIndex];
  while(*testPtr != '\0'){
    oled.print(*testPtr);
    testPtr++;
  }

  oled.display();
  firstMenuRun = false;
  oled.setTextSize(1);
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
      menuIndex = ((menuIndex)<(0)?(0):((menuIndex)>(arraySize - 1)?(arraySize - 1):(menuIndex)));
    }
    encoderChange = 0;
  }

  oled.setTextSize(2);
  oled.setCursor(0,0);
  oled.print(label);



  int textLength = 0;
  const char* testPtr = menu[menuIndex];
  while(*(testPtr++) != '\0'){
    textLength++;
  }

  int txtWidth = 10 * (textLength + 1);
  int txtHeight = 14;
  int txtLocX = (((128 - txtWidth) / 2 - 1)<(0)?(0):(((128 - txtWidth) / 2 - 1)>(128 / 2)?(128 / 2):((128 - txtWidth) / 2 - 1)));
  int txtLocY = (((64 - txtHeight) / 2 - 1)<(0)?(0):(((64 - txtHeight) / 2 - 1)>(64 / 2)?(64 / 2):((64 - txtHeight) / 2 - 1)));

  oled.fillRect(0, txtLocY, 128, txtLocY + txtHeight, 0 /*|< Draw 'off' pixels*/ /*|< Draw 'off' pixels*/);
  oled.setCursor(txtLocX, txtLocY);
  testPtr = menu[menuIndex];
  while(*testPtr != '\0'){
    oled.print(*testPtr);
    testPtr++;
  }

  oled.display();
  firstMenuRun = false;
  oled.setTextSize(1);
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
  oled.setTextSize(2);
  oled.print(label);
  oled.setTextSize(1);

  encoderChange += myEnc.getCount();
  myEnc.setCount(0);

  if(abs(encoderChange) >= detPerMove || firstMenuRun){
    gaugeValue += encoderChange / detPerMove;
    gaugeValue = ((gaugeValue)<(valueMin)?(valueMin):((gaugeValue)>(valueMax)?(valueMax):(gaugeValue)));
    encoderChange = 0;
  }
  // Deactivated Extra Delay
  /*oled.setCursor(OLED_WIDTH / 2 - 1.5 * CH_LW, OLED_HEIGHT - CH_LH);

  oled.fillRect(0, OLED_HEIGHT - CH_LH, OLED_WIDTH, CH_LH, BLACK);    

  oled.print(gaugeValue);

  if(gaugeValue < 10){ oled.print(" "); }

  if(gaugeValue < 100){ oled.print(" "); }*/
# 328 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\RenderHelpers.ino"
  mainGauge->setValue(gaugeValue);
  oled.display();
}


void renderSplash(String splashText){

  oled.setTextSize(2);

  int spaceIndex = splashText.indexOf(' ');

  if(spaceIndex == -1){
    int txtWidth = 10 * (splashText.length() + 1) ;
    int txtLocX = (((128 - txtWidth) / 2)<(0)?(0):(((128 - txtWidth) / 2)>(128 / 2)?(128 / 2):((128 - txtWidth) / 2)));
    int txtLocY = (((64 - 14) / 2)<(0)?(0):(((64 - 14) / 2)>(64 / 2)?(64 / 2):((64 - 14) / 2)));

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

    int charWidth = 10;
    int charHeight = 14;

    if(sub1.length() * 10 > 128){
      oled.setTextSize(1);
      charWidth = 5;
      charHeight = 7;
    }
    else{
      oled.setTextSize(2);
    }

    int txtWidth1 = charWidth * (sub1.length() + 1);
    int txtLocX1 = (((128 - txtWidth1) / 2)<(0)?(0):(((128 - txtWidth1) / 2)>(128 / 2)?(128 / 2):((128 - txtWidth1) / 2)));
    int txtLocY1 = (((64 - charHeight) / 2)<(0)?(0):(((64 - charHeight) / 2)>(64 / 2)?(64 / 2):((64 - charHeight) / 2)));
    txtLocY1 -= (charHeight/2 + 1);

    oled.setCursor(txtLocX1,txtLocY1);

    for(int i = 0; i < sub1.length(); i++){
      oled.print(sub1[i]);
      oled.display();
      delay(50);
    }
    charWidth = 5;
    if(sub2.length() * 10 > 128){
      oled.setTextSize(1);
      charWidth = 5;
      charHeight = 7;
    }
    else{
      oled.setTextSize(2);
      charWidth = 10;
      charHeight = 14;
    }

    int txtWidth2 = charWidth * (sub2.length()+1);
    int txtLocX2 = (((128 - txtWidth2) / 2)<(0)?(0):(((128 - txtWidth2) / 2)>(128 / 2)?(128 / 2):((128 - txtWidth2) / 2)));
    int txtLocY2 = (((64 - charHeight) / 2)<(0)?(0):(((64 - charHeight) / 2)>(64 / 2)?(64 / 2):((64 - charHeight) / 2)));
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
      presetMenuIndex = ((presetMenuIndex)<(0)?(0):((presetMenuIndex)>(arraySize - 1)?(arraySize - 1):(presetMenuIndex)));
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

    oled.setCursor(0,16);
    oled.print("Load");

    oled.setTextSize(2);

    int textLength = 0;
    const char* testPtr = presetMenu[presetMenuIndex];
    while(*(testPtr++) != '\0'){
      textLength++;
    }
    int txtWidth = 10 * (textLength + 1);
    int txtHeight = 14;
    int txtLocX = (((128 - txtWidth) / 2 - 1)<(0)?(0):(((128 - txtWidth) / 2 - 1)>(128 / 2)?(128 / 2):((128 - txtWidth) / 2 - 1)));

    oled.rectFill(0, 26, 55, 26 + txtHeight, 0 /*|< Draw 'off' pixels*/ /*|< Draw 'off' pixels*/, NORM);

    oled.setCursor(txtLocX, 16);
    testPtr = presetMenu[presetMenuIndex];
    while(*testPtr != '\0'){
      oled.print(*testPtr);
      testPtr++;
    }
  }

  oled.display();
  firstMenuRun = false;
  oled.setTextSize(1);
}

void renderPreset(byte preset){
  readPreset(preset);

  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setCursor(0,16);
  oled.print(firemodeMenu[readBlSettings.fireMode]);
  oled.setCursor(0,16 + (7 + 3) * 1);
  oled.print("S:");
  oled.print(readBlSettings.speedValue);
  oled.setCursor(0,16 + (7 + 3) * 2);
  oled.print("B:");
  oled.print(burstMenu[readBlSettings.burstCount]);
  oled.setCursor(0,16 + (7 + 3) * 3);
  oled.print("R:");
  oled.print(readBlSettings.rofValue);

  oled.setCursor(128 / 2,16);
  oled.print("p:");
  oled.print(readBlSettings.minSpinup);
  oled.setCursor(128 / 2,16 + (7 + 3) * 1);
  oled.print("P:");
  oled.print(readBlSettings.maxSpinup);
  oled.setCursor(128 / 2,16 + (7 + 3) * 2);
  oled.print("D:");
  oled.print(readBlSettings.spinDown);
  oled.setCursor(128 / 2,16 + (7 + 3) * 3);
  oled.print("I:");
  oled.print(readBlSettings.idleTime);

  oled.rectFill(57, 39, 8, 9, 1 /*|< Draw 'on' pixels*/ /*|< Draw 'on' pixels*/ , NORM);
  oled.setColor(0 /*|< Draw 'off' pixels*/ /*|< Draw 'off' pixels*/);
  oled.setCursor(0,0);
  oled.setTextSize(2);
  oled.print("Preset: ");
  oled.print(preset);
}
# 1 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Renderer.ino"
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

  if(presetButton != 0){
    if(presetButtonDown() == 4){ //rot button down
      if(!menuBtnWasDown){

        if(liveKnobScrollMode){ //in main menu
          presetMenuIndex = 0;
        }
        else{ //in submenu
          if(knobMenu[knobMenuIndex] == "Load" && presetMenuIndex > 0){
            loadPreset(presetMenuIndex);
          }
          if(knobMenu[knobMenuIndex] == "Save" && presetMenuIndex > 0){
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
# 1 "c:\\Users\\matthias.naumann\\Documents\\FDL-3-ESP32\\Utils.ino"

//roll custom tone generator to avoid conflict with Timer2 and abstract pin
void toneAlt(int frequency, int duration){

  if(!currStSettings.soundOn){ return; }

  int freqDelay = 1000000 / frequency / 2;
  int loopCount = duration * 1000 / (freqDelay * 2);

  unsigned long offTime = millis() + duration;
  while(millis() < offTime){
    digitalWrite(17, 0x1);
    delayMicroseconds(freqDelay);
    digitalWrite(17, 0x0);
    delayMicroseconds(freqDelay);
  }
}

void initBatteryCheck(){
  //init battery average read values
  for(int x = 0; x < 6; x++){
    batteryCheck[x] = 12.0;
  }
  lastBatteryCheck = millis();
  batteryCheckSum = 12.0 * 6;
}

float getVoltLevel(){

 if(millis() > lastBatteryCheck + 300){ //check every 300ms
    const float vPow = 5.042;;
    const float r1 = 100000;
    const float r2 = 10000;

    float v = (analogRead(2) * vPow) / 1024.0;
    float v2 = v / (r2 / (r1 + r2));

    v2 *= 10;
    int v2Int = (int)v2;
    v2 = (float)v2Int / 10;

    batteryCheckSum -= batteryCheck[batteryCheckIndex];
    batteryCheck[batteryCheckIndex] = v2;
    batteryCheckSum += v2;
    batteryCheckIndex++;
    if(batteryCheckIndex >= 6){
      batteryCheckIndex = 0;
    }
    lastBatteryCheck = millis();
  }

  return batteryCheckSum / 6 + (float)currStSettings.batOffset / 10.0;
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
