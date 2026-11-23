#include <Arduino.h>
#include <AudioTools.h>
#include <BluetoothA2DPSource.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "Playback.h"
#include "Synthesizer.h"
#include "ThereminController.h"
#include "UltrasonicDemo.h"

#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <cstring>

#if defined(ARDUINO) && !defined(CONFIG_IDF_TARGET_ESP32)
#error "Sounder requires an original ESP32 with Bluetooth Classic."
#endif

namespace {

constexpr char kSpeakerName[] = "JBL Flip 4";
constexpr size_t kCommandCapacity = 8;

class SpeakerSource : public BluetoothA2DPSource {
public:
    void enableReconnect() {
        // In 1.8.11 a fresh device with no cached address disables this flag.
        // Re-arm it after discovery AND after a retry-to-discovery fallback.
        is_autoreconnect_allowed = true;
        set_auto_reconnect(true, 3);
    }
};

using PlaybackMode = sounder::DemoEngine::Mode;
using ThereminMode = sounder::ThereminController::Mode;

enum class CommandKind { Tone, Pattern, Tour, Showcase };
struct Command {
    CommandKind kind = CommandKind::Tour;
    float hz = 440.0f;
    uint32_t durationMs = 0;
    size_t pattern = 0;
    uint32_t generation = 0;
};

struct SharedState {
    esp_a2d_connection_state_t connection = ESP_A2D_CONNECTION_STATE_DISCONNECTED;
    esp_a2d_audio_state_t audio = ESP_A2D_AUDIO_STATE_STOPPED;
    bool discovering = false;
    float level = sounder::kDefaultLevel;
    sounder::SynthVoice voice = sounder::SynthVoice::Sine;
    uint32_t generation = 0;
    sounder::PlaybackStatus playback;
    ThereminMode theremin = ThereminMode::Off;
    sounder::Frame liveTarget{440.0f, 0.0f};
    uint32_t liveUpdatedAtMs = 0;
};

portMUX_TYPE stateMux = portMUX_INITIALIZER_UNLOCKED;
SharedState shared;
audio_tools::FastIntSineGenerator<int16_t> oscillator;
sounder::Synthesizer synthesizer(oscillator, sounder::kSampleRate);
sounder::Playback playback(synthesizer);
SpeakerSource source;
sounder::UltrasonicDemo sensors;
sounder::ThereminController theremin;
QueueHandle_t commands = nullptr;
uint32_t renderedGeneration = 0;
bool audioReady = false;
bool greetingShown = false;
SharedState reported;
bool progressReported = false;
char input[80];
size_t inputLength = 0;
bool inputOverflow = false;

SharedState readState() {
    portENTER_CRITICAL(&stateMux);
    const SharedState result = shared;
    portEXIT_CRITICAL(&stateMux);
    return result;
}

void requestStop() {
    portENTER_CRITICAL(&stateMux);
    // Invalidates queued commands too. Stop never has to wait for queue space.
    ++shared.generation;
    shared.playback = {};
    shared.theremin = ThereminMode::Off;
    shared.liveTarget = {440.0f, 0.0f};
    portEXIT_CRITICAL(&stateMux);
}

void onConnection(esp_a2d_connection_state_t connection, void*) {
    portENTER_CRITICAL(&stateMux);
    if (connection != shared.connection) {
        ++shared.generation;
        shared.playback = {};
        shared.theremin = ThereminMode::Off;
    }
    shared.connection = connection;
    if (connection != ESP_A2D_CONNECTION_STATE_CONNECTED) {
        shared.audio = ESP_A2D_AUDIO_STATE_STOPPED;
    }
    portEXIT_CRITICAL(&stateMux);
    if (connection == ESP_A2D_CONNECTION_STATE_CONNECTED) {
        source.enableReconnect();
    }
}

void onAudio(esp_a2d_audio_state_t audio, void*) {
    portENTER_CRITICAL(&stateMux);
    shared.audio = audio;
    // Do not resume an old note after a remote pause or stream restart.
    if (audio != ESP_A2D_AUDIO_STATE_STARTED) {
        ++shared.generation;
        shared.playback = {};
        shared.theremin = ThereminMode::Off;
    }
    portEXIT_CRITICAL(&stateMux);
}

void onDiscovery(esp_bt_gap_discovery_state_t state) {
    portENTER_CRITICAL(&stateMux);
    shared.discovering = state == ESP_BT_GAP_DISCOVERY_STARTED;
    portEXIT_CRITICAL(&stateMux);
}

bool selectSpeaker(const char* name, esp_bd_addr_t, int) {
    return name != nullptr && std::strcmp(name, kSpeakerName) == 0;
}

int32_t renderAudio(::Frame* frames, int32_t count) {
    if (frames == nullptr || count <= 0) {
        return 0;
    }
    auto control = readState();
    bool connected = control.connection == ESP_A2D_CONNECTION_STATE_CONNECTED;
    if (renderedGeneration != control.generation || !connected) {
        playback.stop();
        renderedGeneration = control.generation;
    }
    playback.setLevel(control.level);
    Command command;
    // Bounded, nonblocking work, even during a large serial paste.
    for (size_t i = 0; i < kCommandCapacity && commands != nullptr &&
                       xQueueReceive(commands, &command, 0) == pdTRUE; ++i) {
        // A stop and a replacement command can arrive during this callback.
        // Refresh before consuming the replacement, rather than discarding it
        // against the generation captured at the beginning of the buffer.
        control = readState();
        connected = control.connection == ESP_A2D_CONNECTION_STATE_CONNECTED;
        if (renderedGeneration != control.generation || !connected) {
            playback.stop();
            renderedGeneration = control.generation;
        }
        playback.setLevel(control.level);
        if (!connected || command.generation != control.generation) {
            continue;
        }
        // A regular playback command takes ownership from the live controls.
        portENTER_CRITICAL(&stateMux);
        if (shared.generation == command.generation) shared.theremin = ThereminMode::Off;
        portEXIT_CRITICAL(&stateMux);
        control.theremin = ThereminMode::Off;
        switch (command.kind) {
            case CommandKind::Tone: playback.startTone(command.hz, command.durationMs); break;
            case CommandKind::Pattern: playback.startPattern(command.pattern); break;
            case CommandKind::Tour: playback.startTour(); break;
            case CommandKind::Showcase: playback.startShowcase(); break;
        }
    }
    if (connected && control.theremin != ThereminMode::Off) {
        auto target = control.liveTarget;
        // Also fade out if the main task stops publishing controls entirely.
        if (millis() - control.liveUpdatedAtMs >= sounder::ThereminController::kHoldMs) target.drive = 0.0f;
        playback.setLiveTarget(target);
    }
    playback.setVoice(control.voice);
    for (int32_t i = 0; i < count; ++i) {
        const int16_t sample = playback.nextSample();
        frames[i].channel1 = sample;
        frames[i].channel2 = sample;
    }
    portENTER_CRITICAL(&stateMux);
    // A concurrent stop/disconnect must not be overwritten by this old buffer.
    if (shared.generation == control.generation) {
        shared.playback = playback.status();
    }
    portEXIT_CRITICAL(&stateMux);
    return count;
}

void updateTheremin() {
    const auto state = readState();
    if (state.theremin == ThereminMode::Off) {
        theremin.stop();
        return;
    }
    const auto& pitch = sensors.pitchHandReading();
    const auto& volume = sensors.volumeHandReading();
    const uint32_t nowMs = millis();
    const auto target = theremin.update(nowMs,
        {pitch.valid, pitch.distanceCm, pitch.acceptedAtMs},
        {volume.valid, volume.distanceCm, volume.acceptedAtMs});
    portENTER_CRITICAL(&stateMux);
    // A concurrent stop/disconnect or replacement command wins over this update.
    if (shared.generation == state.generation && shared.theremin == state.theremin) {
        shared.liveTarget = target;
        shared.liveUpdatedAtMs = nowMs;
    }
    portEXIT_CRITICAL(&stateMux);
}

void startTheremin(ThereminMode mode) {
    const auto state = readState();
    if (!audioReady) {
        Serial.println("Audio initialization failed; restart the board.");
        return;
    }
    if (state.connection != ESP_A2D_CONNECTION_STATE_CONNECTED) {
        Serial.println("Speaker not connected; put it in pairing mode and check status.");
        return;
    }
    if (state.audio != ESP_A2D_AUDIO_STATE_STARTED) {
        Serial.println("Bluetooth audio is not streaming; resume the speaker and retry.");
        return;
    }
    using Selection = sounder::UltrasonicDemo::Selection;
    sensors.start(mode == ThereminMode::Pitch ? Selection::Pitch :
                  mode == ThereminMode::Volume ? Selection::Volume : Selection::Both,
                  sounder::UltrasonicDemo::Timing::Live);
    theremin.start(mode);
    portENTER_CRITICAL(&stateMux);
    const bool ready = shared.generation == state.generation &&
        shared.connection == ESP_A2D_CONNECTION_STATE_CONNECTED && shared.audio == ESP_A2D_AUDIO_STATE_STARTED;
    if (ready) {
        ++shared.generation; // Discard old queued tones/patterns before starting.
        shared.playback = {};
        shared.theremin = mode;
        shared.liveTarget = {440.0f, 0.0f};
        shared.liveUpdatedAtMs = millis();
    }
    portEXIT_CRITICAL(&stateMux);
    if (!ready) {
        theremin.stop();
        Serial.println("Connection changed; retry theremin.");
        return;
    }
    Serial.printf("Theremin %s started with vibrato (5 Hz, +/-50 cents). Hands 5..50 cm; pitch closer = higher, volume closer = louder. Use stop to finish.\n",
                  sounder::ThereminController::modeName(mode));
}

void showThereminStatus() {
    const auto state = readState();
    Serial.printf("Theremin %s | vibrato 5 Hz +/-50 cents | center %.1f Hz | hand level %.1f%% | master %.1f%% | hands 5..50 cm | hold %lu ms\n",
                  sounder::ThereminController::modeName(state.theremin), state.liveTarget.hz,
                  state.liveTarget.drive * 100.0f, state.level,
                  static_cast<unsigned long>(sounder::ThereminController::kHoldMs));
}

bool parseNumber(const char* text, float low, float high, float& value) {
    if (text == nullptr || *text == '\0') return false;
    char* end = nullptr;
    errno = 0;
    value = std::strtof(text, &end);
    return end != text && *end == '\0' && errno == 0 &&
           std::isfinite(value) && value >= low && value <= high;
}

const char* connectionName(esp_a2d_connection_state_t state) {
    switch (state) {
        case ESP_A2D_CONNECTION_STATE_DISCONNECTED: return "disconnected";
        case ESP_A2D_CONNECTION_STATE_CONNECTING: return "connecting";
        case ESP_A2D_CONNECTION_STATE_CONNECTED: return "connected";
        case ESP_A2D_CONNECTION_STATE_DISCONNECTING: return "disconnecting";
    }
    return "unknown";
}

const char* audioName(esp_a2d_audio_state_t state) {
    // Do not use the library's state-name table: IDF 4 and 5 use different values.
    switch (state) {
        case ESP_A2D_AUDIO_STATE_REMOTE_SUSPEND: return "suspended";
        case ESP_A2D_AUDIO_STATE_STOPPED: return "stopped";
        case ESP_A2D_AUDIO_STATE_STARTED: return "streaming";
    }
    return "unknown";
}

const char* synthName(sounder::SynthVoice voice) {
    for (const auto& option : sounder::kSynthOptions) {
        if (option.voice == voice) return option.name;
    }
    return "unknown";
}

const sounder::SynthOption* findSynthOption(const char* name) {
    for (const auto& option : sounder::kSynthOptions) {
        if (std::strcmp(option.name, name) == 0) return &option;
    }
    return nullptr;
}

void showHelp() {
    Serial.printf("\nSounder | Bluetooth audio | target %s\n", kSpeakerName);
    Serial.println("Put the speaker in pairing mode. Playback stays silent until commanded.");
    Serial.println("  demo             Complete tour once (~45 seconds)");
    Serial.println("  showcase         Play every demo on each synth voice");
    for (size_t i = 0; i < sounder::kPatternCount; ++i) {
        const auto& pattern = sounder::kPatterns[i];
        Serial.printf("  %u %-9s %s\n", static_cast<unsigned>(i + 1),
                      pattern.name, pattern.description);
    }
    Serial.println("  tone 440 [1500]  100..8000 Hz; optional duration 10..30000 ms");
    Serial.println("  note 69          MIDI note 45..119; 69 = 440 Hz");
    Serial.println("  synth [voice]    Set or show voice (sine, triangle, pulse, fm, organ)");
    Serial.println("  volume [0..100]  Set/query PCM amplitude; default 35%; 0 mutes");
    Serial.println("  level 0..100     Alias for setting volume");
    Serial.println("  stop / !         Silence and discard pending playback");
    Serial.println("  sensors on/off   Two-sensor distance demo; no speaker needed");
    Serial.println("  sensors pitch/volume/status/reset  Test one sensor or inspect/reset diagnostics");
    Serial.println("  theremin on/pitch/volume  Live vibrato with hand-controlled pitch/volume");
    Serial.println("  theremin off/status      Stop live audio or show its targets");
    Serial.println("  status / help    Connection, playback, or this menu");
    Serial.println("Audio format is 44100 Hz stereo, 16-bit. Bluetooth adds audible latency.");
}

void showStatus() {
    const auto state = readState();
    const auto& sound = state.playback;
    const char* mode = sound.live ? "theremin/vibrato" : sound.mode == PlaybackMode::Idle ? "idle" :
                       sound.mode == PlaybackMode::Tone ? "tone" :
                       sounder::kPatterns[sound.pattern].name;
    const bool showcase = sound.mode == PlaybackMode::Showcase;
    const char* prefix = showcase ? "showcase/" : sound.mode == PlaybackMode::Tour ? "tour/" : "";
    Serial.printf("%s | %s%s | audio %s | %s%s | synth %s | target %.1f Hz | level %.1f%% | drive %.1f%% | 44100 Hz/stereo/16-bit\n",
                  kSpeakerName, connectionName(state.connection),
                  state.discovering ? "/discovering" : "", audioName(state.audio),
                  prefix, mode, synthName(showcase ? sound.voice : state.voice),
                  sound.hz, state.level, sound.drive * state.level);
}

bool queuePlayback(Command command) {
    if (!audioReady) {
        Serial.println("Audio initialization failed; restart the board.");
        return false;
    }
    const auto state = readState();
    if (state.connection != ESP_A2D_CONNECTION_STATE_CONNECTED) {
        Serial.println("Speaker not connected; put it in pairing mode and check status.");
        return false;
    }
    command.generation = state.generation;
    if (xQueueSend(commands, &command, 0) != pdTRUE) {
        Serial.println("Command queue full; retry. Stop and ! still work.");
        return false;
    }
    return true;
}

void handleCommand(char* line) {
    for (char* p = line; *p; ++p) {
        *p = static_cast<char>(std::tolower(static_cast<unsigned char>(*p)));
    }
    char* saved = nullptr;
    char* name = strtok_r(line, " \t", &saved);
    if (name == nullptr) return;
    char* arg1 = strtok_r(nullptr, " \t", &saved);
    char* arg2 = strtok_r(nullptr, " \t", &saved);
    char* extra = strtok_r(nullptr, " \t", &saved);
    float value = 0.0f;
    float duration = 0.0f;
    Command command;
    if (std::strcmp(name, "theremin") == 0) {
        if (arg2 != nullptr) {
            Serial.println("Use theremin [on|pitch|volume|off|status].");
        } else if (arg1 == nullptr || std::strcmp(arg1, "status") == 0) {
            showThereminStatus();
        } else if (std::strcmp(arg1, "off") == 0) {
            requestStop();
            Serial.println("Theremin stopped; buffered Bluetooth audio may finish first.");
        } else if (std::strcmp(arg1, "on") == 0 || std::strcmp(arg1, "pitch") == 0 || std::strcmp(arg1, "volume") == 0) {
            startTheremin(std::strcmp(arg1, "pitch") == 0 ? ThereminMode::Pitch :
                          std::strcmp(arg1, "volume") == 0 ? ThereminMode::Volume : ThereminMode::Both);
        } else {
            Serial.println("Use theremin [on|pitch|volume|off|status].");
        }
        return;
    } else if (sensors.handleCommand(name, arg1, arg2)) {
        // Changing acquisition mode also ends live audio. Queries/reset do not.
        if (arg2 == nullptr && arg1 != nullptr &&
            (std::strcmp(arg1, "on") == 0 || std::strcmp(arg1, "demo") == 0 ||
             std::strcmp(arg1, "pitch") == 0 || std::strcmp(arg1, "volume") == 0 || std::strcmp(arg1, "off") == 0) &&
            readState().theremin != ThereminMode::Off) requestStop();
        return;
    } else if (std::strcmp(name, "stop") == 0 && arg1 == nullptr) {
        requestStop();
        Serial.println("Stopped; buffered Bluetooth audio may finish first.");
    } else if (std::strcmp(name, "help") == 0 && arg1 == nullptr) {
        showHelp();
    } else if (std::strcmp(name, "status") == 0 && arg1 == nullptr) {
        showStatus();
    } else if (std::strcmp(name, "synth") == 0 && arg2 == nullptr) {
        if (arg1 != nullptr) {
            const auto* option = findSynthOption(arg1);
            if (option == nullptr) {
                Serial.println("Invalid command or value. Type help for ranges.");
                return;
            }
            portENTER_CRITICAL(&stateMux);
            shared.voice = option->voice;
            portEXIT_CRITICAL(&stateMux);
        }
        Serial.printf("Synth %s.\n", synthName(readState().voice));
    } else if (std::strcmp(name, "volume") == 0 && arg1 == nullptr) {
        Serial.printf("PCM amplitude %.1f%%.\n", readState().level);
    } else if ((std::strcmp(name, "level") == 0 || std::strcmp(name, "volume") == 0) && arg2 == nullptr &&
               parseNumber(arg1, 0.0f, 100.0f, value)) {
        portENTER_CRITICAL(&stateMux);
        shared.level = value;
        portEXIT_CRITICAL(&stateMux);
        Serial.printf("PCM amplitude %.1f%%.\n", value);
    } else if (std::strcmp(name, "tone") == 0 && extra == nullptr &&
               parseNumber(arg1, sounder::kMinHz, sounder::kMaxHz, value) &&
               (arg2 == nullptr || parseNumber(arg2, 10.0f, 30000.0f, duration))) {
        command.kind = CommandKind::Tone;
        command.hz = value;
        command.durationMs = static_cast<uint32_t>(duration);
        if (queuePlayback(command)) Serial.println("Tone queued. Use status for rendered playback.");
    } else if (std::strcmp(name, "note") == 0 && arg2 == nullptr &&
               parseNumber(arg1, 45.0f, 119.0f, value) && value == std::floor(value)) {
        command.kind = CommandKind::Tone;
        command.hz = sounder::midiToHz(static_cast<unsigned>(value));
        if (queuePlayback(command)) Serial.println("Note queued. Use status for rendered playback.");
    } else if (std::strcmp(name, "demo") == 0 && arg1 == nullptr) {
        if (queuePlayback(command)) Serial.println("Tour queued.");
    } else if (std::strcmp(name, "showcase") == 0 && arg1 == nullptr) {
        command.kind = CommandKind::Showcase;
        if (queuePlayback(command)) Serial.println("Showcase queued.");
    } else if (arg1 == nullptr) {
        for (size_t i = 0; i < sounder::kPatternCount; ++i) {
            if (std::strcmp(name, sounder::kPatterns[i].name) == 0 ||
                (name[0] == static_cast<char>('1' + i) && name[1] == '\0')) {
                command.kind = CommandKind::Pattern;
                command.pattern = i;
                if (queuePlayback(command)) Serial.printf("Queued %s.\n", sounder::kPatterns[i].name);
                return;
            }
        }
        Serial.println("Unknown command. Type help.");
    } else {
        Serial.println("Invalid command or value. Type help for ranges.");
    }
}

void readCommands() {
    for (unsigned budget = 0; budget < 64 && Serial.available(); ++budget) {
        const char ch = static_cast<char>(Serial.read());
        if (ch == '!') {
            requestStop();
            inputLength = 0;
            inputOverflow = false;
            Serial.println("Stopped; buffered Bluetooth audio may finish first.");
        } else if (ch == '\r' || ch == '\n') {
            if (inputOverflow) {
                Serial.println("Line too long; ignored. Type help.");
            } else {
                input[inputLength] = '\0';
                handleCommand(input);
            }
            inputLength = 0;
            inputOverflow = false;
        } else if (ch == '\b' || ch == 127) {
            if (inputLength != 0) --inputLength;
        } else if (ch >= 32 && ch <= 126 && !inputOverflow) {
            if (inputLength < sizeof(input) - 1) input[inputLength++] = ch;
            else inputOverflow = true;
        }
    }
}

void reportProgress() {
    const auto state = readState();
    if (!progressReported || state.connection != reported.connection ||
        state.audio != reported.audio || state.discovering != reported.discovering) {
        Serial.printf("Bluetooth %s%s | audio %s.\n", connectionName(state.connection),
                      state.discovering ? "/discovering" : "", audioName(state.audio));
    }
    const auto& sound = state.playback;
    const auto& previous = reported.playback;
    const bool patternChanged = !progressReported || sound.mode != previous.mode || sound.pattern != previous.pattern;
    if (sound.mode == PlaybackMode::Showcase && (patternChanged || sound.tourIndex != previous.tourIndex)) {
        const auto section = sound.tourIndex * sounder::kPatternCount + sound.pattern + 1;
        Serial.printf("[showcase %u/%u] %s / %s\n", static_cast<unsigned>(section),
                      static_cast<unsigned>(sounder::kPatternCount * sounder::kSynthVoiceCount),
                      synthName(sound.voice), sounder::kPatterns[sound.pattern].name);
    } else if ((sound.mode == PlaybackMode::Pattern || sound.mode == PlaybackMode::Tour) && patternChanged) {
        Serial.printf("[%u/%u] %s\n", static_cast<unsigned>(sound.pattern + 1),
                      static_cast<unsigned>(sounder::kPatternCount), sounder::kPatterns[sound.pattern].name);
    } else if (sound.mode == PlaybackMode::Idle && previous.mode != PlaybackMode::Idle) {
        Serial.println("Finished; silent.");
    }
    reported = state;
    progressReported = true;
}

} // namespace

void setup() {
    Serial.begin(115200);
    commands = xQueueCreate(kCommandCapacity, sizeof(Command));
    if (commands == nullptr) return;
    audioReady = oscillator.begin(audio_tools::AudioInfo(sounder::kSampleRate, 1, 16), 440.0f);
    synthesizer.setAmplitude(0);
    if (!audioReady) return;
    source.set_local_name("Sounder");
    source.set_data_callback_in_frames(renderAudio);
    source.set_on_connection_state_changed(onConnection);
    source.set_on_audio_state_changed(onAudio);
    source.set_discovery_mode_callback(onDiscovery);
    source.set_ssid_callback(selectSpeaker);
    source.set_ssp_enabled(true);
    // The pinned library installs the GAP PIN with length zero, which is
    // valid for VARIABLE mode; its PIN-request callback replies with "0000".
    source.set_pin_code("0000", ESP_BT_PIN_TYPE_VARIABLE);
    // Discover the configured name at each boot, avoiding another sketch's
    // cached speaker address. enableReconnect() arms retries after connection.
    source.set_auto_reconnect(false);
    source.start();
}

void loop() {
    if (Serial && !greetingShown) {
        showHelp();
        if (!audioReady) Serial.println("Audio initialization failed; restart the board.");
        greetingShown = true;
    }
    readCommands();
    sensors.update();
    updateTheremin();
    if (Serial) reportProgress();
    delay(1);
}
