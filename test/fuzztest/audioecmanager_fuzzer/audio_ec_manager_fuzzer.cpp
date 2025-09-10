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
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;

typedef void (*TestFuncs)();

void GetEcSamplingRateFuzzTest()
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

void GetEcChannelsFuzzTest()
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

void GetEcFormatFuzzTest()
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

void CloseNormalSourceFuzzTest()
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.CloseNormalSource();
}

void UpdateEnhanceEffectStateFuzzTest()
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SourceType source = g_fuzzUtils.GetData<SourceType>();
    ecManager.UpdateEnhanceEffectState(source);
}

void UpdatePrimaryMicModuleInfoFuzzTest()
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SourceType source = g_fuzzUtils.GetData<SourceType>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    CHECK_AND_RETURN(pipeInfo != nullptr);
    ecManager.UpdatePrimaryMicModuleInfo(pipeInfo, source);
}

void UpdateStreamCommonInfoFuzzTest()
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

void GetPipeNameByDeviceForEcFuzzTest()
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

void GetPipeInfoByDeviceTypeForEcFuzzTest()
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

void GetEcTypeFuzzTest()
{
    DeviceType inputDevice = g_fuzzUtils.GetData<DeviceType>();
    DeviceType outputDevice = g_fuzzUtils.GetData<DeviceType>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetEcType(inputDevice, outputDevice);
}

void UpdateAudioEcInfoFuzzTest()
{
    AudioDeviceDescriptor inputDevice;
    AudioDeviceDescriptor outputDevice;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    int32_t ecEnableState = g_fuzzUtils.GetData<bool>();
    int32_t micRefEnableState = 0;
    ecManager.Init(ecEnableState, micRefEnableState);
    ecManager.UpdateAudioEcInfo(inputDevice, outputDevice);
}

void ShouldOpenMicRefFuzzTest()
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    SourceType source = g_fuzzUtils.GetData<SourceType>();
    int32_t ecEnableState = 0;
    int32_t micRefEnableState = g_fuzzUtils.GetData<bool>();
    ecManager.Init(ecEnableState, micRefEnableState);
    ecManager.ShouldOpenMicRef(source);
}

void GetAudioEcInfoFuzzTest()
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetAudioEcInfo();
    ecManager.ResetAudioEcInfo();
}

void PresetArmIdleInputFuzzTest()
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    int32_t ecEnableState = g_fuzzUtils.GetData<bool>();
    ecManager.Init(ecEnableState, 0);
    ecManager.PresetArmIdleInput("AA:BB:CC:DD:EE:FF");
}

void ActivateArmDeviceFuzzTest()
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    int32_t ecEnableState = g_fuzzUtils.GetData<bool>();
    ecManager.Init(ecEnableState, 0);
    DeviceRole role = g_fuzzUtils.GetData<DeviceRole>();
    ecManager.ActivateArmDevice("AA:BB:CC:DD:EE:FF", role);
}

void CloseUsbArmDeviceFuzzTest()
{
    AudioDeviceDescriptor device;
    device.deviceRole_ = g_fuzzUtils.GetData<DeviceRole>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.CloseUsbArmDevice(device);
}

void UpdateArmModuleInfoFuzzTest()
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
    int32_t micRefEnableState = 0;
    ecManager.Init(ecEnableState, micRefEnableState);
    ecManager.UpdateArmModuleInfo(address, role, moduleInfo);
}

void GetTargetSourceTypeAndMatchingFlagFuzzTest()
{
    SourceType source = g_fuzzUtils.GetData<SourceType>();
    SourceType targetSource;
    bool useMatchingPropInfo = g_fuzzUtils.GetData<bool>();
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetTargetSourceTypeAndMatchingFlag(source, targetSource, useMatchingPropInfo);
}

void ReloadSourceForSessionFuzzTest()
{
    SessionInfo sessionInfo;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.ReloadSourceForSession(sessionInfo);
}

void FetchTargetInfoForSessionAddFuzzTest()
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

void SetDpSinkModuleInfoFuzzTest()
{
    AudioModuleInfo moduleInfo;
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.SetDpSinkModuleInfo(moduleInfo);
}

void GetSourceOpenedFuzzTest()
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetSourceOpened();
}

void GetMicRefFeatureEnableFuzzTest()
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetMicRefFeatureEnable();
}

void GetHalNameForDeviceFuzzTest()
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

void PrepareNormalSourceFuzzTest()
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    CHECK_AND_RETURN(pipeInfo != nullptr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    CHECK_AND_RETURN(streamDesc != nullptr);
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.PrepareNormalSource(pipeInfo, streamDesc);
}

void GetOpenedNormalSourceSessionIdFuzzTest()
{
    AudioEcManager& ecManager(AudioEcManager::GetInstance());
    ecManager.GetOpenedNormalSourceSessionId();
}

void ReloadNormalSourceFuzzTest()
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

vector<TestFuncs> g_testFuncs = {
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
    ActivateArmDeviceFuzzTest,
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
};

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}
