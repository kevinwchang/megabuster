#include <Pushbutton.h>

/* LedStripRainbow: Example Arduino sketch that shows
 * how to make a moving rainbow pattern on an
 * Addressable RGB LED Strip from Pololu.
 *
 * To use this, you will need to plug an Addressable RGB LED
 * strip from Pololu into pin 12.  After uploading the sketch,
 * you should see a moving rainbow.
 */
 
Pushbutton button(A0);
 
#include <PololuLedStrip.h>

// Create an ledStrip object and specify the pin it will use.
PololuLedStrip<12> ledStrip;

// Create a buffer for holding the colors (3 bytes per color).
#define LED_COUNT 18
rgb_color colors[LED_COUNT];

#define SOMO_CLK  10
#define SOMO_DATA 11

#define SOUND_SHOT 1
#define SOUND_CHARGE 2
#define SOUND_RELEASE 3

void somoInit()
{
    pinMode(SOMO_CLK, OUTPUT);
    digitalWrite(SOMO_CLK, HIGH);
    pinMode(SOMO_DATA, OUTPUT);
    digitalWrite(SOMO_DATA, HIGH);
    delay(300);
}

void somoCmd(uint16_t cmd)
{   
    digitalWrite(SOMO_CLK, LOW);
    delay(2);
    
    for (int8_t b = 15; b >= 0; b--)
    {
        digitalWrite(SOMO_CLK, LOW);
        digitalWrite(SOMO_DATA, (cmd >> b) & 1);
        delayMicroseconds(90);
        digitalWrite(SOMO_CLK, HIGH);
        delayMicroseconds(90);
    }
    delay(2);
}

void setup()
{
  somoInit();
}

#define CHARGE_THRESHOLD 200
#define CHARGE_HOLD_MAX 15000

void loop()
{
  static boolean pressed = false, charging = false;
  static unsigned int startTime = 0;
  
  /* Update the colors.
  uint16_t time = (millis() >> 7) % 9 ;
  for(byte i = 0; i < LED_COUNT; i++)
  {
    int x = 1 << time;
    if (x == 256) x = 255;
    colors[i] = (rgb_color){255, 0, 0};
  }
  colors[0] = (rgb_color){255, 255, 0};
  colors[1] = (rgb_color){255, 255, 0};
  
  // Write the colors to the LED strip.
  ledStrip.write(colors, LED_COUNT);  
  
  delay(10);
  */
  
  if (button.getSingleDebouncedPress())
  {
    pressed = true;
    startTime = millis();
  }
  
  if (pressed && !charging && (unsigned int)(millis() - startTime) >= CHARGE_THRESHOLD)
  {
    somoCmd(SOUND_CHARGE);
    charging = true;
    startTime = millis();
  }

  if (button.getSingleDebouncedRelease())
  {
    if (charging)
    {
      somoCmd(SOUND_RELEASE);
      charging = false;
    }
    else
    {
      somoCmd(SOUND_SHOT);
    }
    pressed = false;
  }
}
