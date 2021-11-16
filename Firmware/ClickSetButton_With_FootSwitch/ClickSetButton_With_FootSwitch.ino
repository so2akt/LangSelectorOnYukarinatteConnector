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

#define HORIZ_POS_ADJ           (1920)
#define VERT_POS_ADJ            (1760)
#define BTN_POS_DIFF            (720)
#define MOUSE_MOVE_ITR          (5)
#define PIN_IN_DAMPER_PEDAL     (1)
#define PIN_IN_SOSTENUTO_PEDAL  (2)
#define PIN_IN_SOFT_PEDAL       (3)
#define PIN_BTN_LANG_1ST        (4)
#define PIN_BTN_LANG_2ND        (5)
#define PIN_BTN_LANG_3RD        (6)

typedef enum TAG_BTN_NAME
{
  LANG_1ST,
  LANG_2ND,
  LANG_3RD,
  LANG_MAX
}btn_name_t;

void MouseMoveAndClick(btn_name_t btn_no)
{
  AbsoluteMouse.moveTo(SHRT_MAX - HORIZ_POS_ADJ,
                        VERT_POS_ADJ + btn_no * BTN_POS_DIFF);
  for(int i=0;i<MOUSE_MOVE_ITR;i++)
  {
    Mouse.move(SCHAR_MAX, 0);
  }
  Mouse.click();

  return;
}

void setup() {
  // Prepare led + buttons
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_IN_DAMPER_PEDAL, INPUT);
  pinMode(PIN_IN_SOSTENUTO_PEDAL, INPUT);
  pinMode(PIN_IN_SOFT_PEDAL, INPUT);
  pinMode(PIN_BTN_LANG_1ST, INPUT_PULLUP);
  pinMode(PIN_BTN_LANG_2ND, INPUT_PULLUP);
  pinMode(PIN_BTN_LANG_3RD, INPUT_PULLUP);
  
  Serial.begin(115200);
  analogReadResolution(10);

  // Sends a clean report to the host. This is important on any Arduino type.
  AbsoluteMouse.begin();
  
  // Sends a clean report to the host. This is important on any Arduino type.
  Mouse.begin();
}

void loop() {
  int valAnalogInDamperPedal = analogRead(PIN_IN_DAMPER_PEDAL);
  int valAnalogInSostenutoPedal = analogRead(PIN_IN_SOSTENUTO_PEDAL);
  int valAnalogInSoftPedal = analogRead(PIN_IN_SOFT_PEDAL);

  Serial.println("Damper(A1)\tSostenuto(A2)\tSoft(A3)");
  Serial.print(valAnalogInDamperPedal);Serial.print("\t");
  Serial.print(valAnalogInSostenutoPedal);Serial.print("\t");
  Serial.print(valAnalogInSoftPedal);Serial.println("");
  delay(500);
  if (!digitalRead(PIN_BTN_LANG_1ST)) {
    digitalWrite(PIN_LED, HIGH);
    MouseMoveAndClick(LANG_1ST);

    // Simple debounce
    delay(300);
    digitalWrite(PIN_LED, LOW);
  }

  if (!digitalRead(PIN_BTN_LANG_2ND)) {
    digitalWrite(PIN_LED, HIGH);
    MouseMoveAndClick(LANG_2ND);

    // Simple debounce
    delay(300);
    digitalWrite(PIN_LED, LOW);
  }

  if (!digitalRead(PIN_BTN_LANG_3RD)) {
    digitalWrite(PIN_LED, HIGH);
    MouseMoveAndClick(LANG_3RD);

    // simple debounce
    delay(300);
    digitalWrite(PIN_LED, LOW);
  }
}