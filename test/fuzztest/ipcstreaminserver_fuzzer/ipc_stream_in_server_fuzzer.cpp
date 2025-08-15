/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ipc_stream_in_server.h"
#include "audio_info.h"
#include "hpae_renderer_stream_impl.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;

typedef void (*TestFuncs)();

static shared_ptr<IpcStreamInServer> CreateIpcStreamInServer()
{
    AudioProcessConfig configRet;
    AudioMode modeRet = g_fuzzUtils.GetData<AudioMode>();
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = make_shared<IpcStreamInServer>(configRet, modeRet);
    if (ipcStreamInServerRet == nullptr) {
        return nullptr;
    }
    ipcStreamInServerRet->streamListenerHolder_ = std::make_shared<StreamListenerHolder>();
    ipcStreamInServerRet->ConfigRenderer();
    if (ipcStreamInServerRet->rendererInServer_ == nullptr) {
        return ipcStreamInServerRet;
    }
    bool isMoveAble = g_fuzzUtils.GetData<bool>();
    ipcStreamInServerRet->rendererInServer_->stream_ = std::make_shared<HpaeRendererStreamImpl>(configRet, isMoveAble);
    ipcStreamInServerRet->ConfigCapturer();
    return ipcStreamInServerRet;
}

void StreamListenerHolderIsWakeUpLaterNeededFuzzTest()
{
    StreamListenerHolder streamListenerHolderRet;
    Operation operation = g_fuzzUtils.GetData<Operation>();
    streamListenerHolderRet.IsWakeUpLaterNeeded(operation);
}

void StreamListenerHolderRegisterStreamListenerFuzzTest()
{
    StreamListenerHolder streamListenerHolderRet;
    sptr<IIpcStreamListener> listener = nullptr;
    streamListenerHolderRet.RegisterStreamListener(listener);
}

void IpcStreamInServerConfigFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    ipcStreamInServerRet->Config();
}

void IpcStreamInServerGetRendererFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    ipcStreamInServerRet->GetRenderer();
}

void IpcStreamInServerGetCapturerFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    ipcStreamInServerRet->GetCapturer();
}

void IpcStreamInServerResolveBufferFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    std::shared_ptr<OHAudioBuffer> buffer;
    ipcStreamInServerRet->ResolveBuffer(buffer);
}

void IpcStreamInServerGetAudioSessionIDFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    ipcStreamInServerRet->GetAudioSessionID(sessionId);
}

void IpcStreamInServerStartFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    ipcStreamInServerRet->Start();
}

void IpcStreamInServerPauseFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    ipcStreamInServerRet->Pause();
}

void IpcStreamInServerStopFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    ipcStreamInServerRet->Stop();
}

void IpcStreamInServerReleaseFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    bool isSwitchStream = g_fuzzUtils.GetData<bool>();
    ipcStreamInServerRet->Release(isSwitchStream);
}

void IpcStreamInServerFlushFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    ipcStreamInServerRet->Flush();
}

void IpcStreamInServerDrainFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    bool stopFlag = g_fuzzUtils.GetData<bool>();
    ipcStreamInServerRet->Drain(stopFlag);
}

void IpcStreamInServerUpdatePlaybackCaptureConfigFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    AudioPlaybackCaptureConfig config;
    ipcStreamInServerRet->UpdatePlaybackCaptureConfig(config);
}

void IpcStreamInServerGetAudioTimeFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    uint64_t framePos = g_fuzzUtils.GetData<uint64_t>();
    uint64_t timestamp = g_fuzzUtils.GetData<uint64_t>();
    ipcStreamInServerRet->GetAudioTime(framePos, timestamp);
}

void IpcStreamInServerGetAudioPositionFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    uint64_t framePos = g_fuzzUtils.GetData<uint64_t>();
    uint64_t timestamp = g_fuzzUtils.GetData<uint64_t>();
    uint64_t latency = g_fuzzUtils.GetData<uint64_t>();
    int32_t base = g_fuzzUtils.GetData<int32_t>();
    ipcStreamInServerRet->GetAudioPosition(framePos, timestamp, latency, base);
}

void IpcStreamInServerGetSpeedPositionFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    uint64_t framePos = g_fuzzUtils.GetData<uint64_t>();
    uint64_t timestamp = g_fuzzUtils.GetData<uint64_t>();
    uint64_t latency = g_fuzzUtils.GetData<uint64_t>();
    int32_t base = g_fuzzUtils.GetData<int32_t>();
    ipcStreamInServerRet->GetSpeedPosition(framePos, timestamp, latency, base);
}

void IpcStreamInServerGetLatencyFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    uint64_t latency = g_fuzzUtils.GetData<uint64_t>();
    ipcStreamInServerRet->GetLatency(latency);
}

void IpcStreamInServerSetRateFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    int32_t rate = g_fuzzUtils.GetData<int32_t>();
    ipcStreamInServerRet->SetRate(rate);
}

void IpcStreamInServerGetRateFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    int32_t rate = g_fuzzUtils.GetData<int32_t>();
    ipcStreamInServerRet->GetRate(rate);
}

void IpcStreamInServerSetLowPowerVolumeFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    float volume = g_fuzzUtils.GetData<float>();
    ipcStreamInServerRet->SetLowPowerVolume(volume);
}

void IpcStreamInServerGetLowPowerVolumeFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    float volume = g_fuzzUtils.GetData<float>();
    ipcStreamInServerRet->GetLowPowerVolume(volume);
}

void IpcStreamInServerSetAudioEffectModeFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    int32_t effectMode = g_fuzzUtils.GetData<int32_t>();
    ipcStreamInServerRet->SetAudioEffectMode(effectMode);
}

void IpcStreamInServerGetAudioEffectModeFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    int32_t effectMode = g_fuzzUtils.GetData<int32_t>();
    ipcStreamInServerRet->GetAudioEffectMode(effectMode);
}

void IpcStreamInServerSetPrivacyTypeFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    int32_t privacyType = g_fuzzUtils.GetData<int32_t>();
    ipcStreamInServerRet->SetPrivacyType(privacyType);
}

void IpcStreamInServerGetPrivacyTypeFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    int32_t privacyType = g_fuzzUtils.GetData<int32_t>();
    ipcStreamInServerRet->GetPrivacyType(privacyType);
}

void IpcStreamInServerSetOffloadModeFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    int32_t state = g_fuzzUtils.GetData<int32_t>();
    bool isAppBack = g_fuzzUtils.GetData<bool>();
    ipcStreamInServerRet->SetOffloadMode(state, isAppBack);
}

void IpcStreamInServerUnsetOffloadModeFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    ipcStreamInServerRet->UnsetOffloadMode();
}

void IpcStreamInServerGetOffloadApproximatelyCacheTimeFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    uint64_t timestamp = g_fuzzUtils.GetData<uint64_t>();
    uint64_t paWriteIndex = g_fuzzUtils.GetData<uint64_t>();
    uint64_t cacheTimeDsp = g_fuzzUtils.GetData<uint64_t>();
    uint64_t cacheTimePa = g_fuzzUtils.GetData<uint64_t>();
    ipcStreamInServerRet->GetOffloadApproximatelyCacheTime(timestamp, paWriteIndex, cacheTimeDsp, cacheTimePa);
}

void IpcStreamInServerUpdateSpatializationStateFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    bool spatializationEnabled = g_fuzzUtils.GetData<bool>();
    bool headTrackingEnabled = g_fuzzUtils.GetData<bool>();
    ipcStreamInServerRet->UpdateSpatializationState(spatializationEnabled, headTrackingEnabled);
}

void IpcStreamInServerGetStreamManagerTypeFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    ipcStreamInServerRet->GetStreamManagerType();
}

void IpcStreamInServerSetSilentModeAndMixWithOthersFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    bool on = g_fuzzUtils.GetData<bool>();
    ipcStreamInServerRet->SetClientVolume();
    ipcStreamInServerRet->SetSilentModeAndMixWithOthers(on);
}

void IpcStreamInServerSetLoudnessGainFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    float loudnessGain = g_fuzzUtils.GetData<float>();
    ipcStreamInServerRet->SetLoudnessGain(loudnessGain);
}

void IpcStreamInServerSetMuteFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    bool isMute = g_fuzzUtils.GetData<bool>();
    ipcStreamInServerRet->SetMute(isMute);
}

void IpcStreamInServerSetDuckFactorFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    float duckFactor = g_fuzzUtils.GetData<float>();
    ipcStreamInServerRet->SetDuckFactor(duckFactor);
}

void IpcStreamInServerRegisterThreadPriorityFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    int32_t tid = g_fuzzUtils.GetData<int32_t>();
    std::string bundleName = "testBundleName";
    uint32_t method = g_fuzzUtils.GetData<uint32_t>();
    ipcStreamInServerRet->RegisterThreadPriority(tid, bundleName, method);
}

void IpcStreamInServerSetDefaultOutputDeviceFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    int32_t defaultOutputDevice = g_fuzzUtils.GetData<int32_t>();
    ipcStreamInServerRet->SetDefaultOutputDevice(defaultOutputDevice);
}

void IpcStreamInServerSetSourceDurationFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    int64_t duration = g_fuzzUtils.GetData<int64_t>();
    ipcStreamInServerRet->SetSourceDuration(duration);
}

void IpcStreamInServerSetSpeedFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    float speed = g_fuzzUtils.GetData<float>();
    ipcStreamInServerRet->SetSpeed(speed);
}

void IpcStreamInServerSetOffloadDataCallbackStateFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    int32_t state = g_fuzzUtils.GetData<int32_t>();
    ipcStreamInServerRet->SetOffloadDataCallbackState(state);
}

void IpcStreamInServerResolveBufferBaseAndGetServerSpanSizeFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    uint32_t totalSizeInFrame = 1;
    uint32_t byteSizePerFrame = 1;
    uint32_t spanSizeInFrame = g_fuzzUtils.GetData<uint32_t>();
    uint64_t engineTotalSizeInFrame = g_fuzzUtils.GetData<uint64_t>();
    std::shared_ptr<OHAudioBufferBase> buffer = OHAudioBufferBase::CreateFromLocal(totalSizeInFrame, byteSizePerFrame);

    ipcStreamInServerRet->ResolveBufferBaseAndGetServerSpanSize(buffer, spanSizeInFrame, engineTotalSizeInFrame);
}

void IpcStreamInServerSetAudioHapticsSyncIdFuzzTest()
{
    shared_ptr<IpcStreamInServer> ipcStreamInServerRet = CreateIpcStreamInServer();
    CHECK_AND_RETURN(ipcStreamInServerRet != nullptr);
    int32_t audioHapticsSyncId = g_fuzzUtils.GetData<int32_t>();

    ipcStreamInServerRet->SetAudioHapticsSyncId(audioHapticsSyncId);
}

vector<TestFuncs> g_testFuncs = {
    StreamListenerHolderIsWakeUpLaterNeededFuzzTest,
    StreamListenerHolderRegisterStreamListenerFuzzTest,
    IpcStreamInServerConfigFuzzTest,
    IpcStreamInServerGetRendererFuzzTest,
    IpcStreamInServerGetCapturerFuzzTest,
    IpcStreamInServerResolveBufferFuzzTest,
    IpcStreamInServerGetAudioSessionIDFuzzTest,
    IpcStreamInServerStartFuzzTest,
    IpcStreamInServerPauseFuzzTest,
    IpcStreamInServerStopFuzzTest,
    IpcStreamInServerReleaseFuzzTest,
    IpcStreamInServerFlushFuzzTest,
    IpcStreamInServerDrainFuzzTest,
    IpcStreamInServerUpdatePlaybackCaptureConfigFuzzTest,
    IpcStreamInServerGetAudioTimeFuzzTest,
    IpcStreamInServerGetAudioPositionFuzzTest,
    IpcStreamInServerGetSpeedPositionFuzzTest,
    IpcStreamInServerGetLatencyFuzzTest,
    IpcStreamInServerSetRateFuzzTest,
    IpcStreamInServerGetRateFuzzTest,
    IpcStreamInServerSetLowPowerVolumeFuzzTest,
    IpcStreamInServerGetLowPowerVolumeFuzzTest,
    IpcStreamInServerSetAudioEffectModeFuzzTest,
    IpcStreamInServerGetAudioEffectModeFuzzTest,
    IpcStreamInServerSetPrivacyTypeFuzzTest,
    IpcStreamInServerGetPrivacyTypeFuzzTest,
    IpcStreamInServerSetOffloadModeFuzzTest,
    IpcStreamInServerUnsetOffloadModeFuzzTest,
    IpcStreamInServerGetOffloadApproximatelyCacheTimeFuzzTest,
    IpcStreamInServerUpdateSpatializationStateFuzzTest,
    IpcStreamInServerGetStreamManagerTypeFuzzTest,
    IpcStreamInServerSetSilentModeAndMixWithOthersFuzzTest,
    IpcStreamInServerSetLoudnessGainFuzzTest,
    IpcStreamInServerSetMuteFuzzTest,
    IpcStreamInServerSetDuckFactorFuzzTest,
    IpcStreamInServerRegisterThreadPriorityFuzzTest,
    IpcStreamInServerSetDefaultOutputDeviceFuzzTest,
    IpcStreamInServerSetSourceDurationFuzzTest,
    IpcStreamInServerSetSpeedFuzzTest,
    IpcStreamInServerSetOffloadDataCallbackStateFuzzTest,
    IpcStreamInServerResolveBufferBaseAndGetServerSpanSizeFuzzTest,
    IpcStreamInServerSetAudioHapticsSyncIdFuzzTest,
};
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}