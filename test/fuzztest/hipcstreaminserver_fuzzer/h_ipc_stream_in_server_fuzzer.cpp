/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include "token_setproc.h"
#include <fuzzer/FuzzedDataProvider.h>
#include "../fuzz_utils.h"
#include "ipc_stream_in_server.h"
using namespace std;
static int32_t NUM_32 = 32;
namespace OHOS {
namespace AudioStandard {

constexpr int32_t DEFAULT_STREAM_ID = 10;

static AudioProcessConfig InitProcessConfig()
{
    AudioProcessConfig config;
    config.appInfo.appUid = DEFAULT_STREAM_ID;
    config.appInfo.appPid = DEFAULT_STREAM_ID;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    return config;
}

void FuzzExecute(FuzzedDataProvider &provider)
{
    AudioProcessConfig config = InitProcessConfig();
    int32_t createRet = 0;
    sptr<IpcStreamInServer> ptrIpcStreamStub = IpcStreamInServer::Create(config, createRet);
    if (ptrIpcStreamStub == nullptr) {
        return;
    }
    ExecutePartOne(ptrIpcStreamStub, provider);
    ExecutePartTwo(ptrIpcStreamStub, provider);
}

void ExecutePartOne(sptr<IpcStreamInServer> ptrIpcStreamStub, FuzzedDataProvider &provider)
{
    ptrIpcStreamStub->Start();
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    ptrIpcStreamStub->GetAudioSessionID(sessionId);
    ptrIpcStreamStub->UpdatePosition();
    bool stopFlag = provider.ConsumeBool();
    ptrIpcStreamStub->Drain(stopFlag);
    uint64_t framePos = provider.ConsumeIntegral<uint64_t>();
    uint64_t timestamp = provider.ConsumeIntegral<uint64_t>();
    ptrIpcStreamStub->GetAudioTime(framePos, timestamp);
    uint64_t getFramePos = provider.ConsumeIntegral<uint64_t>();
    uint64_t getTimestamp = provider.ConsumeIntegral<uint64_t>();
    uint64_t getLatency = provider.ConsumeIntegral<uint64_t>();
    uint32_t getBase = provider.ConsumeIntegral<uint32_t>();
    ptrIpcStreamStub->GetAudioPosition(getFramePos, getTimestamp, getLatency, getBase);
    uint64_t latency = provider.ConsumeIntegral<uint64_t>();
    uint32_t base = provider.ConsumeIntegral<uint32_t>();
    ptrIpcStreamStub->GetSpeedPosition(framePos, timestamp, latency, base);
    ptrIpcStreamStub->GetLatency(latency);
    int32_t rate = provider.ConsumeIntegral<int32_t>();
    ptrIpcStreamStub->SetRate(rate);
    ptrIpcStreamStub->GetRate(rate);
    float volume = provider.ConsumeFloatingPoint<float>();
    ptrIpcStreamStub->SetLowPowerVolume(volume);
    ptrIpcStreamStub->GetLowPowerVolume(volume);
    int32_t privacyType = provider.ConsumeIntegral<int32_t>();
    ptrIpcStreamStub->SetPrivacyType(privacyType);
    ptrIpcStreamStub->GetPrivacyType(privacyType);
    int32_t state = provider.ConsumeIntegral<int32_t>();
    bool isAppBack = provider.ConsumeBool();
    ptrIpcStreamStub->SetOffloadMode(state, isAppBack);
    ptrIpcStreamStub->UnsetOffloadMode();
    uint64_t paWriteIndex = provider.ConsumeIntegral<uint64_t>();
    uint64_t cacheTimeDsp = provider.ConsumeIntegral<uint64_t>();
    uint64_t cacheTimePa = provider.ConsumeIntegral<uint64_t>();
    ptrIpcStreamStub->GetOffloadApproximatelyCacheTime(timestamp, paWriteIndex, cacheTimeDsp, cacheTimePa);
    ptrIpcStreamStub->GetStreamManagerType();
    bool on = provider.ConsumeBool();
    ptrIpcStreamStub->SetSilentModeAndMixWithOthers(on);
    ptrIpcStreamStub->SetClientVolume();
}

void ExecutePartTwo(sptr<IpcStreamInServer> ptrIpcStreamStub, FuzzedDataProvider &provider)
{
    bool isMute = provider.ConsumeBool();
    ptrIpcStreamStub->SetMute(isMute);
    float duckFactor = provider.ConsumeFloatingPoint<float>();
    uint32_t durationMs = provider.ConsumeIntegral<uint32_t>();
    ptrIpcStreamStub->SetDuckFactor(duckFactor, durationMs);
    int32_t defaultOutputDevice = provider.ConsumeIntegral<int32_t>();
    bool skipForce = provider.ConsumeBool();
    ptrIpcStreamStub->SetDefaultOutputDevice(defaultOutputDevice, skipForce);
    int64_t duration = provider.ConsumeIntegral<int64_t>();
    ptrIpcStreamStub->SetSourceDuration(duration);
    float speed = provider.ConsumeFloatingPoint<float>();
    ptrIpcStreamStub->SetSpeed(speed);
    int32_t offloadState = provider.ConsumeIntegral<int32_t>();
    ptrIpcStreamStub->SetOffloadDataCallbackState(offloadState);
    std::shared_ptr<OHAudioBufferBase> buffer;
    uint32_t spanSizeInFrame = provider.ConsumeIntegral<uint32_t>();
    uint64_t engineTotalSizeInFrame = provider.ConsumeIntegral<uint64_t>();
    ptrIpcStreamStub->ResolveBufferBaseAndGetServerSpanSize(buffer, spanSizeInFrame, engineTotalSizeInFrame);
    sptr<IRemoteObject> iRemoteObject = nullptr;
    ptrIpcStreamStub->RegisterStreamListener(iRemoteObject);
    std::shared_ptr<OHAudioBuffer> oHAudioBuffer;
    ptrIpcStreamStub->ResolveBuffer(oHAudioBuffer);
    AudioPlaybackCaptureConfig audioPlaybackCaptureConfig = {
        {
            {},
            FilterMode::INCLUDE,
            {},
            FilterMode::INCLUDE
        },
        false
    };
    audioPlaybackCaptureConfig.filterOptions.usages.emplace_back(STREAM_USAGE_MEDIA);
    audioPlaybackCaptureConfig.filterOptions.usages.emplace_back(STREAM_USAGE_ALARM);
    ptrIpcStreamStub->UpdatePlaybackCaptureConfig(audioPlaybackCaptureConfig);
    int32_t audioHapticsSyncId = provider.ConsumeIntegral<uint32_t>();
    ptrIpcStreamStub->SetAudioHapticsSyncId(audioHapticsSyncId);
    int32_t target = provider.ConsumeIntegral<uint32_t>();
    int32_t targetRet = provider.ConsumeIntegral<uint32_t>();
    ptrIpcStreamStub->SetTarget(target, targetRet);
    ptrIpcStreamStub->SetRebuildFlag();
    ptrIpcStreamStub->Flush();
    ptrIpcStreamStub->Pause();
    ptrIpcStreamStub->Stop();
    bool isSwitchStream = provider.ConsumeBool();
    ptrIpcStreamStub->Release(isSwitchStream);
}
void FuzzTest(FuzzedDataProvider &provider)
{
    auto func = provider.PickValueInArray({
        FuzzExecute,
    });
    func(provider);
}
} // namespace AudioStandard
} // namesapce OHOS

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    if (SetSelfTokenID(718336240uLL | (1uLL << NUM_32)) < 0) {
        return -1;
    }
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::FuzzTest(fdp);
    return 0;
}
