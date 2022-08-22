#include "colors.h"

// hello

COLOR randomColorWheel() {
  int r = random(0, 384);
  return getColorWheel(r);
}

COLOR randomColorRGB() {
  return getColorRGB(random(0, 256), random(0, 256), random(0, 256));
}

COLOR getColorWheel(int val) {
  while (val >= 384) {
    val -= 384;
  }
  while (val < 0) {
    val += 384;
  }

  byte r, g, b;

  switch (val / 128)
  {
    case 0:
      r = 255 - (val % 128) * 2;   //Red down
      g = (val % 128) * 2;      // Green up
      b = 0;                  //blue off
      break;
    case 1:
      g = 255 - (val % 128) * 2;  //green down
      b = (val % 128) * 2;      //blue up
      r = 0;                  //red off
      break;
    case 2:
      b = 255 - (val % 128) * 2;  //blue down
      r = (val % 128) * 2;      //red up
      g = 0;                  //green off
      break;
  }

  return getColorRGB(r, g, b);
}

COLOR getColorWheel(int val, int brightness) {
  
}

COLOR getColorRGB(int r, int g, int b) {
  return COLOR(r, g, b);
}

COLOR getColorRGB(int r, int g, int b, int brightness) {
  return COLOR(r, g, b);
}

byte getColorComponent(int val, char color) {
  byte r, g, b;

  switch (val / 128)
  {
    case 0:
      r = 255 - (val % 128) * 2;   //Red down
      g = (val % 128) * 2;      // Green up
      b = 0;                  //blue off
      break;
    case 1:
      g = 255 - (val % 128) * 2;  //green down
      b = (val % 128) * 2;      //blue up
      r = 0;                  //red off
      break;
    case 2:
      b = 255 - (val % 128) * 2;  //blue down
      r = (val % 128) * 2;      //red up
      g = 0;                  //green off
      break;
  }

  if (color == 'r') {
    return r;
  }
  else if (color == 'g') {
    return g;
  }
  else {
    return b;
  }
}

ColorManager::ColorManager(COLOR leds[], int numLEDs) {
  _leds = leds;
  _numLEDs = numLEDs;
}

void ColorManager::turnOff() {
  setStripColor(OFF, 0, _numLEDs);
}

void ColorManager::turnOff(byte start, byte count) {
  setStripColor(OFF, start, count);
}

void ColorManager::setStripColor(COLOR color) {
  setStripColor(color, 0, _numLEDs);
}

void ColorManager::setStripColor(byte start, byte count, COLOR color) {
  for (int i = 0; i < count; i++) {
    setPixelColor(i + start, color);
  }
  FastLED.show();
}

void ColorManager::setPixelColor(int pixel, COLOR color) {
  _leds[pixel] = color;
}

void ColorManager::setPixelColor(int pixel, COLOR color, boolean update) {
  _leds[pixel] = color;
  if (update) {
    FastLED.show();
  }
}
