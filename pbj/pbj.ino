#include "FastLED.h"
#include <colors.h>

#define NUM_POLE_LEDS 32
#define NUM_VERTICAL_LEDS 24
#define NUM_HORIZONTAL_LEDS 55

#define BRIGHTNESS 200

#define POLE_1_DATA_PIN 2
#define POLE_1_CLOCK_PIN 3
#define POLE_2_DATA_PIN 4
#define POLE_2_CLOCK_PIN 5
#define VERTICAL_1_DATA_PIN 9
#define VERTICAL_2_DATA_PIN 11
#define HORIZONTAL_DATA_PIN 10
#define BUTTON_PIN 7

#define COMBINE_MODE_BUTTON 8
#define PATTERN_MODE_BUTTON 9

#define COLOR CRGB

// vert1 is the left side when facing it
// pole2 is the left side
// c'est la vie

CRGB vert1Leds[NUM_VERTICAL_LEDS];
CRGB vert2Leds[NUM_VERTICAL_LEDS];
CRGB pole1Leds[NUM_POLE_LEDS];
CRGB pole2Leds[NUM_POLE_LEDS];
CRGB horizLeds[NUM_HORIZONTAL_LEDS];

// just using it as the color picker
ColorManager colorManager({}, 0);

/*
6: rainbow fill stack
7: rainbow fill queue
*/
int pattern = 0;
int numPatterns = 10;

// fire params
int minGreen = 13;
int maxGreen = 65;
int minScale = 10;
int maxScale = 255;
int ledsPerLoop = 3;

// misc params
int horizMiddle = NUM_HORIZONTAL_LEDS / 2;
int numSideLeds = 13;
int horizToVertOffsetFromMiddle = 14;

int lastButtonState;

void snake();
void progressiveRainbowIntersect();
void progressiveRainbowConverge();
void marquee();
void sparkle();
void sparkleBlack();
void sparkleBlue();
void fire();
void fillFromCenter();
void fillFromLeft();

void (*patterns[])() = {
    fillFromLeft,
    fillFromCenter,
    snake,
    progressiveRainbowConverge,
    progressiveRainbowIntersect,
    marquee,
    sparkle,
    sparkleBlack,
    sparkleBlue,
    fire
};

void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    FastLED.addLeds<WS2812B, VERTICAL_1_DATA_PIN, GRB>(vert1Leds, NUM_VERTICAL_LEDS);
    FastLED.addLeds<WS2812B, VERTICAL_2_DATA_PIN, GRB>(vert2Leds, NUM_VERTICAL_LEDS);
    FastLED.addLeds<WS2812B, HORIZONTAL_DATA_PIN, GRB>(horizLeds, NUM_HORIZONTAL_LEDS);
    FastLED.addLeds<LPD8806, POLE_1_DATA_PIN, POLE_1_CLOCK_PIN, GRB>(pole1Leds, NUM_POLE_LEDS);
    FastLED.addLeds<LPD8806, POLE_2_DATA_PIN, POLE_2_CLOCK_PIN, GRB>(pole2Leds, NUM_POLE_LEDS);
    FastLED.setBrightness(BRIGHTNESS);

    lastButtonState = HIGH;
}

void loop() {
    patterns[pattern]();

    // every time a method returns, it is because the button was pressed
    incrementAndReset(&pattern, numPatterns);
}

void fillFromCenter() {

    int pixel = horizMiddle;
    int wheelColor = random(0, 384);
    COLOR color = getColorWheel(wheelColor);
    int longestSide = max(numSideLeds, max(NUM_VERTICAL_LEDS, NUM_POLE_LEDS));

    while (true) {
        if (checkButton()) {
            return;
        }

        if (pixel >= 0) {
            horizLeds[pixel] = color;
            horizLeds[NUM_HORIZONTAL_LEDS - pixel - 1] = color;
        }

        if (pixel <= numSideLeds) {
            int polePixel = numSideLeds - pixel;
            if (polePixel < NUM_POLE_LEDS) {
                pole1Leds[polePixel] = color;
                pole2Leds[polePixel] = color;
            }

            int vertPixel = NUM_VERTICAL_LEDS - 1 - (numSideLeds - pixel);
            if (vertPixel >= 0) {
                vert1Leds[vertPixel] = color;
                vert2Leds[vertPixel] = color;
            }
        }

        pixel--;

        if (pixel == numSideLeds - longestSide - 1) {
            pixel = horizMiddle;
            wheelColor += random(75, 150);
            wheelColor = wheelColor % 384;
            color = getColorWheel(wheelColor);
        }

        FastLED.show();
        delay(100);
    }
}

void fillFromLeft() {

    int pixel = 0;
    int wheelColor = random(0, 384);
    COLOR color = getColorWheel(wheelColor);
    int longestSide = max(numSideLeds, max(NUM_VERTICAL_LEDS, NUM_POLE_LEDS));

    while (true) {
        if (checkButton()) {
            return;
        }

        if (pixel < NUM_HORIZONTAL_LEDS) {
            horizLeds[pixel] = color;
        }

        if (pixel >= numSideLeds) {
            int polePixel = pixel - numSideLeds;
            if (polePixel < NUM_POLE_LEDS) {
                pole2Leds[polePixel] = color;
            }

            int vertPixel = NUM_VERTICAL_LEDS - 1 - polePixel;
            if (vertPixel >= 0) {
                vert1Leds[vertPixel] = color;
            }
        }

        if (pixel >= NUM_HORIZONTAL_LEDS - numSideLeds) {
            int polePixel = pixel - (NUM_HORIZONTAL_LEDS - numSideLeds);
            if (polePixel < NUM_POLE_LEDS) {
                pole1Leds[polePixel] = color;
            }

            int vertPixel = NUM_VERTICAL_LEDS - 1 - polePixel;
            if (vertPixel >= 0) {
                vert2Leds[vertPixel] = color;
            }
        }

        pixel++;

        if (pixel == NUM_HORIZONTAL_LEDS + longestSide - numSideLeds) {
            pixel = 0;
            wheelColor += random(75, 150);
            wheelColor = wheelColor % 384;
            color = getColorWheel(wheelColor);
        }

        FastLED.show();
        delay(100);
    }
}

void snake() {
    int colorWheelIndex = random(0, 384);
    int colorStepSize = 4;

    // 0: on left side
    // 1: in left pole (pole2)
    // 2: in left vert (vert1)
    // 3: in middle
    // 4: in right vert (vert2)
    // 5: in right pole (pole1)
    // 6: in right side
    int snakeStrip = 0;
    int overlapSize = 0;
    int nextStrip = -1;

    int stripLengths[] = {
        numSideLeds,
        NUM_POLE_LEDS,
        NUM_VERTICAL_LEDS,
        NUM_HORIZONTAL_LEDS - numSideLeds * 2,
        NUM_VERTICAL_LEDS,
        NUM_POLE_LEDS,
        numSideLeds
    };

    int snakeLength = 5;

    // snake indexes are relative to the strip
    // so strip 0 goes from horizontal indexes 0 to numSideLeds
    // strip 3 goes from 0 to numHorizontal - numSideLeds
    // indexes always start at the end of the strip at the edge
    // except the center horizontal goes left to right
    int snakeStart = 0;
    int snakeEnd = snakeStart + snakeLength;

    // forward means going towards the center, or left to right for the middle
    bool forward = true;
    bool nextStripForward = false;

    int snakeStartPole = NUM_POLE_LEDS - snakeEnd;
    int snakeEndPole = NUM_POLE_LEDS - snakeStart;

    int snakeStartHoriz = snakeStart + numSideLeds;
    int snakeEndHoriz = snakeEnd + numSideLeds;

    int snakeStartSide2 = NUM_HORIZONTAL_LEDS - snakeEnd;
    int snakeEndSide2 = NUM_HORIZONTAL_LEDS - snakeStart;

    while (true) {
        if (checkButton()) {
            return;
        }

        COLOR color = getColorWheel(colorWheelIndex);
        fillAll(color, false);

        // normal cases - snake entirely within strip
        if (snakeStrip == 0) {
            fillRange(horizLeds, snakeStart, min(snakeEnd, stripLengths[snakeStrip]), CRGB::Black, false);
        }
        else if (snakeStrip == 1) {
            fillRange(pole2Leds, snakeStartPole, min(snakeEndPole, stripLengths[snakeStrip]), CRGB::Black, false);
        }
        else if (snakeStrip == 2) {
            fillRange(vert1Leds, snakeStart, min(snakeEnd, stripLengths[snakeStrip]), CRGB::Black, false);
        }
        else if (snakeStrip == 3 && forward) {
            fillRange(horizLeds, snakeStartHoriz, min(snakeEndHoriz, NUM_HORIZONTAL_LEDS - numSideLeds), CRGB::Black, false);
        }
        else if (snakeStrip == 3 && !forward) {
            fillRange(horizLeds, max(snakeStartHoriz, numSideLeds), snakeEndHoriz, CRGB::Black, false);
        }
        else if (snakeStrip == 4) {
            fillRange(vert2Leds, snakeStart, min(snakeEnd, stripLengths[snakeStrip]), CRGB::Black, false);
        }
        else if (snakeStrip == 5) {
            fillRange(pole1Leds, snakeStartPole, min(snakeEndPole, NUM_POLE_LEDS), CRGB::Black, false);
        }
        else if (snakeStrip == 6) {
            fillRange(horizLeds, max(snakeStartSide2, NUM_HORIZONTAL_LEDS - numSideLeds), snakeEndSide2, CRGB::Black, false);
        }

        // handle overlap
        if (overlapSize > 0) {
            if (nextStrip == 0) {
                fillRange(horizLeds, numSideLeds - overlapSize, numSideLeds, CRGB::Black, false);
            }
            else if (nextStrip == 1) {
                fillRange(pole2Leds, 0, overlapSize, CRGB::Black, false);
            }
            else if (nextStrip == 2) {
                fillRange(vert1Leds, NUM_VERTICAL_LEDS - overlapSize, NUM_VERTICAL_LEDS, CRGB::Black, false);
            }
            else if (nextStrip == 3 && (snakeStrip == 0 || snakeStrip == 1 || snakeStrip == 2)) {
                fillRange(horizLeds, numSideLeds, numSideLeds + overlapSize, CRGB::Black, false);
            }
            else if (nextStrip == 3 && (snakeStrip == 4 || snakeStrip == 5 || snakeStrip == 6)) {
                fillRange(horizLeds, NUM_HORIZONTAL_LEDS - numSideLeds - overlapSize, NUM_HORIZONTAL_LEDS - numSideLeds, CRGB::Black, false);
            }
            else if (nextStrip == 4) {
                fillRange(vert2Leds, NUM_VERTICAL_LEDS - 1 - overlapSize, NUM_VERTICAL_LEDS, CRGB::Black, false);
            }
            else if (nextStrip == 5) {
                fillRange(pole1Leds, 0, overlapSize, CRGB::Black, false);
            }
            else if (nextStrip == 6) {
                fillRange(horizLeds, NUM_HORIZONTAL_LEDS - numSideLeds, NUM_HORIZONTAL_LEDS - numSideLeds + overlapSize, CRGB::Black, false);
            }
        }

        if (forward) {
            snakeStart++;
            snakeEnd++;

            if (snakeEnd == stripLengths[snakeStrip]) {
                // we have hit the end - do we reverse or go to a new segment?
                // 0 = reverse
                // 1 = turn "left"
                // 2 = straight
                // 3 = right
                int r = random(0, 4);
                if (r == 0) {
                    forward = false;
                }
                else {
                    if (snakeStrip == 0) {
                        if (r == 1) {
                            nextStrip = 1;
                            nextStripForward = false;
                        }
                        else if (r == 2) {
                            nextStrip = 3;
                            nextStripForward = true;
                        }
                        else {
                            nextStrip = 2;
                            nextStripForward = false;
                        }
                    }
                    else if (snakeStrip == 1) {
                        if (r == 1) {
                            nextStrip = 3;
                            nextStripForward = true;
                        }
                        else if (r == 2) {
                            nextStrip = 2;
                            nextStripForward = false;
                        }
                        else {
                            nextStrip = 0;
                            nextStripForward = false;
                        }
                    }
                    else if (snakeStrip == 2) {
                        if (r == 1) {
                            nextStrip = 0;
                            nextStripForward = false;
                        }
                        else if (r == 2) {
                            nextStrip = 1;
                            nextStripForward = false;
                        }
                        else {
                            nextStrip = 3;
                            nextStripForward = true;
                        }
                    }
                    else if (snakeStrip == 3) {
                        if (r == 1) {
                            nextStrip = 5;
                            nextStripForward = false;
                        }
                        else if (r == 2) {
                            nextStrip = 6;
                            nextStripForward = false;
                        }
                        else {
                            nextStrip = 4;
                            nextStripForward = false;
                        }
                    }
                    else if (snakeStrip == 4) {
                        if (r == 1) {
                            nextStrip = 3;
                            nextStripForward = false;
                        }
                        else if (r == 2) {
                            nextStrip = 5;
                            nextStripForward = false;
                        }
                        else {
                            nextStrip = 6;
                            nextStripForward = false;
                        }
                    }
                    else if (snakeStrip == 5) {
                        if (r == 1) {
                            nextStrip = 6;
                            nextStripForward = false;
                        }
                        else if (r == 2) {
                            nextStrip = 4;
                            nextStripForward = false;
                        }
                        else {
                            nextStrip = 3;
                            nextStripForward = false;
                        }
                    }
                    else if (snakeStrip == 6) {
                        if (r == 1) {
                            nextStrip = 4;
                            nextStripForward = false;
                        }
                        else if (r == 2) {
                            nextStrip = 3;
                            nextStripForward = false;
                        }
                        else {
                            nextStrip = 5;
                            nextStripForward = false;
                        }
                    }
                }
            }
            else if (snakeEnd > stripLengths[snakeStrip]) {
                overlapSize++;
                // once we have the whole snake on to the next strip via an overlap, then we reset things for that strip
                // use an offset of 1 to avoid painting the same state twice
                if (overlapSize == snakeLength + 1) {
                    if (nextStripForward) {
                        snakeStart = 1;
                        snakeEnd = snakeStart + snakeLength;
                    }
                    else {
                        snakeEnd = stripLengths[nextStrip] - 1;
                        snakeStart = snakeEnd - snakeLength;
                    }

                    snakeStrip = nextStrip;
                    forward = nextStripForward;

                    overlapSize = 0;
                }
            }
        }
        else {
            snakeStart--;
            snakeEnd--;
            if (snakeStart == 0) {
                // for everything but the middle, this is just hitting the end and bouncing back
                if (snakeStrip != 3) {
                    forward = true;
                }
                else {
                    int r = random(0, 4);
                    if (r == 0) {
                        forward = true;
                    }
                    else if (r == 1) {
                        nextStrip = 2;
                        nextStripForward = false;
                    }
                    else if (r == 2) {
                        nextStrip = 0;
                        nextStripForward = false;
                    }
                    else {
                        nextStrip = 1;
                        nextStripForward = false;
                    }
                }
            }
            else if (snakeStart < 0) {
                overlapSize++;

                // use an offset of 1 to avoid painting the same state twice
                if (overlapSize == snakeLength + 1) {
                    if (nextStripForward) {
                        snakeStart = 1;
                        snakeEnd = snakeStart + snakeLength;
                    }
                    else {
                        snakeEnd = stripLengths[nextStrip] - 1;
                        snakeStart = snakeEnd - snakeLength;
                    }

                    snakeStrip = nextStrip;
                    forward = nextStripForward;

                    overlapSize = 0;
                }
            }
        }

        snakeStartPole = NUM_POLE_LEDS - snakeEnd;
        snakeEndPole = NUM_POLE_LEDS - snakeStart;

        snakeStartHoriz = snakeStart + numSideLeds;
        snakeEndHoriz = snakeEnd + numSideLeds;

        snakeStartSide2 = NUM_HORIZONTAL_LEDS - snakeEnd;
        snakeEndSide2 = NUM_HORIZONTAL_LEDS - snakeStart;

        FastLED.show();
        delay(100);
    }
}

void progressiveRainbowConverge() {
    int colorStepSize = 6;
    int colorChangeSpeed = 2;
    int masterColorWheelIndex = random(0, 384);

    COLOR color;

    while (true) {
        if (checkButton()) {
            return;
        }

        int colorWheelIndex = masterColorWheelIndex;

        for (int i = horizMiddle; i >= numSideLeds; i--) {
            color = getColorWheel(colorWheelIndex);
            horizLeds[i] = color;
            horizLeds[NUM_HORIZONTAL_LEDS - i - 1] = color;
            incrementAndReset(&colorWheelIndex, 384, colorStepSize);
        }

        int verticalColorStart = colorWheelIndex;

        for (int i = numSideLeds - 1; i >= 0; i--) {
            color = getColorWheel(colorWheelIndex);
            horizLeds[i] = color;
            horizLeds[NUM_HORIZONTAL_LEDS - i - 1] = color;
            decrementAndReset(&colorWheelIndex, 384, colorStepSize);
        }

        for (int i = 0; i < NUM_POLE_LEDS; i++) {
            color = getColorWheel(verticalColorStart - i * colorStepSize);
            pole1Leds[i] = color;
            pole2Leds[i] = color;
        }

        for (int i = NUM_VERTICAL_LEDS - 1; i >= 0; i--) {
            color = getColorWheel(verticalColorStart - (NUM_VERTICAL_LEDS - 1 - i) * colorStepSize);
            vert1Leds[i] = color;
            vert2Leds[i] = color;
        }

        FastLED.show();
        delay(500);
        incrementAndReset(&masterColorWheelIndex, 384, colorChangeSpeed);
    }
}

void progressiveRainbowIntersect() {
    int colorStepSize = 3;
    int colorChangeSpeed = 2;
    int masterColorWheelIndex = random(0, 384);

    COLOR color;

    while (true) {
        if (checkButton()) {
            return;
        }

        int colorWheelIndex = masterColorWheelIndex;

        int vertical1ColorStart = 0;
        int vertical2ColorStart = 0;

        for (int i = 0; i < NUM_HORIZONTAL_LEDS; i++) {
            int wheel = colorWheelIndex + i * colorStepSize;
            color = getColorWheel(wheel);
            horizLeds[i] = color;
            if (i == numSideLeds - 1) {
                vertical1ColorStart = wheel;
            }
            else if (i == NUM_HORIZONTAL_LEDS - numSideLeds) {
                vertical2ColorStart = wheel;
            }
        }

        for (int i = NUM_VERTICAL_LEDS - 1; i >= 0; i--) {
            color = getColorWheel(vertical1ColorStart + (NUM_VERTICAL_LEDS - i - 1) * colorStepSize);
            vert1Leds[i] = color;
        }

        for (int i = 0; i < NUM_POLE_LEDS; i++) {
            color = getColorWheel(vertical1ColorStart - i * colorStepSize);
            pole2Leds[i] = color;
        }

        for (int i = NUM_VERTICAL_LEDS - 1; i >= 0; i--) {
            color = getColorWheel(vertical2ColorStart - (NUM_VERTICAL_LEDS - i - 1) * colorStepSize);
            vert2Leds[i] = color;
        }

        for (int i = 0; i < NUM_POLE_LEDS; i++) {
            color = getColorWheel(vertical2ColorStart + i * colorStepSize);
            pole1Leds[i] = color;
        }

        FastLED.show();
        delay(500);
        incrementAndReset(&masterColorWheelIndex, 384, colorChangeSpeed);
    }
}

void marquee() {
    fillAll(CRGB::Black, true);

    int colorWheelIndex = random(0, 384);
    COLOR color = getColorWheel(colorWheelIndex);

    int sideOffset = 5;
    int middleOffset = 2;
    int vertOffset = 3;
    int loopNumber = 0;

    while (true) {
        if (checkButton()) {
            return;
        }

        for (int i = 0; i < numSideLeds; i++) {
            if ((6 + i - sideOffset) / 3 % 2 == 0) {
                horizLeds[i] = color;
                horizLeds[NUM_HORIZONTAL_LEDS - i - 1] = color;
            }
            else {
                horizLeds[i] = CRGB::Black;
                horizLeds[NUM_HORIZONTAL_LEDS - i - 1] = CRGB::Black;
            }
        }

        for (int i = horizMiddle - 2; i < NUM_HORIZONTAL_LEDS - numSideLeds; i++) {
            if (i < horizMiddle) {
                continue;
            }
            if ((6 + i - horizMiddle - middleOffset) / 3 % 2 == 0) {
                horizLeds[i] = color;
                horizLeds[NUM_HORIZONTAL_LEDS - i - 1] = color;
            }
            else {
                horizLeds[i] = CRGB::Black;
                horizLeds[NUM_HORIZONTAL_LEDS - i - 1] = CRGB::Black;
            }
        }

        // pole starts at 0, vert starts at len - 1
        for (int i = 0; i < max(NUM_POLE_LEDS, NUM_VERTICAL_LEDS); i++) {
            if ((6 + i - sideOffset) / 3 % 2 == 0) {
                if (i < NUM_POLE_LEDS) {
                    pole1Leds[i] = color;
                    pole2Leds[i] = color;
                }

                if (i < NUM_VERTICAL_LEDS) {
                    vert1Leds[NUM_VERTICAL_LEDS - i - 1] = color;
                    vert2Leds[NUM_VERTICAL_LEDS - i - 1] = color;
                }
            }
            else {
                if (i < NUM_POLE_LEDS) {
                    pole1Leds[i] = CRGB::Black;
                    pole2Leds[i] = CRGB::Black;
                }

                if (i < NUM_VERTICAL_LEDS) {
                    vert1Leds[NUM_VERTICAL_LEDS - i - 1] = CRGB::Black;
                    vert2Leds[NUM_VERTICAL_LEDS - i - 1] = CRGB::Black;
                }
            }
        }

        incrementAndReset(&sideOffset, 6);
        incrementAndReset(&middleOffset, 6);
        incrementAndReset(&vertOffset, 6);

        FastLED.show();
        delay(500);

        incrementAndReset(&colorWheelIndex, 384);
        color = getColorWheel(colorWheelIndex);
    }
}

void sparkle() {
    sparkleBackground(CRGB::Black, false);
}

void sparkleBlack() {
    sparkleBackground(CRGB::Black, true);
}

void sparkleBlue() {
    sparkleBackground(CRGB::Blue, true);
}

void sparkleBackground(COLOR background, bool useBackground) {
    if (useBackground) {
        fillAll(background, true);
    }

    while (true) {
        if (checkButton()) {
            return;
        }

        if (useBackground) {
            setOrResetPixel(vert1Leds, random(0, NUM_VERTICAL_LEDS), background);
            setOrResetPixel(vert2Leds, random(0, NUM_VERTICAL_LEDS), background);
            setOrResetPixel(pole1Leds, random(0, NUM_POLE_LEDS), background);
            setOrResetPixel(pole2Leds, random(0, NUM_POLE_LEDS), background);
            setOrResetPixel(horizLeds, random(0, NUM_HORIZONTAL_LEDS), background);
            FastLED.show();
            delay(100);
        }
        else {
            vert1Leds[random(0, NUM_VERTICAL_LEDS)] = randomColorWheel();
            vert2Leds[random(0, NUM_VERTICAL_LEDS)] = randomColorWheel();
            pole1Leds[random(0, NUM_POLE_LEDS)] = randomColorWheel();
            pole2Leds[random(0, NUM_POLE_LEDS)] = randomColorWheel();
            horizLeds[random(0, NUM_HORIZONTAL_LEDS)] = randomColorWheel();
            FastLED.show();
            delay(100);
        }
    }
}

void setOrResetPixel(COLOR leds[], int pixel, COLOR background) {
    if (leds[pixel] == background) {
        leds[pixel] = randomColorWheel();
    }
    else {
        leds[pixel] = background;
    }
}

void fire() {
    for (int i = 0; i < NUM_VERTICAL_LEDS; i++) {
        COLOR color = getRandomFireColor();
        vert1Leds[i] = color;
    }

    for (int i = 0; i < NUM_VERTICAL_LEDS; i++) {
        COLOR color = getRandomFireColor();
        vert2Leds[i] = color;
    }

    for (int i = 0; i < NUM_POLE_LEDS; i++) {
        COLOR color = getRandomFireColor();
        pole1Leds[i] = color;
    }

    for (int i = 0; i < NUM_POLE_LEDS; i++) {
        COLOR color = getRandomFireColor();
        pole2Leds[i] = color;
    }

    for (int i = 0; i < NUM_HORIZONTAL_LEDS; i++) {
        COLOR color = getRandomFireColor();
        horizLeds[i] = color;
    }

    FastLED.show();

    while (true) {
        if (checkButton()) {
            return;
        }

        for (int i = 0; i < ledsPerLoop; i++) {
            int pixel = random(0, NUM_VERTICAL_LEDS);
            COLOR color = getRandomFireColor();
            vert1Leds[pixel] = color;
        }

        for (int i = 0; i < ledsPerLoop; i++) {
            int pixel = random(0, NUM_VERTICAL_LEDS);
            COLOR color = getRandomFireColor();
            vert2Leds[pixel] = color;
        }

        for (int i = 0; i < ledsPerLoop; i++) {
            int pixel = random(0, NUM_POLE_LEDS);
            COLOR color = getRandomFireColor();
            pole1Leds[pixel] = color;
        }

        for (int i = 0; i < ledsPerLoop; i++) {
            int pixel = random(0, NUM_POLE_LEDS);
            COLOR color = getRandomFireColor();
            pole2Leds[pixel] = color;
        }

        for (int i = 0; i < ledsPerLoop * 2; i++) {
            int pixel = random(0, NUM_HORIZONTAL_LEDS);
            COLOR color = getRandomFireColor();
            horizLeds[pixel] = color;
        }

        FastLED.show();
        delay(100);
    }
}

COLOR getRandomFireColor() {
  int green = random(minGreen, maxGreen);
  int scale = random(minScale, maxScale);
  COLOR color = COLOR(255, green, 0) % scale;
  return color;
}

boolean checkButton() {
    int value = digitalRead(7);
    if (value == LOW && lastButtonState != LOW) {
        lastButtonState = LOW;
        return true;
    }
    else if (value == HIGH && lastButtonState == LOW) {
        lastButtonState = HIGH;
    }

    return false;
}

void fillAll(COLOR color, bool show) {
    for (int i = 0; i < NUM_VERTICAL_LEDS; i++) {
        vert1Leds[i] = color;
        vert2Leds[i] = color;
    }

    for (int i = 0; i < NUM_POLE_LEDS; i++) {
        pole1Leds[i] = color;
        pole2Leds[i] = color;
    }

    for (int i = 0; i < NUM_HORIZONTAL_LEDS; i++) {
        horizLeds[i] = color;
    }

    if (show) {
        FastLED.show();
    }
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

void fillRange(CRGB leds[], int start, int end, COLOR color, boolean show) {
    for (int i = start; i < end; i++) {
        leds[i] = color;
    }

    if (show) {
        FastLED.show();
    }
}