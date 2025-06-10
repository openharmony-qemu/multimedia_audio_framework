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

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint8_t TESTSIZE = 26;
static int32_t NUM_2 = 2;

typedef void (*TestFuncs)();

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

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

vector<AudioFlag> AudioFlagVec = {
    AUDIO_FLAG_NONE,
    AUDIO_OUTPUT_FLAG_NORMAL,
    AUDIO_OUTPUT_FLAG_DIRECT,
    AUDIO_OUTPUT_FLAG_HD,
    AUDIO_OUTPUT_FLAG_MULTICHANNEL,
    AUDIO_OUTPUT_FLAG_LOWPOWER,
    AUDIO_OUTPUT_FLAG_FAST,
    AUDIO_OUTPUT_FLAG_VOIP,
    AUDIO_OUTPUT_FLAG_VOIP_FAST,
    AUDIO_OUTPUT_FLAG_HWDECODING,
    AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD,
    AUDIO_INPUT_FLAG_NORMAL,
    AUDIO_INPUT_FLAG_FAST,
    AUDIO_INPUT_FLAG_VOIP,
    AUDIO_INPUT_FLAG_VOIP_FAST,
    AUDIO_INPUT_FLAG_WAKEUP,
    AUDIO_FLAG_MAX,
};

void HandleScoInputDeviceFetchedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::string addr = "12345678901234567";
    audioCoreService->GetEncryptAddr(addr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioCoreService->HandleScoInputDeviceFetched(streamDesc);
}

void ScoInputDeviceFetchedForRecongnitionFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    bool handleFlag = GetData<uint32_t>() % NUM_2;
    std::string address = "abc";
    constexpr int32_t connectStateCount = static_cast<int32_t>(ConnectState::DEACTIVE_CONNECTED) + 1;
    ConnectState connectState = static_cast<ConnectState>(GetData<uint8_t>() % connectStateCount);
    audioCoreService->ScoInputDeviceFetchedForRecongnition(handleFlag, address, connectState);
}

void BluetoothScoFetchFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->capturerInfo_.sourceType = GetData<SourceType>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioCoreService->BluetoothScoFetch(streamDesc);
}

void CheckModemSceneFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = GetData<AudioStreamDeviceChangeReasonExt::ExtEnum>();
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    audioCoreService->CheckModemScene(reason);
}

void HandleAudioCaptureStateFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    constexpr int32_t modeCount = static_cast<int32_t>(AudioMode::AUDIO_MODE_RECORD) + 1;
    AudioMode mode = static_cast<AudioMode>(GetData<uint8_t>() % modeCount);
    AudioStreamChangeInfo streamChangeInfo;
    constexpr int32_t capturerStateCount = static_cast<int32_t>(CapturerState::CAPTURER_PAUSED) + 1;
    streamChangeInfo.audioCapturerChangeInfo.capturerState =
        static_cast<CapturerState>(GetData<uint8_t>() % capturerStateCount);
    streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sourceType = GetData<SourceType>();
    audioCoreService->HandleAudioCaptureState(mode, streamChangeInfo);
}

void BluetoothDeviceFetchOutputHandleFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    deviceDesc->deviceType_ = DeviceTypeVec[deviceTypeCount];
    desc->newDeviceDescs_.push_back(deviceDesc);
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = GetData<AudioStreamDeviceChangeReasonExt::ExtEnum>();
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    std::string encryptMacAddr = "abc";
    audioCoreService->BluetoothDeviceFetchOutputHandle(desc, reason, encryptMacAddr);
}

void ActivateA2dpDeviceWhenDescEnabledFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->isEnable_ = GetData<uint32_t>() % NUM_2;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = GetData<AudioStreamDeviceChangeReasonExt::ExtEnum>();
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->ActivateA2dpDeviceWhenDescEnabled(desc, reason);
}

void LoadA2dpModuleFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    AudioStreamInfo audioStreamInfo;
    std::string networkId = "abc";
    std::string sinkName = "abc";
    SourceType sourceType = GetData<SourceType>();
    audioCoreService->LoadA2dpModule(deviceType, audioStreamInfo, networkId, sinkName, sourceType);
}

void ReloadA2dpAudioPortFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    AudioModuleInfo moduleInfo;
    std::vector<std::string> roleList = {"abc", "link"};
    uint32_t roleListCount = GetData<uint32_t>() % roleList.size();
    moduleInfo.role = roleList[roleListCount];
    uint32_t deviceTypeCount = GetData<uint32_t>() % roleList.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    AudioStreamInfo audioStreamInfo;
    std::string networkId = "abc";
    std::string sinkName = "abc";
    SourceType sourceType = GetData<SourceType>();
    audioCoreService->ReloadA2dpAudioPort(moduleInfo, deviceType, audioStreamInfo,
        networkId, sinkName, sourceType);
}

void GetA2dpModuleInfoFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    AudioModuleInfo moduleInfo;
    std::vector<std::string> roleList = {"abc", "link", "source"};
    uint32_t roleListCount = GetData<uint32_t>() % roleList.size();
    moduleInfo.role = roleList[roleListCount];
    AudioStreamInfo audioStreamInfo;
    SourceType sourceType = GetData<SourceType>();
    audioCoreService->GetA2dpModuleInfo(moduleInfo, audioStreamInfo, sourceType);
}

void IsSameDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::string> networkIdList = {"abc", "networkId"};
    std::vector<std::string> macAddressList = {"abc", "macAddress"};

    uint32_t roleListCount = GetData<uint32_t>() % networkIdList.size();
    desc->networkId_ = networkIdList[roleListCount];
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    desc->deviceType_ = DeviceTypeVec[deviceTypeCount];
    uint32_t macAddressCount = GetData<uint32_t>() % networkIdList.size();
    desc->macAddress_ = macAddressList[macAddressCount];
    int32_t connectStateCount = static_cast<int32_t>(ConnectState::DEACTIVE_CONNECTED) + 1;
    desc->connectState_ = static_cast<ConnectState>(GetData<uint8_t>() % connectStateCount);

    AudioDeviceDescriptor deviceInfo;
    roleListCount = GetData<uint32_t>() % networkIdList.size();
    deviceInfo.networkId_ = networkIdList[roleListCount];
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    deviceInfo.deviceType_ = DeviceTypeVec[deviceTypeCount];
    macAddressCount = GetData<uint32_t>() % networkIdList.size();
    deviceInfo.macAddress_ = macAddressList[macAddressCount];
    connectStateCount = static_cast<int32_t>(ConnectState::DEACTIVE_CONNECTED) + 1;
    deviceInfo.connectState_ = static_cast<ConnectState>(GetData<uint8_t>() % connectStateCount);

    deviceInfo.descriptorType_ = AudioDeviceDescriptor::DEVICE_INFO;
    int32_t a2dpOffloadFlagCount = static_cast<int32_t>(BluetoothOffloadState::A2DP_OFFLOAD) + 1;
    deviceInfo.a2dpOffloadFlag_ = static_cast<BluetoothOffloadState>(GetData<uint8_t>() % a2dpOffloadFlagCount);

    audioCoreService->IsSameDevice(desc, deviceInfo);
}

void ProcessOutputPipeNewFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    int32_t streamActionCount = static_cast<int32_t>(AudioStreamAction::AUDIO_STREAM_ACTION_RECREATE) + 1;
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(GetData<uint8_t>() % streamActionCount);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = 0;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = GetData<AudioStreamDeviceChangeReasonExt::ExtEnum>();
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->ProcessOutputPipeNew(pipeInfo, flag, reason);
}

void HasLowLatencyCapabilityFuzzTest()
{
    bool isRemote = GetData<uint32_t>() % NUM_2;
    auto audioCoreService = AudioCoreService::GetCoreService();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    audioCoreService->HasLowLatencyCapability(deviceType, isRemote);
}

void GetRealUidFuzzTest()
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto audioCoreService = AudioCoreService::GetCoreService();
    streamDesc->callerUid_ = GetData<int32_t>();
    streamDesc->appInfo_.appUid = GetData<int32_t>();
    audioCoreService->GetRealUid(streamDesc);
}

void UpdateRendererInfoWhenNoPermissionFuzzTest()
{
    auto audioRendererChangeInfos = std::make_shared<AudioRendererChangeInfo>();
    bool hasSystemPermission = GetData<int32_t>() % NUM_2;
    auto audioCoreService = AudioCoreService::GetCoreService();
    audioCoreService->UpdateRendererInfoWhenNoPermission(audioRendererChangeInfos, hasSystemPermission);
}

void UpdateCapturerInfoWhenNoPermissionFuzzTest()
{
    auto audioCapturerChangeInfos = std::make_shared<AudioCapturerChangeInfo>();
    bool hasSystemPermission = GetData<int32_t>() % NUM_2;
    auto audioCoreService = AudioCoreService::GetCoreService();
    audioCoreService->UpdateCapturerInfoWhenNoPermission(audioCapturerChangeInfos, hasSystemPermission);
}

void GetFastControlParamFuzzTest()
{
    auto audioCoreService = AudioCoreService::GetCoreService();
    audioCoreService->isFastControlled_ = GetData<int32_t>() % NUM_2;
    int32_t value = GetData<int32_t>() % NUM_2;
    SetSysPara("persist.multimedia.audioflag.fastcontrolled", value);
    audioCoreService->GetFastControlParam();
}

void NeedRehandleA2DPDeviceFuzzTest()
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    auto audioCoreService = AudioCoreService::GetCoreService();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    desc->deviceType_ = DeviceTypeVec[deviceTypeCount];
    std::string moduleName = BLUETOOTH_MIC;
    AudioIOHandle moduleId = 0;
    audioCoreService->audioIOHandleMap_.AddIOHandleInfo(moduleName, moduleId);
    audioCoreService->NeedRehandleA2DPDevice(desc);
}

void TriggerRecreateRendererStreamCallbackFuzzTest()
{
    int32_t callerPid = 0;
    int32_t sessionId = 0;
    uint32_t routeFlag = GetData<int32_t>() % NUM_2;
    AudioStreamDeviceChangeReasonExt::ExtEnum reason = GetData<AudioStreamDeviceChangeReasonExt::ExtEnum>();
    auto audioCoreService = AudioCoreService::GetCoreService();
    std::shared_ptr<AudioPolicyServerHandler> handler = std::make_shared<AudioPolicyServerHandler>();
    audioCoreService->SetCallbackHandler(handler);
    audioCoreService->TriggerRecreateRendererStreamCallback(callerPid, sessionId, routeFlag, reason);
}

void TriggerRecreateCapturerStreamCallbackFuzzTest()
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = GetData<uint32_t>(),
    streamDesc->callerUid_ = 0;
    streamDesc->appInfo_.appUid = 0;
    streamDesc->appInfo_.appPid = 0;
    streamDesc->appInfo_.appTokenId = 0;
    streamDesc->streamStatus_ = STREAM_STATUS_NEW;
    streamDesc->routeFlag_ = true;
    auto audioCoreService = AudioCoreService::GetCoreService();
    std::shared_ptr<AudioPolicyServerHandler> handler = std::make_shared<AudioPolicyServerHandler>();
    audioCoreService->SetCallbackHandler(handler);
    audioCoreService->TriggerRecreateCapturerStreamCallback(streamDesc);
    SwitchStreamUtil::RemoveAllRecordBySessionId(streamDesc->sessionId_);
}

void ProcessOutputPipeUpdateFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    int32_t streamActionCount = static_cast<int32_t>(AudioStreamAction::AUDIO_STREAM_ACTION_RECREATE) + 1;
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(GetData<uint8_t>() % streamActionCount);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = 0;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = GetData<AudioStreamDeviceChangeReasonExt::ExtEnum>();
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioCoreService->ProcessOutputPipeUpdate(pipeInfo, flag, reason);
}

void ProcessInputPipeNewFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    int32_t streamActionCount = static_cast<int32_t>(AudioStreamAction::AUDIO_STREAM_ACTION_RECREATE) + 1;
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(GetData<uint8_t>() % streamActionCount);
    uint32_t routeFlagCount = GetData<uint32_t>() % AudioFlagVec.size();
    audioStreamDescriptor->routeFlag_ = AudioFlagVec[routeFlagCount];
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = 0;
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioCoreService->ProcessInputPipeNew(pipeInfo, flag);
}

void ProcessInputPipeUpdateFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    int32_t streamActionCount = static_cast<int32_t>(AudioStreamAction::AUDIO_STREAM_ACTION_RECREATE) + 1;
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(GetData<uint8_t>() % streamActionCount);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    uint32_t flag = 0;
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioCoreService->ProcessInputPipeUpdate(pipeInfo, flag);
}

void SwitchActiveA2dpDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    A2dpDeviceConfigInfo a2dpDeviceConfigInfo;
    std::vector<std::string> insertList1 = {"00:00:00:00:00:00", "abc"};
    uint32_t insertListCount = GetData<uint32_t>() % insertList1.size();
    std::string macAddress = insertList1[insertListCount];
    audioCoreService->audioA2dpDevice_.connectedA2dpDeviceMap_.insert({macAddress, a2dpDeviceConfigInfo});
    deviceDescriptor->macAddress_ = macAddress;
    AudioIOHandle audioIOHandle;
    std::vector<std::string> insertList2 = {"Bt_Speaker", "abc"};
    auto insertNum = GetData<uint32_t>() % insertList2.size();
    audioCoreService->audioIOHandleMap_.IOHandles_.insert({insertList2[insertNum], audioIOHandle});
    audioCoreService->SwitchActiveA2dpDevice(deviceDescriptor);
}

void MoveToNewInputDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->oldDeviceDescs_.push_back(audioDeviceDescriptor);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor2 = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->newDeviceDescs_.push_back(audioDeviceDescriptor2);
    audioCoreService->MoveToNewInputDevice(streamDesc);
}

void IsNewDevicePlaybackSupportedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc = std::make_shared<AudioDeviceDescriptor>();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    newDeviceDesc->deviceType_ = DeviceTypeVec[deviceTypeCount];
    streamDesc->newDeviceDescs_.push_back(newDeviceDesc);
    int32_t encodingCount =
        static_cast<int32_t>(AudioEncodingType::ENCODING_EAC3 - AudioEncodingType::ENCODING_INVALID) + 1;
    streamDesc->streamInfo_.encoding = static_cast<AudioEncodingType>(GetData<uint8_t>() % encodingCount - 1);
    audioCoreService->IsNewDevicePlaybackSupported(streamDesc);
}

TestFuncs g_testFuncs[TESTSIZE] = {
    HandleScoInputDeviceFetchedFuzzTest,
    ScoInputDeviceFetchedForRecongnitionFuzzTest,
    BluetoothScoFetchFuzzTest,
    CheckModemSceneFuzzTest,
    HandleAudioCaptureStateFuzzTest,
    BluetoothDeviceFetchOutputHandleFuzzTest,
    ActivateA2dpDeviceWhenDescEnabledFuzzTest,
    LoadA2dpModuleFuzzTest,
    ReloadA2dpAudioPortFuzzTest,
    GetA2dpModuleInfoFuzzTest,
    IsSameDeviceFuzzTest,
    ProcessOutputPipeNewFuzzTest,
    HasLowLatencyCapabilityFuzzTest,
    GetRealUidFuzzTest,
    UpdateRendererInfoWhenNoPermissionFuzzTest,
    UpdateCapturerInfoWhenNoPermissionFuzzTest,
    GetFastControlParamFuzzTest,
    NeedRehandleA2DPDeviceFuzzTest,
    TriggerRecreateRendererStreamCallbackFuzzTest,
    TriggerRecreateCapturerStreamCallbackFuzzTest,
    ProcessOutputPipeUpdateFuzzTest,
    ProcessInputPipeNewFuzzTest,
    ProcessInputPipeUpdateFuzzTest,
    SwitchActiveA2dpDeviceFuzzTest,
    MoveToNewInputDeviceFuzzTest,
    IsNewDevicePlaybackSupportedFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return true;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
