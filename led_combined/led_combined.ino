#include "FastLED.h"
#include <colors.h>

#define NUM_LEDS_LPD 100
#define NUM_LEDS_WS 150

#define DATA_PIN_LPD 2
#define CLOCK_PIN_LPD 3
#define DATA_PIN_WS 11

#define COLOR CRGB
#define BRIGHTNESS 50

CRGB leds_lpd[NUM_LEDS_LPD];
CRGB leds_ws[NUM_LEDS_WS];

void setup() {
  delay(1000);
  FastLED.addLeds<LPD8806, DATA_PIN_LPD, CLOCK_PIN_LPD, GRB>(leds_lpd, NUM_LEDS_LPD);
  FastLED.addLeds<WS2812B, DATA_PIN_WS, GRB>(leds_ws, NUM_LEDS_WS);
  FastLED.setBrightness(BRIGHTNESS);
}

int lpdPixel = 0;
bool lpdUp = true;
CRGB lpdColor = CRGB::Green;
int wsPixel = 0;
bool wsUp = true;
CRGB wsColor = CRGB::Blue;

void loop() {
    if (lpdUp) {
        leds_lpd[lpdPixel] = lpdColor;
        lpdPixel++;
        if (lpdPixel == NUM_LEDS_LPD) {
            lpdUp = false;
            lpdPixel--;
        }
    } else {
        leds_lpd[lpdPixel] = CRGB::Black;
        lpdPixel--;
        if (lpdPixel == -1) {
            lpdUp = true;
            lpdPixel++;
        }
    }

    if (wsUp) {
        leds_ws[wsPixel] = wsColor;
        wsPixel++;
        if (wsPixel == NUM_LEDS_WS) {
            wsUp = false;
            wsPixel--;
        }
    } else {
        leds_ws[wsPixel] = CRGB::Black;
        wsPixel--;
        if (wsPixel == -1) {
            wsUp = true;
            wsPixel++;
        }
    }

    FastLED.show();
    delay(50);
}