// The emoji Telegram shows, written as their UTF-8 bytes.
//
// Why not just type them? The course rule is ASCII-only source files:
// different editors, terminals and toolchains mangle non-ASCII characters.
// Telegram still gets the very same bytes, so the phone shows the emoji.
//
// These are #defines (not constexpr) on purpose: two string literals next
// to each other are joined by the compiler, so  EMO_BELL " Unmute"  is one
// constant string, usable for the button texts.
//
// Serial output never uses these: Serial stays plain ASCII.
#pragma once

#define EMO_OK "\xE2\x9C\x85"          // white heavy check mark
#define EMO_SIREN "\xF0\x9F\x9A\xA8"   // police car light: movement
#define EMO_DROP "\xF0\x9F\x92\xA7"    // droplet: humidity, condensation
#define EMO_YELLOW "\xF0\x9F\x9F\xA1"  // yellow circle: a system fault
#define EMO_RED "\xF0\x9F\x94\xB4"     // red circle: the battery
#define EMO_WAIT "\xE2\x8F\xB3"        // hourglass
#define EMO_BATTERY "\xF0\x9F\x94\x8B"
#define EMO_ZAP "\xE2\x9A\xA1"         // high voltage: charging
#define EMO_THERMO "\xF0\x9F\x8C\xA1"
#define EMO_SIGNAL "\xF0\x9F\x93\xA1"  // satellite antenna
#define EMO_NO_SIGNAL "\xF0\x9F\x93\xB4"  // mobile phone off
#define EMO_ONLINE "\xF0\x9F\x93\xB6"  // antenna with bars
#define EMO_MUTED "\xF0\x9F\x94\x95"   // bell with a stroke
#define EMO_BELL "\xF0\x9F\x94\x94"
#define EMO_PIN "\xF0\x9F\x93\x8D"     // round pushpin
#define EMO_CHART "\xF0\x9F\x93\x88"
#define EMO_BIKE "\xF0\x9F\x8F\x8D"    // racing motorcycle

#define TXT_DEG "\xC2\xB0"             // degree sign
#define TXT_DOT "\xC2\xB7"             // middle dot, between the small-print items
#define TXT_BAR_FULL "\xE2\x96\xB0"    // filled segment of the battery bar
#define TXT_BAR_EMPTY "\xE2\x96\xB1"   // empty segment
