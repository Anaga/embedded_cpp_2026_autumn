// Copy this file to secrets.h (same folder) and fill it in.
// secrets.h is listed in .gitignore and must never be committed.
//
// WiFi passwords are NOT here any more: they live in NVS on the board.
// Type in the serial monitor:   wifi add "<ssid>" <password>
//
// The bot token is a password: whoever has it IS your bot.
// If it ever lands in git, send /revoke to @BotFather and use the new one.
#pragma once

// From @BotFather after /newbot, looks like "123456789:AAH...".
constexpr const char *BOT_TOKEN = "123456789:replace-me";
