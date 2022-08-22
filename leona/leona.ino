#include "FastLED.h"
#include <colors.h>

#define NUM_LEDS 20
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

  // Serial.begin(9600);

  // for (int i = 0; i < 10; i++) {
  //   COLOR color = COLOR(255, 20, 0);
  //   int scale = int(((10.0 - i) / 10.0) * 256) - 1;
  //   // Serial.println(scale);
  //   color %= scale;
  //   colorManager.setStripColor(10 * i, 9, color);
  //   // colorManager.setPixelColor(10 * i + 9, OFF, true);
  // }

  for (int pixel = 0; pixel < NUM_LEDS; pixel++) {
    COLOR color = getRandomFireColor();
    colorManager.setPixelColor(pixel, color);
  }

  FastLED.show();
}

void loop() {
  int pixel = random(0, NUM_LEDS);
  COLOR color = getRandomFireColor();
  colorManager.setPixelColor(pixel, color, true);
  delay(100);
}

COLOR getRandomFireColor() {
  int green = random(minGreen, maxGreen);
  int scale = random(minScale, maxScale);
  COLOR color = COLOR(255, green, 0) % scale;
  return color;
}
