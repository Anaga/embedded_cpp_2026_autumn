// Copy this file to secrets.h (same folder) and fill it in.
// secrets.h is listed in .gitignore and must never be committed.
//
// The bot token is a password: whoever has it IS your bot.
// If it ever lands in git, send /revoke to @BotFather and use the new one.
#pragma once

constexpr const char *WIFI_SSID = "your-ssid";
constexpr const char *WIFI_PASS = "your-password";

// From @BotFather after /newbot, looks like "123456789:AAH...".
constexpr const char *BOT_TOKEN = "123456789:replace-me";

// Optional: the chat that gets alerts right after a reboot.
// Leave it empty and send /start to the bot instead - the board prints
// your chat id on Serial, so you can paste it here later.
constexpr const char *ALERT_CHAT_ID = "";
