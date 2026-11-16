// Lesson 22 demo - one picture, three chips
//
// WS2812B 8x8 panel on ESP32-C3, Raspberry Pi Pico (RP2040) or Pico 2 (RP2350).
// The same source builds for every board. LED_DATA_PIN comes from
// platformio.ini. Adafruit NeoPixel drives the panel with RMT on the ESP32-C3
// and with PIO on the RP2040 / RP2350: same API, different hardware underneath.
//
// Five phases repeat forever:
//   ORDER     pixel 0 red, pixel 1 green, pixel 7 blue, pixel 8 white
//   CHANNELS  whole panel red, then green, then blue
//   WALK      one white dot walks left -> right on every row through xy()
//   PICTURE   kPicture below - change it to your own picture
//   RAINBOW   diagonal rainbow
//
// Once per second Serial shows the min / max time spent inside strip.show().

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#ifndef LED_DATA_PIN
#error "LED_DATA_PIN not set: add -D LED_DATA_PIN=<pin> to build_flags in platformio.ini"
#endif

#if defined(ARDUINO_ARCH_ESP32)
static const char* const kPlatform = "ESP32-C3 (RMT)";
#elif defined(ARDUINO_ARCH_RP2040) && defined(PICO_RP2350) && PICO_RP2350
static const char* const kPlatform = "RP2350 (PIO)";
#elif defined(ARDUINO_ARCH_RP2040)
static const char* const kPlatform = "RP2040 (PIO)";
#else
static const char* const kPlatform = "unknown";
#endif

// ---- Panel ----------------------------------------------------------------
// Panel held with the connector on the right: pixel 0 is the top-right corner,
// row 0 runs right -> left, row 1 left -> right, and so on (serpentine).
static constexpr uint8_t  kWidth           = 8U;
static constexpr uint8_t  kHeight          = 8U;
static constexpr uint16_t kNumPixels       = kWidth * kHeight;
static constexpr uint8_t  kBrightness      = 20U;   // of 255; about 0.3 A on full white
static constexpr bool     kSerpentine      = true;
static constexpr bool     kRow0RightToLeft = true;

// ---- Your picture ---------------------------------------------------------
// One string per row, top row first, 8 characters each.
// . off   R red   G green   B blue   Y yellow   C cyan   M magenta   W white
static const char* const kPicture[kHeight] = {
    "..YYYY..",
    ".Y....Y.",
    "Y.B..B.Y",
    "Y......Y",
    "Y.R..R.Y",
    "Y..RR..Y",
    ".Y....Y.",
    "..YYYY..",
};

// ---- Timing ---------------------------------------------------------------
static constexpr uint32_t kStartupDelayMs = 1500U;  // let USB CDC come up
static constexpr uint32_t kFrameMs        = 20U;    // 50 frames per second
static constexpr uint32_t kReportMs       = 1000U;
static constexpr uint32_t kWalkStepMs     = 60U;
static constexpr uint32_t kChannelMs      = 1000U;
static constexpr uint16_t kHueStepXY      = 4096U;  // hue change per pixel along the diagonal
static constexpr uint16_t kHuePerMs       = 16U;    // hue change per millisecond

enum class Phase : uint8_t { Order, Channels, Walk, Picture, Rainbow };

struct PhaseInfo {
    Phase       phase;
    const char* name;
    const char* expect;
    uint32_t    durationMs;
};

static const PhaseInfo kPhases[] = {
    { Phase::Order,    "ORDER",    "px0 red, px1 green, px7 blue, px8 white", 4000U },
    { Phase::Channels, "CHANNELS", "all red -> all green -> all blue",        3U * kChannelMs },
    { Phase::Walk,     "WALK",     "dot runs left -> right on every row",     kNumPixels * kWalkStepMs },
    { Phase::Picture,  "PICTURE",  "kPicture, the right way up",              6000U },
    { Phase::Rainbow,  "RAINBOW",  "diagonal rainbow moving",                 6000U },
};
static constexpr uint8_t kPhaseCount = sizeof(kPhases) / sizeof(kPhases[0]);

static Adafruit_NeoPixel strip(kNumPixels, LED_DATA_PIN, NEO_GRB + NEO_KHZ800);

static uint8_t  gPhaseIdx     = 0U;
static uint32_t gPhaseStartMs = 0U;
static uint32_t gLastFrameMs  = 0U;
static uint32_t gLastReportMs = 0U;
static uint32_t gShowMinUs    = UINT32_MAX;
static uint32_t gShowMaxUs    = 0U;

// Panel coordinates -> index in the chain. The one place that knows the wiring.
// (0, 0) is the top-left pixel as the viewer sees it.
static uint16_t xy(uint8_t x, uint8_t y) {
    const bool oddRow = (y & 1U) != 0U;
    bool reversed = kRow0RightToLeft;
    if (kSerpentine && oddRow) {
        reversed = !reversed;
    }
    const uint8_t col = reversed ? static_cast<uint8_t>(kWidth - 1U - x) : x;
    return static_cast<uint16_t>(y * kWidth + col);
}

static uint32_t colourFor(char c) {
    switch (c) {
        case 'R': return strip.Color(255U, 0U, 0U);
        case 'G': return strip.Color(0U, 255U, 0U);
        case 'B': return strip.Color(0U, 0U, 255U);
        case 'Y': return strip.Color(255U, 160U, 0U);
        case 'C': return strip.Color(0U, 255U, 255U);
        case 'M': return strip.Color(255U, 0U, 255U);
        case 'W': return strip.Color(255U, 255U, 255U);
        default:  return 0U;   // '.' and anything unknown: off
    }
}

static void drawOrder() {
    strip.clear();
    strip.setPixelColor(0U, strip.Color(255U, 0U, 0U));
    strip.setPixelColor(1U, strip.Color(0U, 255U, 0U));
    strip.setPixelColor(7U, strip.Color(0U, 0U, 255U));
    strip.setPixelColor(8U, strip.Color(255U, 255U, 255U));
}

static void drawChannels(uint32_t elapsedMs) {
    const uint32_t step = elapsedMs / kChannelMs;
    uint32_t colour = strip.Color(0U, 0U, 255U);
    if (step == 0U) {
        colour = strip.Color(255U, 0U, 0U);
    } else if (step == 1U) {
        colour = strip.Color(0U, 255U, 0U);
    }
    strip.fill(colour);
}

static void drawWalk(uint32_t elapsedMs) {
    const uint16_t step = static_cast<uint16_t>((elapsedMs / kWalkStepMs) % kNumPixels);
    const uint8_t x = static_cast<uint8_t>(step % kWidth);
    const uint8_t y = static_cast<uint8_t>(step / kWidth);
    strip.clear();
    strip.setPixelColor(xy(x, y), strip.Color(255U, 255U, 255U));
}

static void drawPicture() {
    for (uint8_t y = 0U; y < kHeight; ++y) {
        for (uint8_t x = 0U; x < kWidth; ++x) {
            strip.setPixelColor(xy(x, y), colourFor(kPicture[y][x]));
        }
    }
}

static void drawRainbow(uint32_t elapsedMs) {
    const uint16_t base = static_cast<uint16_t>(elapsedMs * kHuePerMs);
    for (uint8_t y = 0U; y < kHeight; ++y) {
        for (uint8_t x = 0U; x < kWidth; ++x) {
            const uint16_t hue = static_cast<uint16_t>(base + (x + y) * kHueStepXY);
            strip.setPixelColor(xy(x, y), strip.gamma32(strip.ColorHSV(hue)));
        }
    }
}

static void announcePhase() {
    const PhaseInfo& p = kPhases[gPhaseIdx];
    Serial.printf("Phase %s: expect %s\n", p.name, p.expect);
}

void setup() {
    Serial.begin(115200);
    delay(kStartupDelayMs);

    strip.begin();
    strip.setBrightness(kBrightness);
    strip.clear();
    strip.show();

    Serial.printf("Lesson 22 demo on %s, data pin %d, %u pixels, brightness %u\n",
                  kPlatform, LED_DATA_PIN,
                  static_cast<unsigned>(kNumPixels), static_cast<unsigned>(kBrightness));
    gPhaseStartMs = millis();
    announcePhase();
}

void loop() {
    const uint32_t now = millis();

    if (now - gPhaseStartMs >= kPhases[gPhaseIdx].durationMs) {
        gPhaseIdx = static_cast<uint8_t>((gPhaseIdx + 1U) % kPhaseCount);
        gPhaseStartMs = now;
        announcePhase();
    }

    if (now - gLastFrameMs >= kFrameMs) {
        gLastFrameMs = now;
        const uint32_t elapsed = now - gPhaseStartMs;

        switch (kPhases[gPhaseIdx].phase) {
            case Phase::Order:    drawOrder();            break;
            case Phase::Channels: drawChannels(elapsed);  break;
            case Phase::Walk:     drawWalk(elapsed);      break;
            case Phase::Picture:  drawPicture();          break;
            case Phase::Rainbow:  drawRainbow(elapsed);   break;
        }

        const uint32_t t0 = micros();
        strip.show();
        const uint32_t dt = micros() - t0;
        if (dt < gShowMinUs) { gShowMinUs = dt; }
        if (dt > gShowMaxUs) { gShowMaxUs = dt; }
    }

    if (now - gLastReportMs >= kReportMs) {
        gLastReportMs = now;
        Serial.printf("[%s] show(): min %lu us, max %lu us\n",
                      kPhases[gPhaseIdx].name,
                      static_cast<unsigned long>(gShowMinUs),
                      static_cast<unsigned long>(gShowMaxUs));
        gShowMinUs = UINT32_MAX;
        gShowMaxUs = 0U;
    }
}
