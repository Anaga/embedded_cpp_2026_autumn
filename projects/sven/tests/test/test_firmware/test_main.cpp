#include <gtest/gtest.h>

#include <algorithm>
#include <limits>
#include <new>

// Compile the production adapter exactly once, with test-only hardware headers.
#include "../../../src/main.cpp"

namespace {
using Mode = sounder::DemoEngine::Mode;
using Oscillator = audio_tools::FastIntSineGenerator<int16_t>;
using Synth = decltype(synthesizer);
using Player = decltype(playback);

const std::string kStop = "Stopped; buffered Bluetooth audio may finish first.\n";
const std::string kTone = "Tone queued. Use status for rendered playback.\n";
const std::string kNote = "Note queued. Use status for rendered playback.\n";
const std::string kOffline = "Speaker not connected; put it in pairing mode and check status.\n";
const std::string kInitFailed = "Audio initialization failed; restart the board.\n";
const std::string kFull = "Command queue full; retry. Stop and ! still work.\n";
const std::string kUnknown = "Unknown command. Type help.\n";
const std::string kInvalid = "Invalid command or value. Type help for ranges.\n";
const std::string kIdleStatus =
    "JBL Flip 4 | disconnected | audio stopped | idle | synth sine | target 0.0 Hz | level 35.0% | drive 0.0% | 44100 Hz/stereo/16-bit\n";
const std::string kHelp =
    "\nSounder | Bluetooth audio | target JBL Flip 4\n"
    "Put the speaker in pairing mode. Playback stays silent until commanded.\n"
    "  demo             Complete tour once (~45 seconds)\n"
    "  showcase         Play every demo on each synth voice\n"
    "  1 scale     C major: C4 to C5 and back\n"
    "  2 sweep     100 to 8000 Hz and back\n"
    "  3 levels    1000 Hz: amplitude steps 5, 10, 20, 35, 50, 75, 100%\n"
    "  4 vibrato   440 Hz with 5 Hz pitch modulation (+/-50 cents)\n"
    "  5 tremolo   440 Hz with 4 Hz amplitude modulation\n"
    "  6 envelope  880 Hz: repeated attack, decay, sustain, release\n"
    "  7 arp       Fast C major arpeggio; one note at a time\n"
    "  8 siren     Continuous glide between 400 and 1600 Hz\n"
    "  9 echo      440 Hz hit followed by four weaker repeats\n"
    "  tone 440 [1500]  100..8000 Hz; optional duration 10..30000 ms\n"
    "  note 69          MIDI note 45..119; 69 = 440 Hz\n"
    "  synth [voice]    Set or show voice (sine, triangle, pulse, fm, organ)\n"
    "  volume [0..100]  Set/query PCM amplitude; default 35%; 0 mutes\n"
    "  level 0..100     Alias for setting volume\n"
    "  stop / !         Silence and discard pending playback\n"
    "  sensors on/off   Two-sensor distance demo; no speaker needed\n"
    "  sensors pitch/volume/status/reset  Test one sensor or inspect/reset diagnostics\n"
    "  theremin on/pitch/volume  Live vibrato with hand-controlled pitch/volume\n"
    "  theremin off/status      Stop live audio or show its targets\n"
    "  status / help    Connection, playback, or this menu\n"
    "Audio format is 44100 Hz stereo, 16-bit. Bluetooth adds audible latency.\n";

class FirmwareTest : public testing::Test {
protected:
    void SetUp() override { reset(); }
    void TearDown() override { EXPECT_EQ(fake_rtos::lockDepth, 0); }

    void reset(bool connected = false) {
        sensors.stop();
        sensors.~UltrasonicDemo();
        new (&sensors) sounder::UltrasonicDemo();
        theremin = {};
        fake_arduino::now = 0;
        fake_arduino::resetGpio();
        fake_arduino::delays.clear();
        fake_audio::beginFails = false;
        fake_audio::afterSample = nullptr;
        fake_rtos::queueFails = false;
        fake_rtos::afterReceive = nullptr;
        fake_rtos::queues.clear();
        fake_rtos::lockDepth = 0;
        Serial = {};
        shared = {};
        oscillator = {};
        playback.~Player();
        synthesizer.~Synth();
        new (&synthesizer) Synth(oscillator, sounder::kSampleRate);
        new (&playback) Player(synthesizer);
        source = {};
        commands = nullptr;
        renderedGeneration = 0;
        audioReady = false;
        greetingShown = false;
        reported = {};
        progressReported = false;
        inputLength = 0;
        inputOverflow = false;
        std::memset(input, 0, sizeof(input));
        if (connected) {
            setup();
            onConnection(ESP_A2D_CONNECTION_STATE_CONNECTED, nullptr);
            onAudio(ESP_A2D_AUDIO_STATE_STARTED, nullptr);
        }
    }

    void command(const std::string& text) {
        std::vector<char> line(text.begin(), text.end());
        line.push_back('\0');
        handleCommand(line.data());
    }

    std::vector<::Frame> render(size_t count) {
        std::vector<::Frame> frames(count);
        EXPECT_EQ(renderAudio(frames.data(), static_cast<int32_t>(count)), static_cast<int32_t>(count));
        return frames;
    }

    void sensorEcho(uint32_t atMs, uint8_t echoPin, uint32_t pulseUs) {
        fake_arduino::now = atMs;
        loop(); // Schedule this sensor's trigger.
        delayMicroseconds(100);
        fake_arduino::edge(echoPin, HIGH);
        delayMicroseconds(pulseUs);
        fake_arduino::edge(echoPin, LOW);
        loop(); // Consume the result and publish the audio target.
    }

    void singleSensorEcho(uint32_t atMs, uint8_t echoPin, uint32_t pulseUs) {
        fake_arduino::now = atMs - 35;
        loop(); // Leave the alternate live scheduling slot quiet.
        sensorEcho(atMs, echoPin, pulseUs);
    }
};

TEST(TriangleSynthTest, ReadSample) {
    sounder::TriangleSynth triangle(8000);
    triangle.setFrequency(1000);
    triangle.setAmplitude(1000);
    for (int16_t sample : {0, 500, 1000, 500, 0, -500, -1000, -500, 0}) {
        EXPECT_EQ(triangle.readSample(), sample);
    }
    triangle.setFrequency(2000);
    EXPECT_EQ(triangle.readSample(), 500);
    EXPECT_EQ(triangle.readSample(), 500);
    EXPECT_EQ(triangle.readSample(), -500);
    triangle.setAmplitude(0);
    EXPECT_EQ(triangle.readSample(), 0);
}

TEST(PulseSynthTest, ReadSample) {
    sounder::PulseSynth pulse(8000);
    pulse.setFrequency(1000);
    pulse.setAmplitude(3000);
    // The first period includes the smoothed rising and falling edges.
    for (int16_t sample : {1000, 3000, 1000, -1000, -1000, -1000, -1000, -1000, 1000}) {
        EXPECT_NEAR(pulse.readSample(), sample, 1);
    }
    pulse.setAmplitude(0);
    EXPECT_EQ(pulse.readSample(), 0);
}

TEST(FmRobotSynthTest, ReadSample) {
    sounder::FmRobotSynth fm(8000);
    fm.setFrequency(500);
    fm.setAmplitude(1000);
    EXPECT_EQ(fm.readSample(), 0);
    EXPECT_NEAR(fm.readSample(), 857, 1);
    EXPECT_NEAR(fm.readSample(), 993, 1);

    // At this pitch the third harmonic exceeds Nyquist and modulation fades out.
    fm.setFrequency(1500);
    EXPECT_NEAR(fm.readSample(), 924, 1);
    fm.setAmplitude(0);
    EXPECT_EQ(fm.readSample(), 0);
}

TEST(OrganSynthTest, ReadSample) {
    sounder::OrganSynth organ(8000);
    organ.setFrequency(500);
    organ.setAmplitude(1000);
    EXPECT_EQ(organ.readSample(), 0);
    EXPECT_NEAR(organ.readSample(), 561, 1);
    EXPECT_NEAR(organ.readSample(), 795, 1);

    // Both upper harmonics have reached or passed Nyquist at 2000 Hz.
    organ.setFrequency(2000);
    EXPECT_NEAR(organ.readSample(), 508, 1);
    organ.setAmplitude(0);
    EXPECT_EQ(organ.readSample(), 0);
}

TEST(PlaybackTest, Constructor) {
    Oscillator osc;
    Synth synth(osc, sounder::kSampleRate);
    Player player(synth, 123);
    EXPECT_EQ(player.status().mode, Mode::Idle);
    EXPECT_EQ(player.timeMs(), 123U);
    EXPECT_EQ(osc.beginCalls, 0U);
    EXPECT_EQ(osc.samples, 0U);
}

TEST(PlaybackTest, StartTone) {
    const float rows[] = {100, 440.5f, 8000, 0, -1, 8001,
                          std::numeric_limits<float>::quiet_NaN()};
    for (float hz : rows) {
        SCOPED_TRACE(hz);
        Oscillator osc;
        Synth synth(osc, sounder::kSampleRate);
        Player player(synth);
        player.startTour();
        player.startTone(hz, 10);
        player.nextSample();
        const bool valid = std::isfinite(hz) && hz >= 100 && hz <= 8000;
        EXPECT_EQ(player.status().mode, valid ? Mode::Tone : Mode::Idle);
        if (valid) EXPECT_FLOAT_EQ(osc.hz, hz);
        EXPECT_FLOAT_EQ(osc.amplitude, valid ? 11468.45f : 0.0f);
    }
}

TEST(PlaybackTest, StartPattern) {
    for (size_t index : {size_t{0}, size_t{4}, size_t{8}, size_t{9}, SIZE_MAX}) {
        SCOPED_TRACE(index);
        Oscillator osc;
        Synth synth(osc, sounder::kSampleRate);
        Player player(synth);
        player.startPattern(index);
        player.nextSample();
        EXPECT_EQ(player.status().mode, index < 9 ? Mode::Pattern : Mode::Idle);
        if (index < 9) EXPECT_EQ(player.status().pattern, index);
    }
}

TEST(PlaybackTest, StartTour) {
    Oscillator osc;
    Synth synth(osc, sounder::kSampleRate);
    Player player(synth);
    player.startPattern(8);
    player.startTour();
    player.nextSample();
    EXPECT_EQ(player.status().mode, Mode::Tour);
    EXPECT_EQ(player.status().pattern, 0U);
    EXPECT_NEAR(player.status().hz, 261.6256f, 0.001f);
    for (uint32_t i = 1; i < 1993320; ++i) player.nextSample();
    EXPECT_EQ(player.timeMs(), 45200U);
    player.nextSample();
    EXPECT_EQ(player.status().mode, Mode::Idle);
    EXPECT_FLOAT_EQ(osc.amplitude, 0);
}

TEST(PlaybackTest, Stop) {
    Oscillator osc;
    Synth synth(osc, sounder::kSampleRate);
    Player player(synth);
    for (bool playing : {false, true}) {
        if (playing) { player.startTone(440); player.nextSample(); }
        player.stop();
        EXPECT_EQ(player.status().mode, Mode::Idle);
        EXPECT_FLOAT_EQ(player.status().hz, 0);
        EXPECT_EQ(player.nextSample(), 0);
        EXPECT_FLOAT_EQ(osc.amplitude, 0);
        player.stop();
        EXPECT_EQ(player.nextSample(), 0);
    }
}

TEST(PlaybackTest, SetLevel) {
    struct Row { float input; float amplitude; };
    const Row rows[] = {{0, 0}, {100, 32767}, {50, 16383.5f}, {35, 11468.45f},
                        {-1, 11468.45f}, {101, 11468.45f},
                        {std::numeric_limits<float>::quiet_NaN(), 11468.45f}};
    for (const auto& row : rows) {
        SCOPED_TRACE(row.input);
        Oscillator osc;
        Synth synth(osc, sounder::kSampleRate);
        Player player(synth);
        player.startTone(440);
        player.setLevel(row.input);
        player.nextSample();
        EXPECT_FLOAT_EQ(osc.amplitude, row.amplitude);
        player.setLevel(row.input);
        player.nextSample();
        EXPECT_EQ(osc.amplitudes.size(), 1U);
    }
}

TEST(PlaybackTest, NextSample) {
    for (unsigned hz : {100U, 440U, 1000U, 8000U}) {
        SCOPED_TRACE(hz);
        Oscillator osc;
        Synth synth(osc, sounder::kSampleRate);
        Player player(synth);
        player.startTone(static_cast<float>(hz));
        int crossings = 0;
        int peak = 0;
        int16_t previous = 0;
        for (uint32_t i = 0; i < 44100; ++i) {
            const int16_t sample = player.nextSample();
            if (previous <= 0 && sample > 0) ++crossings;
            previous = sample;
            peak = std::max(peak, std::abs(static_cast<int>(sample)));
        }
        EXPECT_NEAR(crossings, hz, 1);
        EXPECT_LE(peak, 11469);
        EXPECT_GE(peak, 11000);
        EXPECT_EQ(player.timeMs(), 1000U);
        EXPECT_EQ(osc.samples, 44100U);
        EXPECT_EQ(osc.frequencies.size(), 1U);
    }
    // Boundaries around the 5 ms control interval, including clock rollover.
    for (uint32_t start : {0U, UINT32_MAX - 2}) {
        Oscillator osc;
        Synth synth(osc, sounder::kSampleRate);
        Player player(synth, start);
        player.startTone(440, 10);
        for (unsigned i = 0; i < 221; ++i) player.nextSample();
        EXPECT_EQ(osc.amplitudes.size(), 1U);
        player.nextSample();
        EXPECT_EQ(osc.amplitudes.size(), 2U);
        for (unsigned i = 222; i < 441; ++i) player.nextSample();
        EXPECT_EQ(player.status().mode, Mode::Tone);
        EXPECT_EQ(player.timeMs(), start + 10);
        EXPECT_EQ(player.nextSample(), 0);
        EXPECT_EQ(player.status().mode, Mode::Idle);
    }
    // Updating pitch must not reinitialize the oscillator or reset its phase.
    Oscillator osc;
    Synth synth(osc, sounder::kSampleRate);
    Player player(synth);
    player.startTone(440);
    for (unsigned i = 0; i < 17; ++i) player.nextSample();
    const double phase = osc.phase;
    player.startTone(880);
    EXPECT_NEAR(player.nextSample(), 11468.45 * std::sin(phase * 6.283185307179586), 1);
    EXPECT_EQ(osc.beginCalls, 0U);
}

TEST(PlaybackTest, Status) {
    Oscillator osc;
    Synth synth(osc, sounder::kSampleRate);
    Player player(synth);
    EXPECT_FLOAT_EQ(player.status().hz, 0);
    EXPECT_FLOAT_EQ(player.status().drive, 0);
    player.startTone(1000);
    player.nextSample();
    EXPECT_EQ(player.status().mode, Mode::Tone);
    EXPECT_FLOAT_EQ(player.status().hz, 1000);
    EXPECT_FLOAT_EQ(player.status().drive, 1);
}

TEST(PlaybackTest, TimeMs) {
    Oscillator osc;
    Synth synth(osc, sounder::kSampleRate);
    Player player(synth);
    for (unsigned i = 0; i < 44; ++i) player.nextSample();
    EXPECT_EQ(player.timeMs(), 0U);
    player.nextSample();
    EXPECT_EQ(player.timeMs(), 1U);
    for (unsigned i = 45; i < 44100; ++i) player.nextSample();
    EXPECT_EQ(player.timeMs(), 1000U);
}

TEST_F(FirmwareTest, EnableReconnect) {
    source.enableReconnect();
    EXPECT_TRUE(source.reconnectAllowed());
    EXPECT_TRUE(source.reconnectEnabled);
    EXPECT_EQ(source.retries, 3);
    source.set_auto_reconnect(false);
    source.enableReconnect();
    EXPECT_TRUE(source.reconnectEnabled);
    EXPECT_EQ(Serial.output, "");
}

TEST_F(FirmwareTest, ReadState) {
    shared.level = 50;
    auto copy = readState();
    copy.level = 0;
    EXPECT_FLOAT_EQ(shared.level, 50);
    EXPECT_EQ(copy.connection, ESP_A2D_CONNECTION_STATE_DISCONNECTED);
    EXPECT_EQ(Serial.output, "");
}

TEST_F(FirmwareTest, RequestStop) {
    reset(true);
    command("tone 440");
    const auto generation = shared.generation;
    requestStop();
    EXPECT_EQ(shared.generation, generation + 1);
    EXPECT_EQ(shared.playback.mode, Mode::Idle);
    EXPECT_EQ(Serial.output, kTone);
    for (const auto& frame : render(128)) EXPECT_EQ(frame.channel1, 0);
    EXPECT_EQ(shared.playback.mode, Mode::Idle);
}

TEST_F(FirmwareTest, OnConnection) {
    const esp_a2d_connection_state_t rows[] = {
        ESP_A2D_CONNECTION_STATE_CONNECTING, ESP_A2D_CONNECTION_STATE_CONNECTED,
        ESP_A2D_CONNECTION_STATE_DISCONNECTING, ESP_A2D_CONNECTION_STATE_DISCONNECTED};
    for (auto state : rows) {
        SCOPED_TRACE(state);
        reset();
        shared.playback.mode = Mode::Tone;
        shared.audio = ESP_A2D_AUDIO_STATE_STARTED;
        onConnection(state, nullptr);
        EXPECT_EQ(shared.connection, state);
        EXPECT_EQ(shared.generation, state == ESP_A2D_CONNECTION_STATE_DISCONNECTED ? 0U : 1U);
        if (state != ESP_A2D_CONNECTION_STATE_DISCONNECTED) EXPECT_EQ(shared.playback.mode, Mode::Idle);
        EXPECT_EQ(shared.audio, state == ESP_A2D_CONNECTION_STATE_CONNECTED ?
                  ESP_A2D_AUDIO_STATE_STARTED : ESP_A2D_AUDIO_STATE_STOPPED);
        EXPECT_EQ(source.reconnectEnabled, state == ESP_A2D_CONNECTION_STATE_CONNECTED);
        const auto generation = shared.generation;
        onConnection(state, nullptr);
        EXPECT_EQ(shared.generation, generation);
        EXPECT_EQ(Serial.output, "");
    }
}

TEST_F(FirmwareTest, OnAudio) {
    for (auto state : {ESP_A2D_AUDIO_STATE_STARTED, ESP_A2D_AUDIO_STATE_STOPPED,
                       ESP_A2D_AUDIO_STATE_REMOTE_SUSPEND}) {
        reset();
        shared.playback.mode = Mode::Tone;
        onAudio(state, nullptr);
        EXPECT_EQ(shared.audio, state);
        EXPECT_EQ(shared.generation, state == ESP_A2D_AUDIO_STATE_STARTED ? 0U : 1U);
        EXPECT_EQ(shared.playback.mode, state == ESP_A2D_AUDIO_STATE_STARTED ? Mode::Tone : Mode::Idle);
        EXPECT_EQ(Serial.output, "");
    }
}

TEST_F(FirmwareTest, OnDiscovery) {
    onDiscovery(ESP_BT_GAP_DISCOVERY_STARTED);
    EXPECT_TRUE(shared.discovering);
    onDiscovery(ESP_BT_GAP_DISCOVERY_STOPPED);
    EXPECT_FALSE(shared.discovering);
    EXPECT_EQ(Serial.output, "");
}

TEST_F(FirmwareTest, SelectSpeaker) {
    struct Row { const char* name; bool accepted; };
    const Row rows[] = {{nullptr, false}, {"", false}, {"JBL Flip 4", true},
                        {"JBL Flip 4 Other", false}, {"jbl flip 4", false}, {"Other", false}};
    for (const auto& row : rows) {
        EXPECT_EQ(selectSpeaker(row.name, nullptr, -40), row.accepted);
    }
    EXPECT_EQ(Serial.output, "");
}

TEST_F(FirmwareTest, RenderAudio) {
    ::Frame sentinel{123, 456};
    EXPECT_EQ(renderAudio(nullptr, 1), 0);
    EXPECT_EQ(renderAudio(&sentinel, 0), 0);
    EXPECT_EQ(renderAudio(&sentinel, -1), 0);
    EXPECT_EQ(sentinel.channel1, 123);
    // No initialized queue and no connection must still produce exact silence.
    for (const auto& frame : render(20)) EXPECT_EQ(frame.channel1, 0);
    reset(true);
    command("tone 440");
    auto frames = render(512);
    EXPECT_EQ(shared.playback.mode, Mode::Tone);
    EXPECT_FLOAT_EQ(shared.playback.hz, 440);
    EXPECT_TRUE(std::any_of(frames.begin(), frames.end(), [](const auto& f) { return f.channel1 != 0; }));
    for (const auto& frame : frames) EXPECT_EQ(frame.channel1, frame.channel2);
    EXPECT_EQ(oscillator.samples, 512U);
    EXPECT_EQ(Serial.output, kTone); // No printing from the callback.
    // Callback boundaries do not affect the audio sample sequence.
    reset(true);
    command("tone 440");
    auto first = render(17);
    auto rest = render(495);
    first.insert(first.end(), rest.begin(), rest.end());
    for (size_t i = 0; i < frames.size(); ++i) EXPECT_EQ(first[i].channel1, frames[i].channel1);
    // Stale queued notes must not resume after disconnect/reconnect.
    command("tone 880");
    onConnection(ESP_A2D_CONNECTION_STATE_DISCONNECTED, nullptr);
    for (const auto& frame : render(32)) EXPECT_EQ(frame.channel1, 0);
    onConnection(ESP_A2D_CONNECTION_STATE_CONNECTED, nullptr);
    for (const auto& frame : render(32)) EXPECT_EQ(frame.channel1, 0);
    EXPECT_EQ(shared.playback.mode, Mode::Idle);
    // Commands after a stop, arriving within the same callback, remain valid.
    command("tone 100");
    Serial.output.clear();
    fake_rtos::afterReceive = [this] { requestStop(); command("tone 1000"); };
    render(100);
    EXPECT_FLOAT_EQ(shared.playback.hz, 1000);
    EXPECT_EQ(Serial.output, kTone);
    // A stop while the queue is being drained cannot be overwritten by a buffer.
    command("tone 100");
    fake_rtos::afterReceive = [] { requestStop(); };
    Serial.output.clear();
    for (const auto& frame : render(32)) EXPECT_EQ(frame.channel1, 0);
    EXPECT_EQ(shared.playback.mode, Mode::Idle);
    EXPECT_EQ(Serial.output, "");
    // Pattern and tour commands reach the real demo engine.
    command("echo");
    render(1);
    EXPECT_EQ(shared.playback.mode, Mode::Pattern);
    EXPECT_EQ(shared.playback.pattern, 8U);
    command("demo");
    render(1);
    EXPECT_EQ(shared.playback.mode, Mode::Tour);
    EXPECT_EQ(shared.playback.pattern, 0U);
    EXPECT_EQ(Serial.output, "Queued echo.\nTour queued.\n");
    // A stop during sample generation must not publish the old tone as current.
    Serial.output.clear();
    fake_audio::afterSample = [] { requestStop(); };
    render(32);
    EXPECT_EQ(shared.playback.mode, Mode::Idle);
    for (const auto& frame : render(32)) EXPECT_EQ(frame.channel1, 0);
    EXPECT_EQ(Serial.output, "");
}

TEST_F(FirmwareTest, SwitchSynthDuringTour) {
    struct Row { std::string name; int16_t secondSample; };
    const Row rows[] = {{"triangle", 272}, {"pulse", 11468}, {"fm", 1194}, {"organ", 683}};
    for (const auto& row : rows) {
        SCOPED_TRACE(row.name);
        reset(true);
        command("demo");
        render(4410); // 100 ms into the first note.
        ASSERT_EQ(shared.playback.mode, Mode::Tour);
        EXPECT_EQ(oscillator.samples, 4410U);

        Serial.output.clear();
        Serial.feed("synth " + row.name + "\n");
        loop();
        EXPECT_EQ(Serial.output, "Synth " + row.name + ".\n");
        const auto frames = render(10143); // Render to the end of the 330 ms note.
        EXPECT_EQ(oscillator.samples, 4410U);
        EXPECT_NEAR(frames[1].channel1, row.secondSample, 2);
        for (const auto& frame : frames) EXPECT_EQ(frame.channel1, frame.channel2);

        // The first note's gap and the second note still start on schedule.
        for (const auto& frame : render(3087)) EXPECT_EQ(frame.channel1, 0);
        render(1);
        EXPECT_EQ(shared.playback.mode, Mode::Tour);
        EXPECT_EQ(shared.playback.pattern, 0U);
        EXPECT_FLOAT_EQ(shared.playback.hz, sounder::midiToHz(62));
        EXPECT_FLOAT_EQ(shared.playback.drive, 1.0f);

        command("synth sine");
        render(128);
        EXPECT_EQ(oscillator.samples, 4410U + 128U);
        EXPECT_FLOAT_EQ(oscillator.hz, sounder::midiToHz(62));
        EXPECT_EQ(shared.playback.mode, Mode::Tour);
    }
}

TEST_F(FirmwareTest, ShowcasePlaysEveryPatternOnEveryVoice) {
    reset(true);
    command("synth fm");
    reportProgress();
    Serial.output.clear();
    Serial.feed("ShOwCaSe\n");
    loop();
    EXPECT_EQ(Serial.output, "Showcase queued.\n");

    struct Row { std::string name; int16_t secondSample; };
    const Row voices[] = {{"sine", 427}, {"triangle", 272}, {"pulse", 11468},
                          {"fm", 1194}, {"organ", 683}};
    size_t section = 0;
    for (const auto& voice : voices) {
        SCOPED_TRACE(voice.name);
        for (size_t i = 0; i < sounder::kPatternCount; ++i) {
            const auto& pattern = sounder::kPatterns[i];
            const auto opening = render(2);
            ASSERT_EQ(shared.playback.mode, Mode::Showcase);
            EXPECT_EQ(shared.playback.pattern, i);
            if (i == 0) EXPECT_NEAR(opening[1].channel1, voice.secondSample, 2);

            Serial.output.clear();
            reportProgress();
            EXPECT_EQ(Serial.output, "[showcase " + std::to_string(++section) + "/45] " +
                      voice.name + " / " + pattern.name + "\n");
            if (section == 1) {
                // The manual selection can change while the showcase is active.
                command("synth triangle");
                Serial.output.clear();
                showStatus();
                EXPECT_NE(Serial.output.find("showcase/scale | synth sine"), std::string::npos);
            }
            const size_t frames = pattern.durationMs * sounder::kSampleRate / 1000;
            render(frames - opening.size());
        }
    }

    EXPECT_EQ(section, 45U);
    EXPECT_EQ(playback.timeMs(), 226000U);
    EXPECT_EQ(render(1).front().channel1, 0);
    EXPECT_EQ(shared.playback.mode, Mode::Idle);
    Serial.output.clear();
    reportProgress();
    EXPECT_EQ(Serial.output, "Finished; silent.\n");
    command("demo");
    render(2);
    EXPECT_EQ(shared.playback.mode, Mode::Tour);
    EXPECT_EQ(shared.playback.voice, sounder::SynthVoice::Triangle);
}

TEST_F(FirmwareTest, ShowcaseCanBeInterrupted) {
    reset(true);
    command("synth fm");
    command("showcase");
    render(32);
    ASSERT_EQ(shared.playback.mode, Mode::Showcase);
    command("siren");
    render(32);
    EXPECT_EQ(shared.playback.mode, Mode::Pattern);
    EXPECT_EQ(shared.playback.voice, sounder::SynthVoice::Fm);

    command("showcase");
    render(32);
    ASSERT_EQ(shared.playback.mode, Mode::Showcase);
    Serial.feed("!");
    loop();
    for (const auto& frame : render(32)) EXPECT_EQ(frame.channel1, 0);
    EXPECT_EQ(shared.playback.mode, Mode::Idle);

    command("showcase");
    render(32);
    ASSERT_EQ(shared.playback.mode, Mode::Showcase);
    onConnection(ESP_A2D_CONNECTION_STATE_DISCONNECTED, nullptr);
    onConnection(ESP_A2D_CONNECTION_STATE_CONNECTED, nullptr);
    for (const auto& frame : render(32)) EXPECT_EQ(frame.channel1, 0);
    EXPECT_EQ(shared.playback.mode, Mode::Idle);
}

TEST_F(FirmwareTest, ParseNumber) {
    struct Row { const char* text; bool valid; float value; };
    const Row rows[] = {{nullptr, false, 0}, {"", false, 0}, {"0", true, 0},
                        {"35.5", true, 35.5f}, {"100", true, 100}, {"-1", false, 0},
                        {"101", false, 0}, {"nan", false, 0}, {"inf", false, 0},
                        {"1e99", false, 0}, {"12hz", false, 0}, {"12 ", false, 0}};
    for (const auto& row : rows) {
        float value = -1;
        EXPECT_EQ(parseNumber(row.text, 0, 100, value), row.valid);
        if (row.valid) EXPECT_FLOAT_EQ(value, row.value);
    }
    EXPECT_EQ(Serial.output, "");
}

TEST_F(FirmwareTest, ConnectionName) {
    EXPECT_STREQ(connectionName(ESP_A2D_CONNECTION_STATE_DISCONNECTED), "disconnected");
    EXPECT_STREQ(connectionName(ESP_A2D_CONNECTION_STATE_CONNECTING), "connecting");
    EXPECT_STREQ(connectionName(ESP_A2D_CONNECTION_STATE_CONNECTED), "connected");
    EXPECT_STREQ(connectionName(ESP_A2D_CONNECTION_STATE_DISCONNECTING), "disconnecting");
    EXPECT_STREQ(connectionName(static_cast<esp_a2d_connection_state_t>(99)), "unknown");
    EXPECT_EQ(Serial.output, "");
}

TEST_F(FirmwareTest, AudioName) {
    EXPECT_STREQ(audioName(ESP_A2D_AUDIO_STATE_REMOTE_SUSPEND), "suspended");
    EXPECT_STREQ(audioName(ESP_A2D_AUDIO_STATE_STOPPED), "stopped");
    EXPECT_STREQ(audioName(ESP_A2D_AUDIO_STATE_STARTED), "streaming");
    EXPECT_STREQ(audioName(static_cast<esp_a2d_audio_state_t>(99)), "unknown");
    EXPECT_EQ(Serial.output, "");
}

TEST_F(FirmwareTest, ShowHelp) {
    showHelp();
    EXPECT_EQ(Serial.output, kHelp);
}

TEST_F(FirmwareTest, ShowStatus) {
    showStatus();
    EXPECT_EQ(Serial.output, kIdleStatus);
    reset(true);
    command("tone 440");
    render(1);
    Serial.output.clear();
    showStatus();
    EXPECT_EQ(Serial.output,
              "JBL Flip 4 | connected | audio streaming | tone | synth sine | target 440.0 Hz | level 35.0% | drive 35.0% | 44100 Hz/stereo/16-bit\n");
    command("demo");
    render(1);
    Serial.output.clear();
    showStatus();
    EXPECT_EQ(Serial.output,
              "JBL Flip 4 | connected | audio streaming | tour/scale | synth sine | target 261.6 Hz | level 35.0% | drive 35.0% | 44100 Hz/stereo/16-bit\n");
    command("synth triangle");
    command("echo");
    render(1);
    onDiscovery(ESP_BT_GAP_DISCOVERY_STARTED);
    Serial.output.clear();
    showStatus();
    EXPECT_EQ(Serial.output,
              "JBL Flip 4 | connected/discovering | audio streaming | echo | synth triangle | target 440.0 Hz | level 35.0% | drive 35.0% | 44100 Hz/stereo/16-bit\n");
}

TEST_F(FirmwareTest, QueuePlayback) {
    Command next;
    EXPECT_FALSE(queuePlayback(next));
    EXPECT_EQ(Serial.output, kInitFailed);
    setup();
    Serial.output.clear();
    EXPECT_FALSE(queuePlayback(next));
    EXPECT_EQ(Serial.output, kOffline);
    onConnection(ESP_A2D_CONNECTION_STATE_CONNECTED, nullptr);
    Serial.output.clear();
    for (size_t i = 0; i < kCommandCapacity; ++i) EXPECT_TRUE(queuePlayback(next));
    EXPECT_EQ(Serial.output, "");
    EXPECT_FALSE(queuePlayback(next));
    EXPECT_EQ(Serial.output, kFull);
    EXPECT_EQ(commands->items.size(), 8U);
    EXPECT_EQ(shared.playback.mode, Mode::Idle);
}

TEST_F(FirmwareTest, HandleCommand) {
    struct Row { const char* text; std::string output; float hz; uint32_t duration; };
    const Row tones[] = {{"tone 100", kTone, 100, 0}, {"tone 8000", kTone, 8000, 0},
                         {" ToNe\t440.5 10.9 ", kTone, 440.5f, 10},
                         {"tone 440 30000", kTone, 440, 30000},
                         {"note 45", kNote, 110, 0}, {"note 69", kNote, 440, 0},
                         {"note 119", kNote, 7902.1328f, 0}};
    for (const auto& row : tones) {
        SCOPED_TRACE(row.text);
        reset(true);
        command(row.text);
        EXPECT_EQ(Serial.output, row.output);
        Command next;
        ASSERT_EQ(xQueueReceive(commands, &next, 0), pdTRUE);
        EXPECT_EQ(next.kind, CommandKind::Tone);
        EXPECT_NEAR(next.hz, row.hz, 0.001f);
        EXPECT_EQ(next.durationMs, row.duration);
        EXPECT_EQ(next.generation, shared.generation);
    }
    const char* bad[] = {"tone", "tone 0", "tone -440", "tone 99.9", "tone 8000.1", "tone nan",
                         "tone inf", "tone 1e99", "tone 440 0", "tone 440 9", "tone 440 30001",
                         "tone 440 nope", "tone 440 10 extra", "tone 440hz", "note", "note 44",
                         "note 120", "note -1", "note 69.5", "note nan", "note 69 extra",
                         "level", "level -1", "level 100.1", "level nan", "level 35 extra",
                         "volume -1", "volume 100.1", "volume nan", "volume inf", "volume 35 extra",
                         "stop extra", "help extra", "status extra", "demo extra", "showcase extra", "scale extra"};
    for (const char* text : bad) {
        SCOPED_TRACE(text);
        reset(true);
        command("tone 880");
        render(1);
        Serial.output.clear();
        command(text);
        EXPECT_EQ(Serial.output, std::strchr(text, ' ') ? kInvalid : kUnknown);
        EXPECT_EQ(shared.playback.mode, Mode::Tone);
        EXPECT_FLOAT_EQ(shared.playback.hz, 880);
        EXPECT_FLOAT_EQ(shared.level, 35);
        EXPECT_TRUE(commands->items.empty());
    }
    struct LevelRow { const char* text; float level; const char* output; };
    const LevelRow levels[] = {{"level 0", 0, "PCM amplitude 0.0%.\n"},
                               {"level 100", 100, "PCM amplitude 100.0%.\n"},
                               {"level 35.5", 35.5f, "PCM amplitude 35.5%.\n"},
                               {"volume 0", 0, "PCM amplitude 0.0%.\n"},
                               {"volume 100", 100, "PCM amplitude 100.0%.\n"},
                               {" VoLuMe\t20.5 ", 20.5f, "PCM amplitude 20.5%.\n"}};
    for (const auto& row : levels) {
        reset(); // Levels work while disconnected too.
        command(row.text);
        EXPECT_FLOAT_EQ(shared.level, row.level);
        EXPECT_EQ(Serial.output, row.output);
        Serial.output.clear();
        command("volume");
        EXPECT_FLOAT_EQ(shared.level, row.level);
        EXPECT_EQ(Serial.output, row.output);
    }
    reset();
    command(" SyNtH\tTrIaNgLe ");
    EXPECT_EQ(shared.voice, sounder::SynthVoice::Triangle);
    EXPECT_EQ(Serial.output, "Synth triangle.\n");
    for (const char* text : {"synth square", "synth sine extra"}) {
        Serial.output.clear();
        command(text);
        EXPECT_EQ(Serial.output, kInvalid);
        EXPECT_EQ(shared.voice, sounder::SynthVoice::Triangle);
    }
    Serial.output.clear();
    command("synth");
    command("synth sine");
    EXPECT_EQ(Serial.output, "Synth triangle.\nSynth sine.\n");
    EXPECT_EQ(shared.voice, sounder::SynthVoice::Sine);

    struct SimpleRow { const char* text; std::string output; };
    const SimpleRow simple[] = {{"", ""}, {" \t ", ""}, {"unknown", kUnknown},
                                {"0", kUnknown}, {"10", kUnknown}, {"HELP", kHelp},
                                {"status", kIdleStatus}, {"stop", kStop},
                                {"volume", "PCM amplitude 35.0%.\n"}, {"synth", "Synth sine.\n"}};
    for (const auto& row : simple) {
        reset();
        command(row.text);
        EXPECT_EQ(Serial.output, row.output);
    }
    for (size_t i = 0; i < sounder::kPatternCount; ++i) {
        for (const auto& text : {std::string(sounder::kPatterns[i].name), std::to_string(i + 1)}) {
            reset(true);
            command(text);
            EXPECT_EQ(Serial.output, "Queued " + std::string(sounder::kPatterns[i].name) + ".\n");
            Command next;
            ASSERT_EQ(xQueueReceive(commands, &next, 0), pdTRUE);
            EXPECT_EQ(next.kind, CommandKind::Pattern);
            EXPECT_EQ(next.pattern, i);
        }
    }
    reset(true);
    command("demo");
    EXPECT_EQ(Serial.output, "Tour queued.\n");
}

TEST_F(FirmwareTest, ReadCommands) {
    struct Row { std::string input; std::string output; };
    const Row rows[] = {{"", ""}, {"\r\n", ""}, {"stop\n", kStop}, {"stop\r\n", kStop},
                        {"tone 600!", kStop}, {"stopx\b\n", kStop}, {"stopx\177\n", kStop},
                        {"\b\177stop\n", kStop},
                        {std::string("st") + char(1) + char(0xff) + "op\n", kStop},
                        {std::string(79, 'x') + "\n", kUnknown},
                        {std::string(80, 'x') + "\n", "Line too long; ignored. Type help.\n"},
                        {std::string(80, 'x') + "\b\n", "Line too long; ignored. Type help.\n"},
                        {std::string(80, 'x') + "!\n", kStop},
                        {"stop\nunknown\n", kStop + kUnknown}};
    for (const auto& row : rows) {
        SCOPED_TRACE(row.input);
        reset(true);
        command("tone 440");
        Serial.output.clear();
        Serial.feed(row.input);
        do { readCommands(); } while (Serial.available());
        EXPECT_EQ(Serial.output, row.output);
        EXPECT_EQ(inputLength, 0U);
        EXPECT_FALSE(inputOverflow);
        EXPECT_EQ(fake_arduino::now, 0U);
    }
    reset();
    Serial.feed("stop");
    readCommands();
    EXPECT_EQ(inputLength, 4U);
    EXPECT_EQ(Serial.output, "");
    Serial.feed("\n");
    readCommands();
    EXPECT_EQ(Serial.output, kStop);
    reset();
    Serial.feed(std::string(65, 'x') + "\n");
    readCommands();
    EXPECT_EQ(inputLength, 64U);
    EXPECT_EQ(Serial.available(), 2);
    EXPECT_EQ(Serial.output, "");
    readCommands();
    EXPECT_EQ(Serial.output, kUnknown);
}

TEST_F(FirmwareTest, ReportProgress) {
    reportProgress();
    EXPECT_EQ(Serial.output, "Bluetooth disconnected | audio stopped.\n");
    Serial.output.clear();
    reportProgress();
    EXPECT_EQ(Serial.output, "");
    const char* names[] = {"scale", "sweep", "levels", "vibrato", "tremolo", "envelope", "arp", "siren", "echo"};
    for (size_t i = 0; i < 9; ++i) {
        shared.playback = {Mode::Pattern, i, 440, 1};
        Serial.output.clear();
        reportProgress();
        EXPECT_EQ(Serial.output, "[" + std::to_string(i + 1) + "/9] " + names[i] + "\n");
        Serial.output.clear();
        reportProgress();
        EXPECT_EQ(Serial.output, "");
    }
    shared.playback.mode = Mode::Tour;
    reportProgress();
    EXPECT_EQ(Serial.output, "[9/9] echo\n");
    Serial.output.clear();
    shared.playback.mode = Mode::Tone;
    reportProgress();
    EXPECT_EQ(Serial.output, "");
    requestStop();
    reportProgress();
    EXPECT_EQ(Serial.output, "Finished; silent.\n");
    Serial.output.clear();
    onDiscovery(ESP_BT_GAP_DISCOVERY_STARTED);
    reportProgress();
    EXPECT_EQ(Serial.output, "Bluetooth disconnected/discovering | audio stopped.\n");
    Serial.output.clear();
    onConnection(ESP_A2D_CONNECTION_STATE_CONNECTED, nullptr);
    onAudio(ESP_A2D_AUDIO_STATE_STARTED, nullptr);
    reportProgress();
    EXPECT_EQ(Serial.output, "Bluetooth connected/discovering | audio streaming.\n");
}

TEST_F(FirmwareTest, Setup) {
    for (int fail : {0, 1, 2}) {
        SCOPED_TRACE(fail);
        reset();
        fake_rtos::queueFails = fail == 1;
        fake_audio::beginFails = fail == 2;
        setup();
        EXPECT_EQ(Serial.baud, 115200U);
        EXPECT_EQ(Serial.beginCalls, 1U);
        EXPECT_EQ(audioReady, fail == 0);
        EXPECT_EQ(commands == nullptr, fail == 1);
        EXPECT_EQ(oscillator.beginCalls, fail == 1 ? 0U : 1U);
        EXPECT_EQ(source.starts, fail == 0 ? 1U : 0U);
        EXPECT_FLOAT_EQ(oscillator.amplitude, 0);
        EXPECT_EQ(shared.playback.mode, Mode::Idle);
        EXPECT_FALSE(greetingShown);
        EXPECT_TRUE(fake_arduino::delays.empty());
        EXPECT_EQ(Serial.output, "");
        if (fail == 0) {
            EXPECT_EQ(commands->capacity, 8U);
            EXPECT_EQ(commands->itemSize, sizeof(Command));
            EXPECT_EQ(oscillator.info.sample_rate, 44100U);
            EXPECT_EQ(oscillator.info.channels, 1);
            EXPECT_EQ(oscillator.info.bits_per_sample, 16);
            EXPECT_EQ(source.localName, "Sounder");
            EXPECT_EQ(source.render, renderAudio);
            EXPECT_EQ(source.connection, onConnection);
            EXPECT_EQ(source.audio, onAudio);
            EXPECT_EQ(source.discovery, onDiscovery);
            EXPECT_EQ(source.select, selectSpeaker);
            EXPECT_TRUE(source.ssp);
            EXPECT_EQ(source.pin, "0000");
            EXPECT_EQ(source.pinType, ESP_BT_PIN_TYPE_VARIABLE);
            EXPECT_FALSE(source.reconnectEnabled);
        }
    }
}

TEST_F(FirmwareTest, SensorCommandsWorkWithoutBluetoothOrAudioInitialization) {
    fake_rtos::queueFails = true;
    setup();
    Serial.connected = true;
    greetingShown = true;
    progressReported = true;
    Serial.feed("sensors on\n");
    loop();
    ASSERT_TRUE(sensors.active());
    EXPECT_FALSE(audioReady);
    EXPECT_EQ(shared.playback.mode, Mode::Idle);
    fake_arduino::now = 200;
    loop();
    delayMicroseconds(100);
    fake_arduino::edge(33, HIGH);
    delayMicroseconds(1000);
    fake_arduino::edge(33, LOW);
    loop();
    EXPECT_EQ(sensors.pitchStatistics().valid, 1U);
    fake_arduino::now = 300;
    loop(); // Both sensors are included in the first combined line.
    delayMicroseconds(30000);
    loop();
    EXPECT_NE(Serial.output.find("pitch ok | echo 1000 us | raw 17.17 cm"), std::string::npos);
    EXPECT_NE(Serial.output.find(" || volume timeout | echo 0 us | distance n/a"), std::string::npos);
    command("sensors on unexpected extra");
    EXPECT_TRUE(sensors.active());
    EXPECT_NE(Serial.output.find("Use sensors"), std::string::npos);
    command("SENSORS OFF");
    EXPECT_FALSE(sensors.active());
}

TEST_F(FirmwareTest, LiveVibratoPitchChangesAudioAndUsesSelectedVoice) {
    reset(true);
    command("synth pulse");
    command("tone 1000"); // A pending old tone must be invalidated by live mode.
    command("theremin pitch");
    EXPECT_EQ(shared.theremin, ThereminMode::Pitch);
    EXPECT_EQ(sensors.measurementIntervalMs(), 70U);
    for (const auto& frame : render(4410)) EXPECT_EQ(frame.channel1, 0);
    EXPECT_TRUE(shared.playback.live);
    sensorEcho(200, 33, 2911); // Approximately 50 cm: center approximately 220 Hz.
    EXPECT_NEAR(shared.liveTarget.hz, 220, 0.2);
    EXPECT_FLOAT_EQ(shared.liveTarget.drive, 1);
    const auto low = render(4410);
    EXPECT_TRUE(std::any_of(low.begin(), low.end(), [](const auto& frame) { return frame.channel1 != 0; }));
    EXPECT_EQ(shared.playback.voice, sounder::SynthVoice::Pulse);
    EXPECT_GE(shared.playback.hz, 213);
    EXPECT_LE(shared.playback.hz, 227);
    singleSensorEcho(270, 33, 2911); // Keep the stationary far hand fresh.
    singleSensorEcho(340, 33, 292); // First large jump is held.
    EXPECT_NEAR(shared.liveTarget.hz, 220, 0.2);
    EXPECT_EQ(sensors.pitchHandReading().heldJumps, 1U);
    // A persistent new hand position must be confirmed, then approached gently.
    for (uint32_t atMs = 410; atMs <= 2370; atMs += 70) {
        singleSensorEcho(atMs, 33, 292);
        render(3087);
    }
    EXPECT_NEAR(shared.liveTarget.hz, 880, 0.5);
    const auto beforeOutput = Serial.output;
    render(4410);
    EXPECT_EQ(Serial.output, beforeOutput); // No sensor I/O/logging in audio callback.
    EXPECT_GE(shared.playback.hz, 850);
    EXPECT_LE(shared.playback.hz, 907);
    EXPECT_FLOAT_EQ(shared.playback.drive, 1);
    EXPECT_EQ(oscillator.beginCalls, 1U);
    EXPECT_NE(Serial.output.find("vibrato (5 Hz, +/-50 cents)"), std::string::npos);
    command("theremin status");
    command("status");
    EXPECT_NE(Serial.output.find("theremin/vibrato"), std::string::npos);
    EXPECT_NE(Serial.output.find("Theremin pitch | vibrato"), std::string::npos);
}

TEST_F(FirmwareTest, LiveVibratoVolumeUsesOneSensorAndMasterVolumeRemainsACeiling) {
    reset(true);
    command("volume 20");
    command("theremin volume");
    sensorEcho(200, 14, 2911); // Approximately 50 cm: almost silent.
    EXPECT_LT(shared.liveTarget.drive, 0.001f);
    for (const auto& frame : render(4410)) EXPECT_LE(std::abs(static_cast<int>(frame.channel1)), 4);
    EXPECT_FLOAT_EQ(shared.liveTarget.hz, 440);
    EXPECT_EQ(sensors.pitchStatistics().attempts, 0U);
    for (uint32_t atMs = 270; atMs <= 2370; atMs += 70) {
        singleSensorEcho(atMs, 14, 200); // A confirmed near hand approaches full volume.
        render(3087);
    }
    EXPECT_FLOAT_EQ(shared.liveTarget.drive, 1);
    auto loud = render(4410);
    int peak = 0;
    for (const auto& frame : loud) {
        peak = std::max(peak, std::abs(static_cast<int>(frame.channel1)));
        EXPECT_EQ(frame.channel1, frame.channel2);
    }
    EXPECT_GT(peak, 6000);
    EXPECT_LE(peak, 6554);
    command("volume 0");
    for (const auto& frame : render(128)) EXPECT_EQ(frame.channel1, 0);
    command("volume 20");
    loud = render(128);
    EXPECT_TRUE(std::any_of(loud.begin(), loud.end(), [](const auto& frame) { return frame.channel1 != 0; }));
}

TEST_F(FirmwareTest, LiveVolumeIgnoresTheIsolatedJumpSeenInTheSuppliedLog) {
    reset(true);
    Serial.connected = true;
    greetingShown = true;
    command("synth pulse");
    command("theremin volume");
    singleSensorEcho(200, 14, 1055); // 18.12 cm establishes the hand position.
    const float steadyDrive = shared.liveTarget.drive;
    render(4410);
    singleSensorEcho(270, 14, 2285); // Raw 39.24 cm must not change the sound target.
    EXPECT_NEAR(sensors.volumeReading().distanceCm, 39.24f, 0.01f);
    EXPECT_NEAR(sensors.volumeHandReading().distanceCm, 18.12f, 0.01f);
    EXPECT_FLOAT_EQ(shared.liveTarget.drive, steadyDrive);
    EXPECT_EQ(sensors.volumeHandReading().heldJumps, 1U);
    singleSensorEcho(340, 14, 1075); // Return to 18.46 cm resumes gentle tracking.
    fake_arduino::now = 375;
    loop();
    EXPECT_NEAR(shared.liveTarget.drive, steadyDrive, 0.01f);
    EXPECT_NE(Serial.output.find("raw 18.12 cm | hand 18.12 cm | held 0"), std::string::npos);
    command("sensors reset");
    loop();
    EXPECT_TRUE(sensors.volumeHandReading().valid);
    EXPECT_EQ(sensors.volumeHandReading().heldJumps, 0U);
    EXPECT_GT(shared.liveTarget.drive, 0.6f);
}

TEST_F(FirmwareTest, LiveVibratoNeedsBothHandsAndFadesAfterLostReadingsOrStalledLoop) {
    reset(true);
    command("theremin on");
    sensorEcho(200, 33, 1600); // Pitch alone cannot sound in both-hands mode.
    for (const auto& frame : render(4410)) EXPECT_EQ(frame.channel1, 0);
    sensorEcho(300, 14, 1600);
    EXPECT_GT(shared.liveTarget.drive, 0.4f);
    render(4410);
    EXPECT_GT(oscillator.amplitude, 4000);
    // Audio-side watchdog also handles a main loop that stops publishing.
    fake_arduino::now = shared.liveUpdatedAtMs + 250;
    render(4410);
    EXPECT_FLOAT_EQ(shared.playback.drive, 0);
    EXPECT_FLOAT_EQ(oscillator.amplitude, 0);
    // A running loop still mutes when the underlying hand readings expire.
    updateTheremin();
    EXPECT_FLOAT_EQ(shared.liveTarget.drive, 0);
    render(4410);
    EXPECT_FLOAT_EQ(oscillator.amplitude, 0);
}

TEST_F(FirmwareTest, LiveVibratoStopsAndNeverResumesAfterDisconnectOrSuspension) {
    for (const char* action : {"stop", "!", "theremin off", "sensors off", "sensors pitch", "tone 1000", "vibrato",
                                "disconnect", "suspend"}) {
        SCOPED_TRACE(action);
        reset(true);
        command("theremin pitch");
        sensorEcho(200, 33, 1000);
        render(4410);
        ASSERT_GT(oscillator.amplitude, 0);
        const bool disconnected = std::strcmp(action, "disconnect") == 0;
        const bool suspended = std::strcmp(action, "suspend") == 0;
        if (disconnected) onConnection(ESP_A2D_CONNECTION_STATE_DISCONNECTED, nullptr);
        else if (suspended) onAudio(ESP_A2D_AUDIO_STATE_REMOTE_SUSPEND, nullptr);
        else if (std::strcmp(action, "!") == 0) { Serial.feed("!"); readCommands(); }
        else command(action);
        const bool replacement = std::strcmp(action, "tone 1000") == 0 || std::strcmp(action, "vibrato") == 0;
        render(128);
        EXPECT_EQ(shared.theremin, ThereminMode::Off);
        EXPECT_FALSE(shared.playback.live);
        if (!replacement) EXPECT_FLOAT_EQ(oscillator.amplitude, 0);
        if (disconnected) onConnection(ESP_A2D_CONNECTION_STATE_CONNECTED, nullptr);
        if (disconnected || suspended) onAudio(ESP_A2D_AUDIO_STATE_STARTED, nullptr);
        updateTheremin();
        render(128);
        if (!replacement) EXPECT_EQ(shared.playback.mode, Mode::Idle);
    }
}

TEST_F(FirmwareTest, InvalidOrUnavailableLiveCommandsLeavePlaybackUnchanged) {
    command("theremin on");
    EXPECT_EQ(Serial.output, kInitFailed);
    setup();
    Serial.output.clear();
    command("theremin pitch");
    EXPECT_EQ(Serial.output, kOffline);
    onConnection(ESP_A2D_CONNECTION_STATE_CONNECTED, nullptr);
    Serial.output.clear();
    command("theremin volume");
    EXPECT_NE(Serial.output.find("not streaming"), std::string::npos);
    reset(true);
    command("tone 880");
    render(1);
    for (const char* text : {"theremin nope", "theremin on extra", "theremin off extra extra"}) {
        command(text);
        EXPECT_EQ(shared.theremin, ThereminMode::Off);
        EXPECT_EQ(shared.playback.mode, Mode::Tone);
        EXPECT_FLOAT_EQ(shared.playback.hz, 880);
    }
}

TEST_F(FirmwareTest, Loop) {
    for (bool fail : {false, true}) {
        reset();
        fake_rtos::queueFails = fail;
        setup();
        loop();
        EXPECT_EQ(Serial.output, "");
        EXPECT_FALSE(greetingShown);
        Serial.connected = true;
        loop();
        EXPECT_EQ(Serial.output, kHelp + (fail ? kInitFailed : "") + "Bluetooth disconnected | audio stopped.\n");
        EXPECT_TRUE(greetingShown);
        Serial.output.clear();
        loop();
        EXPECT_EQ(Serial.output, "");
        EXPECT_EQ(fake_arduino::now, 3U);
        EXPECT_EQ(fake_arduino::delays, (std::vector<uint32_t>{1, 1, 1}));
    }
    // The serial-to-render path uses the real parser, queue, and demo engine.
    reset(true);
    Serial.connected = true;
    greetingShown = true;
    reportProgress();
    Serial.output.clear();
    Serial.feed("tone 440 10\n");
    loop();
    EXPECT_EQ(Serial.output, kTone);
    render(441);
    EXPECT_EQ(shared.playback.mode, Mode::Tone);
    render(1);
    EXPECT_EQ(shared.playback.mode, Mode::Idle);
    Serial.output.clear();
    // Full queue cannot prevent immediate stop; stale queued tones are dropped.
    for (unsigned i = 0; i < 8; ++i) command("tone 100");
    Serial.output.clear();
    Serial.feed("!");
    loop();
    EXPECT_EQ(Serial.output, kStop);
    for (const auto& frame : render(20)) EXPECT_EQ(frame.channel1, 0);
    // Large pastes are limited to 64 characters per loop and do not advance PCM time.
    Serial.output.clear();
    const auto sampleTime = playback.timeMs();
    Serial.feed(std::string(64, ' ') + "level 0\n");
    loop();
    EXPECT_EQ(Serial.available(), 8);
    EXPECT_EQ(Serial.output, "");
    EXPECT_EQ(playback.timeMs(), sampleTime);
    loop();
    EXPECT_EQ(Serial.output, "PCM amplitude 0.0%.\n");
    EXPECT_FLOAT_EQ(shared.level, 0);
    // The volume alias works through the same serial monitor/terminal path,
    // reaches the renderer, and preserves the tone when muted or unmuted.
    Serial.output.clear();
    Serial.feed("tone 1000\nvolume 0\n");
    loop();
    EXPECT_EQ(Serial.output, kTone + "PCM amplitude 0.0%.\n");
    for (const auto& frame : render(128)) EXPECT_EQ(frame.channel1, 0);
    EXPECT_EQ(shared.playback.mode, Mode::Tone);
    Serial.output.clear();
    Serial.feed("volume 100\nvolume\n");
    loop();
    EXPECT_EQ(Serial.output, "PCM amplitude 100.0%.\nPCM amplitude 100.0%.\n");
    const auto loud = render(128);
    EXPECT_TRUE(std::any_of(loud.begin(), loud.end(), [](const auto& f) { return f.channel1 != 0; }));
    EXPECT_FLOAT_EQ(oscillator.amplitude, 32767);
}
} // namespace

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    const int result = RUN_ALL_TESTS();
#ifdef PIO_UNIT_TESTING
    (void)result;
    return 0;
#else
    return result;
#endif
}
