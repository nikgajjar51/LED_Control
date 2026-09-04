#include <FastLED.h>

/* Data Pins and Configuration */

#define LED_Data_Pin 5
#define LED_Color_Order GRB
#define Ultrasonic_Trigger_Pin 18
#define Ultrasonic_Echo_Pin 17

/* Timings and Measurements */
#define Trigger_Distance 20 // The distance within the LED's will trigger when motion is detected.
#define LED_Hold_Time 5000  // The time the LED's will stay on after being triggered in Milliseconds

/* LED Strip Configuration */
#define LED_Count 239
#define LED_Max_Brightness 255
CRGB LED_Base_Color = CRGB::DarkBlue;
CRGB LED_Strip[LED_Count];

bool lightsOn = false;

unsigned long lastPing = 0;
const int pingInterval = 100; // ms between ultrasonic checks

unsigned long lastDetectionTime = 0;

/* Wipe Animation Configuration */
float wipeProgress = 0;           // current wave position, in LED units
const float wipeSpeed = 0.15;     // LEDs advanced per ms — lower = slower wipe
const float wipeFadeWidth = 15.0; // how many LEDs wide the soft fade edge is
unsigned long lastFrameTime = 0;

long readDistanceCM()
{
  digitalWrite(Ultrasonic_Trigger_Pin, LOW);
  delayMicroseconds(2);
  digitalWrite(Ultrasonic_Trigger_Pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(Ultrasonic_Trigger_Pin, LOW);

  long duration = pulseIn(Ultrasonic_Echo_Pin, HIGH, 25000); // 25ms timeout ~4m max range
  if (duration == 0)
    return -1; // no echo / out of range

  return duration * 0.0343 / 2; // convert to cm
}

void setup()
{
  Serial.begin(115200);
  delay(2500);
  Serial.print("Booting ultrasonic presence LED controller...");

  pinMode(Ultrasonic_Trigger_Pin, OUTPUT);
  pinMode(Ultrasonic_Echo_Pin, INPUT);

  FastLED.addLeds<WS2812B, LED_Data_Pin, LED_Color_Order>(LED_Strip, LED_Count);
  FastLED.setBrightness(LED_Max_Brightness);
  FastLED.clear();
  FastLED.show();

  lastFrameTime = millis();
}

void loop()
{
  // --- Check distance periodically ---
  if (millis() - lastPing >= pingInterval)
  {
    lastPing = millis();
    long current_distance = readDistanceCM();

    bool presenceNow = (current_distance > 0 && current_distance < Trigger_Distance);

    if (presenceNow)
    {
      lastDetectionTime = millis();
      lightsOn = true;
    }
    else if (millis() - lastDetectionTime > LED_Hold_Time)
    {
      lightsOn = false;
    }

    Serial.print("Distance: ");
    Serial.print(current_distance);
    Serial.print(" cm | Lights: ");
    Serial.println(lightsOn ? "ON" : "OFF");
  }

  // --- Advance the wipe wave toward its target ---
  unsigned long now = millis();
  float deltaTime = now - lastFrameTime;
  lastFrameTime = now;

  float wipeTarget = lightsOn ? (LED_Count + wipeFadeWidth) : 0.0;

  if (wipeProgress < wipeTarget)
  {
    wipeProgress += wipeSpeed * deltaTime;
    if (wipeProgress > wipeTarget)
      wipeProgress = wipeTarget;
  }
  else if (wipeProgress > wipeTarget)
  {
    wipeProgress -= wipeSpeed * deltaTime;
    if (wipeProgress < wipeTarget)
      wipeProgress = wipeTarget;
  }

  // --- Render each LED based on wave position ---
  for (int i = 0; i < LED_Count; i++)
  {
    float localPosition = wipeProgress - i;
    uint8_t brightness = (uint8_t)constrain(
        (localPosition / wipeFadeWidth) * 255.0, 0, 255);

    LED_Strip[i] = LED_Base_Color;
    LED_Strip[i].nscale8_video(brightness);
  }

  FastLED.show();
}