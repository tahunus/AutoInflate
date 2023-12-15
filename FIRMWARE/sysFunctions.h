void ARDUINO_ISR_ATTR onTimer1() {
  STOP();
}
void ARDUINO_ISR_ATTR onTimer2() {
  if (airSys.pumpState) {
    airSys.pumpPressure = 0;
    airSys.pumpState = 0;
    airSys.solenoidState = 0;
    timerStop(timer2);
    timerRestart(timer2);
    timerWrite(timer2, (profileVar.offTime * 1000000));
    timerStart(timer2);
  } else {
    airSys.pumpPressure = profileVar.highPressure;
    airSys.pumpState = 1;
    airSys.solenoidState = 1;
    timerStop(timer2);
    timerRestart(timer2);
    timerWrite(timer2, (profileVar.onTime * 1000000));
    timerStart(timer2);
  }
}

void handleEncoderInterrupt() {
  bool encoderState = digitalRead(encoderPinA);
  if (encoderState != lastEncoderState) {
    if (digitalRead(encoderPinB) != encoderState) {
      encoderInput++;
    } else {
      encoderInput--;
    }
    encoderConstain();
  }
  lastEncoderState = encoderState;

  selectorPreviousMillis = millis();  //Reset flash timer.
  flash = false;
}

void handleButtonPress() {
  bool buttonState = digitalRead(buttonPin);
  if (buttonState == false)  //ON - Button is nominal HIGH!
  {
    isButtonPressed = true;
    encoderInteraction();
  } else if (buttonState == true)  //OFF
  {
    isButtonPressed = false;
  }
}
