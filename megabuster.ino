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
#define LED_COUNT 22
rgb_color colors[LED_COUNT];

#define SOMO_CLK  10
#define SOMO_DATA 11

#define SOUND_SHOT 1
#define SOUND_CHARGE 2
#define SOUND_RELEASE 3

unsigned long start_time = 0;

#define LEDS_IDLE 0
#define LEDS_SHOT 1
#define LEDS_CHARGING 2
#define LEDS_CHARGED_SHOT 3

byte led_state = LEDS_IDLE;

void clearLeds()
{
  for(byte i = 0; i < LED_COUNT; i++)
  {
    colors[i] = (rgb_color){0, 0, 0};
  }
  
  // Write the colors to the LED strip.
  ledStrip.write(colors, LED_COUNT);  
  ledStrip.write(colors, LED_COUNT);  
}

void setMuzzle(byte brightness)
{
  for(byte i = 6; i < 22; i++)
  {
    colors[i] = (rgb_color){brightness, 0, 0};
  }
  
  // Write the colors to the LED strip.
  ledStrip.write(colors, LED_COUNT); 
}

#define PM_MAX 63

int power_level = 0;

void setPowerMeter(int level) // max PM_MAX * 6
{
  if (level > PM_MAX * 6)
    level = PM_MAX * 6;
    
  power_level = level;
  
  for(byte i = 0; i < 6; i++)
  {
    int b = level - (i * PM_MAX);
    if (b <= 0)
      colors[i] = (rgb_color){0, 0, 0};
    else if (b >= PM_MAX)
      colors[i] = (rgb_color){PM_MAX, PM_MAX, 0};
    else
      colors[i] = (rgb_color){b, b, 0};
  }
  
  // Write the colors to the LED strip.
  ledStrip.write(colors, LED_COUNT);
}

void decPowerMeter(int amt)
{
  if (power_level > amt)
    power_level -= amt;
  else
    power_level = 0;
  setPowerMeter(power_level);
}

void setPowerBrightness(int brightness)
{
  for(byte i = 0; i < 6; i++)
  {
      colors[i] = (rgb_color){brightness, brightness, 0};
  }
  
  // Write the colors to the LED strip.
  ledStrip.write(colors, LED_COUNT);
}

void updateLeds()
{
  long elapsed = millis() - start_time;
  
  switch (led_state)
  {
    case LEDS_SHOT:
      elapsed -= 50;
      if (elapsed < 0)
        elapsed = 0;
      else
      {
        if (elapsed > 252)
          led_state = LEDS_IDLE;
        else
          setMuzzle(63 - (elapsed / 4)); 
      }
      break;
      
    case LEDS_CHARGING:
      elapsed -= 100;
      if (elapsed < 0)
        elapsed = 0;
        
      if (elapsed < PM_MAX * 18)
      {
        setPowerMeter(elapsed / 3);
        setMuzzle(elapsed / 36);
      }
      else
      {
        int b = PM_MAX;
        int fade_count = (elapsed - PM_MAX * 18) % 1000;
        if (fade_count < 500)
          b -= fade_count / 10;
        else
          b -= (1000 - fade_count) / 10;
        setPowerBrightness(b);
        setMuzzle(31);
      }
      break;
    
    case LEDS_CHARGED_SHOT:
      
      elapsed -= 100;
      if (elapsed < 0)
        elapsed = 0;
      else
      {
        decPowerMeter(3);
        if (elapsed > 255)
          led_state = LEDS_IDLE;
        else
          setMuzzle(255 - elapsed); 
      }
      break;
  }
}

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
  clearLeds();
  somoInit();
}

#define CHARGE_THRESHOLD 200
#define CHARGE_HOLD_MAX 15000

void loop()
{
  static boolean pressed = false, charging = false;
  
  if (button.getSingleDebouncedPress())
  {
    pressed = true;
    start_time = millis();
  }
  
  if (pressed && !charging && (millis() - start_time) >= CHARGE_THRESHOLD)
  {
    somoCmd(SOUND_CHARGE);
    led_state = LEDS_CHARGING;
    charging = true;
    start_time = millis();
  }

  if (button.getSingleDebouncedRelease())
  {
    if (charging)
    {
      somoCmd(SOUND_RELEASE);
      led_state = LEDS_CHARGED_SHOT;
      charging = false;
    }
    else
    {
      somoCmd(SOUND_SHOT);
      led_state = LEDS_SHOT;
    }
    start_time = millis();
    pressed = false;
  }
  
  updateLeds();
}
