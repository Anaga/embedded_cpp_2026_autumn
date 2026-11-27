// Small text helpers, used by the alarms and the bot.
#pragma once
#include <Arduino.h>
#include <time.h>

String clockText();             // " at 14:03", or "" before NTP
String clockAt(time_t t);       // "21:40", or "" before NTP
String agoText(uint32_t s);     // "12 s ago", "4 min ago", "2 h 5 min ago"
String durationText(uint32_t ms);  // "45 s", "3 min 20 s"
String html(String s);          // escape < > & for Telegram HTML
String ascii(const String& s);  // for Serial: one '?' per non-ASCII character

// Lead-acid state of charge from the resting voltage (rough, but what a
// rider needs): 12.7 V = full, 12.2 V = half, 11.6 V = flat.
uint8_t batteryPercent(float v);
String batteryBar(uint8_t percent);  // 10 segments, filled up to percent
