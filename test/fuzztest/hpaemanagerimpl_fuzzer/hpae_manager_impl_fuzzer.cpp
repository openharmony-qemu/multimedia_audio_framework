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
#include <string>
#include <thread>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <streambuf>
#include <algorithm>
#include <unistd.h>
#include "audio_errors.h"
#include "hpae_manager_impl.h"
#include "audio_info.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace OHOS::AudioStandard::HPAE;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
constexpr int32_t FRAME_LENGTH = 882;
constexpr int32_t TEST_STREAM_SESSION_ID = 123456;
static std::string g_rootPath = "/data/";
std::shared_ptr<HpaeManagerImpl> hpaeManagerImpl_ = std::make_shared<HpaeManagerImpl>();

typedef void (*TestFuncs)();

vector<HpaeStreamClassType> HpaeStreamClassTypeVec = {
    HPAE_STREAM_CLASS_TYPE_INVALID,
    HPAE_STREAM_CLASS_TYPE_PLAY,
    HPAE_STREAM_CLASS_TYPE_RECORD,
};

vector<AudioSpatialDeviceType> AudioSpatialDeviceTypeVec = {
    EARPHONE_TYPE_NONE,
    EARPHONE_TYPE_INEAR,
    EARPHONE_TYPE_HALF_INEAR,
    EARPHONE_TYPE_HEADPHONE,
    EARPHONE_TYPE_GLASSES,
    EARPHONE_TYPE_OTHERS,
};

vector<AudioSpatializationSceneType> AudioSpatializationSceneTypeVec {
    SPATIALIZATION_SCENE_TYPE_DEFAULT,
    SPATIALIZATION_SCENE_TYPE_MUSIC,
    SPATIALIZATION_SCENE_TYPE_MOVIE,
    SPATIALIZATION_SCENE_TYPE_AUDIOBOOK,
    SPATIALIZATION_SCENE_TYPE_MAX,
};

vector<SourceType> SourceTypeVec =  {
    SOURCE_TYPE_INVALID,
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VOICE_CALL,
    SOURCE_TYPE_VOICE_COMMUNICATION,
    SOURCE_TYPE_ULTRASONIC,
    SOURCE_TYPE_VIRTUAL_CAPTURE,
    SOURCE_TYPE_VOICE_MESSAGE,
    SOURCE_TYPE_REMOTE_CAST,
    SOURCE_TYPE_VOICE_TRANSCRIPTION,
    SOURCE_TYPE_CAMCORDER,
    SOURCE_TYPE_UNPROCESSED,
    SOURCE_TYPE_EC,
    SOURCE_TYPE_MIC_REF,
    SOURCE_TYPE_LIVE,
    SOURCE_TYPE_MAX
};

vector<DeviceType> DeviceTypeVec = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_INVALID,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WAKEUP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_DP,
    DEVICE_TYPE_REMOTE_CAST,
    DEVICE_TYPE_USB_DEVICE,
    DEVICE_TYPE_ACCESSORY,
    DEVICE_TYPE_REMOTE_DAUDIO,
    DEVICE_TYPE_HDMI,
    DEVICE_TYPE_LINE_DIGITAL,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_NEARLINK_IN,
    DEVICE_TYPE_FILE_SINK,
    DEVICE_TYPE_FILE_SOURCE,
    DEVICE_TYPE_EXTERN_CABLE,
    DEVICE_TYPE_DEFAULT,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_MAX,
};

bool Init()
{
    if (hpaeManagerImpl_ == nullptr) {
        return false;
    }
    hpaeManagerImpl_->manager_ = std::make_shared<HpaeManager>();
    if (hpaeManagerImpl_->manager_ == nullptr) {
        return false;
    }
    return true;
}

AudioModuleInfo GetSinkAudioModeInfo(std::string name = "Speaker_File")
{
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.lib = "libmodule-hdi-sink.z.so";
    audioModuleInfo.channels = "2";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.name = name;
    audioModuleInfo.adapterName = "file_io";
    audioModuleInfo.className = "file_io";
    audioModuleInfo.bufferSize = "7680";
    audioModuleInfo.format = "s32le";
    audioModuleInfo.fixedLatency = "1";
    audioModuleInfo.offloadEnable = "0";
    audioModuleInfo.networkId = "LocalDevice";
    audioModuleInfo.fileName = g_rootPath + audioModuleInfo.adapterName + "_" + audioModuleInfo.rate + "_" +
                               audioModuleInfo.channels + "_" + audioModuleInfo.format + ".pcm";
    std::stringstream typeValue;
    typeValue << static_cast<int32_t>(DEVICE_TYPE_SPEAKER);
    audioModuleInfo.deviceType = typeValue.str();
    return audioModuleInfo;
}

HPAE::HpaeStreamInfo GetRenderStreamInfo()
{
    HPAE::HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_44100;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.frameLen = FRAME_LENGTH;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE::HPAE_STREAM_CLASS_TYPE_PLAY;
    return streamInfo;
}

void DeInitFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    hpaeManagerImpl_->DeInit();
}

void DumpSinkInfoFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    std::string deviceName = "test_device";
    hpaeManagerImpl_->DumpSinkInfo(deviceName);
}

void DumpSourceInfoFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    std::string deviceName = "test_device";
    hpaeManagerImpl_->DumpSourceInfo(deviceName);
}

void DumpSinkInputsInfoFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    hpaeManagerImpl_->DumpSinkInputsInfo();
    hpaeManagerImpl_->DumpSourceOutputsInfo();
}

void ReloadAudioPortFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    hpaeManagerImpl_->ReloadAudioPort(audioModuleInfo);
}

void CloseAudioPortFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    int32_t audioHandleIndex = g_fuzzUtils.GetData<int32_t>();
    hpaeManagerImpl_->CloseAudioPort(audioHandleIndex);
}

void GetSinkInfoByIdxFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    HpaeSinkInfo sinkInfo;
    HpaeSourceInfo sourceInfo;
    int32_t ret = -1;
    hpaeManagerImpl_->GetSinkInfoByIdx(0, [&sinkInfo, &ret](const HpaeSinkInfo &sinkInfoRet, int32_t result) {
        sinkInfo = sinkInfoRet;
        ret = result;
    });
    ret = -1;
    hpaeManagerImpl_->GetSourceInfoByIdx(0, [&sourceInfo, &ret](const HpaeSourceInfo &sourceInfoRet, int32_t result) {
        sourceInfo  = sourceInfoRet;
        ret = result;
    });
}

void HandleMsgFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    hpaeManagerImpl_->HandleMsg();
}

void IsInitFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    hpaeManagerImpl_->IsInit();
}

void IsRunningFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    hpaeManagerImpl_->IsRunning();
}

void IsMsgProcessingFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    hpaeManagerImpl_->IsMsgProcessing();
}

void SetSourceOutputMuteFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    int32_t uid = g_fuzzUtils.GetData<int32_t>();
    bool setMute = g_fuzzUtils.GetData<bool>();
    hpaeManagerImpl_->SetSourceOutputMute(uid, setMute);
}

void StartWithSyncIdFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    uint32_t index = g_fuzzUtils.GetData<uint32_t>() % HpaeStreamClassTypeVec.size();
    HpaeStreamClassType streamClassType = HpaeStreamClassTypeVec[index];
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    int32_t syncId = g_fuzzUtils.GetData<int32_t>();
    hpaeManagerImpl_->StartWithSyncId(streamClassType, sessionId, syncId);
}

void ReleaseFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    uint32_t index = g_fuzzUtils.GetData<uint32_t>() % HpaeStreamClassTypeVec.size();
    HpaeStreamClassType streamClassType = HpaeStreamClassTypeVec[index];
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    hpaeManagerImpl_->Release(streamClassType, sessionId);
}

void GetSourceOutputInfoFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    hpaeManagerImpl_->GetSourceOutputInfo(sessionId, streamInfo);
}

void SetRateFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    int32_t rate = g_fuzzUtils.GetData<int32_t>();
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    hpaeManagerImpl_->SetRate(sessionId, rate);
}

void GetAudioEffectModeFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    int32_t effectMode = g_fuzzUtils.GetData<int32_t>();
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    hpaeManagerImpl_->SetAudioEffectMode(sessionId, effectMode);
    hpaeManagerImpl_->GetAudioEffectMode(sessionId, effectMode);
}

void SetPrivacyTypeFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    int32_t privacyType = g_fuzzUtils.GetData<int32_t>();
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    hpaeManagerImpl_->SetPrivacyType(sessionId, privacyType);
    hpaeManagerImpl_->GetPrivacyType(sessionId, privacyType);
}

void GetWritableSizeFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    hpaeManagerImpl_->GetWritableSize(sessionId);
}

void UpdateSpatializationStateFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    bool spatializationEnabled = g_fuzzUtils.GetData<bool>();
    bool headTrackingEnabled = g_fuzzUtils.GetData<bool>();
    hpaeManagerImpl_->UpdateSpatializationState(sessionId, spatializationEnabled, headTrackingEnabled);
}

void UpdateMaxLengthFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    uint32_t maxLength = g_fuzzUtils.GetData<uint32_t>();
    hpaeManagerImpl_->UpdateMaxLength(sessionId, maxLength);
}

void SetOffloadRenderCallbackTypeFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    int32_t type = g_fuzzUtils.GetData<int32_t>();
    hpaeManagerImpl_->SetOffloadRenderCallbackType(sessionId, type);
}

void SetSpeedFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    float speed = g_fuzzUtils.GetData<float>();
    hpaeManagerImpl_->SetSpeed(sessionId, speed);
}

void SetOutputDeviceSinkFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    int32_t device = g_fuzzUtils.GetData<int32_t>();
    const std::string sinkName = "test_sink";
    hpaeManagerImpl_->SetOutputDeviceSink(device, sinkName);
}

void UpdateSpatializationStateFuzzTest2()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    AudioSpatializationState stateInfo;
    stateInfo.headTrackingEnabled = g_fuzzUtils.GetData<bool>();
    stateInfo.spatializationEnabled = g_fuzzUtils.GetData<bool>();
    hpaeManagerImpl_->UpdateSpatializationState(stateInfo);
}

void UpdateSpatialDeviceTypeFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    uint32_t index = g_fuzzUtils.GetData<uint32_t>() % AudioSpatialDeviceTypeVec.size();
    AudioSpatialDeviceType audioSpatialDeviceType = AudioSpatialDeviceTypeVec[index];
    hpaeManagerImpl_->UpdateSpatialDeviceType(audioSpatialDeviceType);
}

void SetSpatializationSceneTypeFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    uint32_t index = g_fuzzUtils.GetData<uint32_t>() % AudioSpatializationSceneTypeVec.size();
    AudioSpatializationSceneType audioSpatializationSceneType = AudioSpatializationSceneTypeVec[index];
    hpaeManagerImpl_->SetSpatializationSceneType(audioSpatializationSceneType);
}

void EffectRotationUpdateFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    uint32_t rotationState = g_fuzzUtils.GetData<uint32_t>();
    hpaeManagerImpl_->EffectRotationUpdate(rotationState);
}

void SetEffectSystemVolumeFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    int32_t systemVolumeType = g_fuzzUtils.GetData<int32_t>();
    float systemVolume = g_fuzzUtils.GetData<float>();
    hpaeManagerImpl_->SetEffectSystemVolume(systemVolumeType, systemVolume);
}

void SetAbsVolumeStateToEffectFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    bool absVolumeState = g_fuzzUtils.GetData<bool>();
    hpaeManagerImpl_->SetAbsVolumeStateToEffect(absVolumeState);
}

void SetAudioEffectPropertyFuzzTest()
{
    if (!Init() || hpaeManagerImpl_ == nullptr) {
        return;
    }
    AudioEffectPropertyArrayV3 propertyArrayV3;
    AudioEffectPropertyArray propertyArray;
    hpaeManagerImpl_->SetAudioEffectProperty(propertyArrayV3);
    hpaeManagerImpl_->SetAudioEffectProperty(propertyArray);
}

void SetOutputDeviceFuzzTest()
{
    hpaeManagerImpl_ = std::make_shared<HpaeManagerImpl>();
    if (hpaeManagerImpl_ == nullptr) {
        return;
    }
    uint32_t renderId = g_fuzzUtils.GetData<uint32_t>();
    uint32_t index = g_fuzzUtils.GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType outputDevice = DeviceTypeVec[index];
    hpaeManagerImpl_->SetOutputDevice(renderId, outputDevice);
}

void SetVolumeInfoFuzzTest()
{
    hpaeManagerImpl_ = std::make_shared<HpaeManagerImpl>();
    if (hpaeManagerImpl_ == nullptr) {
        return;
    }
    std::vector<AudioVolumeType> validVolumeTypes;
    for (int32_t type = static_cast<int32_t>(AudioStreamType::STREAM_DEFAULT);
         type <= static_cast<int32_t>(AudioStreamType::STREAM_APP);
         ++type) {
        validVolumeTypes.push_back(static_cast<AudioVolumeType>(type));
    }
    uint32_t typeIndex = g_fuzzUtils.GetData<uint32_t>() % validVolumeTypes.size();
    AudioVolumeType volumeType = validVolumeTypes[typeIndex];
    float systemVol = g_fuzzUtils.GetData<float>();
    systemVol = std::clamp(systemVol, 0.0f, 1.0f);
    hpaeManagerImpl_->SetVolumeInfo(volumeType, systemVol);
}

void SetMicrophoneMuteInfoFuzzTest()
{
    hpaeManagerImpl_ = std::make_shared<HpaeManagerImpl>();
    if (hpaeManagerImpl_ == nullptr) {
        return;
    }
    bool isMute = g_fuzzUtils.GetData<bool>();
    hpaeManagerImpl_->SetMicrophoneMuteInfo(isMute);
}

void SetAudioEnhancePropertyFuzzTest()
{
    hpaeManagerImpl_ = std::make_shared<HpaeManagerImpl>();
    if (hpaeManagerImpl_ == nullptr) {
        return;
    }
    AudioEffectPropertyArrayV3 propertyArrayV3;
    AudioEnhancePropertyArray propertyArray;
    uint32_t index = g_fuzzUtils.GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType outputDevice = DeviceTypeVec[index];
    hpaeManagerImpl_->SetAudioEnhanceProperty(propertyArrayV3, outputDevice);
    hpaeManagerImpl_->SetAudioEnhanceProperty(propertyArray, outputDevice);
}

void UpdateExtraSceneTypeFuzzTest()
{
    hpaeManagerImpl_ = std::make_shared<HpaeManagerImpl>();
    if (hpaeManagerImpl_ == nullptr) {
        return;
    }
    std::string mainkey = "audio_effect";
    std::string subkey = "extra_scene_type";
    std::string extraSceneType = "default";
    hpaeManagerImpl_->UpdateExtraSceneType(mainkey, subkey, extraSceneType);
}

void NotifySettingsDataReadyFuzzTest()
{
    hpaeManagerImpl_ = std::make_shared<HpaeManagerImpl>();
    if (hpaeManagerImpl_ == nullptr) {
        return;
    }
    hpaeManagerImpl_->NotifySettingsDataReady();
}

void NotifyAccountsChangedFuzzTest()
{
    hpaeManagerImpl_ = std::make_shared<HpaeManagerImpl>();
    if (hpaeManagerImpl_ == nullptr) {
        return;
    }
    hpaeManagerImpl_->NotifyAccountsChanged();
}

void IsAcousticEchoCancelerSupportedFuzzTest()
{
    hpaeManagerImpl_ = std::make_shared<HpaeManagerImpl>();
    if (hpaeManagerImpl_ == nullptr) {
        return;
    }
    uint32_t index = g_fuzzUtils.GetData<uint32_t>() % SourceTypeVec.size();
    SourceType sourceType = SourceTypeVec[index];
    hpaeManagerImpl_->IsAcousticEchoCancelerSupported(sourceType);
}

void UpdateCollaborativeStateFuzzTest()
{
    hpaeManagerImpl_ = std::make_shared<HpaeManagerImpl>();
    if (hpaeManagerImpl_ == nullptr) {
        return;
    }
    bool isCollaborationEnabled = g_fuzzUtils.GetData<bool>();
    hpaeManagerImpl_->UpdateCollaborativeState(isCollaborationEnabled);
}

vector<TestFuncs> g_testFuncs = {
    DeInitFuzzTest,
    DumpSinkInfoFuzzTest,
    DumpSourceInfoFuzzTest,
    DumpSinkInputsInfoFuzzTest,
    ReloadAudioPortFuzzTest,
    CloseAudioPortFuzzTest,
    GetSinkInfoByIdxFuzzTest,
    HandleMsgFuzzTest,
    IsInitFuzzTest,
    IsRunningFuzzTest,
    IsMsgProcessingFuzzTest,
    SetSourceOutputMuteFuzzTest,
    StartWithSyncIdFuzzTest,
    ReleaseFuzzTest,
    GetSourceOutputInfoFuzzTest,
    SetRateFuzzTest,
    GetAudioEffectModeFuzzTest,
    SetPrivacyTypeFuzzTest,
    GetWritableSizeFuzzTest,
    UpdateSpatializationStateFuzzTest,
    UpdateMaxLengthFuzzTest,
    SetOffloadRenderCallbackTypeFuzzTest,
    SetSpeedFuzzTest,
    SetOutputDeviceSinkFuzzTest,
    UpdateSpatializationStateFuzzTest2,
    UpdateSpatialDeviceTypeFuzzTest,
    SetSpatializationSceneTypeFuzzTest,
    EffectRotationUpdateFuzzTest,
    SetEffectSystemVolumeFuzzTest,
    SetAbsVolumeStateToEffectFuzzTest,
    SetAudioEffectPropertyFuzzTest,
    SetOutputDeviceFuzzTest,
    SetVolumeInfoFuzzTest,
    SetMicrophoneMuteInfoFuzzTest,
    SetAudioEnhancePropertyFuzzTest,
    UpdateExtraSceneTypeFuzzTest,
    NotifySettingsDataReadyFuzzTest,
    NotifyAccountsChangedFuzzTest,
    IsAcousticEchoCancelerSupportedFuzzTest,
    UpdateCollaborativeStateFuzzTest,
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