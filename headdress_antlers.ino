#include "FastLED.h"

// Hardware definitions
#define DATA_PIN    1
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    66

CRGB leds[NUM_LEDS];

// Software definitions
#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  60
#define HUE_CHANGE_MILLISECONDS 20
#define PATTERN_CHANGE_SECONDS 60

//For fire
#define COOLING  55
#define SPARKING 120

CRGBPalette16 currentPalette(PartyColors_p);
uint8_t       colorLoop = 1;

void setup() {
    delay(1000); // 1 second delay for recovery

    FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {sinelon, Fire2012, sinelon, rainbow, rainbowWithGlitter, glitterOnlyWhite, glitterOnlyRainbow, sinelon, confetti, sinelon, juggle, sinelon, bpm};

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void loop()
{
    // Call the current pattern function once, updating the 'leds' array
    gPatterns[gCurrentPatternNumber]();

    // send the 'leds' array out to the actual LED strip
    FastLED.show();
    // insert a delay to keep the framerate modest
    FastLED.delay(1000/FRAMES_PER_SECOND);

    // do some periodic updates
    EVERY_N_MILLISECONDS(HUE_CHANGE_MILLISECONDS) { gHue++; } // slowly cycle the "base color" through the rainbow
    EVERY_N_SECONDS(PATTERN_CHANGE_SECONDS) {nextPattern();} // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
    // add one to the current pattern number, and wrap around at the end
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow()
{
    // FastLED's built-in rainbow generator
    fill_rainbow(leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    addGlitter(80);
}

void addGlitter(fract8 chanceOfGlitter)
{
    if(random8() < chanceOfGlitter) {
        leds[random16(NUM_LEDS)] += CRGB::White;
    }
}

void glitterOnlyWhite() {
    leds[random16(NUM_LEDS)] += CRGB::White
    fadeToBlackBy(leds, NUM_LEDS, 50);
    delay(15);
}

void glitterOnlyRainbow() {
    CRGB onePix[1];
    fill_rainbow (onePix, 1, gHue, 7);
    leds[random16(NUM_LEDS)] = onePix[0];
    fadeToBlackBy(leds, NUM_LEDS, 50);
    delay(15);
}

void confetti()
{
    // random colored speckles that blink in and fade smoothly
    fadeToBlackBy(leds, NUM_LEDS, 10);
    int pos = random16(NUM_LEDS);
    leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

void sinelon()
{
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy(leds, NUM_LEDS, 20);
    int pos = beatsin16(13, 0, NUM_LEDS);
    leds[pos] += CHSV(gHue, 255, 192);
}

void bpm()
{
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t beatsPerMinute = 62;
    uint8_t beat = beatsin8(beatsPerMinute, 64, 255);
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette(currentPalette, gHue + (i * 2), beat - gHue + (i * 10));
    }
}

void juggle() {
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy(leds, NUM_LEDS, 20);
    byte dothue = 0;
    for(uint8_t i = 0; i < 8; i++) {
        leds[beatsin16(i + 7, 0, NUM_LEDS)] |= CHSV(dothue, 200, 255);
        dothue += 32;
    }
}

void Fire2012()
{
    // Array of temperature readings at each simulation cell
    static byte heat[NUM_LEDS];

    // Step 1.  Cool down every cell a little
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        heat[i] = qsub8(heat[i], random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (uint8_t k = NUM_LEDS - 1; k >= 2; k--) {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random8() < SPARKING) {
        int yFire = random8(7);
        heat[yFire] = qadd8(heat[yFire], random8(160, 255));
    }

    // Step 4.  Map from heat cells to LED colors
    for (int j = 0; j < NUM_LEDS; j++) {
        leds[j] = HeatColor(heat[j]);
    }
}
