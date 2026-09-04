#include <FastLED.h>

#define LED_Data_Pin 5
#define LED_Count 100
#define LED_Color_Order GRB
#define LED_Midpoint 50
#define LED_Max_Brightness 200

#define Ultrasonic_Trigger_Pin 18
#define Ultrasonic_Echo_Pin 17

#define Trigger_Distance 100 // The distance within the LED's will trigger when motion is detected.
#define LED_Hold_Time 5000 // The time the LED's will stay on after being triggered in Milliseconds

CRGB LED_Strip[LED_Count];

bool lightsOn = false;
uint8_t currentBrightness = 0;
unsigned long lastFadeStep = 0;
const int fadeStepDelay = 8; // ms between brightness steps

unsigned long lastPing = 0;
const int pingInterval = 100; // ms between ultrasonic checks

unsigned long lastDetectionTime = 0;

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
  fill_gradient_RGB(LED_Strip,LED_Count,CRGB::White,CRGB::Red);
  //fill_solid(LED_Strip, LED_Count, CRGB::White); // set your desired "on" color here
  FastLED.setBrightness(0);
  FastLED.show();
}

void loop()
{
  // --- Check distance periodically ---
  if (millis() - lastPing >= pingInterval)
  {
    lastPing = millis();
    long dist = readDistanceCM();

    bool presenceNow = (dist > 0 && dist < Trigger_Distance);

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
    Serial.print(dist);
    Serial.print(" cm | Lights: ");
    Serial.println(lightsOn ? "ON" : "OFF");
  }

  // --- Non-blocking fade toward target brightness ---
  if (millis() - lastFadeStep >= fadeStepDelay)
  {
    lastFadeStep = millis();
    uint8_t target = lightsOn ? LED_Max_Brightness : 0;

    if (currentBrightness < target)
      currentBrightness++;
    else if (currentBrightness > target)
      currentBrightness--;

    FastLED.setBrightness(currentBrightness);
    FastLED.show();
  }
}