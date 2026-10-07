#include "FastLED.h"

#define NUM_LEDS 32
#define BRIGHTNESS 75
#define DATA_PIN 0
#define CLOCK_PIN 1

#define COLOR CRGB

CRGB leds[NUM_LEDS];

int pattern = 2;
int numPatterns = 6;

void snake();
void progressiveRainbow();
void sparkle();
void fire();
void fillFromCenter();
void marquee();

void (*patterns[])() = {
    fillFromCenter,
    snake,
    progressiveRainbow,
    sparkle,
    fire,
    marquee};

void setup()
{
    FastLED.addLeds<LPD8806, DATA_PIN, CLOCK_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
}

void loop()
{
    patterns[pattern]();
    pattern++;
    if (pattern == numPatterns)
    {
        pattern = 0;
    }
}

void fire()
{
    int ledsPerLoop = 5;
    for (int i = 0; i < NUM_LEDS; i++)
    {
        COLOR color = getRandomFireColor();
        leds[i] = color;
    }

    FastLED.show();

    for (int n = 0; n < 1000; n++)
    {
        for (int i = 0; i < ledsPerLoop; i++)
        {
            int pixel = random(0, NUM_LEDS);
            COLOR color = getRandomFireColor();
            leds[pixel] = color;
        }

        FastLED.show();
        delay(100);
    }
}

void fillFromCenter()
{
    int wheelColor = random(0, 384);
    for (int i = 0; i < 60; i++)
    {
        COLOR color = getColorWheel(wheelColor);

        for (int pixel = NUM_LEDS / 2 - 1; pixel >= 0; pixel--)
        {
            leds[pixel] = color;
            leds[NUM_LEDS - pixel - 1] = color;
            FastLED.show();
            delay(100);
        }

        wheelColor += random(25, 75);
    }
}

void progressiveRainbow()
{
    int wheelColor = random(0, 384);
    int stepSize = 3;
    for (int n = 0; n < 240; n++)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            COLOR color = getColorWheel(wheelColor + stepSize * i);
            leds[i] = color;
        }
        FastLED.show();
        delay(500);
        wheelColor -= stepSize;
        if (wheelColor < 0)
        {
            wheelColor += 384;
        }
    }
}

void sparkle()
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        if (random(0, 2) == 0)
        {
            leds[i] = CRGB::Black;
        }
        else
        {
            leds[i] = randomColorWheel();
        }
    }
    FastLED.show();
    for (int n = 0; n < 1000; n++)
    {
        int pixel = random(0, NUM_LEDS);
        if (leds[pixel] == (CRGB)CRGB::Black)
        {
            leds[pixel] = randomColorWheel();
        }
        else
        {
            leds[pixel] = CRGB::Black;
        }
        FastLED.show();
        delay(100);
    }
}

void snake()
{
    int snakeLength = 5;

    for (int n = 0; n < 30; n++)
    {
        COLOR color = randomColorWheel();

        for (int snakeStart = 0; snakeStart <= NUM_LEDS - snakeLength; snakeStart++)
        {
            for (int i = 0; i < snakeStart; i++)
            {
                leds[i] = CRGB::Black;
            }
            for (int i = snakeStart; i < snakeStart + snakeLength; i++)
            {
                leds[i] = color;
            }
            for (int i = snakeStart + snakeLength; i < NUM_LEDS; i++)
            {
                leds[i] = CRGB::Black;
            }

            FastLED.show();
            delay(100);
        }
        for (int snakeStart = NUM_LEDS - 2; snakeStart >= snakeLength; snakeStart--)
        {
            for (int i = NUM_LEDS - 1; i > snakeStart; i--)
            {
                leds[i] = CRGB::Black;
            }
            for (int i = snakeStart; i > snakeStart - snakeLength; i--)
            {
                leds[i] = color;
            }
            for (int i = snakeStart - snakeLength; i >= 0; i--)
            {
                leds[i] = CRGB::Black;
            }

            FastLED.show();
            delay(100);
        }
    }
}

void marquee()
{
    COLOR color = randomColorWheel();
    int offset = 0;
    for (int n = 0; n < 50; n++)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            bool lit = (i + offset) / 3 % 2 == 0;
            leds[i] = lit ? color : CRGB::Black;
        }
        offset++;
        if (offset == 6)
        {
            offset = 0;
        }
        FastLED.show();
        delay(500);
    }
}

COLOR randomColorWheel()
{
    int r = random(0, 384);
    return getColorWheel(r);
}

COLOR getColorWheel(int val)
{
    while (val >= 384)
    {
        val -= 384;
    }
    while (val < 0)
    {
        val += 384;
    }

    byte r, g, b;

    switch (val / 128)
    {
    case 0:
        r = 255 - (val % 128) * 2; // Red down
        g = (val % 128) * 2;       // Green up
        b = 0;                     // blue off
        break;
    case 1:
        g = 255 - (val % 128) * 2; // green down
        b = (val % 128) * 2;       // blue up
        r = 0;                     // red off
        break;
    case 2:
        b = 255 - (val % 128) * 2; // blue down
        r = (val % 128) * 2;       // red up
        g = 0;                     // green off
        break;
    }

    return getColorRGB(r, g, b);
}

COLOR getColorRGB(int r, int g, int b)
{
    return COLOR(r, g, b);
}

int minGreen = 13;
int maxGreen = 65;
int minScale = 10;
int maxScale = 255;

COLOR getRandomFireColor()
{
    int green = random(13, 65);
    int scale = random(10, 255);
    COLOR color = COLOR(255, green, 0) % scale;
    return color;
}