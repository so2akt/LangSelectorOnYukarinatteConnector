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
#include <TimerTC3.h>

#define SERIAL_BAUDRATE (115200)
#define TC3_INT_COUNT   (10 * 1000)  // microsecond
#define AD_RESOLUTION   (10)
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
#define NUM_OF_EDGE             (1)
#define NUM_OF_DEBOUNCE         (3)
#define NUM_OF_POLLING          (NUM_OF_EDGE + NUM_OF_DEBOUNCE)

// buttons name on Yukarinette Connector Voice Recognition Window
typedef enum TAG_BUTTON_NAME
{
  LANG_1ST,   // 1st language: Japanese (default)
  LANG_2ND,   // 2nd language: English US (default)
  LANG_3RD,   // 3rd language: Chinese Mainland (default)
  LANG_MAX
}button_number_t;

typedef enum TAG_PEDAL_NUMBER
{
  PEDAL_DAMPER,     // right pedal
  PEDAL_SOSTENUTO,  // middle pedal
  PEDAL_SOFT,       // left pedal
  PEDAL_MAX
}pedal_number_t;

typedef enum TAG_PEDAL_COND
{
  PEDAL_COND_UNPUSHED,      // unpushed or continuing pushed after pushed
  PEDAL_COND_DETECT_PUSHED, // detect pushed edge
  PEDAL_COND_PUSHED,        // pushed
  PEDAL_COND_MAX
}pedal_cond_t;

typedef struct TAG_ANALOG_PEDAL
{
  uint32_t ulPinIndicator;        // indicator led output: ON when pedal pushed
  uint32_t ulPinPedal;            // pedal analog input
  uint16_t usThresholdPushed;     // pedal threshold when pedal pushed
  uint16_t usThresholdUnpushed;   // pedal threshold when pedal unpushed
  bool bIsPushedPoll[NUM_OF_POLLING];  // debounce polling pattern: true when pedal pushed
  pedal_cond_t kPedalCond;        // pedal condition
  uint8_t ucPushedCount;          // number of pushed
}analog_pedal_t;

analog_pedal_t kPedal[PEDAL_MAX] = 
{
  { // damper pedal (right pedal) setting
    PIN_LED,
    PIN_IN_DAMPER_PEDAL,
    DAMPER_PEDAL_CLOSE,
    DAMPER_PEDAL_OPEN,
    {false, false, false, true},  // 4th element prevents from PUSHED detection when started with pedal pushing
    PEDAL_COND_UNPUSHED,
    0
  },
  { // sostenuto pedal (middle pedal) setting
    PIN_LED2,
    PIN_IN_SOSTENUTO_PEDAL,
    SOSTENUTO_PEDAL_CLOSE,
    SOSTENUTO_PEDAL_OPEN,
    {false, false, false, true},  // 4th element prevents from PUSHED detection when started with pedal pushing
    PEDAL_COND_UNPUSHED,
    0
  },
  { // soft pedal (left pedal) setting
    PIN_LED3,
    PIN_IN_SOFT_PEDAL,
    SOFT_PEDAL_CLOSE,
    SOFT_PEDAL_OPEN,
    {false, false, false, true},  // 4th element prevents from PUSHED detection when started with pedal pushing
    PEDAL_COND_UNPUSHED,
    0
  }
};

void MouseMoveAndClick(button_number_t kButtonNumber);
void vInputPoll(void);

void setup()
{
  SerialUSB.begin(SERIAL_BAUDRATE, SERIAL_8N1); // default SERIAL_8N1
  while(!SerialUSB)
  {
    ; // wait for SerialUSB enabled
  }
  SerialUSB.print("serial com opened as ");
  SerialUSB.print("baudrate: ");
  SerialUSB.print(SERIAL_BAUDRATE);
  SerialUSB.println(", data bit: 8, parity bit: none, stop bit: 1");

  pinMode(kPedal[PEDAL_DAMPER].ulPinIndicator, OUTPUT);
  SerialUSB.print("set damper pedal indicator OUTPUT #");
  SerialUSB.println(kPedal[PEDAL_DAMPER].ulPinIndicator);
  pinMode(kPedal[PEDAL_SOSTENUTO].ulPinIndicator, OUTPUT);
  SerialUSB.print("set sostenuto pedal indicator OUTPUT #");
  SerialUSB.println(kPedal[PEDAL_SOSTENUTO].ulPinIndicator);
  pinMode(kPedal[PEDAL_SOFT].ulPinIndicator, OUTPUT);
  SerialUSB.print("set soft pedal indicator OUTPUT #");
  SerialUSB.println(kPedal[PEDAL_SOFT].ulPinIndicator);

  pinMode(kPedal[PEDAL_DAMPER].ulPinPedal, INPUT);
  SerialUSB.print("set damper pedal analog INPUT #");
  SerialUSB.println(kPedal[PEDAL_DAMPER].ulPinPedal);
  pinMode(kPedal[PEDAL_SOSTENUTO].ulPinPedal, INPUT);
  SerialUSB.print("sostenuto pedal analog INPUT #");
  SerialUSB.println(kPedal[PEDAL_SOSTENUTO].ulPinPedal);
  pinMode(kPedal[PEDAL_SOFT].ulPinPedal, INPUT);
  SerialUSB.print("soft pedal analog INPUT #");
  SerialUSB.println(kPedal[PEDAL_SOFT].ulPinPedal);

  analogReadResolution(AD_RESOLUTION);  // default 10-bit on Seeeduino XIAO
  SerialUSB.print("set A/D converter resolution: ");
  SerialUSB.print(AD_RESOLUTION);
  SerialUSB.println(" bits");

  AbsoluteMouse.begin();
  SerialUSB.println("absolute positioning mouse started");
  
  Mouse.begin();
  SerialUSB.println("relative positioning mouse started");

  TimerTc3.initialize(TC3_INT_COUNT);
  TimerTc3.attachInterrupt(vInputPoll);
}

void loop()
{
  ;
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

void vInputPoll(void)
{
  static uint8_t ucPollCount = 0;
  uint8_t ucPollCountPrev = (ucPollCount + NUM_OF_POLLING - 1) % NUM_OF_POLLING;

  kPedal[PEDAL_DAMPER].bIsPushedPoll[ucPollCount]
    = (analogRead(kPedal[PEDAL_DAMPER].ulPinPedal) >= kPedal[PEDAL_DAMPER].usThresholdPushed) ? true : false;
  switch (kPedal[PEDAL_DAMPER].kPedalCond)
  {
    case PEDAL_COND_UNPUSHED:
      if ((!kPedal[PEDAL_DAMPER].bIsPushedPoll[ucPollCountPrev])
        && (kPedal[PEDAL_DAMPER].bIsPushedPoll[ucPollCount]))
      {
        kPedal[PEDAL_DAMPER].kPedalCond = PEDAL_COND_DETECT_PUSHED;
        kPedal[PEDAL_DAMPER].ucPushedCount = 1;
      }
      break;
    case PEDAL_COND_DETECT_PUSHED:
      if (kPedal[PEDAL_DAMPER].bIsPushedPoll[ucPollCount])
      {
        if (NUM_OF_DEBOUNCE == ++kPedal[PEDAL_DAMPER].ucPushedCount)
        {
          kPedal[PEDAL_DAMPER].kPedalCond = PEDAL_COND_PUSHED;
        }
      }
      else
      {
        kPedal[PEDAL_DAMPER].kPedalCond = PEDAL_COND_UNPUSHED;
      }
      break;
    case PEDAL_COND_PUSHED:
      digitalWrite(PIN_LED2, HIGH);
      MouseMoveAndClick(LANG_1ST);
      digitalWrite(PIN_LED2, LOW);
    default:
      kPedal[PEDAL_DAMPER].kPedalCond = PEDAL_COND_UNPUSHED;
      kPedal[PEDAL_DAMPER].ucPushedCount = 0;
      break;
  }

  kPedal[PEDAL_SOSTENUTO].bIsPushedPoll[ucPollCount]
    = (analogRead(kPedal[PEDAL_SOSTENUTO].ulPinPedal) < kPedal[PEDAL_SOSTENUTO].usThresholdPushed) ? true : false;
  switch (kPedal[PEDAL_SOSTENUTO].kPedalCond)
  {
    case PEDAL_COND_UNPUSHED:
      if ((!kPedal[PEDAL_SOSTENUTO].bIsPushedPoll[ucPollCountPrev])
        && (kPedal[PEDAL_SOSTENUTO].bIsPushedPoll[ucPollCount]))
      {
        kPedal[PEDAL_SOSTENUTO].kPedalCond = PEDAL_COND_DETECT_PUSHED;
        kPedal[PEDAL_SOSTENUTO].ucPushedCount = 1;
      }
      break;
    case PEDAL_COND_DETECT_PUSHED:
      if (kPedal[PEDAL_SOSTENUTO].bIsPushedPoll[ucPollCount])
      {
        if (NUM_OF_DEBOUNCE == ++kPedal[PEDAL_SOSTENUTO].ucPushedCount)
        {
          kPedal[PEDAL_SOSTENUTO].kPedalCond = PEDAL_COND_PUSHED;
        }
      }
      else
      {
        kPedal[PEDAL_SOSTENUTO].kPedalCond = PEDAL_COND_UNPUSHED;
      }
      break;
    case PEDAL_COND_PUSHED:
      digitalWrite(PIN_LED3, HIGH);
      MouseMoveAndClick(LANG_2ND);
      digitalWrite(PIN_LED3, LOW);
    default:
      kPedal[PEDAL_SOSTENUTO].kPedalCond = PEDAL_COND_UNPUSHED;
      kPedal[PEDAL_SOSTENUTO].ucPushedCount = 0;
      break;
  }

  kPedal[PEDAL_SOFT].bIsPushedPoll[ucPollCount]
    = (analogRead(kPedal[PEDAL_SOFT].ulPinPedal) < kPedal[PEDAL_SOFT].usThresholdPushed) ? true : false;
  switch (kPedal[PEDAL_SOFT].kPedalCond)
  {
    case PEDAL_COND_UNPUSHED:
      if ((!kPedal[PEDAL_SOFT].bIsPushedPoll[ucPollCountPrev])
        && (kPedal[PEDAL_SOFT].bIsPushedPoll[ucPollCount]))
      {
        kPedal[PEDAL_SOFT].kPedalCond = PEDAL_COND_DETECT_PUSHED;
        kPedal[PEDAL_SOFT].ucPushedCount = 1;
      }
      break;
    case PEDAL_COND_DETECT_PUSHED:
      if (kPedal[PEDAL_SOFT].bIsPushedPoll[ucPollCount])
      {
        if (NUM_OF_DEBOUNCE == ++kPedal[PEDAL_SOFT].ucPushedCount)
        {
          kPedal[PEDAL_SOFT].kPedalCond = PEDAL_COND_PUSHED;
        }
      }
      else
      {
        kPedal[PEDAL_SOFT].kPedalCond = PEDAL_COND_UNPUSHED;
      }
      break;
    case PEDAL_COND_PUSHED:
      digitalWrite(PIN_LED, HIGH);
      MouseMoveAndClick(LANG_3RD);
      digitalWrite(PIN_LED, LOW);
    default:
      kPedal[PEDAL_SOFT].kPedalCond = PEDAL_COND_UNPUSHED;
      kPedal[PEDAL_SOFT].ucPushedCount = 0;
      break;
  }

  ucPollCount = (ucPollCount + 1) % NUM_OF_POLLING;

  return;
}