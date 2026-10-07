#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

// inputs
#define POWER_PIN 7 // used to be mute on pin 2, but we are saving pin 2 for another possible interrupt
#define TIME_SIGNATURE_BUTTON_PIN 3
#define TEMPO_PIN1 A0
#define TEMPO_PIN2 A1

// outputs
#define BUZZER_PIN 8
#define SEVEN_SEG_DISPLAY_ADDRESS   0x74

// constants
#define FIRST_CLICK_FREQ 2000
#define OTHER_CLICK_FREQ 1000
#define CLICK_LENGTH_MS 100
#define TIME_SIG_DISPLAY_DURATION 3000 // ms
#define MASTER_MUTE true // for debugging to mute the sound without turning off the power

// these determine the tempo bands:
// 40-100
// 80-140
// 120-180
// 160-220
// 200-260
#define MIN_TEMPO 40
#define MAX_TEMPO 250
#define TEMPO_JUMP 5 // can adjust to multiples of this
// #define TEMPO_BANDS 5
// #define TEMPO_BAND_SIZE 60
// #define TEMPO_BAND_OVERLAP 20

#define DEBOUNCE_DELAY 250
#define ANALOG_READ_MIN_DIFF 30

// 40-100
// 80-140
// 120-180
// 160-220
// 200-260

// input settings / control state
volatile byte tempoIndex;
// volatile byte tempoFine; // tempo within the band, 0 - band size
volatile byte tempo; // the actual tempo
volatile byte timeSignature = 4;
boolean emphasizeFirstBeat = true;
volatile boolean power = false;

// calculated consts
// const int MAX_TEMPO = MIN_TEMPO + TEMPO_BANDS * (TEMPO_BAND_SIZE - TEMPO_BAND_OVERLAP) + TEMPO_BAND_OVERLAP;
const int TEMPO_COUNT = (MAX_TEMPO - MIN_TEMPO) / TEMPO_JUMP + 1; // the number of tempos in the range, by the jump
const float TEMPO_INPUT_BUCKET_SIZE = 1024.0 / TEMPO_COUNT;
// int tempoBands[TEMPO_BANDS][2];

// values recalculated on demand
unsigned int beatLengthMs;
unsigned int clickDelayMs; // the time from the end of a click to the start of the next (beat length - click length)

// global state variables
volatile byte curBeat = 0;
Adafruit_7segment display = Adafruit_7segment();
volatile bool recentlySetTimeSig = false;
volatile unsigned long timeSigDisplayStart = 0;

// function-specific state variables defined next to function

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(TIME_SIGNATURE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(POWER_PIN, INPUT_PULLUP);

  display.begin(SEVEN_SEG_DISPLAY_ADDRESS);
  display.drawColon(false);

  attachInterrupt(digitalPinToInterrupt(TIME_SIGNATURE_BUTTON_PIN), timeSignatureInterrupt, FALLING);
  
  // int t = MIN_TEMPO;
  // for (int b = 0; b < TEMPO_BANDS; b++) {
  //   tempoBands[b][0] = t;
  //   tempoBands[b][1] = t + TEMPO_BAND_SIZE;
  //   t += (TEMPO_BAND_SIZE - TEMPO_BAND_OVERLAP);
  // }

  setClickDelayMs();

  Serial.begin(9600);

  Serial.print("Min tempo: ");
  Serial.println(MIN_TEMPO);
  Serial.print("Max tempo: ");
  Serial.println(MAX_TEMPO);
  Serial.print("Tempo count: ");
  Serial.println(TEMPO_COUNT);

}

bool standbyLedState = LOW;
void loop() {
  checkPower();
  if (MASTER_MUTE || !power) {
    digitalWrite(LED_BUILTIN, standbyLedState);
    standbyLedState = !standbyLedState;
    delay(beatLengthMs);
  }
  else {
    unsigned long curTime = millis();
    unsigned int freq = emphasizeFirstBeat && curBeat == 0 ? FIRST_CLICK_FREQ : OTHER_CLICK_FREQ;
    digitalWrite(LED_BUILTIN, HIGH);
    tone(BUZZER_PIN, freq);
    delay(CLICK_LENGTH_MS);
    digitalWrite(LED_BUILTIN, LOW);
    noTone(BUZZER_PIN);
    delay(clickDelayMs);

    if (emphasizeFirstBeat) {
      curBeat++;
      if (curBeat >= timeSignature) {
        curBeat = 0;
      }
    }
  }
  
  setClickDelayMs();
  set7SegmentOutput();
}

byte lastTempoWritten = 0;
void set7SegmentOutput() {
  // TODO there is some point where it forever stops showing the time signature, even though changing time signature does work (the clicks change)
  if (!power) {
    display.clear();
    display.drawColon(false);
    display.writeDisplay();
    lastTempoWritten = 0;
  }
  else if (recentlySetTimeSig && millis() - timeSigDisplayStart < TIME_SIG_DISPLAY_DURATION) {
    lastTempoWritten = 0;
    display.clear();
    display.drawColon(true);
    if (emphasizeFirstBeat) {
      display.writeDigitNum(1, timeSignature);
      display.writeDigitNum(3, 4);
      display.writeDisplay();
    } else {
      display.writeDigitRaw(1, 0b01000000);
      display.writeDigitRaw(3, 0b01000000);
      display.writeDisplay();
    }
  } else if (tempo != lastTempoWritten) {
    recentlySetTimeSig = false;
    display.clear();
    display.print(tempo, DEC);
    display.drawColon(false);
    display.writeDisplay();
    lastTempoWritten = tempo;
  }
}

bool firstCall = true;
unsigned int lastPot1Value = 9999;
// unsigned int lastPot2Value = 9999;
// bool editing = false;
void setClickDelayMs() {
  // TODO possible need to make sure we read the same value 2 or 3 times before actually changing - occasionally saw it fail a read and then return to normal?
  // TODO the potentiometer or the wire is pretty unstable, so the tempo fluctuates by a few bpm constantly unless the read diff is high - but if the
  // read diff is larger than the bucket size, then it is hard to make minor adjustments. Ideas:
  // - detect when it is being edited, and then just show the raw conversion value. When the adjustment stops (within some delta for a couple seconds), then lock it in
  // - buttons to fine tune tempo - if one of them has been pressed and the pot has not changed beyond some delta, then do not update the tempo (out of interrupts)
  // - another pot to set some multiplier, i.e. 1-4, and then pot 2 only shows a window (tempo 40-80, 80-120, 120-160, 160-200 for the example of 4 ranges)
  //    - The ranges should overlap to give a grace window, i.e. if 4 ranges, 40-100, 80-140, 120-180-, 160-220
  // - only allow multiples of 5

  unsigned int pot1Value = analogRead(TEMPO_PIN1);
  // unsigned int pot2Value = analogRead(TEMPO_PIN2);
  // Serial.print(pot1Value);
  // Serial.println(" <-pot1");
  // Serial.println(pot2Value);
  if (firstCall || abs(lastPot1Value - pot1Value) >= ANALOG_READ_MIN_DIFF // || abs(lastPot2Value - pot2Value) >= ANALOG_READ_MIN_DIFF
    ) {
    Serial.println("Calculating new tempo");
    lastPot1Value = pot1Value;
    // lastPot2Value = pot2Value;

    tempoIndex = (int)(pot1Value / TEMPO_INPUT_BUCKET_SIZE);
    // tempoFine = (int)(pot2Value / (float)TEMPO_BAND_SIZE);

    tempo = MIN_TEMPO + tempoIndex * TEMPO_JUMP;
    Serial.print("New tempo: ");
    Serial.print(tempo);
    Serial.print(" (pot value: ");
    Serial.print(pot1Value);
    Serial.print(") (tempo index");
    Serial.print(tempoIndex);
    Serial.println(")");
    float clicksPerSecond = tempo / 60.0;
    beatLengthMs = (int)(1000.0 / clicksPerSecond);
    clickDelayMs = beatLengthMs - CLICK_LENGTH_MS;
  }
  firstCall = false;
}

unsigned long lastTimeSignatureInterruptTime = 0;
void timeSignatureInterrupt() {
  unsigned long curTime = millis();
  if (curTime - lastTimeSignatureInterruptTime > DEBOUNCE_DELAY) {
    lastTimeSignatureInterruptTime = curTime;
    curBeat = 0; // always start over on a first click
    if (!emphasizeFirstBeat) {
      timeSignature = 4;
      emphasizeFirstBeat = true;
    } else if (timeSignature == 4) {
      timeSignature = 3;
    } else {
      emphasizeFirstBeat = false;
    }
    recentlySetTimeSig = true;
    timeSigDisplayStart = curTime;
  }
}

// power switch
void checkPower() {
  power = digitalRead(POWER_PIN) == LOW;
  // Serial.print("Power: ");
  // Serial.println(power);
}

// mute button
// unsigned long lastMuteInterruptTime = 0;
// void muteInterrupt() {
//   unsigned long curTime = millis();
//   if (curTime - lastMuteInterruptTime > DEBOUNCE_DELAY) {
//     lastMuteInterruptTime = curTime;
//     mute = !mute;
//   }
// }
