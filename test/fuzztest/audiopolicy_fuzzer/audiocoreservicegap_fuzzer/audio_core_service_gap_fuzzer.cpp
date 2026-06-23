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

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

#include "audio_core_service.h"
#include "audio_errors.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
namespace {
const size_t MAX_STRING_LEN = 64;
const size_t THRESHOLD = sizeof(uint64_t) + sizeof(uint16_t);
const int32_t BOOT_ANIMATION_UID = 20010041;

const std::vector<DeviceType> DEVICE_TYPES = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_REMOTE_CAST,
    DEVICE_TYPE_MIC,
};

const std::vector<StreamUsage> STREAM_USAGES = {
    STREAM_USAGE_UNKNOWN,
    STREAM_USAGE_MUSIC,
    STREAM_USAGE_MOVIE,
    STREAM_USAGE_VOICE_COMMUNICATION,
    STREAM_USAGE_VIDEO_COMMUNICATION,
    STREAM_USAGE_VOICE_MODEM_COMMUNICATION,
    STREAM_USAGE_INTERPHONE,
};

const std::vector<SourceType> SOURCE_TYPES = {
    SOURCE_TYPE_INVALID,
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_VOICE_COMMUNICATION,
    SOURCE_TYPE_VOICE_CALL,
    SOURCE_TYPE_ULTRASONIC,
    SOURCE_TYPE_REMOTE_CAST,
    SOURCE_TYPE_INTERPHONE,
};

template<typename T>
T PickValue(FuzzedDataProvider &fdp, const std::vector<T> &values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
}

template<typename T>
T ConsumeEnum(FuzzedDataProvider &fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

std::shared_ptr<AudioDeviceDescriptor> MakeDevice(FuzzedDataProvider &fdp, DeviceRole role)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    if (desc == nullptr) {
        return nullptr;
    }
    desc->deviceType_ = PickValue(fdp, DEVICE_TYPES);
    desc->deviceRole_ = role;
    desc->networkId_ = fdp.ConsumeBool() ? LOCAL_NETWORK_ID : fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    desc->macAddress_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    desc->connectState_ = fdp.ConsumeBool() ? CONNECTED : VIRTUAL_CONNECTED;
    return desc;
}

std::shared_ptr<AudioStreamDescriptor> MakeRendererDesc(FuzzedDataProvider &fdp)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (streamDesc == nullptr) {
        return nullptr;
    }
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->sessionId_ = fdp.ConsumeIntegral<uint32_t>();
    streamDesc->callerUid_ = fdp.ConsumeIntegral<int32_t>();
    streamDesc->callerPid_ = fdp.ConsumeIntegral<int32_t>();
    streamDesc->appInfo_.appUid = streamDesc->callerUid_;
    streamDesc->appInfo_.appPid = streamDesc->callerPid_;
    streamDesc->rendererInfo_.streamUsage = PickValue(fdp, STREAM_USAGES);
    streamDesc->rendererInfo_.originalFlag = fdp.ConsumeBool() ? AUDIO_FLAG_FORCED_NORMAL : AUDIO_FLAG_NORMAL;
    streamDesc->rendererInfo_.rendererFlags = fdp.ConsumeBool() ? AUDIO_FLAG_FORCED_NORMAL : AUDIO_FLAG_NORMAL;
    streamDesc->rendererInfo_.forceToNormal = fdp.ConsumeBool();
    streamDesc->rendererInfo_.isOffloadAllowed = fdp.ConsumeBool();
    streamDesc->streamInfo_.samplingRate = SAMPLE_RATE_48000;
    streamDesc->streamInfo_.format = SAMPLE_S16LE;
    streamDesc->streamInfo_.channels = fdp.ConsumeBool() ? STEREO : CHANNEL_6;
    streamDesc->streamInfo_.encoding = fdp.ConsumeBool() ? ENCODING_AUDIOVIVID : ENCODING_PCM;
    streamDesc->bundleName_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    streamDesc->newDeviceDescs_.push_back(MakeDevice(fdp, OUTPUT_DEVICE));
    return streamDesc;
}

std::shared_ptr<AudioStreamDescriptor> MakeCapturerDesc(FuzzedDataProvider &fdp)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (streamDesc == nullptr) {
        return nullptr;
    }
    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->sessionId_ = fdp.ConsumeIntegral<uint32_t>();
    streamDesc->callerUid_ = fdp.ConsumeIntegral<int32_t>();
    streamDesc->callerPid_ = fdp.ConsumeIntegral<int32_t>();
    streamDesc->appInfo_.appUid = streamDesc->callerUid_;
    streamDesc->appInfo_.appPid = streamDesc->callerPid_;
    streamDesc->capturerInfo_.sourceType = PickValue(fdp, SOURCE_TYPES);
    streamDesc->capturerInfo_.originalFlag = fdp.ConsumeBool() ? AUDIO_FLAG_FORCED_NORMAL : AUDIO_FLAG_NORMAL;
    streamDesc->capturerInfo_.capturerFlags = fdp.ConsumeBool() ? AUDIO_FLAG_FORCED_NORMAL : AUDIO_FLAG_NORMAL;
    streamDesc->streamStatus_ = fdp.ConsumeBool() ? STREAM_STATUS_STARTED : STREAM_STATUS_NEW;
    streamDesc->bundleName_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    streamDesc->voipPrivacyMuted_.store(false);
    streamDesc->newDeviceDescs_.push_back(MakeDevice(fdp, INPUT_DEVICE));
    return streamDesc;
}

void AudioCoreServiceBasicGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }

    audioCoreService->RecordBootStateTime(ConsumeEnum<AudioPolicyServerBootState>(fdp),
        fdp.ConsumeIntegral<int64_t>());
    audioCoreService->RecordBootStateTime(ConsumeEnum<BootAnimationState>(fdp), BOOT_ANIMATION_UID,
        fdp.ConsumeIntegral<int64_t>());
    audioCoreService->GetEventEntry();

    std::shared_ptr<AsyncActionHandler> asyncHandler = nullptr;
    audioCoreService->SetAsyncActionHandler(asyncHandler);
    audioCoreService->GetAsyncActionHandler();
    audioCoreService->SetConfigParserFlag();
    audioCoreService->SetEcAndMicRefEnableState(fdp.ConsumeIntegral<int32_t>(), fdp.ConsumeIntegral<int32_t>());

    std::string dumpString;
    audioCoreService->DumpSelectHistory(dumpString);
    audioCoreService->RecordSelectDevice(fdp.ConsumeRandomLengthString(MAX_STRING_LEN));
}

void AudioCoreServiceModuleInfoGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }

    AudioModuleInfo moduleInfo;
    moduleInfo.name = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    moduleInfo.rate = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    moduleInfo.channels = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    audioCoreService->SetUsbSinkModuleInfo(moduleInfo);
    audioCoreService->SetUsbSourceModuleInfo(moduleInfo);
    audioCoreService->ClearModuleInfoForEc(moduleInfo);
    audioCoreService->ClearModuleInfoForMicRef(moduleInfo);
    audioCoreService->UpdateModuleInfoDeviceType(moduleInfo, nullptr);
    auto inputDesc = MakeDevice(fdp, INPUT_DEVICE);
    audioCoreService->UpdateModuleInfoDeviceType(moduleInfo, inputDesc);
}

void AudioCoreServicePlaybackFlagGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    auto streamDesc = MakeRendererDesc(fdp);
    if (streamDesc == nullptr) {
        return;
    }

    uint32_t audioFlag = AUDIO_FLAG_NONE;
    audioCoreService->CheckStreamConflicts(streamDesc, audioFlag);
    audioCoreService->IsStreamSupportMultiChannel(streamDesc);
    audioCoreService->IsStreamSupportDirect(streamDesc);
    audioCoreService->IsStreamSupportOutputInterPhone(streamDesc);
    audioCoreService->IsForcedNormal(streamDesc);
    audioCoreService->IsHWDecoding(streamDesc);
    audioCoreService->SetFlagForSpecialStream(streamDesc, fdp.ConsumeBool());
    audioCoreService->UpdatePlaybackStreamFlag(streamDesc, fdp.ConsumeBool());
    audioCoreService->CheckStaticModeAndSelectFlag(streamDesc);
}

void AudioCoreServiceRecordFlagGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    auto streamDesc = MakeCapturerDesc(fdp);
    if (streamDesc == nullptr) {
        return;
    }

    audioCoreService->CheckCapturerStreamConflicts(streamDesc);
    audioCoreService->RecordIsForcedNormal(streamDesc);
    audioCoreService->UpdateRecordStreamInfo(streamDesc);

    AudioStreamDescriptor runningStream;
    runningStream.capturerInfo_.sourceType = PickValue(fdp, SOURCE_TYPES);
    audioCoreService->ReloadCheckForStart(fdp.ConsumeBool(), streamDesc, runningStream);
    audioCoreService->IsHigherPrioritySourceType(streamDesc->capturerInfo_.sourceType,
        runningStream.capturerInfo_.sourceType);
}

void AudioCoreServiceClientStateGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }

    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    audioCoreService->PauseClient(sessionId);
    audioCoreService->StandbyClient(sessionId);
    audioCoreService->NotifyStandbyStatus(sessionId, fdp.ConsumeBool());
    audioCoreService->StopClient(sessionId);
    audioCoreService->ReleaseClient(sessionId, ConsumeEnum<SessionOperationMsg>(fdp), fdp.ConsumeBool());
    audioCoreService->ForceStopInterphoneRenderer();
}

void AudioCoreServiceSelectionGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }

    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    audioCoreService->GetSelectedInputDeviceByUid(uid);
    audioCoreService->ClearSelectedInputDeviceByUid(uid);
    audioCoreService->PreferBluetoothAndNearlinkRecordByUid(
        uid, ConsumeEnum<BluetoothAndNearlinkPreferredRecordCategory>(fdp));
    audioCoreService->FetchOutputDeviceAndRoute(fdp.ConsumeRandomLengthString(MAX_STRING_LEN));
    audioCoreService->FetchInputDeviceAndRoute(fdp.ConsumeRandomLengthString(MAX_STRING_LEN));
    audioCoreService->DeactivateRemoteDevice(LOCAL_NETWORK_ID, PickValue(fdp, DEVICE_TYPES));
    audioCoreService->NotifyRemoteDeviceStatusUpdate(nullptr);
}

void AudioCoreServiceVoipPrivacyGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    auto streamDesc = MakeCapturerDesc(fdp);
    if (streamDesc == nullptr) {
        return;
    }

    bool hasActiveVoipPrivacyCapturer = fdp.ConsumeBool();
    audioCoreService->IsVoipPrivacyMuteCandidateSource(streamDesc->capturerInfo_.sourceType);
    audioCoreService->IsVoipPrivacyMuteBypassedForUltrasonic(streamDesc->capturerInfo_.sourceType);
    audioCoreService->ShouldMuteCapturerByVoipPrivacy(streamDesc, hasActiveVoipPrivacyCapturer);
    audioCoreService->WriteVoipMutedCaptureStats(streamDesc, fdp.ConsumeBool());

    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs = { nullptr, streamDesc };
    audioCoreService->EvaluateActiveVoipPrivacyPresence(streamDescs, hasActiveVoipPrivacyCapturer);
    std::unordered_set<uint32_t> targetMuteSessions;
    audioCoreService->CollectSessionsToMuteByVoipPrivacy(streamDescs, hasActiveVoipPrivacyCapturer,
        targetMuteSessions);
    targetMuteSessions.clear();
    audioCoreService->SyncVoipPrivacyMuteState(streamDescs, targetMuteSessions);
    audioCoreService->NotifyVoipPriorityLoweredByMute(streamDesc->sessionId_, fdp.ConsumeBool());
}

void AudioCoreServiceVolumeGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::vector<ActiveStreamVolumeInfo> activeStreamsVolumeInfo;
    if (fdp.ConsumeBool()) {
        activeStreamsVolumeInfo.push_back({STREAM_MUSIC, fdp.ConsumeIntegral<int32_t>(),
            fdp.ConsumeIntegral<int32_t>()});
    }
    audioCoreService->GetActiveStreamsVolumeInfo(activeStreamsVolumeInfo);
}

void Test(FuzzedDataProvider &fdp)
{
    auto func = fdp.PickValueInArray({
        AudioCoreServiceBasicGapFuzzTest,
        AudioCoreServiceModuleInfoGapFuzzTest,
        AudioCoreServicePlaybackFlagGapFuzzTest,
        AudioCoreServiceRecordFlagGapFuzzTest,
        AudioCoreServiceClientStateGapFuzzTest,
        AudioCoreServiceSelectionGapFuzzTest,
        AudioCoreServiceVoipPrivacyGapFuzzTest,
        AudioCoreServiceVolumeGapFuzzTest,
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
