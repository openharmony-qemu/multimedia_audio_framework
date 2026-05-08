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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"

#include "audio_policy_utils.h"
#include "audio_stream_descriptor.h"
#include "audio_limiter_manager.h"
#include "dfx_msg_manager.h"

#include "audio_source_clock.h"
#include "capturer_clock_manager.h"
#include "hpae_policy_manager.h"
#include "audio_policy_state_monitor.h"
#include "audio_device_info.h"
#include "audio_spatialization_service.h"
#include "suspend/sync_sleep_callback_ipc_interface_code.h"
#include "hibernate/sync_hibernate_callback_ipc_interface_code.h"
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <vector>
namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;

typedef void (*TestFuncs)();

template<typename T>
T ConsumeEnum(FuzzedDataProvider& fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

template<typename T>
const T& PickValue(FuzzedDataProvider& fdp, const std::vector<T>& values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
}

void GetEcSamplingRateFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<const char*> deviceList = {
        USB_CLASS,
        DP_CLASS,
    };
    string halName = PickValue(fdp, deviceList);
    std::shared_ptr<PipeStreamPropInfo> outModuleInfo = std::make_shared<PipeStreamPropInfo>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    outModuleInfo->sampleRate_ = fdp.ConsumeIntegral<uint32_t>();
    ecManager.GetEcSamplingRate(halName, outModuleInfo);
}

void GetEcChannelsFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<const char*> deviceList = {
        USB_CLASS,
        DP_CLASS,
    };
    string halName = PickValue(fdp, deviceList);
    std::shared_ptr<PipeStreamPropInfo> outModuleInfo = std::make_shared<PipeStreamPropInfo>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    outModuleInfo->channelLayout_ = CH_LAYOUT_STEREO;
    std::vector<std::string> insertList = {"", to_string(fdp.ConsumeIntegral<uint32_t>())};
    ecManager.dpSinkModuleInfo_.channels = PickValue(fdp, insertList);
    ecManager.GetEcChannels(halName, outModuleInfo);
}

void CloseNormalSourceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.CloseNormalSource();
}

void GetEcFormatFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<const char*> deviceList = {
        USB_CLASS,
        DP_CLASS,
    };
    string halName = PickValue(fdp, deviceList);
    std::shared_ptr<PipeStreamPropInfo> outModuleInfo = std::make_shared<PipeStreamPropInfo>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    outModuleInfo->format_ = SAMPLE_S32LE;
    std::vector<std::string> insertList = {"", to_string(fdp.ConsumeIntegral<uint32_t>())};
    ecManager.dpSinkModuleInfo_.format = PickValue(fdp, insertList);
    ecManager.GetEcFormat(halName, outModuleInfo);
}

void UpdatePrimaryMicModuleInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SourceType source = ConsumeEnum<SourceType>(fdp);
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    CHECK_AND_RETURN(pipeInfo != nullptr);
    ecManager.UpdatePrimaryMicModuleInfo(pipeInfo, source);
}

void UpdateEnhanceEffectStateFuzzTest(FuzzedDataProvider& fdp)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SourceType source = ConsumeEnum<SourceType>(fdp);
    ecManager.UpdateEnhanceEffectState(source);
}

void GetPipeNameByDeviceForEcFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<string> roleList = {
        "source",
        "role_source",
    };
    DeviceType deviceType = ConsumeEnum<DeviceType>(fdp);
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetPipeNameByDeviceForEc(PickValue(fdp, roleList), deviceType);
}

void UpdateStreamCommonInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    int32_t ecEnableState = fdp.ConsumeBool();
    int32_t micRefEnableState = 0;
    ecManager.Init(ecEnableState, micRefEnableState);
    AudioModuleInfo moduleInfo;
    PipeStreamPropInfo targetInfo;
    SourceType sourceType = ConsumeEnum<SourceType>(fdp);
    ecManager.UpdateStreamCommonInfo(moduleInfo, targetInfo, sourceType);
}

void GetEcTypeFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType inputDevice = ConsumeEnum<DeviceType>(fdp);
    DeviceType outputDevice = ConsumeEnum<DeviceType>(fdp);
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetEcType(inputDevice, outputDevice);
}

void GetPipeInfoByDeviceTypeForEcFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<string> roleList = {
        "source",
        "role_source",
    };
    DeviceType deviceType = ConsumeEnum<DeviceType>(fdp);
    std::shared_ptr<AdapterPipeInfo> pipeInfo = std::make_shared<AdapterPipeInfo>();
    CHECK_AND_RETURN(pipeInfo != nullptr);
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetPipeInfoByDeviceTypeForEc(PickValue(fdp, roleList), deviceType, pipeInfo);
}

void ShouldOpenMicRefFuzzTest(FuzzedDataProvider& fdp)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SourceType source = ConsumeEnum<SourceType>(fdp);
    int32_t ecEnableState = 0;
    int32_t micRefEnableState = fdp.ConsumeBool();
    ecManager.Init(ecEnableState, micRefEnableState);
    ecManager.ShouldOpenMicRef(source);
}

void UpdateAudioEcInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceDescriptor inputDevice;
    AudioDeviceDescriptor outputDevice;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    int32_t ecEnableState = fdp.ConsumeBool();
    int32_t micRefEnableState = 0;
    ecManager.Init(ecEnableState, micRefEnableState);
    ecManager.UpdateAudioEcInfo(inputDevice, outputDevice);
}

void GetAudioEcInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetAudioEcInfo();
    ecManager.ResetAudioEcInfo();
}

void PresetArmIdleInputFuzzTest(FuzzedDataProvider& fdp)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    int32_t ecEnableState = fdp.ConsumeBool();
    ecManager.Init(ecEnableState, 0);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    ecManager.PresetArmIdleInput(deviceDesc);
}

void CloseUsbArmDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceDescriptor device;
    device.deviceRole_ = ConsumeEnum<DeviceRole>(fdp);
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.CloseUsbArmDevice(device);
}

void UpdateArmModuleInfoFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceRole role = ConsumeEnum<DeviceRole>(fdp);
    AudioModuleInfo moduleInfo;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    int32_t ecEnableState = fdp.ConsumeBool();
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceRole_ = role;
    int32_t micRefEnableState = 0;
    ecManager.Init(ecEnableState, micRefEnableState);
    ecManager.UpdateArmModuleInfo(deviceDesc, moduleInfo);
}

void ReloadSourceForSessionFuzzTest(FuzzedDataProvider& fdp)
{
    SessionInfo sessionInfo;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.ReloadSourceForSession(sessionInfo);
}

void GetTargetSourceTypeAndMatchingFlagFuzzTest(FuzzedDataProvider& fdp)
{
    SourceType source = ConsumeEnum<SourceType>(fdp);
    SourceType targetSource;
    bool useMatchingPropInfo = fdp.ConsumeBool();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetTargetSourceTypeAndMatchingFlag(source, targetSource, useMatchingPropInfo);
}

void FetchTargetInfoForSessionAddFuzzTest(FuzzedDataProvider& fdp)
{
    SessionInfo sessionInfo;
    sessionInfo.sourceType = ConsumeEnum<SourceType>(fdp);
    sessionInfo.channels = fdp.ConsumeIntegral<uint32_t>();
    sessionInfo.rate = fdp.ConsumeIntegral<uint32_t>();
    PipeStreamPropInfo targetInfo;
    SourceType targetSourceType = ConsumeEnum<SourceType>(fdp);
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    int32_t ecEnableState = fdp.ConsumeBool();
    int32_t micRefEnableState = 0;
    ecManager.Init(ecEnableState, micRefEnableState);
    AudioModuleInfo moduleInfo;
    std::vector<std::string> OpenMicSpeakerList = {
        "1",
        "OpenMicSpeaker"
    };
    moduleInfo.OpenMicSpeaker = PickValue(fdp, OpenMicSpeakerList);
    ecManager.SetPrimaryMicModuleInfo(moduleInfo);
    ecManager.FetchTargetInfoForSessionAdd(sessionInfo, targetInfo, targetSourceType);
}

void GetSourceOpenedFuzzTest(FuzzedDataProvider& fdp)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetSourceOpened();
}

void SetDpSinkModuleInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioModuleInfo moduleInfo;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.SetDpSinkModuleInfo(moduleInfo);
}

void GetMicRefFeatureEnableFuzzTest(FuzzedDataProvider& fdp)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetMicRefFeatureEnable();
}

void GetHalNameForDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<string> roleList = {
        "source",
        "role_source",
    };
    DeviceType deviceType = ConsumeEnum<DeviceType>(fdp);
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetHalNameForDevice(PickValue(fdp, roleList), deviceType);
}

void GetOpenedNormalSourceSessionIdFuzzTest(FuzzedDataProvider& fdp)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetOpenedNormalSourceSessionId();
}

void PrepareNormalSourceFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    CHECK_AND_RETURN(pipeInfo != nullptr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    CHECK_AND_RETURN(streamDesc != nullptr);
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.PrepareNormalSource(pipeInfo, streamDesc);
}

void ReloadNormalSourceFuzzTest(FuzzedDataProvider& fdp)
{
    SessionInfo sessionInfo;
    PipeStreamPropInfo targetInfo;
    SourceType targetSource = ConsumeEnum<SourceType>(fdp);
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    int32_t ecEnableState = fdp.ConsumeBool();
    int32_t micRefEnableState = 0;
    ecManager.Init(ecEnableState, micRefEnableState);
    ecManager.ReloadNormalSource(sessionInfo, targetInfo, targetSource);
}

void UpdateModuleInfoForEcFuzzTest(FuzzedDataProvider& fdp)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    AudioModuleInfo moduleInfo;

    ecManager.audioEcInfo_.ecType = ConsumeEnum<EcType>(fdp);
    ecManager.audioEcInfo_.ecOutputAdapter = fdp.ConsumeBool() ? USB_CLASS : DP_CLASS;
    ecManager.audioEcInfo_.samplingRate = std::to_string(fdp.ConsumeIntegral<uint32_t>());
    ecManager.audioEcInfo_.format = fdp.ConsumeBool() ? "s16le" : "s32le";
    ecManager.audioEcInfo_.channels = std::to_string(fdp.ConsumeIntegral<uint32_t>());

    ecManager.UpdateModuleInfoForEc(moduleInfo);
}

void UpdateModuleInfoForMicRefFuzzTest(FuzzedDataProvider& fdp)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    AudioModuleInfo moduleInfo;

    ecManager.Init(0, 0);
    SourceType source1 = SOURCE_TYPE_VOICE_COMMUNICATION;
    ecManager.UpdateModuleInfoForMicRef(moduleInfo, source1);

    ecManager.Init(1, 1);
    SourceType source2 = ConsumeEnum<SourceType>(fdp);
    ecManager.UpdateModuleInfoForMicRef(moduleInfo, source2);

    ecManager.isMicRefVoipUpOn_ = fdp.ConsumeBool();
    ecManager.UpdateModuleInfoForMicRef(moduleInfo, SOURCE_TYPE_VOICE_COMMUNICATION);

    ecManager.isMicRefRecordOn_ = fdp.ConsumeBool();
    ecManager.UpdateModuleInfoForMicRef(moduleInfo, SOURCE_TYPE_MIC);
}

void UpdateArmModuleInfoWithdataFuzzTest(FuzzedDataProvider& fdp)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());

    AudioModuleInfo moduleInfo1;
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc1 = nullptr;
    ecManager.UpdateArmModuleInfo(deviceDesc1, moduleInfo1);

    AudioModuleInfo moduleInfo2;
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc2 = std::make_shared<AudioDeviceDescriptor>();
    ecManager.UpdateArmModuleInfo(deviceDesc2, moduleInfo2);

    AudioModuleInfo moduleInfo3;
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc3 = std::make_shared<AudioDeviceDescriptor>();
    DeviceStreamInfo streamInfo;
    streamInfo.samplingRate = {ConsumeEnum<AudioSamplingRate>(fdp)};
    streamInfo.format = fdp.ConsumeBool() ? SAMPLE_S16LE : SAMPLE_S32LE;
    deviceDesc3->audioStreamInfo_.push_back(streamInfo);
    moduleInfo3.rate = std::to_string(fdp.ConsumeIntegral<uint32_t>());
    moduleInfo3.format = fdp.ConsumeBool() ? "s16le" : "s32le";
    moduleInfo3.channels = std::to_string(fdp.ConsumeIntegral<uint32_t>());
    ecManager.UpdateArmModuleInfo(deviceDesc3, moduleInfo3);

    AudioModuleInfo moduleInfo4;
    moduleInfo4.rate = "";
    moduleInfo4.format = "";
    moduleInfo4.channels = "";
    ecManager.UpdateArmModuleInfo(deviceDesc3, moduleInfo4);

    AudioModuleInfo moduleInfo5;
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc5 = std::make_shared<AudioDeviceDescriptor>();
    DeviceStreamInfo streamInfo5;
    streamInfo5.samplingRate = {SAMPLE_RATE_48000};
    streamInfo5.format = SAMPLE_S16LE;
    deviceDesc5->audioStreamInfo_.push_back(streamInfo5);
    deviceDesc5->macAddress_ = "AA:BB:CC:DD:EE:FF";
    moduleInfo5.rate = "48000";
    moduleInfo5.format = "s16le";
    moduleInfo5.channels = "2";
    ecManager.UpdateArmModuleInfo(deviceDesc5, moduleInfo5);
}

void ReloadSourceSoftLinkFuzzTest(FuzzedDataProvider& fdp)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    CHECK_AND_RETURN(pipeInfo != nullptr);

    AudioModuleInfo moduleInfo;
    moduleInfo.name = "test_module";
    moduleInfo.rate = std::to_string(fdp.ConsumeIntegral<uint32_t>());
    moduleInfo.channels = std::to_string(fdp.ConsumeIntegral<uint32_t>());
    moduleInfo.format = fdp.ConsumeBool() ? "s16le" : "s32le";

    ecManager.ReloadSourceSoftLink(pipeInfo, moduleInfo);
}

void GetEcFeatureEnableFuzzTest(FuzzedDataProvider& fdp)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetEcFeatureEnable();
}

void ReloadSourceForInputPipeFuzzTest(FuzzedDataProvider& fdp)
{
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    CHECK_AND_RETURN(pipeInfo != nullptr);
    uint32_t targetSessionId = fdp.ConsumeIntegral<uint32_t>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.ReloadSourceForInputPipe(pipeInfo, targetSessionId);
}

void SetPrimaryMicModuleInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioModuleInfo moduleInfo;
    moduleInfo.name = "primary_mic_module";
    moduleInfo.rate = "48000";
    moduleInfo.channels = "2";
    moduleInfo.format = "s16le";
    moduleInfo.bufferSize = "3840";
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.SetPrimaryMicModuleInfo(moduleInfo);
}

void UpdateStreamEcAndMicRefInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioModuleInfo moduleInfo;
    moduleInfo.name = "test_module";
    moduleInfo.rate = "48000";
    moduleInfo.channels = "2";
    moduleInfo.format = "s16le";
    SourceType sourceType = ConsumeEnum<SourceType>(fdp);
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.UpdateStreamEcAndMicRefInfo(moduleInfo, sourceType);
}

void SetOpenedNormalSourceFuzzTest(FuzzedDataProvider& fdp)
{
    SourceType sourceType = ConsumeEnum<SourceType>(fdp);
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.SetOpenedNormalSource(sourceType);
}

void SetOpenedNormalSourceSessionIdFuzzTest(FuzzedDataProvider& fdp)
{
    uint64_t sessionId = fdp.ConsumeIntegral<uint64_t>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.SetOpenedNormalSourceSessionId(sessionId);
}

void UpdateStreamEcInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioModuleInfo moduleInfo;
    moduleInfo.name = "test_module";
    moduleInfo.rate = "48000";
    moduleInfo.channels = "2";
    moduleInfo.format = "s16le";
    SourceType sourceType = ConsumeEnum<SourceType>(fdp);
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.UpdateStreamEcInfo(moduleInfo, sourceType);
}

void UpdateStreamMicRefInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioModuleInfo moduleInfo;
    moduleInfo.name = "test_module";
    moduleInfo.rate = "48000";
    moduleInfo.channels = "2";
    moduleInfo.format = "s16le";
    SourceType sourceType = ConsumeEnum<SourceType>(fdp);
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.UpdateStreamMicRefInfo(moduleInfo, sourceType);
}

void IsValidSourcePipeFuzzTest(FuzzedDataProvider& fdp)
{
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    CHECK_AND_RETURN(pipeInfo != nullptr);
    pipeInfo->name_ = "primary_input";
    bool isFromEcMicRef = fdp.ConsumeBool();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.IsValidSourcePipe(pipeInfo, isFromEcMicRef);
}

void UpdateModuleInfoForPrimaryFuzzTest(FuzzedDataProvider& fdp)
{
    AudioModuleInfo moduleInfo;
    moduleInfo.name = "test_module";
    moduleInfo.adapterName = "primary";
    PipeStreamPropInfo targetInfo;
    targetInfo.channels_ = fdp.ConsumeIntegral<uint32_t>();
    targetInfo.sampleRate_ = fdp.ConsumeIntegral<uint32_t>();
    targetInfo.format_ = ConsumeEnum<AudioSampleFormat>(fdp);
    targetInfo.bufferSize_ = fdp.ConsumeIntegral<uint32_t>();
    targetInfo.channelLayout_ = fdp.ConsumeIntegral<uint32_t>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.UpdateModuleInfoForPrimary(moduleInfo, targetInfo);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    GetEcSamplingRateFuzzTest,
    GetEcChannelsFuzzTest,
    GetEcFormatFuzzTest,
    CloseNormalSourceFuzzTest,
    UpdateEnhanceEffectStateFuzzTest,
    UpdateStreamCommonInfoFuzzTest,
    GetPipeNameByDeviceForEcFuzzTest,
    GetPipeInfoByDeviceTypeForEcFuzzTest,
    GetEcTypeFuzzTest,
    UpdateAudioEcInfoFuzzTest,
    ShouldOpenMicRefFuzzTest,
    GetAudioEcInfoFuzzTest,
    PresetArmIdleInputFuzzTest,
    CloseUsbArmDeviceFuzzTest,
    UpdateArmModuleInfoFuzzTest,
    GetTargetSourceTypeAndMatchingFlagFuzzTest,
    ReloadSourceForSessionFuzzTest,
    FetchTargetInfoForSessionAddFuzzTest,
    SetDpSinkModuleInfoFuzzTest,
    GetSourceOpenedFuzzTest,
    GetMicRefFeatureEnableFuzzTest,
    GetHalNameForDeviceFuzzTest,
    PrepareNormalSourceFuzzTest,
    GetOpenedNormalSourceSessionIdFuzzTest,
    ReloadNormalSourceFuzzTest,
    UpdateModuleInfoForEcFuzzTest,
    UpdateModuleInfoForMicRefFuzzTest,
    UpdateArmModuleInfoWithdataFuzzTest,
    ReloadSourceSoftLinkFuzzTest,
    GetEcFeatureEnableFuzzTest,
    ReloadSourceForInputPipeFuzzTest,
    SetPrimaryMicModuleInfoFuzzTest,
    UpdateStreamEcAndMicRefInfoFuzzTest,
    SetOpenedNormalSourceFuzzTest,
    SetOpenedNormalSourceSessionIdFuzzTest,
    UpdateStreamEcInfoFuzzTest,
    UpdateStreamMicRefInfoFuzzTest,
    IsValidSourcePipeFuzzTest,
    UpdateModuleInfoForPrimaryFuzzTest,
});
    func(fdp);
}
void Init()
{
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}
