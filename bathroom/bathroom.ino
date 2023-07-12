#include "FastLED.h"
#include <colors.h>

#define NUM_LEDS 32
#define DATA_PIN 2
#define CLOCK_PIN 3
#define LED_TYPE LPD8806

#define COLOR CRGB

#define BRIGHTNESS 150

CRGB leds[NUM_LEDS];

ColorManager colorManager(leds, NUM_LEDS);

int i = 0;
int loopNum = 0;
CRGB colors[] = {
  COLOR(255, 220, 0),
  COLOR(255, 180, 0),
  COLOR(255, 140, 0),
  COLOR(255, 100, 0),
  COLOR(255, 60, 0),
  COLOR(255, 20, 0),
  COLOR(255, 0, 0)
};

int minGreen = 13;
int maxGreen = 65;

int minScale = 10;
int maxScale = 255;

void setup() {
  delay(1000);
  FastLED.addLeds<LED_TYPE, DATA_PIN, CLOCK_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  colorManager.turnOff();
  FastLED.show();
}


void loop() {

  // first mode - up and down

  int reps = random(5, 11);

  for (int r = 0; r < reps; r++) {

    COLOR c = randomColorWheel();

    for (int i = 0; i < NUM_LEDS; i++) {
      colorManager.setPixelColor(i, c, true);
      delay(50);
    }

    for (int i = NUM_LEDS - 1; i >= 0; i--) {
      colorManager.setPixelColor(i, OFF, true);
      delay(50);
    }
  }
}
