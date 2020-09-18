/////////////
///RENDER////
/////////////
void renderScreen(){

  renderVoltMeter();
  renderMagRemainig();

  if(liveKnobScrollMode){
    renderKnobScrollMenu();
  }
  
  else{
    String menuName = knobMenu[knobMenuIndex];

    if(menuName == MENU_SPEED)
    {
      renderGauge(currBlSettings.speedValue, MENU_SPEED, 0, 100, currStSettings.minSpeed, currStSettings.maxSpeed, 1);
    } 
    else if(menuName == MENU_ROF)
    {
      renderGauge(currBlSettings.rofValue, MENU_ROF, 0, 100, 0, 100, 1);
    }
    else if(menuName == MENU_BURST)
    {
      renderMenu(currBlSettings.burstCount, MENU_BURST, burstMenu, sizeof(burstMenu) / sizeof(size_t));
    }
    else if(menuName == MENU_MINSPIN)
    {
      renderGauge(currBlSettings.minSpinup, MENU_MINSPIN, 150, 500, 150, 500, 1);
    }
    else if(menuName == MENU_MAXSPIN)
    {
      renderGauge(currBlSettings.maxSpinup, MENU_MAXSPIN, 150, 500, 150, 500, 1);
    }
    else if(menuName == MENU_FIREMODE)
    {
      renderMenu(currBlSettings.fireMode, MENU_FIREMODE, firemodeMenu, sizeof(firemodeMenu) / sizeof(size_t));
    }
    else if(menuName == MENU_SPINDOWN)
    {
      renderGauge(currBlSettings.spinDown, MENU_SPINDOWN, 6, 25, 6, 25, 8);
    }
    else if(menuName == MENU_IDLE)
    {
      renderGauge(currBlSettings.idleTime, MENU_IDLE, 0, 10, 0, 10, 8);
    }
    else if(menuName == MENU_LOAD)
    {
      renderPresetMenu();
    }
    else if(menuName == MENU_SAVE)
    {
      renderMenu(presetMenuIndex, MENU_SAVE, presetMenu, sizeof(presetMenu) / sizeof(size_t));
    }
    else if(menuName == MENU_MINSPD)
    {
      renderGauge(currStSettings.minSpeed, MENU_MINSPD, 0, 100, 0, currStSettings.maxSpeed, 1);
    }
    else if(menuName == MENU_MAXSPD)
    {
      if(!speedLocked){
          renderGauge(currStSettings.maxSpeed, MENU_MAXSPD, 0, 100, currStSettings.minSpeed, 100, 1);
        }
    }
    else if(menuName == MENU_BTNMODE)
    {
      renderMenu(currStSettings.btnMode, MENU_BTNMODE, btnmodeMenu, sizeof(btnmodeMenu) / sizeof(size_t));
    }
    else if(menuName == MENU_BRKAG)
    {
      renderGauge(currStSettings.brkAgr, MENU_BRKAG, 3, 25, 3, 25, 8);
    }
    else if(menuName == MENU_USERLOCK)
    {
      renderUserLock();
    }
    else if(menuName == MENU_SOUND)
    {
      renderMenu(currStSettings.soundOn, MENU_SOUND, soundMenu, sizeof(soundMenu) / sizeof(size_t));
    }
    else if(menuName == MENU_BATOFFSET)
    {
      renderGauge(currStSettings.batOffset, MENU_BATOFFSET, -8, 8, -8, 8, 8);
    }
    else if(menuName == MENU_INFO)
    {
      renderInfoMenu();
    }
    else if(menuName == MENU_MAGSIZE)
    {
      renderGauge(currStSettings.magSize, MENU_MAGSIZE, 1, 100, 1, 100, 1);
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
  if(currStSettings.btnMode == 4){// reload
    currentMagCount = currStSettings.magSize;
    renderMagRemainig();
  }
}
