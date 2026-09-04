#include <FastLED.h>

#define DATA_PIN    5
#define NUM_LEDS    30
#define COLOR_ORDER GRB
#define BRIGHTNESS  100

CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(115200);
  delay(500);

  FastLED.addLeds<SK6812, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setRgbw();
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
}

void loop() {
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.show();
  delay(2000);

  FastLED.clear();
  FastLED.show();
  delay(1000);
}