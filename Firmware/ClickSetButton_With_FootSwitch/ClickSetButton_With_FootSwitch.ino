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

#define HORIZ_POS_ADJ   (1920)
#define VERT_POS_ADJ    (1760)
#define BTN_POS_DIFF    (720)
#define MOUSE_MOVE_ITR  (5)
#define DAMPER_PEDAL_OPEN       (261)
#define DAMPER_PEDAL_CLOSE      (800)
#define SOSTENUTO_PEDAL_OPEN    (1023)
#define SOSTENUTO_PEDAL_CLOSE   (530)
#define SOFT_PEDAL_OPEN         SOSTENUTO_PEDAL_OPEN
#define SOFT_PEDAL_CLOSE        SOSTENUTO_PEDAL_CLOSE
#define PIN_IN_DAMPER_PEDAL     (1)
#define PIN_IN_SOSTENUTO_PEDAL  (2)
#define PIN_IN_SOFT_PEDAL       (3)

typedef enum TAG_BUTTON_NAME
{
  LANG_1ST,
  LANG_2ND,
  LANG_3RD,
  LANG_MAX
}button_number_t;

void MouseMoveAndClick(button_number_t kButtonNumber);

void setup()
{
  // Prepare led + buttons
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_IN_DAMPER_PEDAL, INPUT);
  pinMode(PIN_IN_SOSTENUTO_PEDAL, INPUT);
  pinMode(PIN_IN_SOFT_PEDAL, INPUT);
  
  Serial.begin(115200);
  analogReadResolution(10);

  // Sends a clean report to the host. This is important on any Arduino type.
  AbsoluteMouse.begin();
  
  // Sends a clean report to the host. This is important on any Arduino type.
  Mouse.begin();
}

void loop()
{
  static bool IsUnpushedDamperPedal = false;
  static bool IsUnpushedSostenutoPedal = false;
  static bool IsUnpushedSoftPedal = false;
  static int16_t PrevValDamperPedal = DAMPER_PEDAL_OPEN;
  static int16_t PrevValSostenutoPedal = SOSTENUTO_PEDAL_OPEN;
  static int16_t PrevValSoftPedal = SOFT_PEDAL_OPEN;

  int16_t ValDamperPedal = analogRead(PIN_IN_DAMPER_PEDAL);
  int16_t ValSostenutoPedal = analogRead(PIN_IN_SOSTENUTO_PEDAL);
  int16_t ValSoftPedal = analogRead(PIN_IN_SOFT_PEDAL);

  Serial.print("Damper: ");Serial.println(ValDamperPedal);
  Serial.print("Sostenuto: ");Serial.println(ValSostenutoPedal);
  Serial.print("Soft: ");Serial.println(ValSoftPedal);

  if(ValDamperPedal < DAMPER_PEDAL_OPEN)
  {
    IsUnpushedDamperPedal = true;
  }
  else if(IsUnpushedDamperPedal && (ValDamperPedal > DAMPER_PEDAL_CLOSE))
  {
    IsUnpushedDamperPedal = false;
    digitalWrite(PIN_LED, HIGH);
    MouseMoveAndClick(LANG_1ST);
    digitalWrite(PIN_LED, LOW);
  }

  if(ValSostenutoPedal >= SOSTENUTO_PEDAL_OPEN)
  {
    IsUnpushedSostenutoPedal = true;
  }
  else if(IsUnpushedSostenutoPedal && (ValSostenutoPedal < SOSTENUTO_PEDAL_CLOSE))
  {
    IsUnpushedSostenutoPedal = false;
    digitalWrite(PIN_LED2, HIGH);
    MouseMoveAndClick(LANG_2ND);
    digitalWrite(PIN_LED2, LOW);
  }

  if(ValSoftPedal >= SOFT_PEDAL_OPEN)
  {
    IsUnpushedSoftPedal = true;
  }
  else if(IsUnpushedSoftPedal && (ValSoftPedal < SOFT_PEDAL_CLOSE))
  {
    IsUnpushedSoftPedal = false;
    digitalWrite(PIN_LED3, HIGH);
    MouseMoveAndClick(LANG_3RD);
    digitalWrite(PIN_LED3, LOW);
  }
}

void MouseMoveAndClick(button_number_t kButtonNumber)
{
  AbsoluteMouse.moveTo(SHRT_MAX - HORIZ_POS_ADJ,
                        VERT_POS_ADJ + kButtonNumber * BTN_POS_DIFF);
  for(int i = 0; i < MOUSE_MOVE_ITR; i++)
  {
    Mouse.move(SCHAR_MAX, 0);
  }
  Mouse.click();

  return;
}
