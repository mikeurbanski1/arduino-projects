#include "FastLED.h"
#include <colors.h>

#define NUM_LEDS 24
#define LED_PIN 11
#define BRIGHTNESS 50
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

#define COLOR CRGB

CRGB leds[NUM_LEDS];

void setup() {
    delay(2000);
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
    CRGB color = CRGB::Red;
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = color;
    }

    FastLED.show();

    delay(1000);

    for (int i = NUM_LEDS - 1; i >= 0; i--) {
        leds[i] = CRGB::Black;
    }

    FastLED.show();

    delay(1000);
}
