#ifndef colors_h
#define colors_h

#include "Arduino.h"
#include "FastLED.h"

#define COLOR CRGB

const COLOR OFF = COLOR(0,0,0);

COLOR randomColorWheel();
COLOR randomColorRGB();
COLOR getColorWheel(int val);
COLOR getColorWheel(int val, int brightness);
COLOR getColorRGB(int r, int g, int b);
COLOR getColorRGB(int r, int g, int b, int brightness);
byte getColorComponent(int val, char color);

class ColorManager {
  public:
    ColorManager(COLOR leds[], int numLEDs);
    void turnOff();
    void turnOff(byte start, byte count);
    void setStripColor(COLOR color);
    void setStripColor(byte start, byte count, COLOR color);
    void setPixelColor(int pixel, COLOR color);
    void setPixelColor(int pixel, COLOR color, boolean update);
  private:
    COLOR* _leds;
    int _numLEDs;
};

#endif