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
const uint8_t TESTSIZE = 91;
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

const vector<SourceType> g_testSourceTypes = {
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
    SOURCE_TYPE_MAX,
};

const vector<DeviceRole> g_testDeviceRoles = {
    DEVICE_ROLE_NONE,
    INPUT_DEVICE,
    OUTPUT_DEVICE,
    DEVICE_ROLE_MAX,
};

void AudioCoreServicePrivateFetchRendererPipesAndExecuteFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = GetData<AudioStreamDeviceChangeReasonExt::ExtEnum>();
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->FetchRendererPipesAndExecute(streamDescs, reason);
}

void AudioCoreServicePrivateUpdateActiveDeviceAndVolumeBeforeMoveSessionFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = GetData<AudioStreamDeviceChangeReasonExt::ExtEnum>();
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->UpdateActiveDeviceAndVolumeBeforeMoveSession(streamDescs, reason);
}

void AudioCoreServicePrivateFetchCapturerPipesAndExecuteFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    audioCoreService->FetchCapturerPipesAndExecute(streamDescs);
}

void AudioCoreServicePrivateFetchDeviceAndRouteFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = GetData<AudioStreamDeviceChangeReasonExt::ExtEnum>();
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    std::string caller = "SetAudioScene";
    audioCoreService->FetchDeviceAndRoute("SetAudioScene", reason);
}

void AudioCoreServicePrivateGetAdapterNameBySessionIdFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    int32_t uid = 0;
    audioCoreService->GetAdapterNameBySessionId(uid);
}

void AudioCoreServicePrivateAddSessionIdFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    int32_t uid = 0;
    audioCoreService->AddSessionId(uid);
}

void AudioCoreServicePrivateDeleteSessionIdFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    int32_t uid = 0;
    audioCoreService->DeleteSessionId(uid);
}

void AudioCoreServicePrivateOnDeviceStatusUpdatedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    AudioDeviceDescriptor desc;
    bool isConnect = GetData<uint32_t>() % NUM_2;
    audioCoreService->OnDeviceStatusUpdated(desc, isConnect);
}

void AudioCoreServicePrivateOnPnpDeviceStatusUpdatedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    AudioDeviceDescriptor desc;
    audioCoreService->OnPnpDeviceStatusUpdated(desc, true);
}

void AudioCoreServicePrivateOnDeviceConfigurationChangedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamInfo.channels = AudioChannel::STEREO;
    A2dpDeviceConfigInfo configInfo = {audioStreamInfo, true};
    std::string macAddress = "11-22-33-44-55-66";
    std::string deviceName = "deviceName";
    std::vector<DeviceType> deviceTypesTmp = {DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_BLUETOOTH_A2DP,
        DEVICE_TYPE_WIRED_HEADSET, DEVICE_TYPE_EARPIECE};
    for (const auto& deviceType : deviceTypesTmp) {
        audioCoreService->OnDeviceConfigurationChanged(deviceType, macAddress, deviceName, audioStreamInfo);
    }
}

void AudioCoreServicePrivateOpenRemoteAudioDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::vector<SourceOutput> sourceOutputs;
    std::string networkId = "abc";
    DeviceRole deviceRole = DeviceRole::DEVICE_ROLE_NONE;
    std::vector<DeviceType> deviceTypesTmp = {DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_BLUETOOTH_A2DP,
        DEVICE_TYPE_WIRED_HEADSET, DEVICE_TYPE_EARPIECE};
    for (const auto& deviceType : deviceTypesTmp) {
        std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
        audioCoreService->OpenRemoteAudioDevice(networkId, deviceRole, deviceType, remoteDeviceDescriptor);
    }
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
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    audioCoreService->CheckModemScene(modemDescs, reason);
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
    shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->callerPid_ = 0;
    streamDesc->sessionId_ = 0;
    streamDesc->routeFlag_ = GetData<int32_t>() % NUM_2;
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = GetData<AudioStreamDeviceChangeReasonExt::ExtEnum>();
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    auto audioCoreService = AudioCoreService::GetCoreService();
    std::shared_ptr<AudioPolicyServerHandler> handler = std::make_shared<AudioPolicyServerHandler>();
    audioCoreService->SetCallbackHandler(handler);
    audioCoreService->TriggerRecreateRendererStreamCallback(streamDesc, reason);
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

void AudioCoreServicePrivateGetEncryptAddrFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }

    std::string addr = "12345678901234567";
    audioCoreService->GetEncryptAddr(addr);
}

void AudioCoreServicePrivateUpdateDefaultOutputDeviceWhenStoppingFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }

    int32_t uid = GetData<int32_t>();
    audioCoreService->UpdateDefaultOutputDeviceWhenStopping(uid);
}

void AudioCoreServicePrivateUpdateInputDeviceWhenStoppingFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }

    int32_t uid = GetData<int32_t>();
    audioCoreService->UpdateInputDeviceWhenStopping(uid);
}

void AudioCoreServicePrivateLoadA2dpModuleFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr || DeviceTypeVec.size() == 0 || g_testSourceTypes.size() == 0) {
        return;
    }

    DeviceType deviceType = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    AudioStreamInfo audioStreamInfo;
    std::string networkId = "abc";
    std::string sinkName = "abc";
    SourceType sourceType = g_testSourceTypes[GetData<uint32_t>() % g_testSourceTypes.size()];

    audioCoreService->LoadA2dpModule(deviceType, audioStreamInfo, networkId, sinkName, sourceType);
}

void AudioCoreServicePrivateRemoveUnusedPipeFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    if (audioCoreService->pipeManager_ == nullptr || pipeInfo == nullptr) {
        return;
    }
    audioCoreService->pipeManager_->AddAudioPipeInfo(pipeInfo);
    audioCoreService->RemoveUnusedPipe();
}

void AudioCoreServicePrivateRemoveUnusedRecordPipeFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    if (audioCoreService->pipeManager_ == nullptr || pipeInfo == nullptr) {
        return;
    }
    audioCoreService->pipeManager_->AddAudioPipeInfo(pipeInfo);
    audioCoreService->RemoveUnusedRecordPipe();
}

void AudioCoreServicePrivateIsStreamBelongToUidFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    uid_t uid = GetData<uid_t>();
    uint32_t sessionId = GetData<uint32_t>();
    audioCoreService->sessionIdMap_.insert(std::make_pair(sessionId, uid));
    bool isClear = GetData<bool>();
    if (isClear) {
        audioCoreService->sessionIdMap_.clear();
    }
    audioCoreService->IsStreamBelongToUid(uid, sessionId);
}

void AudioCoreServicePrivateMoveToNewOutputDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (audioCoreService == nullptr || streamDesc == nullptr) {
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorNew = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->newDeviceDescs_.push_back(audioDeviceDescriptorNew);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorOld = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->oldDeviceDescs_.push_back(audioDeviceDescriptorOld);
    bool isClear = GetData<bool>();
    if (isClear) {
        streamDesc->oldDeviceDescs_.clear();
    }
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    AudioStreamDeviceChangeReasonExt reason;
    audioCoreService->MoveToNewOutputDevice(streamDesc, pipeInfo, reason);
}

void AudioCoreServicePrivateOnForcedDeviceSelectedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr || DeviceTypeVec.size() == 0) {
        return;
    }
    DeviceType deviceType = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    std::string macAddress = "00:00:00:00:00:00";
    std::string deviceName = "TestDevice";
    AudioStreamInfo streamInfo;
    DeviceType devType = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    audioCoreService->OnDeviceConfigurationChanged(deviceType, macAddress, deviceName, streamInfo);
}

void AudioCoreServicePrivateMoveToRemoteOutputDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::vector<SinkInput> sinkInputIds;
    SinkInput sinkInputId;
    sinkInputIds.push_back(sinkInputId);
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioCoreService->MoveToRemoteOutputDevice(sinkInputIds, pipeInfo, remoteDeviceDescriptor);
}

void AudioCoreServicePrivateMoveToRemoteInputDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::vector<SourceOutput> sourceOutputs;
    SourceOutput sourceOutput;
    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioCoreService->MoveToRemoteInputDevice(sourceOutputs, remoteDeviceDescriptor);
}

void AudioCoreServicePrivateUpdateOutputRouteFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (streamDesc == nullptr) {
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioCoreService->UpdateOutputRoute(streamDesc);
}

void AudioCoreServicePrivateOnPreferredOutputDeviceUpdatedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr || DeviceTypeVec.size() == 0) {
        return;
    }
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    AudioStreamDeviceChangeReason reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
    audioCoreService->OnPreferredOutputDeviceUpdated(deviceDescriptor, reason);
}

void AudioCoreServicePrivateOnPreferredInputDeviceUpdatedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr || DeviceTypeVec.size() == 0) {
        return;
    }
    DeviceType deviceType = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    std::string networkId = "testNetworkId";
    audioCoreService->OnPreferredInputDeviceUpdated(deviceType, networkId);
}

void AudioCoreServicePrivateClearRingMuteWhenCallStartFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    bool pre = GetData<bool>();
    bool after = false;
    std::pair<AudioStreamType, StreamUsage> preType;
    AudioStreamType preStreamType = STREAM_DEFAULT;
    StreamUsage preUsage = STREAM_USAGE_UNKNOWN;
    audioCoreService->streamsWhenRingDualOnPrimarySpeaker_.push_back(std::make_pair(preStreamType, preUsage));
    audioCoreService->ClearRingMuteWhenCallStart(pre, after);
}

void AudioCoreServicePrivateSelectRingerOrAlarmDevicesFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (audioCoreService == nullptr || streamDesc == nullptr) {
        return;
    }

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioCoreService->SelectRingerOrAlarmDevices(streamDesc);
}

void AudioCoreServicePrivateUpdateDualToneStateFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }

    bool enable = GetData<bool>();
    int32_t sessionId = GetData<int32_t>();
    audioCoreService->UpdateDualToneState(enable, sessionId);
}

void AudioCoreServicePrivateHandleStreamStatusToCapturerStateFuzzTest()
{
    static const vector<AudioStreamStatus> testAudioStreamStatus = {
        STREAM_STATUS_NEW,
        STREAM_STATUS_STARTED,
        STREAM_STATUS_PAUSED,
        STREAM_STATUS_STOPPED,
        STREAM_STATUS_RELEASED,
    };
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr || testAudioStreamStatus.size() == 0) {
        return;
    }

    AudioStreamStatus status = testAudioStreamStatus[GetData<uint32_t>() % testAudioStreamStatus.size()];
    audioCoreService->HandleStreamStatusToCapturerState(status);
}

void AudioCoreServicePrivateIsPaRouteFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }

    uint32_t routeFlag = GetData<uint32_t>();
    audioCoreService->IsPaRoute(routeFlag);
}

void AudioCoreServicePrivateHandleScoOutputDeviceFetchedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }

    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStreamDeviceChangeReasonExt reason;
    audioCoreService->HandleScoOutputDeviceFetched(desc, reason);
}

void AudioCoreServicePrivateSendA2dpConnectedWhileRunningFuzzTest()
{
    static const vector<RendererState> testRendererStates = {
        RENDERER_INVALID,
        RENDERER_NEW,
        RENDERER_PREPARED,
        RENDERER_RUNNING,
        RENDERER_STOPPED,
        RENDERER_RELEASED,
        RENDERER_PAUSED
    };
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr || testRendererStates.size() == 0) {
        return;
    }

    uint32_t sessionId = GetData<uint32_t>();
    RendererState rendererState = testRendererStates[GetData<uint32_t>() % testRendererStates.size()];
    audioCoreService->SendA2dpConnectedWhileRunning(rendererState, sessionId);
}

void AudioCoreServicePrivateUpdateTrackerDeviceChangeFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr || g_testDeviceRoles.size() == 0) {
        return;
    }

    vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptor == nullptr) {
        return;
    }
    audioDeviceDescriptor->deviceRole_ = g_testDeviceRoles[GetData<uint32_t>() % g_testDeviceRoles.size()];
    desc.push_back(audioDeviceDescriptor);
    audioCoreService->UpdateTrackerDeviceChange(desc);
}

void AudioCoreServicePrivateUpdateOutputDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr || g_testDeviceRoles.size() == 0) {
        return;
    }

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptor == nullptr) {
        return;
    }
    audioDeviceDescriptor->deviceRole_ = g_testDeviceRoles[GetData<uint32_t>() % g_testDeviceRoles.size()];
    int32_t uid = GetData<int32_t>();
    AudioStreamDeviceChangeReasonExt reason;
    audioCoreService->UpdateOutputDevice(audioDeviceDescriptor, uid, reason);
}

void AudioCoreServicePrivateUpdateInputDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr || g_testDeviceRoles.size() == 0) {
        return;
    }

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptor == nullptr) {
        return;
    }
    audioDeviceDescriptor->deviceRole_ = g_testDeviceRoles[GetData<uint32_t>() % g_testDeviceRoles.size()];
    int32_t uid = GetData<int32_t>();
    AudioStreamDeviceChangeReasonExt reason;
    audioCoreService->UpdateInputDevice(audioDeviceDescriptor, uid, reason);
}

void AudioCoreServicePrivateWriteOutputRouteChangeEventFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr || g_testDeviceRoles.size() == 0) {
        return;
    }

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptor == nullptr) {
        return;
    }
    audioDeviceDescriptor->deviceRole_ = g_testDeviceRoles[GetData<uint32_t>() % g_testDeviceRoles.size()];
    AudioStreamDeviceChangeReasonExt reason;
    audioCoreService->WriteOutputRouteChangeEvent(audioDeviceDescriptor, reason);
}

void AudioCoreServicePrivateHandleDeviceChangeForFetchOutputDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (audioCoreService == nullptr || streamDesc == nullptr || DeviceTypeVec.size() == 0) {
        return;
    }

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorNew = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptorNew == nullptr) {
        return;
    }
    audioDeviceDescriptorNew->deviceType_ = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    streamDesc->newDeviceDescs_.push_back(audioDeviceDescriptorNew);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorOld = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->oldDeviceDescs_.push_back(audioDeviceDescriptorOld);
    bool isClear = GetData<bool>();
    if (isClear) {
        streamDesc->oldDeviceDescs_.clear();
    }
    AudioStreamDeviceChangeReasonExt reason;
    audioCoreService->HandleDeviceChangeForFetchOutputDevice(streamDesc, reason);
}

void AudioCoreServicePrivateHandleDeviceChangeForFetchInputDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (audioCoreService == nullptr || streamDesc == nullptr || DeviceTypeVec.size() == 0) {
        return;
    }

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorNew = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptorNew == nullptr) {
        return;
    }
    audioDeviceDescriptorNew->deviceType_ = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    streamDesc->newDeviceDescs_.push_back(audioDeviceDescriptorNew);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorOld = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->oldDeviceDescs_.push_back(audioDeviceDescriptorOld);
    bool isClear = GetData<bool>();
    if (isClear) {
        streamDesc->oldDeviceDescs_.clear();
    }
    audioCoreService->HandleDeviceChangeForFetchInputDevice(streamDesc);
}

void AudioCoreServicePrivateCheckOffloadStreamFuzzTest()
{
    static const vector<RendererState> testRendererStates = {
        RENDERER_INVALID,
        RENDERER_NEW,
        RENDERER_PREPARED,
        RENDERER_RUNNING,
        RENDERER_STOPPED,
        RENDERER_RELEASED,
        RENDERER_PAUSED
    };
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr || testRendererStates.size() == 0) {
        return;
    }

    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.rendererState =
        testRendererStates[GetData<uint32_t>() % testRendererStates.size()];
    audioCoreService->CheckOffloadStream(streamChangeInfo);
}

void AudioCoreServicePrivateActivateInputDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (audioCoreService == nullptr || streamDesc == nullptr || DeviceTypeVec.size() == 0) {
        return;
    }

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorNew = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptorNew == nullptr) {
        return;
    }
    audioDeviceDescriptorNew->deviceType_ = DeviceTypeVec[GetData<uint32_t>() % DeviceTypeVec.size()];
    streamDesc->newDeviceDescs_.push_back(audioDeviceDescriptorNew);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorOld = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->oldDeviceDescs_.push_back(audioDeviceDescriptorOld);
    bool isClear = GetData<bool>();
    if (isClear) {
        streamDesc->oldDeviceDescs_.clear();
    }
    audioCoreService->ActivateInputDevice(streamDesc);
}

void LoadSplitModuleFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    CHECK_AND_RETURN(audioCoreService != nullptr);
    audioCoreService->LoadSplitModule("", "networkId");
    audioCoreService->LoadSplitModule("splitArgs", "networkId");
}

void AudioCoreServicePrivateGetSourceOutputsFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->GetSourceOutputs();
}

void AudioCoreServicePrivateIsRingerOrAlarmerDualDevicesRangeFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::vector<InternalDeviceType> deviceTypesTmp = {DEVICE_TYPE_SPEAKER, DEVICE_TYPE_WIRED_HEADSET,
        DEVICE_TYPE_WIRED_HEADPHONES, DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_BLUETOOTH_A2DP, DEVICE_TYPE_USB_HEADSET,
        DEVICE_TYPE_USB_ARM_HEADSET, DEVICE_TYPE_NEARLINK, DEVICE_TYPE_HEARING_AID};
    for (const auto& deviceType : deviceTypesTmp) {
        audioCoreService->IsRingerOrAlarmerDualDevicesRange(deviceType);
    }
}

void AudioCoreServicePrivateOpenNewAudioPortAndRouteFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    int32_t streamActionCount = static_cast<int32_t>(AudioStreamAction::AUDIO_STREAM_ACTION_RECREATE) + 1;
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(GetData<uint8_t>() % streamActionCount);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    uint32_t paIndex = GetData<uint32_t>() % NUM_2;
    audioCoreService->OpenNewAudioPortAndRoute(pipeInfo, paIndex);
}

void AudioCoreServicePrivateHandleFetchInputWhenNoRunningStreamFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->HandleFetchInputWhenNoRunningStream();
}

void AudioCoreServicePrivateWriteInputRouteChangeEventFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStreamDeviceChangeReason reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
    audioCoreService->WriteInputRouteChangeEvent(desc, reason);
}

void AudioCoreServicePrivateIsDeviceSwitchingFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStreamDeviceChangeReasonExt reason;
    audioCoreService->IsDeviceSwitching(reason);
}

void AudioCoreServicePrivateUpdateTrackerFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    constexpr int32_t modeCount = static_cast<int32_t>(AudioMode::AUDIO_MODE_RECORD) + 1;
    AudioMode mode = static_cast<AudioMode>(GetData<uint8_t>() % modeCount);
    AudioStreamChangeInfo streamChangeInfo;
    constexpr int32_t capturerStateCount = static_cast<int32_t>(CapturerState::CAPTURER_PAUSED) + 1;
    streamChangeInfo.audioCapturerChangeInfo.capturerState =
        static_cast<CapturerState>(GetData<uint8_t>() % capturerStateCount);
    streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sourceType = GetData<SourceType>();
    static const vector<RendererState> testRendererStates = {
        RENDERER_INVALID,
        RENDERER_NEW,
        RENDERER_PREPARED,
        RENDERER_RUNNING,
        RENDERER_STOPPED,
        RENDERER_RELEASED,
        RENDERER_PAUSED
    };
    RendererState rendererState = testRendererStates[GetData<uint32_t>() % testRendererStates.size()];
    audioCoreService->UpdateTracker(mode, streamChangeInfo, rendererState);
}

void AudioCoreServicePrivateHandleCommonSourceOpenedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    int32_t streamActionCount = static_cast<int32_t>(AudioStreamAction::AUDIO_STREAM_ACTION_RECREATE) + 1;
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(GetData<uint8_t>() % streamActionCount);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioCoreService->HandleCommonSourceOpened(pipeInfo);
}

void AudioCoreServicePrivateDelayReleaseOffloadPipeFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    AudioIOHandle id = GetData<uint32_t>() % NUM_2;
    uint32_t paIndex = GetData<uint32_t>() % NUM_2;
    OffloadType type = LOCAL_OFFLOAD;
    audioCoreService->DelayReleaseOffloadPipe(id, paIndex, type);
}

void AudioCoreServicePrivateReleaseOffloadPipeFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    AudioIOHandle id = GetData<uint32_t>() % NUM_2;
    uint32_t paIndex = GetData<uint32_t>() % NUM_2;
    OffloadType type = LOCAL_OFFLOAD;
    audioCoreService->ReleaseOffloadPipe(id, paIndex, type);
}

void AudioCoreServicePrivateReConfigOffloadStatusFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    int32_t streamActionCount = static_cast<int32_t>(AudioStreamAction::AUDIO_STREAM_ACTION_RECREATE) + 1;
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(GetData<uint8_t>() % streamActionCount);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    uint32_t sessionId = GetData<uint32_t>() % NUM_2;
    std::string oldSinkName = "oldSinkName";
    audioCoreService->ReConfigOffloadStatus(sessionId, pipeInfo, oldSinkName);
}

void AudioCoreServicePrivatePrepareMoveAttrsFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    int32_t streamActionCount = static_cast<int32_t>(AudioStreamAction::AUDIO_STREAM_ACTION_RECREATE) + 1;
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(GetData<uint8_t>() % streamActionCount);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->oldDeviceDescs_.push_back(audioDeviceDescriptor1);
    AudioStreamDeviceChangeReasonExt reason;
    std::string oldSinkName = "";
    bool isNeedTriggerCallback = true;
    DeviceType oldDeviceType;
    audioCoreService->PrepareMoveAttrs(audioStreamDescriptor,
        oldDeviceType, isNeedTriggerCallback, oldSinkName, reason);
}

void AudioCoreServicePrivateMuteSinkPortForSwitchDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    int32_t streamActionCount = static_cast<int32_t>(AudioStreamAction::AUDIO_STREAM_ACTION_RECREATE) + 1;
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(GetData<uint8_t>() % streamActionCount);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->oldDeviceDescs_.push_back(audioDeviceDescriptor1);
    AudioStreamDeviceChangeReasonExt reason;
    audioCoreService->MuteSinkPortForSwitchDevice(audioStreamDescriptor, reason);
}

void AudioCoreServicePrivateSetVoiceCallMuteForSwitchDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->SetVoiceCallMuteForSwitchDevice();
}

void AudioCoreServicePrivateMuteSinkPortFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    const std::string oldSinkName = OFFLOAD_PRIMARY_SPEAKER;
    const std::string newSinkName = OFFLOAD_PRIMARY_SPEAKER;
    AudioStreamDeviceChangeReasonExt reason;
    audioCoreService->MuteSinkPort(oldSinkName, newSinkName, reason);
}

void AudioCoreServicePrivateMuteSinkPortLogicFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    const std::string oldSinkName = OFFLOAD_PRIMARY_SPEAKER;
    const std::string newSinkName = OFFLOAD_PRIMARY_SPEAKER;
    AudioStreamDeviceChangeReasonExt reason;
    audioCoreService->MuteSinkPortLogic(oldSinkName, newSinkName, reason);
}

void AudioCoreServicePrivateActivateOutputDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    int32_t streamActionCount = static_cast<int32_t>(AudioStreamAction::AUDIO_STREAM_ACTION_RECREATE) + 1;
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(GetData<uint8_t>() % streamActionCount);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->oldDeviceDescs_.push_back(audioDeviceDescriptor1);
    AudioStreamDeviceChangeReasonExt reason;
    audioCoreService->ActivateOutputDevice(audioStreamDescriptor, reason);
}

void AudioCoreServicePrivateOnAudioSceneChangeFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    AudioScene audioScene = AudioScene::AUDIO_SCENE_DEFAULT;
    audioCoreService->OnAudioSceneChange(audioScene);
}

void AudioCoreServicePrivateHandleOutputStreamInRunningFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    int32_t streamActionCount = static_cast<int32_t>(AudioStreamAction::AUDIO_STREAM_ACTION_RECREATE) + 1;
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(GetData<uint8_t>() % streamActionCount);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->oldDeviceDescs_.push_back(audioDeviceDescriptor1);
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = GetData<AudioStreamDeviceChangeReasonExt::ExtEnum>();
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->HandleOutputStreamInRunning(audioStreamDescriptor, reason);
}

void AudioCoreServicePrivateHandleInputStreamInRunningFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    int32_t streamActionCount = static_cast<int32_t>(AudioStreamAction::AUDIO_STREAM_ACTION_RECREATE) + 1;
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(GetData<uint8_t>() % streamActionCount);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->oldDeviceDescs_.push_back(audioDeviceDescriptor1);
    audioCoreService->HandleInputStreamInRunning(audioStreamDescriptor);
}

void AudioCoreServicePrivateHandleDualStartClientFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    int32_t streamActionCount = static_cast<int32_t>(AudioStreamAction::AUDIO_STREAM_ACTION_RECREATE) + 1;
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(GetData<uint8_t>() % streamActionCount);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->oldDeviceDescs_.push_back(audioDeviceDescriptor1);
    std::vector<std::pair<DeviceType, DeviceFlag>> activeDevices;
    activeDevices.push_back({DEVICE_TYPE_ACCESSORY, OUTPUT_DEVICES_FLAG});
    audioCoreService->HandleDualStartClient(activeDevices, audioStreamDescriptor);
}

void AudioCoreServicePrivateHandlePlaybackStreamInA2dpFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    int32_t streamActionCount = static_cast<int32_t>(AudioStreamAction::AUDIO_STREAM_ACTION_RECREATE) + 1;
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(GetData<uint8_t>() % streamActionCount);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->oldDeviceDescs_.push_back(audioDeviceDescriptor1);
    bool isCreateProcess = true;
    audioCoreService->HandlePlaybackStreamInA2dp(audioStreamDescriptor, isCreateProcess);
}

void AudioCoreServicePrivateGetDisableFastStreamParamFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->GetDisableFastStreamParam();
}

void AudioCoreServicePrivateActivateNearlinkDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    int32_t streamActionCount = static_cast<int32_t>(AudioStreamAction::AUDIO_STREAM_ACTION_RECREATE) + 1;
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(GetData<uint8_t>() % streamActionCount);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->oldDeviceDescs_.push_back(audioDeviceDescriptor1);
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = GetData<AudioStreamDeviceChangeReasonExt::ExtEnum>();
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->ActivateNearlinkDevice(audioStreamDescriptor, reason);
}

void AudioCoreServicePrivateSwitchActiveHearingAidDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioCoreService->SwitchActiveHearingAidDevice(audioDeviceDescriptor);
}

TestFuncs g_testFuncs[TESTSIZE] = {
    AudioCoreServicePrivateFetchRendererPipesAndExecuteFuzzTest,
    AudioCoreServicePrivateUpdateActiveDeviceAndVolumeBeforeMoveSessionFuzzTest,
    AudioCoreServicePrivateFetchCapturerPipesAndExecuteFuzzTest,
    AudioCoreServicePrivateFetchDeviceAndRouteFuzzTest,
    AudioCoreServicePrivateGetAdapterNameBySessionIdFuzzTest,
    AudioCoreServicePrivateAddSessionIdFuzzTest,
    AudioCoreServicePrivateDeleteSessionIdFuzzTest,
    AudioCoreServicePrivateOnDeviceStatusUpdatedFuzzTest,
    AudioCoreServicePrivateOnPnpDeviceStatusUpdatedFuzzTest,
    AudioCoreServicePrivateOnDeviceConfigurationChangedFuzzTest,
    AudioCoreServicePrivateOpenRemoteAudioDeviceFuzzTest,
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
    AudioCoreServicePrivateGetEncryptAddrFuzzTest,
    AudioCoreServicePrivateUpdateDefaultOutputDeviceWhenStoppingFuzzTest,
    AudioCoreServicePrivateUpdateInputDeviceWhenStoppingFuzzTest,
    AudioCoreServicePrivateLoadA2dpModuleFuzzTest,
    AudioCoreServicePrivateRemoveUnusedPipeFuzzTest,
    AudioCoreServicePrivateRemoveUnusedRecordPipeFuzzTest,
    AudioCoreServicePrivateIsStreamBelongToUidFuzzTest,
    AudioCoreServicePrivateMoveToNewOutputDeviceFuzzTest,
    AudioCoreServicePrivateOnForcedDeviceSelectedFuzzTest,
    AudioCoreServicePrivateMoveToRemoteOutputDeviceFuzzTest,
    AudioCoreServicePrivateMoveToRemoteInputDeviceFuzzTest,
    AudioCoreServicePrivateUpdateOutputRouteFuzzTest,
    AudioCoreServicePrivateOnPreferredOutputDeviceUpdatedFuzzTest,
    AudioCoreServicePrivateOnPreferredInputDeviceUpdatedFuzzTest,
    AudioCoreServicePrivateClearRingMuteWhenCallStartFuzzTest,
    AudioCoreServicePrivateSelectRingerOrAlarmDevicesFuzzTest,
    AudioCoreServicePrivateUpdateDualToneStateFuzzTest,
    AudioCoreServicePrivateHandleStreamStatusToCapturerStateFuzzTest,
    AudioCoreServicePrivateIsPaRouteFuzzTest,
    AudioCoreServicePrivateHandleScoOutputDeviceFetchedFuzzTest,
    AudioCoreServicePrivateSendA2dpConnectedWhileRunningFuzzTest,
    AudioCoreServicePrivateUpdateTrackerDeviceChangeFuzzTest,
    AudioCoreServicePrivateUpdateOutputDeviceFuzzTest,
    AudioCoreServicePrivateUpdateInputDeviceFuzzTest,
    AudioCoreServicePrivateWriteOutputRouteChangeEventFuzzTest,
    AudioCoreServicePrivateHandleDeviceChangeForFetchOutputDeviceFuzzTest,
    AudioCoreServicePrivateHandleDeviceChangeForFetchInputDeviceFuzzTest,
    AudioCoreServicePrivateCheckOffloadStreamFuzzTest,
    AudioCoreServicePrivateActivateInputDeviceFuzzTest,
    LoadSplitModuleFuzzTest,
    AudioCoreServicePrivateGetSourceOutputsFuzzTest,
    AudioCoreServicePrivateIsRingerOrAlarmerDualDevicesRangeFuzzTest,
    AudioCoreServicePrivateOpenNewAudioPortAndRouteFuzzTest,
    AudioCoreServicePrivateHandleFetchInputWhenNoRunningStreamFuzzTest,
    AudioCoreServicePrivateWriteInputRouteChangeEventFuzzTest,
    AudioCoreServicePrivateIsDeviceSwitchingFuzzTest,
    AudioCoreServicePrivateUpdateTrackerFuzzTest,
    AudioCoreServicePrivateHandleCommonSourceOpenedFuzzTest,
    AudioCoreServicePrivateDelayReleaseOffloadPipeFuzzTest,
    AudioCoreServicePrivateReleaseOffloadPipeFuzzTest,
    AudioCoreServicePrivateReConfigOffloadStatusFuzzTest,
    AudioCoreServicePrivatePrepareMoveAttrsFuzzTest,
    AudioCoreServicePrivateMuteSinkPortForSwitchDeviceFuzzTest,
    AudioCoreServicePrivateSetVoiceCallMuteForSwitchDeviceFuzzTest,
    AudioCoreServicePrivateMuteSinkPortFuzzTest,
    AudioCoreServicePrivateMuteSinkPortLogicFuzzTest,
    AudioCoreServicePrivateActivateOutputDeviceFuzzTest,
    AudioCoreServicePrivateOnAudioSceneChangeFuzzTest,
    AudioCoreServicePrivateHandleOutputStreamInRunningFuzzTest,
    AudioCoreServicePrivateHandleInputStreamInRunningFuzzTest,
    AudioCoreServicePrivateHandleDualStartClientFuzzTest,
    AudioCoreServicePrivateHandlePlaybackStreamInA2dpFuzzTest,
    AudioCoreServicePrivateGetDisableFastStreamParamFuzzTest,
    AudioCoreServicePrivateActivateNearlinkDeviceFuzzTest,
    AudioCoreServicePrivateSwitchActiveHearingAidDeviceFuzzTest,
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
