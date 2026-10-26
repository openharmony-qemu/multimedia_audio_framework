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
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

#include "audio_device_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_zone_info.h"
#include "i_hpae_manager.h"
#include "manager/hdi_adapter_manager.h"
#include "util/id_handler.h"

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int32_t SYSTEM_ABILITY_ID = 3009;
constexpr bool RUN_ON_CREATE = false;
constexpr size_t THRESHOLD = sizeof(uint64_t);
constexpr size_t MAX_STRING_LEN = 64;
constexpr float MIN_LOOPBACK_VOLUME = 0.0F;
constexpr float MAX_LOOPBACK_VOLUME = 1.0F;
constexpr double MIN_SEPARATION_VOLUME = 0.0;
constexpr double MAX_SEPARATION_VOLUME = 1.0;

bool g_hasServerInit = false;

template<typename T>
T ConsumeEnum(FuzzedDataProvider &fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

sptr<AudioPolicyServer> GetServerPtr()
{
    static sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (!g_hasServerInit && server != nullptr) {
        IdHandler::GetInstance();
        HdiAdapterManager::GetInstance();
        HPAE::IHpaeManager::GetHpaeManager().Init();
        server->OnStart();
        server->OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        server->OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
#endif
        server->OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        server->OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        server->OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
        server->audioPolicyService_.SetDefaultDeviceLoadFlag(true);
        g_hasServerInit = true;
    }
    return server;
}

void AudioPolicyServerBasicGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto server = GetServerPtr();
    if (server == nullptr) {
        return;
    }
    bool retBool = false;
    int32_t retInt = {};

    server->TranslateErrorCode(fdp.ConsumeIntegral<int32_t>());
    server->GetApiTargetVersion();
    server->IsPublishCalled();
    server->GetVirtualCall(fdp.ConsumeIntegral<int32_t>(), retBool);
    server->IsOtherMediaPlaying(retBool);
    server->GetAudioSceneFromAllZones(retInt);
}

void AudioPolicyServerCallbackGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto server = GetServerPtr();
    if (server == nullptr) {
        return;
    }
    bool enabled = fdp.ConsumeBool();

    server->SetRegisterInterruptCallbackFlag(fdp.ConsumeIntegral<uint32_t>(), enabled);
    server->SetRegisterStateChangeCallbackFlag(enabled);
    server->NotifyStreamSilentChange(fdp.ConsumeIntegral<uint32_t>(), fdp.ConsumeIntegral<uint32_t>());
    server->SetSourceOutputStreamMuteByStreamId(fdp.ConsumeIntegral<int32_t>(), enabled);
    server->SetAncoSourceOutputStreamMute(fdp.ConsumeIntegral<int32_t>(), enabled);
}

void AudioPolicyServerLoopbackGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto server = GetServerPtr();
    if (server == nullptr) {
        return;
    }
    bool retBool = false;
    float volume = {};
    auto device = std::make_shared<AudioDeviceDescriptor>();
    if (device == nullptr) {
        return;
    }
    device->deviceType_ = ConsumeEnum<DeviceType>(fdp);
    device->deviceRole_ = ConsumeEnum<DeviceRole>(fdp);

    server->SetLoopbackVolume(fdp.ConsumeFloatingPointInRange<float>(MIN_LOOPBACK_VOLUME, MAX_LOOPBACK_VOLUME),
        retBool);
    server->GetLoopbackVolume(volume, retBool);
    server->IsDeviceSupportLoopback(device);
}

void AudioPolicyServerVoiceAssistantGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto server = GetServerPtr();
    if (server == nullptr) {
        return;
    }
    sptr<IRemoteObject> remoteObject = nullptr;

    server->GetVADeviceBroker(remoteObject);
    server->GetVADeviceController(fdp.ConsumeRandomLengthString(MAX_STRING_LEN), remoteObject);
}

void AudioPolicyServerAudioZoneGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto server = GetServerPtr();
    if (server == nullptr) {
        return;
    }

    server->SetAudioSessionBehavior(fdp.ConsumeIntegral<uint32_t>());
    server->TriggerAudioZoneVolumeSync();
}

void AudioPolicyServerAudioSeparationGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto server = GetServerPtr();
    if (server == nullptr) {
        return;
    }
    bool retBool = false;

    server->IsAudioSeparationEffectSupported(retBool);
    server->SetAudioSeparationEffectEnabled(fdp.ConsumeBool(), fdp.ConsumeIntegral<int32_t>(),
        fdp.ConsumeIntegral<int64_t>());
    server->SetAudioSeparationEffectVolume(fdp.ConsumeIntegral<int32_t>(),
        fdp.ConsumeFloatingPointInRange<double>(MIN_SEPARATION_VOLUME, MAX_SEPARATION_VOLUME));
}

void AudioPolicyServerRoutingVolumeGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto server = GetServerPtr();
    if (server == nullptr) {
        return;
    }
    int32_t zoneId = {};
    AudioStreamType streamType = STREAM_DEFAULT;
    bool retBool = false;
    std::set<int32_t> deviceTypes = { fdp.ConsumeIntegral<int32_t>() };

    server->GetScreenStates();
    server->GetActiveAudioInterruptZone(zoneId, streamType);
    server->IsScreenOnAdjustVolume();
    server->GetSystemActiveVolumeTypeInternal(fdp.ConsumeIntegral<int32_t>());
    server->IsEnhancedRoutingAvailableForSelectMode(fdp.ConsumeIntegral<int32_t>());
    server->SetDeclaredDeviceTypes(deviceTypes);
    server->HandleRouteSelectorCallbackEnable(fdp.ConsumeBool());
    server->SetRingerModeInner(ConsumeEnum<AudioRingerMode>(fdp));
    server->SetMicrophoneMuteCommon(fdp.ConsumeBool(), fdp.ConsumeBool());
    server->IsMicrophoneMuteLegacy(retBool);
    server->SetAudioSceneInternal(ConsumeEnum<AudioScene>(fdp), fdp.ConsumeIntegral<int32_t>(),
        fdp.ConsumeIntegral<int32_t>());
    server->GetCurrentStreamInFocus(zoneId);
}

void AudioPolicyServerInterruptSessionGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto server = GetServerPtr();
    if (server == nullptr) {
        return;
    }
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamUsage = ConsumeEnum<StreamUsage>(fdp);
    audioInterrupt.contentType = ConsumeEnum<ContentType>(fdp);
    audioInterrupt.streamId = fdp.ConsumeIntegral<uint32_t>();
    audioInterrupt.uid = fdp.ConsumeIntegral<int32_t>();
    audioInterrupt.pid = fdp.ConsumeIntegral<int32_t>();
    std::shared_ptr<AudioSessionDebugInfo> sessionDebugInfo;

    server->VerifyUidAndPid(audioInterrupt.uid, audioInterrupt.pid);
    server->ValidateAudioInterrupt(audioInterrupt);
    server->OnAudioStreamRemoved(fdp.ConsumeIntegral<uint64_t>());
    server->GetSessionDebugInfo(sessionDebugInfo);
    server->SetAudioSessionCapturerMuteHint(fdp.ConsumeBool());
}

void AudioPolicyServerDebugSpatialGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto server = GetServerPtr();
    if (server == nullptr) {
        return;
    }
    std::string path;
    bool retBool = false;
    int32_t retInt = {};
    std::vector<AudioZoneMix> audioMixes;

    server->GetSystemSoundPath(fdp.ConsumeIntegral<int32_t>(), path);
    server->IsAdaptiveSpatialRenderingEnabled(fdp.ConsumeRandomLengthString(MAX_STRING_LEN), retBool);
    server->GetCurrentSpatialAudioSourceType(retInt);
    server->UpdateSafeVolumeByStrMode();
    server->SetCustomAudioMix(fdp.ConsumeRandomLengthString(MAX_STRING_LEN), audioMixes);
}

void Test(FuzzedDataProvider &fdp)
{
    auto func = fdp.PickValueInArray({
        AudioPolicyServerBasicGapFuzzTest,
        AudioPolicyServerCallbackGapFuzzTest,
        AudioPolicyServerLoopbackGapFuzzTest,
        AudioPolicyServerVoiceAssistantGapFuzzTest,
        AudioPolicyServerAudioZoneGapFuzzTest,
        AudioPolicyServerAudioSeparationGapFuzzTest,
        AudioPolicyServerRoutingVolumeGapFuzzTest,
        AudioPolicyServerInterruptSessionGapFuzzTest,
        AudioPolicyServerDebugSpatialGapFuzzTest,
    });
    func(fdp);
}
} // namespace
} // namespace AudioStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
