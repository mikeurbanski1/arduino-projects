#include "FastLED.h"
#include <colors.h>

#define NUM_STRIPS 2
#define STRIP_LENGTH 8
#define NUM_LEDS NUM_STRIPS * STRIP_LENGTH
#define POT_DIFF = 5;
#define DATA_PIN 2
#define CLOCK_PIN 3
#define LED_TYPE LPD8806

#define COLOR CRGB

#define BRIGHTNESS 150

int strip_switch_pins[] = {4, 5};
int strip_color_pins[] = {6, 7};

CRGB leds[NUM_LEDS];

ColorManager colorManager(leds, NUM_LEDS);

boolean lastOnOffState[2];
int lastBrightnessState[2];
int lastColorState[2];

void setup() {
  Serial.begin(9600);
  delay(1000);
  FastLED.addLeds<LED_TYPE, DATA_PIN, CLOCK_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  colorManager.turnOff();
  FastLED.show();

  for (int i = 0; i < NUM_STRIPS; i++) {
    pinMode(strip_switch_pins[i], INPUT_PULLUP);
  }

  Serial.println(1);
}


void loop() {
  Serial.println(2);
  COLOR color = randomColorWheel();

  for (int i = 0; i < STRIP_LENGTH; i++) {
    for (int s = 0; s < NUM_STRIPS; s++) {
      int v = digitalRead(strip_switch_pins[s]);
      Serial.println(v);
      COLOR c = v == 1 ? color : OFF;
      colorManager.setPixelColor(i + s * STRIP_LENGTH, c, false);
      delay(50);
    }
    FastLED.show();
  }

  for (int i = 0; i < STRIP_LENGTH; i++) {
    for (int s = 0; s < NUM_STRIPS; s++) {
      colorManager.setPixelColor(i + s * STRIP_LENGTH, OFF, false);
      delay(50);
    }
    FastLED.show();
  }
}
