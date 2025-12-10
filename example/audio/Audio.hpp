#ifndef EMS_AUDIO_HPP
#define EMS_AUDIO_HPP

#include <vector>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <miniaudio.h>


struct Note
{
    double ratio;
    uint32_t duration_ms;
};

template <typename Container>
static void generatePCM(const Container& notes, std::vector<float>& pcm, uint32_t sampleRate = 48000)
{
    pcm.clear();
    constexpr double baseFreq = 440.0; // A4
    const uint32_t fadeSamples = sampleRate / 200; // ~5ms 淡入淡出

    for (const auto& n : notes)
    {
        constexpr double twoPi = 6.283185307179586;
        const double freq = baseFreq * n.ratio;
        const auto samples = static_cast<uint32_t>(static_cast<uint64_t>(n.duration_ms) * sampleRate / 1000);
        double phase = 0.0;
        const double phaseInc = twoPi * freq / sampleRate;

        size_t startIndex = pcm.size();
        pcm.resize(startIndex + samples);

        for (uint32_t i = 0; i < samples; ++i)
        {
            constexpr float amplitude = 0.2f;
            float env = 1.0f;
            if (i < fadeSamples)
            {
                env = static_cast<float>(i) / static_cast<float>(fadeSamples);
            }
            else if (i > samples - fadeSamples && samples > fadeSamples)
            {
                uint32_t tail = samples - i;
                env = static_cast<float>(tail) / static_cast<float>(fadeSamples);
            }
            pcm[startIndex + i] = amplitude * env * static_cast<float>(std::sin(phase));
            phase += phaseInc;
            if (phase > twoPi) phase -= twoPi;
        }

        // 音符间 1ms 静音
        const uint32_t gap = sampleRate / 1000;
        pcm.resize(pcm.size() + gap, 0.0f);
    }
}

template <typename Container>
static int playMelody(const Container& notes, uint32_t sampleRate = 48000)
{
    std::vector<float> pcm;
    generatePCM(notes, pcm, sampleRate);

    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_f32;
    config.playback.channels = 1;
    config.sampleRate = sampleRate;

    struct PlaybackState
    {
        const float* data = nullptr;
        size_t total = 0;
        size_t cursor = 0;
    } state{pcm.data(), pcm.size(), 0};

    config.dataCallback = [](ma_device* device, void* out, const void*, ma_uint32 frameCount)
    {
        auto* st = static_cast<PlaybackState*>(device->pUserData);
        auto* dst = static_cast<float*>(out);
        const auto samplesToWrite = static_cast<size_t>(frameCount); // 单声道
        const size_t samplesLeft = st->total - st->cursor;
        size_t n = samplesToWrite < samplesLeft ? samplesToWrite : samplesLeft;
        if (n > 0)
        {
            std::memcpy(dst, st->data + st->cursor, n * sizeof(float));
            st->cursor += n;
        }
        if (n < samplesToWrite)
        {
            std::memset(dst + n, 0, (samplesToWrite - n) * sizeof(float));
        }
    };
    config.pUserData = &state;

    ma_device device;
    if (ma_device_init(nullptr, &config, &device) != MA_SUCCESS)
    {
        std::cerr << "音频设备初始化失败\n";
        return 1;
    }
    if (ma_device_start(&device) != MA_SUCCESS)
    {
        std::cerr << "音频设备启动失败\n";
        ma_device_uninit(&device);
        return 1;
    }

    while (state.cursor < state.total)
    {
    }

    ma_device_uninit(&device);
    return 0;
}

#endif //EMS_AUDIO_HPP
