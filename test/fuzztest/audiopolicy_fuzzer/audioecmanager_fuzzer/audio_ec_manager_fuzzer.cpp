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
#include <cstring>
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
#include "../../fuzz_utils.h"
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;

typedef void (*TestFuncs)();

void GetEcSamplingRateFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<const char*> deviceList = {
        USB_CLASS,
        DP_CLASS,
    };
    uint32_t deviceListCount = g_fuzzUtils.GetData<uint32_t>() % deviceList.size();
    string halName = deviceList[deviceListCount];
    std::shared_ptr<PipeStreamPropInfo> outModuleInfo = std::make_shared<PipeStreamPropInfo>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    outModuleInfo->sampleRate_ = g_fuzzUtils.GetData<uint32_t>();
    ecManager.GetEcSamplingRate(halName, outModuleInfo);
}

void GetEcChannelsFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<const char*> deviceList = {
        USB_CLASS,
        DP_CLASS,
    };
    uint32_t deviceListCount = g_fuzzUtils.GetData<uint32_t>() % deviceList.size();
    string halName = deviceList[deviceListCount];
    std::shared_ptr<PipeStreamPropInfo> outModuleInfo = std::make_shared<PipeStreamPropInfo>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    outModuleInfo->channelLayout_ = CH_LAYOUT_STEREO;
    std::vector<std::string> insertList = {"", to_string(g_fuzzUtils.GetData<uint32_t>())};
    uint32_t insertListCount = g_fuzzUtils.GetData<uint32_t>() % insertList.size();
    ecManager.dpSinkModuleInfo_.channels = insertList[insertListCount];
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
    uint32_t deviceListCount = g_fuzzUtils.GetData<uint32_t>() % deviceList.size();
    string halName = deviceList[deviceListCount];
    std::shared_ptr<PipeStreamPropInfo> outModuleInfo = std::make_shared<PipeStreamPropInfo>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    outModuleInfo->format_ = SAMPLE_S32LE;
    std::vector<std::string> insertList = {"", to_string(g_fuzzUtils.GetData<uint32_t>())};
    uint32_t insertListCount = g_fuzzUtils.GetData<uint32_t>() % insertList.size();
    ecManager.dpSinkModuleInfo_.format = insertList[insertListCount];
    ecManager.GetEcFormat(halName, outModuleInfo);
}

void UpdatePrimaryMicModuleInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SourceType source = g_fuzzUtils.GetData<SourceType>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    CHECK_AND_RETURN(pipeInfo != nullptr);
    ecManager.UpdatePrimaryMicModuleInfo(pipeInfo, source);
}

void UpdateEnhanceEffectStateFuzzTest(FuzzedDataProvider& fdp)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SourceType source = g_fuzzUtils.GetData<SourceType>();
    ecManager.UpdateEnhanceEffectState(source);
}

void GetPipeNameByDeviceForEcFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<string> roleList = {
        "source",
        "role_source",
    };
    size_t index = g_fuzzUtils.GetData<size_t>() % roleList.size();
    DeviceType deviceType = g_fuzzUtils.GetData<DeviceType>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetPipeNameByDeviceForEc(roleList[index], deviceType);
}

void UpdateStreamCommonInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    int32_t ecEnableState = g_fuzzUtils.GetData<bool>();
    int32_t micRefEnableState = 0;
    ecManager.Init(ecEnableState, micRefEnableState);
    AudioModuleInfo moduleInfo;
    PipeStreamPropInfo targetInfo;
    SourceType sourceType = g_fuzzUtils.GetData<SourceType>();
    ecManager.UpdateStreamCommonInfo(moduleInfo, targetInfo, sourceType);
}

void GetEcTypeFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType inputDevice = g_fuzzUtils.GetData<DeviceType>();
    DeviceType outputDevice = g_fuzzUtils.GetData<DeviceType>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetEcType(inputDevice, outputDevice);
}

void GetPipeInfoByDeviceTypeForEcFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<string> roleList = {
        "source",
        "role_source",
    };
    size_t index = g_fuzzUtils.GetData<size_t>() % roleList.size();
    DeviceType deviceType = g_fuzzUtils.GetData<DeviceType>();
    std::shared_ptr<AdapterPipeInfo> pipeInfo = std::make_shared<AdapterPipeInfo>();
    CHECK_AND_RETURN(pipeInfo != nullptr);
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetPipeInfoByDeviceTypeForEc(roleList[index], deviceType, pipeInfo);
}

void ShouldOpenMicRefFuzzTest(FuzzedDataProvider& fdp)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SourceType source = g_fuzzUtils.GetData<SourceType>();
    int32_t ecEnableState = 0;
    int32_t micRefEnableState = g_fuzzUtils.GetData<bool>();
    ecManager.Init(ecEnableState, micRefEnableState);
    ecManager.ShouldOpenMicRef(source);
}

void UpdateAudioEcInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceDescriptor inputDevice;
    AudioDeviceDescriptor outputDevice;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    int32_t ecEnableState = g_fuzzUtils.GetData<bool>();
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
    int32_t ecEnableState = g_fuzzUtils.GetData<bool>();
    ecManager.Init(ecEnableState, 0);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    ecManager.PresetArmIdleInput(deviceDesc);
}

void CloseUsbArmDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioDeviceDescriptor device;
    device.deviceRole_ = g_fuzzUtils.GetData<DeviceRole>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.CloseUsbArmDevice(device);
}

void UpdateArmModuleInfoFuzzTest(FuzzedDataProvider& fdp)
{
    std::vector<std::string> addressList = {
        "12:34:56:78:90:AB",
        "AA:BB:CC:DD:EE:FF",
        "",
        "invalid_address"
    };
    uint32_t addressIndex = g_fuzzUtils.GetData<uint32_t>() % addressList.size();
    std::string address = addressList[addressIndex];
    DeviceRole role = g_fuzzUtils.GetData<DeviceRole>();
    AudioModuleInfo moduleInfo;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    int32_t ecEnableState = g_fuzzUtils.GetData<bool>();
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
    SourceType source = g_fuzzUtils.GetData<SourceType>();
    SourceType targetSource;
    bool useMatchingPropInfo = g_fuzzUtils.GetData<bool>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetTargetSourceTypeAndMatchingFlag(source, targetSource, useMatchingPropInfo);
}

void FetchTargetInfoForSessionAddFuzzTest(FuzzedDataProvider& fdp)
{
    SessionInfo sessionInfo;
    sessionInfo.sourceType = g_fuzzUtils.GetData<SourceType>();
    sessionInfo.channels = g_fuzzUtils.GetData<uint32_t>();
    sessionInfo.rate = g_fuzzUtils.GetData<uint32_t>();
    PipeStreamPropInfo targetInfo;
    SourceType targetSourceType = g_fuzzUtils.GetData<SourceType>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    int32_t ecEnableState = g_fuzzUtils.GetData<bool>();
    int32_t micRefEnableState = 0;
    ecManager.Init(ecEnableState, micRefEnableState);
    AudioModuleInfo moduleInfo;
    std::vector<std::string> OpenMicSpeakerList = {
        "1",
        "OpenMicSpeaker"
    };
    uint32_t index = g_fuzzUtils.GetData<uint32_t>() % OpenMicSpeakerList.size();
    moduleInfo.OpenMicSpeaker = OpenMicSpeakerList[index];
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
    size_t index = g_fuzzUtils.GetData<size_t>() % roleList.size();
    DeviceType deviceType = g_fuzzUtils.GetData<DeviceType>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetHalNameForDevice(roleList[index], deviceType);
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
    SourceType targetSource = g_fuzzUtils.GetData<SourceType>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    int32_t ecEnableState = g_fuzzUtils.GetData<bool>();
    int32_t micRefEnableState = 0;
    ecManager.Init(ecEnableState, micRefEnableState);
    ecManager.ReloadNormalSource(sessionInfo, targetInfo, targetSource);
}

void UpdateModuleInfoForEcFuzzTest(FuzzedDataProvider& fdp)
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    AudioModuleInfo moduleInfo;

    ecManager.audioEcInfo_.ecType = g_fuzzUtils.GetData<EcType>();
    ecManager.audioEcInfo_.ecOutputAdapter = g_fuzzUtils.GetData<bool>() ? USB_CLASS : DP_CLASS;
    ecManager.audioEcInfo_.samplingRate = std::to_string(g_fuzzUtils.GetData<uint32_t>());
    ecManager.audioEcInfo_.format = g_fuzzUtils.GetData<bool>() ? "s16le" : "s32le";
    ecManager.audioEcInfo_.channels = std::to_string(g_fuzzUtils.GetData<uint32_t>());

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
    SourceType source2 = g_fuzzUtils.GetData<SourceType>();
    ecManager.UpdateModuleInfoForMicRef(moduleInfo, source2);

    ecManager.isMicRefVoipUpOn_ = g_fuzzUtils.GetData<bool>();
    ecManager.UpdateModuleInfoForMicRef(moduleInfo, SOURCE_TYPE_VOICE_COMMUNICATION);

    ecManager.isMicRefRecordOn_ = g_fuzzUtils.GetData<bool>();
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
    streamInfo.samplingRate = {g_fuzzUtils.GetData<AudioSamplingRate>()};
    streamInfo.format = g_fuzzUtils.GetData<bool>() ? SAMPLE_S16LE : SAMPLE_S32LE;
    deviceDesc3->audioStreamInfo_.push_back(streamInfo);
    moduleInfo3.rate = std::to_string(g_fuzzUtils.GetData<uint32_t>());
    moduleInfo3.format = g_fuzzUtils.GetData<bool>() ? "s16le" : "s32le";
    moduleInfo3.channels = std::to_string(g_fuzzUtils.GetData<uint32_t>());
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
    moduleInfo.rate = std::to_string(g_fuzzUtils.GetData<uint32_t>());
    moduleInfo.channels = std::to_string(g_fuzzUtils.GetData<uint32_t>());
    moduleInfo.format = g_fuzzUtils.GetData<bool>() ? "s16le" : "s32le";

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
    uint32_t targetSessionId = g_fuzzUtils.GetData<uint32_t>();
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
    SourceType sourceType = g_fuzzUtils.GetData<SourceType>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.UpdateStreamEcAndMicRefInfo(moduleInfo, sourceType);
}

void SetOpenedNormalSourceFuzzTest(FuzzedDataProvider& fdp)
{
    SourceType sourceType = g_fuzzUtils.GetData<SourceType>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.SetOpenedNormalSource(sourceType);
}

void SetOpenedNormalSourceSessionIdFuzzTest(FuzzedDataProvider& fdp)
{
    uint64_t sessionId = g_fuzzUtils.GetData<uint64_t>();
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
    SourceType sourceType = g_fuzzUtils.GetData<SourceType>();
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
    SourceType sourceType = g_fuzzUtils.GetData<SourceType>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.UpdateStreamMicRefInfo(moduleInfo, sourceType);
}

void IsValidSourcePipeFuzzTest(FuzzedDataProvider& fdp)
{
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    CHECK_AND_RETURN(pipeInfo != nullptr);
    pipeInfo->name_ = "primary_input";
    bool isFromEcMicRef = g_fuzzUtils.GetData<bool>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.IsValidSourcePipe(pipeInfoInfo, isFromEcMicRef);
}

void UpdateModuleInfoForPrimaryFuzzTest(FuzzedDataProvider& fdp)
{
    AudioModuleInfo moduleInfo;
    moduleInfo.name = "test_module";
    moduleInfo.adapterName = "primary";
    PipeStreamPropInfo targetInfo;
    targetInfo.channels_ = g_fuzzUtils.GetData<uint32_t>();
    targetInfo.sampleRate_ = g_fuzzUtils.GetData<uint32_t>();
    targetInfo.format_ = g_fuzzUtils.GetData<AudioSampleFormat>();
    targetInfo.bufferSize_ = g_fuzzUtils.GetData<uint32_t>();
    targetInfo.channelLayout_ = g_fuzzUtils.GetData<uint32_t>();
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
    GetEcFeatureEnableFuzzTest
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
void Init(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    RAW_DATA = data;
    g_dataSize = size;
    g_pos = 0;
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

    OHOS::AudioStandard::Init(data, size);
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}