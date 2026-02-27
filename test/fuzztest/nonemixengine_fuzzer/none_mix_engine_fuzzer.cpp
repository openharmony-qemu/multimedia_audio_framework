/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <memory>
#include <vector>

#include "audio_errors.h"
#include "none_mix_engine.h"
#include "pro_renderer_stream_impl.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();

constexpr int32_t DEFAULT_STREAM_ID = 10;
constexpr uint32_t MIN_CHANNEL_COUNT = 1;
constexpr uint32_t MAX_CHANNEL_COUNT = 8;
constexpr uint32_t MAX_RENDERER_CHANNEL_COUNT = 2;
constexpr uint32_t MAX_FRAME_COUNT = 64;
constexpr uint32_t MAX_TEST_RENDER_ID = 4096;
constexpr int32_t ONE_BYTE = 1;
constexpr uint32_t MIN_FRAME_COUNT = 1;
constexpr uint32_t BUFFER_DEQUEUE_TIMEOUT_MS = 0;
constexpr size_t EMPTY_BUFFER_LENGTH = 0;
constexpr uint32_t WRITE_COUNT_MOD_BASE = 2;
constexpr uint32_t NEXT_STREAM_INDEX_OFFSET = 1;
constexpr uint32_t RESET_VALUE = 0;
constexpr int32_t FUZZ_TEST_SUCCESS = 0;

typedef void (*TestFuncs)();

const std::vector<AudioSamplingRate> g_audioSamplingRates = {
    SAMPLE_RATE_8000,
    SAMPLE_RATE_11025,
    SAMPLE_RATE_12000,
    SAMPLE_RATE_16000,
    SAMPLE_RATE_22050,
    SAMPLE_RATE_24000,
    SAMPLE_RATE_32000,
    SAMPLE_RATE_44100,
    SAMPLE_RATE_48000,
    SAMPLE_RATE_64000,
    SAMPLE_RATE_88200,
    SAMPLE_RATE_96000,
    SAMPLE_RATE_176400,
    SAMPLE_RATE_192000,
    SAMPLE_RATE_384000,
};

const std::vector<AudioSampleFormat> g_audioSampleFormats = {
    SAMPLE_U8,
    SAMPLE_S16LE,
    SAMPLE_S24LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE,
    INVALID_WIDTH,
};

const std::vector<AudioSampleFormat> g_rendererSampleFormats = {
    SAMPLE_S16LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE,
};

const std::vector<AudioStreamType> g_audioStreamTypes = {
    STREAM_DEFAULT,
    STREAM_MUSIC,
    STREAM_MEDIA,
    STREAM_VOICE_CALL,
    STREAM_VOICE_COMMUNICATION,
};

const std::vector<DeviceType> g_deviceTypes = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_INVALID,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_BLUETOOTH_SCO,
};

template <class T>
T PickValue(const std::vector<T> &values, T fallback)
{
    if (values.empty()) {
        return fallback;
    }
    return values[g_fuzzUtils.GetData<uint32_t>() % values.size()];
}

AudioProcessConfig BuildProcessConfig()
{
    AudioProcessConfig config {};
    config.appInfo.appUid = g_fuzzUtils.GetData<int32_t>();
    config.appInfo.appPid = g_fuzzUtils.GetData<int32_t>();
    config.streamInfo.format = PickValue(g_rendererSampleFormats, SAMPLE_S32LE);
    config.streamInfo.samplingRate = PickValue(g_audioSamplingRates, SAMPLE_RATE_48000);
    uint32_t channels = (g_fuzzUtils.GetData<uint32_t>() % MAX_RENDERER_CHANNEL_COUNT) + MIN_CHANNEL_COUNT;
    config.streamInfo.channels = static_cast<AudioChannel>(channels);
    config.streamInfo.channelLayout = channels >= STEREO ? AudioChannelLayout::CH_LAYOUT_STEREO :
        AudioChannelLayout::CH_LAYOUT_MONO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = PickValue(g_audioStreamTypes, STREAM_MUSIC);
    config.deviceType = PickValue(g_deviceTypes, DEVICE_TYPE_SPEAKER);
    return config;
}

AudioStreamInfo BuildStreamInfo()
{
    AudioStreamInfo streamInfo {};
    streamInfo.samplingRate = PickValue(g_audioSamplingRates, SAMPLE_RATE_48000);
    uint32_t channels = (g_fuzzUtils.GetData<uint32_t>() % MAX_CHANNEL_COUNT) + MIN_CHANNEL_COUNT;
    streamInfo.channels = static_cast<AudioChannel>(channels);
    streamInfo.channelLayout = channels >= STEREO ? AudioChannelLayout::CH_LAYOUT_STEREO :
        AudioChannelLayout::CH_LAYOUT_MONO;
    streamInfo.format = PickValue(g_audioSampleFormats, SAMPLE_S32LE);
    return streamInfo;
}

std::shared_ptr<ProRendererStreamImpl> CreateRendererStream(bool startStream, bool enqueueData)
{
    AudioProcessConfig config = BuildProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> stream = std::make_shared<ProRendererStreamImpl>(config, true);
    if (stream == nullptr) {
        return nullptr;
    }
    if (stream->InitParams() != SUCCESS) {
        return nullptr;
    }
    stream->SetStreamIndex(g_fuzzUtils.GetData<uint32_t>() + DEFAULT_STREAM_ID);
    if (startStream) {
        (void)stream->Start();
    }
    if (startStream && enqueueData) {
        BufferDesc bufferDesc = stream->DequeueBuffer(BUFFER_DEQUEUE_TIMEOUT_MS);
        if (bufferDesc.buffer != nullptr && bufferDesc.bufLength > EMPTY_BUFFER_LENGTH) {
            uint8_t fill = g_fuzzUtils.GetData<uint8_t>();
            std::fill_n(bufferDesc.buffer, bufferDesc.bufLength, fill);
            (void)stream->EnqueueBuffer(bufferDesc);
        }
    }
    return stream;
}

void NoneMixEngineInitFuzzTest()
{
    NoneMixEngine engine;
    AudioDeviceDescriptor firstType(AudioDeviceDescriptor::DEVICE_INFO);
    firstType.deviceType_ = PickValue(g_deviceTypes, DEVICE_TYPE_SPEAKER);
    (void)engine.Init(firstType, g_fuzzUtils.GetData<bool>());

    engine.isInit_ = true;
    engine.renderId_ = g_fuzzUtils.GetData<uint32_t>() % MAX_TEST_RENDER_ID;
    AudioDeviceDescriptor secondType(AudioDeviceDescriptor::DEVICE_INFO);
    secondType.deviceType_ = PickValue(g_deviceTypes, DEVICE_TYPE_USB_HEADSET);
    (void)engine.Init(secondType, g_fuzzUtils.GetData<bool>());
}

void NoneMixEngineStateApiFuzzTest()
{
    NoneMixEngine engine;
    AudioDeviceDescriptor type(AudioDeviceDescriptor::DEVICE_INFO);
    type.deviceType_ = PickValue(g_deviceTypes, DEVICE_TYPE_SPEAKER);
    (void)engine.Init(type, g_fuzzUtils.GetData<bool>());
    (void)engine.Start();
    (void)engine.Flush();
    (void)engine.GetLatency();
    (void)engine.IsPlaybackEngineRunning();
    (void)engine.Stop();
}

void NoneMixEnginePauseAndStopFuzzTest()
{
    auto engine = std::make_shared<NoneMixEngine>();
    if (engine == nullptr) {
        return;
    }
    engine->playbackThread_ = std::make_unique<AudioThreadTask>("noneMixPauseThread");
    engine->isStart_ = false;
    (void)engine->Pause();
    (void)engine->Stop();

    engine->playbackThread_ = std::make_unique<AudioThreadTask>("noneMixPauseAsyncThread");
    engine->isStart_ = true;
    (void)engine->Pause(true);
    engine->PauseAsync();
    (void)engine->StopAudioSink();
}

void NoneMixEngineFormatMapFuzzTest()
{
    NoneMixEngine engine;
    (void)engine.GetDirectSampleRate(PickValue(g_audioSamplingRates, SAMPLE_RATE_48000));
    (void)engine.GetDirectSampleRate(static_cast<AudioSamplingRate>(g_fuzzUtils.GetData<uint32_t>()));

    (void)engine.GetDirectVoipSampleRate(PickValue(g_audioSamplingRates, SAMPLE_RATE_16000));
    (void)engine.GetDirectVoipSampleRate(static_cast<AudioSamplingRate>(g_fuzzUtils.GetData<uint32_t>()));

    (void)engine.GetDirectDeviceFormate(PickValue(g_audioSampleFormats, SAMPLE_S16LE));
    (void)engine.GetDirectDeviceFormate(static_cast<AudioSampleFormat>(g_fuzzUtils.GetData<uint32_t>()));

    (void)engine.GetDirectVoipDeviceFormat(PickValue(g_audioSampleFormats, SAMPLE_S16LE));
    (void)engine.GetDirectVoipDeviceFormat(static_cast<AudioSampleFormat>(g_fuzzUtils.GetData<uint32_t>()));

    (void)engine.GetDirectFormatByteSize(PickValue(g_audioSampleFormats, SAMPLE_S16LE));
    (void)engine.GetDirectFormatByteSize(static_cast<AudioSampleFormat>(g_fuzzUtils.GetData<uint32_t>()));
}

void NoneMixEngineTargetStreamInfoFuzzTest()
{
    NoneMixEngine engine;
    AudioStreamInfo clientStreamInfo = BuildStreamInfo();
    uint32_t targetSampleRate = RESET_VALUE;
    uint32_t targetChannel = RESET_VALUE;
    AudioSampleFormat targetFormat = SAMPLE_S16LE;
    bool isVoip = g_fuzzUtils.GetData<bool>();
    engine.GetTargetSinkStreamInfo(clientStreamInfo, targetSampleRate, targetChannel, targetFormat, isVoip);
}

void NoneMixEngineFadeFuzzTest()
{
    NoneMixEngine engine;
    engine.uChannel_ = (g_fuzzUtils.GetData<uint32_t>() % WRITE_COUNT_MOD_BASE) + MIN_CHANNEL_COUNT;
    engine.uFormat_ = g_fuzzUtils.GetData<bool>() ? static_cast<int32_t>(sizeof(int16_t)) :
        static_cast<int32_t>(sizeof(int32_t));

    size_t frameCount = (g_fuzzUtils.GetData<uint32_t>() % MAX_FRAME_COUNT) + MIN_FRAME_COUNT;
    size_t bufferSize = frameCount * engine.uChannel_ * static_cast<size_t>(engine.uFormat_);
    std::vector<char> buffer(bufferSize, static_cast<char>(g_fuzzUtils.GetData<uint8_t>()));

    engine.DoFadeinOut(g_fuzzUtils.GetData<bool>(), buffer.data(), buffer.size());
    engine.uFormat_ = ONE_BYTE;
    engine.DoFadeinOut(g_fuzzUtils.GetData<bool>(), buffer.data(), buffer.size());
    engine.DoFadeinOut(g_fuzzUtils.GetData<bool>(), nullptr, EMPTY_BUFFER_LENGTH);
}

void NoneMixEngineMixStreamsNoStreamFuzzTest()
{
    NoneMixEngine engine;
    engine.fwkSyncTime_ = RESET_VALUE;
    engine.writeCount_ = g_fuzzUtils.GetData<uint32_t>() % WRITE_COUNT_MOD_BASE;
    engine.MixStreams();
    engine.StandbySleep();
}

void NoneMixEngineMixStreamsWithStreamFuzzTest()
{
    NoneMixEngine engine;
    std::shared_ptr<ProRendererStreamImpl> stream = CreateRendererStream(true, true);
    if (stream == nullptr) {
        return;
    }

    engine.stream_ = stream;
    engine.isVoip_ = g_fuzzUtils.GetData<bool>();
    engine.firstSetVolume_ = g_fuzzUtils.GetData<bool>();
    engine.fwkSyncTime_ = RESET_VALUE;
    engine.writeCount_ = RESET_VALUE;
    engine.uChannel_ = STEREO;
    engine.uFormat_ = static_cast<int32_t>(sizeof(int32_t));
    engine.startFadeout_.store(g_fuzzUtils.GetData<bool>());
    engine.startFadein_.store(!engine.startFadeout_.load() && g_fuzzUtils.GetData<bool>());
    engine.MixStreams();

    engine.startFadeout_.store(true);
    engine.startFadein_.store(false);
    stream->BlockStream();
    engine.MixStreams();
}

void NoneMixEngineRendererManageFuzzTest()
{
    auto engine = std::make_shared<NoneMixEngine>();
    std::shared_ptr<ProRendererStreamImpl> stream1 = CreateRendererStream(false, false);
    std::shared_ptr<ProRendererStreamImpl> stream2 = CreateRendererStream(false, false);
    if (engine == nullptr || stream1 == nullptr || stream2 == nullptr) {
        return;
    }

    stream1->SetStreamIndex(DEFAULT_STREAM_ID);
    stream2->SetStreamIndex(DEFAULT_STREAM_ID + NEXT_STREAM_INDEX_OFFSET);
    (void)engine->AddRenderer(stream1);
    engine->stream_ = stream1;
    (void)engine->AddRenderer(stream2);
    engine->RemoveRenderer(stream2);
    engine->RemoveRenderer(stream1);
    engine->RemoveRenderer(stream1);
}

void NoneMixEngineLatencyFetcherFuzzTest()
{
    NoneMixEngine engine;
    engine.isStart_ = false;
    (void)engine.GetLatency();

    engine.isStart_ = true;
    engine.latency_ = g_fuzzUtils.GetData<uint32_t>();
    (void)engine.GetLatency();

    engine.latency_ = RESET_VALUE;
    engine.renderId_ = g_fuzzUtils.GetData<uint32_t>() % MAX_TEST_RENDER_ID;
    (void)engine.GetLatency();
    engine.RegisterSinkLatencyFetcher(engine.renderId_);
    engine.RegisterSinkLatencyFetcherToStreamIfNeeded();

    engine.sinkLatencyFetcher_ = [] (uint32_t &latency) -> int32_t {
        latency = RESET_VALUE;
        return SUCCESS;
    };
    engine.stream_ = CreateRendererStream(false, false);
    engine.RegisterSinkLatencyFetcherToStreamIfNeeded();
}

void NoneMixEngineSinkFuzzTest()
{
    NoneMixEngine engine;
    AudioDeviceDescriptor type(AudioDeviceDescriptor::DEVICE_INFO);
    type.deviceType_ = PickValue(g_deviceTypes, DEVICE_TYPE_SPEAKER);
    (void)engine.Init(type, g_fuzzUtils.GetData<bool>());

    AudioStreamInfo streamInfo = BuildStreamInfo();
    (void)engine.InitSink(streamInfo);

    uint32_t channel = (g_fuzzUtils.GetData<uint32_t>() % WRITE_COUNT_MOD_BASE) + MIN_CHANNEL_COUNT;
    AudioSampleFormat format = PickValue(g_audioSampleFormats, SAMPLE_S16LE);
    uint32_t rate = static_cast<uint32_t>(PickValue(g_audioSamplingRates, SAMPLE_RATE_48000));
    (void)engine.InitSink(channel, format, rate);
    (void)engine.SwitchSink(streamInfo, g_fuzzUtils.GetData<bool>());
}

void NoneMixEngineAdjustVoipVolumeFuzzTest()
{
    NoneMixEngine engine;
    engine.isVoip_ = false;
    engine.AdjustVoipVolume();

    std::shared_ptr<ProRendererStreamImpl> stream = CreateRendererStream(false, false);
    if (stream == nullptr) {
        return;
    }
    engine.stream_ = stream;
    engine.isVoip_ = true;
    engine.firstSetVolume_ = true;
    engine.AdjustVoipVolume();

    engine.firstSetVolume_ = false;
    engine.AdjustVoipVolume();
}

void NoneMixEngineRenderFrameFuzzTest()
{
    NoneMixEngine engine;
    std::shared_ptr<ProRendererStreamImpl> stream = CreateRendererStream(false, false);
    if (stream == nullptr) {
        return;
    }
    engine.stream_ = stream;

    std::vector<char> audioBuffer((g_fuzzUtils.GetData<uint32_t>() % MAX_FRAME_COUNT) + MIN_FRAME_COUNT,
        static_cast<char>(g_fuzzUtils.GetData<uint8_t>()));
    engine.DoRenderFrame(audioBuffer, g_fuzzUtils.GetData<int32_t>(), g_fuzzUtils.GetData<int32_t>());
}

std::vector<TestFuncs> g_testFuncs = {
    NoneMixEngineInitFuzzTest,
    NoneMixEngineStateApiFuzzTest,
    NoneMixEnginePauseAndStopFuzzTest,
    NoneMixEngineFormatMapFuzzTest,
    NoneMixEngineTargetStreamInfoFuzzTest,
    NoneMixEngineFadeFuzzTest,
    NoneMixEngineMixStreamsNoStreamFuzzTest,
    NoneMixEngineMixStreamsWithStreamFuzzTest,
    NoneMixEngineRendererManageFuzzTest,
    NoneMixEngineLatencyFetcherFuzzTest,
    NoneMixEngineSinkFuzzTest,
    NoneMixEngineAdjustVoipVolumeFuzzTest,
    NoneMixEngineRenderFrameFuzzTest,
};
} // namespace AudioStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return OHOS::AudioStandard::FUZZ_TEST_SUCCESS;
}
