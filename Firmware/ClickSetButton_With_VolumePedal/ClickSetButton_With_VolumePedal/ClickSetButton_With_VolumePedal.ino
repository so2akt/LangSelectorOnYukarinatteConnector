/*
  Copyright (c) 2014-2015 NicoHood
  See the readme for credit to other people.

  Mouse example
  Press a button to click or move the mouse.

  You may also use BootMouse to enable a bios compatible (single report) mouse.
  However this is very unusual and not recommended.
  Bios mice dont support wheels which can cause problems after a reboot.

  See HID Project documentation for more Information.
  https://github.com/NicoHood/HID/wiki/Mouse-API
*/

#include "HID-Project.h"

const int pinLed = LED_BUILTIN;
const int pinButtonClick = 2;
const int pinButtonMove = 3;
const int pinButtonScroll = 4;
bool isExe = false;

void setup() {
  // Prepare led + buttons
  pinMode(pinLed, OUTPUT);
  pinMode(pinButtonClick, INPUT_PULLUP);
  pinMode(pinButtonMove, INPUT_PULLUP);
  pinMode(pinButtonScroll, INPUT_PULLUP);
  
  // Sends a clean report to the host. This is important on any Arduino type.
  AbsoluteMouse.begin();
  
  // Sends a clean report to the host. This is important on any Arduino type.
  Mouse.begin();
}

void loop() {
  for(int i=0;i<2 && !isExe;i++)
  {
    AbsoluteMouse.moveTo(SHRT_MAX-1920, 1760);
    for(int i=0;i<5;i++)
    {
      Mouse.move(127, 0);
    }
    Mouse.click();
    delay(5000);
    AbsoluteMouse.moveTo(SHRT_MAX-1920, 1760+720);
    for(int i=0;i<5;i++)
    {
      Mouse.move(127, 0);
    }
    Mouse.click();
    delay(5000);
    AbsoluteMouse.moveTo(SHRT_MAX-1920, 1760+720+720);
    for(int i=0;i<5;i++)
    {
      Mouse.move(127, 0);
    }
    Mouse.click();
    delay(5000);
  }
  isExe = true;
  if (!digitalRead(pinButtonClick)) {
    digitalWrite(pinLed, HIGH);
    AbsoluteMouse.moveTo(32767-1920, 1760);
    for(int i=0;i<5;i++)
    {
      Mouse.move(127, 0);
    }
    Mouse.click();
#if 0
    // Same use as the official library, pretty much self explaining
    Mouse.click();
    //Mouse.click(MOUSE_RIGHT);

    // Simple debounce
    delay(300);
    digitalWrite(pinLed, LOW);
#endif
  }

  if (!digitalRead(pinButtonMove)) {
    digitalWrite(pinLed, HIGH);
    AbsoluteMouse.moveTo(32767-1920, 1760+720);
    for(int i=0;i<5;i++)
    {
      Mouse.move(127, 0);
    }
    Mouse.click();
#if 0
    // Same use as the official library, pretty much self explaining
    for(int i=0;i<8;i++)
      Mouse.move(127, 0);

    // Simple debounce
    delay(300);
    digitalWrite(pinLed, LOW);
#endif
  }

  if (!digitalRead(pinButtonScroll)) {
    digitalWrite(pinLed, HIGH);
    AbsoluteMouse.moveTo(32767-1920, 1760+720+720);
    for(int i=0;i<5;i++)
    {
      Mouse.move(127, 0);
    }
    Mouse.click();
#if 0
    // Scroll down a bit, make sure the value is high enough
    Mouse.move(0, 0, 160);

    // simple debounce
    delay(300);
    digitalWrite(pinLed, LOW);
#endif
  }
}
