#include "FastLED.h"
#include <colors.h>

#define NUM_LEDS 32
#define DATA_PIN 2
#define CLOCK_PIN 3
#define LED_TYPE LPD8806

#define COLOR CRGB

#define BRIGHTNESS 200

CRGB leds[NUM_LEDS];
ColorManager colorManager(leds, NUM_LEDS);


// fire params
int minGreen = 13;
int maxGreen = 65;
int minScale = 10;
int maxScale = 255;
int ledsPerLoop = 5;

int mode = 0;
int numModes = 6;

// seconds
int modeLength = 180;

void snake();
void fire();
void progressiveRainbow();
void fill();
void marquee();
void sparkle();

void (*patterns[])() = {
    sparkle,
    snake,
    marquee,
    fill,
    fire,
    progressiveRainbow
};

void setup() {
  delay(1000);
  FastLED.addLeds<LED_TYPE, DATA_PIN, CLOCK_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  for (int pixel = 0; pixel < NUM_LEDS; pixel++) {
    COLOR color = getRandomFireColor();
    colorManager.setPixelColor(pixel, color);
  }

  FastLED.show();
}

void loop() {
    patterns[mode]();

    // every time a method returns, it is because the mode ended
    incrementAndReset(&mode, numModes);
}

void sparkle() {
    unsigned long start = millis();

    int pixels[NUM_LEDS];

    for (int i = 0; i < NUM_LEDS; i++) {
        if (random(0, 2) == 0) {
            leds[i] = CRGB::Black;
            pixels[i] = 0;
        }
        else {
            leds[i] = getColorWheel(384 / (NUM_LEDS / (i + 1.0)) - 1);
            pixels[i] = 1;
        }
    }

    FastLED.show();
    delay(50);

    while (timeDiff(start, millis()) < modeLength) {
        int i = random(0, NUM_LEDS);
        if (pixels[i] == 0) {
            leds[i] = getColorWheel(384 / (NUM_LEDS / (i + 1.0)) - 1);
            pixels[i] = 1;
        }
        else {
            leds[i] = CRGB::Black;
            pixels[i] = 0;
        }

        FastLED.show();
        delay(50);
    }
}

void fire() {
    unsigned long start = millis();

    while (timeDiff(start, millis()) < modeLength) {
        for (int i = 0; i < ledsPerLoop; i++) {
            int pixel = random(0, NUM_LEDS);
            COLOR color = getRandomFireColor();
            leds[pixel] = color;
        }

        FastLED.show();
        delay(100);
    }
}

void progressiveRainbow() {
    unsigned long start = millis();

    int colorStepSize = 3;
    int colorChangeSpeed = 3;
    int colorWheelIndex = random(0, 384);

    while (timeDiff(start, millis()) < modeLength) {
        for (int i = 0; i < NUM_LEDS; i++) {
            int wheel = colorWheelIndex - i * colorStepSize;
            leds[i] = getColorWheel(wheel);
        }

        FastLED.show();
        delay(500);
        incrementAndReset(&colorWheelIndex, 384, colorChangeSpeed);
    }
}

void fill() {
    unsigned long start = millis();
    int colorWheelIndex = random(0, 384);
    COLOR color = getColorWheel(colorWheelIndex);
    int pixel = 0;

    while (timeDiff(start, millis()) < modeLength) {
        leds[pixel] = color;
        FastLED.show();
        delay(50);
        incrementAndReset(&pixel, NUM_LEDS);

        if (pixel == 0) {
            incrementAndReset(&colorWheelIndex, 384, random(75, 150));
            color = getColorWheel(colorWheelIndex);
        }
    }
}

void marquee() {
    unsigned long start = millis();
    int offset = 0;
    COLOR color = randomColorWheel();

    while (timeDiff(start, millis()) < modeLength) {

        for (int i = 0; i < NUM_LEDS; i++) {
            if ((6 + i - offset) / 3 % 2 == 0) {
                leds[i] = color;
            }
            else {
                leds[i] = CRGB::Black;
            }
        }

        FastLED.show();
        delay(500);
        incrementAndReset(&offset, 6);
    }
}

void snake() {
    unsigned long start = millis();

    int snakeLength = 4;
    int snakeStart = 25;
    int snakeEnd = snakeStart + snakeLength;

    boolean forward = true;

    COLOR color = randomColorWheel();

    fillRange(leds, 0, snakeStart, color, false);
    fillRange(leds, snakeStart, snakeEnd, CRGB::Black, false);
    fillRange(leds, snakeEnd, NUM_LEDS, color, true);

    FastLED.show();
    delay(1000);

    while (timeDiff(start, millis()) < modeLength) {
        if (forward) {
            leds[snakeStart] = color;
            leds[snakeEnd] = CRGB::Black;

            snakeEnd++;
            snakeStart++;

            if (snakeEnd == NUM_LEDS) {
                forward = false;
            }
        }
        else {
            leds[snakeEnd - 1] = color;
            leds[snakeStart - 1] = CRGB::Black;

            snakeStart--;
            snakeEnd--;

            if (snakeStart == 0) {
                forward = true;
            }
        }

        FastLED.show();
        delay(100);
    }
}

void fillRange(CRGB leds[], int start, int end, COLOR color, boolean show) {
    for (int i = start; i < end; i++) {
        leds[i] = color;
    }

    if (show) {
        FastLED.show();
    }
}

unsigned long timeDiff(unsigned long start, unsigned long end) {
    return (end - start) / 1000;
}

COLOR getRandomFireColor() {
  int green = random(minGreen, maxGreen);
  int scale = random(minScale, maxScale);
  COLOR color = COLOR(255, green, 0) % scale;
  return color;
}

void incrementAndReset(int *value, int max) {
    *value = *value + 1;
    if (*value == max) {
        *value = 0;
    }
}

void incrementAndReset(int *value, int max, int step) {
    *value = *value + step;
    if (*value >= max) {
        *value = *value - max;
    }
}

void decrementAndReset(int *value, int max) {
    *value = *value - 1;
    if (*value == -1) {
        *value = max - 1;
    }
}

void decrementAndReset(int *value, int max, int step) {
    *value = *value - step;
    if (*value < 0) {
        *value = *value + max;
    }
}
