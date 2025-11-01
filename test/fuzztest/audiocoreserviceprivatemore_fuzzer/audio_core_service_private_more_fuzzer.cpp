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
static size_t g_count = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
static int32_t NUM_2 = 2;
static const int32_t MEDIA_SERVICE_UID = 1013;

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
    AUDIO_INPUT_FLAG_AI,
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

vector<AudioStreamStatus> AudioStreamStatusVec = {
    STREAM_STATUS_NEW,
    STREAM_STATUS_STARTED,
    STREAM_STATUS_PAUSED,
    STREAM_STATUS_STOPPED,
    STREAM_STATUS_RELEASED,
};

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
    streamDesc->streamStatus_ = GetData<AudioStreamStatus>();
    streamDesc->routeFlag_ = GetData<bool>();
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
    std::vector<SourceOutput> sourceOutputs = audioCoreService->GetSourceOutputs();
    audioCoreService->MoveToNewInputDevice(streamDesc, sourceOutputs);
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
    audioDeviceDescriptor1->networkId_ = "networkId";
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    audioDeviceDescriptor1->deviceType_ = DeviceTypeVec[deviceTypeCount];
    audioStreamDescriptor->oldDeviceDescs_.push_back(audioDeviceDescriptor1);
    AudioStreamDeviceChangeReasonExt reason;
    audioCoreService->MuteSinkPortForSwitchDevice(audioStreamDescriptor, reason);
}

void AudioCoreServicePrivateSetVoiceCallMuteForSwitchDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    CHECK_AND_RETURN(audioCoreService != nullptr);
    audioCoreService->SetVoiceCallMuteForSwitchDevice();
    audioCoreService->GetDisableFastStreamParam();
    audioCoreService->IsSceneRequireMuteAndSleep();
}

void AudioCoreServicePrivateMuteSinkPortFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    const std::string oldSinkName = OFFLOAD_PRIMARY_SPEAKER;
    const std::string newSinkName = OFFLOAD_PRIMARY_SPEAKER;
    AudioStreamDeviceChangeReasonExt reason;
    audioCoreService->MuteSinkPort(oldSinkName, newSinkName, reason);
    reason.reason_ = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    audioCoreService->MuteSinkPort(oldSinkName, newSinkName, reason);
    reason.reason_ = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
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
    audioStreamDescriptor->streamStatus_ = STREAM_STATUS_STARTED;
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
    audioStreamDescriptor->streamStatus_ = STREAM_STATUS_STARTED;
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
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor1);
    std::vector<std::pair<DeviceType, DeviceFlag>> activeDevices;
    activeDevices.push_back({DEVICE_TYPE_ACCESSORY, OUTPUT_DEVICES_FLAG});
    audioCoreService->HandleDualStartClient(activeDevices, audioStreamDescriptor);
}

void AudioCoreServicePrivateSelectA2dpTypeFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    int32_t streamActionCount = static_cast<int32_t>(AudioStreamAction::AUDIO_STREAM_ACTION_RECREATE) + 1;
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(GetData<uint8_t>() % streamActionCount);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->oldDeviceDescs_.push_back(audioDeviceDescriptor1);
    bool isCreateProcess = true;
    audioCoreService->SelectA2dpType(audioStreamDescriptor, isCreateProcess);
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
    if (audioCoreService == nullptr || audioDeviceDescriptor == nullptr) {
        return;
    }
    std::vector<std::string> insertList = {"00:00:00:00:00:00", "abc"};
    uint32_t insertListCount = GetData<uint32_t>() % insertList.size();
    audioDeviceDescriptor->macAddress_ = insertList[insertListCount];
    A2dpDeviceConfigInfo a2dpDeviceConfigInfo;
    audioCoreService->audioA2dpDevice_.connectedHearingAidDeviceMap_.insert({audioDeviceDescriptor->macAddress_,
        a2dpDeviceConfigInfo});
    audioCoreService->SwitchActiveHearingAidDevice(audioDeviceDescriptor);
}

void AudioCoreServiceUpdateModemRouteFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    if (audioStreamDescriptor == nullptr) {
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptor == nullptr) {
        return;
    }
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    descs.push_back(audioDeviceDescriptor);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    streamDescs.push_back(audioStreamDescriptor);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    if (audioCoreService->pipeManager_ == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (streamDesc == nullptr) {
        return;
    }
    audioCoreService->pipeManager_->modemCommunicationIdMap_.insert(std::make_pair(0, streamDesc));
    audioCoreService->UpdateModemRoute(descs);
}

void AudioCoreServicePrivateUpdateOffloadStateFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    if (audioCoreService == nullptr || pipeInfo == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    if (audioStreamDescriptor == nullptr) {
        return;
    }
    int32_t streamActionCount = static_cast<int32_t>(AudioStreamAction::AUDIO_STREAM_ACTION_RECREATE) + 1;
    audioStreamDescriptor->streamAction_ = static_cast<AudioStreamAction>(GetData<uint8_t>() % streamActionCount);
    pipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    pipeInfo->moduleInfo_.name = OFFLOAD_PRIMARY_SPEAKER;
    pipeInfo->moduleInfo_.className == "remote_offload";
    audioCoreService->UpdateOffloadState(pipeInfo);
}

void AudioCoreServicePrivateNotifyRouteUpdateFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    if (audioStreamDescriptor == nullptr) {
        return;
    }
    audioStreamDescriptor->newDeviceDescs_.clear();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptor == nullptr) {
        return;
    }
    std::vector<std::string> networkIdList = {"abc", "networkId"};
    uint32_t roleListCount = GetData<uint32_t>() % networkIdList.size();
    audioDeviceDescriptor->networkId_ = networkIdList[roleListCount];
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    streamDescs.push_back(audioStreamDescriptor);
    audioCoreService->NotifyRouteUpdate(streamDescs);
}

void AudioCoreServicePrivateFetchRendererPipesAndExecuteFuzzTest2()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    if (audioStreamDescriptor == nullptr) {
        return;
    }
    uint32_t index = GetData<uint32_t>() % AudioStreamStatusVec.size();
    audioStreamDescriptor->streamStatus_ = AudioStreamStatusVec[index];
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptor == nullptr) {
        return;
    }
    std::vector<std::string> insertList1 = {"00:00:00:00:00:00", "abc"};
    uint32_t insertListCount = GetData<uint32_t>() % insertList1.size();
    std::string macAddress = insertList1[insertListCount];
    audioDeviceDescriptor->macAddress_ = macAddress;
    audioDeviceDescriptor->deviceRole_ = g_testDeviceRoles[GetData<uint32_t>() % g_testDeviceRoles.size()];
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    audioDeviceDescriptor->deviceType_ = DeviceTypeVec[deviceTypeCount];
    audioStreamDescriptor->newDeviceDescs_.clear();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioStreamDescriptor->oldDeviceDescs_.clear();
    audioStreamDescriptor->oldDeviceDescs_.push_back(audioDeviceDescriptor);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    streamDescs.push_back(audioStreamDescriptor);
    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = GetData<AudioStreamDeviceChangeReasonExt::ExtEnum>();
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();
    if (audioCoreService->audioA2dpOffloadManager_ == nullptr) {
        return;
    }
    audioCoreService->FetchRendererPipesAndExecute(streamDescs, reason);
}

void AudioCoreServicePrivateUpdateRemoteOffloadModuleNameFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    if (audioCoreService == nullptr || pipeInfo == nullptr) {
        return;
    }
    pipeInfo->moduleInfo_.className == "remote_offload";
    pipeInfo->moduleInfo_.name = OFFLOAD_PRIMARY_SPEAKER;
    std::string moduleName;
    audioCoreService->UpdateRemoteOffloadModuleName(pipeInfo, moduleName);
}

void AudioCoreServicePrivateGetRealPidFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (streamDesc == nullptr) {
        return;
    }
    streamDesc->callerUid_ = MEDIA_SERVICE_UID;
    audioCoreService->GetRealPid(streamDesc);
    streamDesc->callerUid_ = GetData<int32_t>();
    audioCoreService->GetRealPid(streamDesc);
}

void AudioCoreServicePrivateSetWakeUpAudioCapturerFromAudioServerFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    AudioProcessConfig config;
    audioCoreService->SetWakeUpAudioCapturerFromAudioServer(config);
}

void AudioCoreServicePrivateWriteCapturerConcurrentEventFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    auto result = std::make_unique<struct ConcurrentCaptureDfxResult>();
    audioCoreService->WriteCapturerConcurrentEvent(result);
}

void AudioCoreServicePrivateLogCapturerConcurrentResultFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    auto result = std::make_unique<struct ConcurrentCaptureDfxResult>();
    audioCoreService->LogCapturerConcurrentResult(result);
}

void AudioCoreServicePrivateWriteCapturerConcurrentMsgFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    auto result = std::make_unique<struct ConcurrentCaptureDfxResult>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    if (audioStreamDescriptor == nullptr) {
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptor == nullptr) {
        return;
    }
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    audioDeviceDescriptor->deviceType_ = DeviceTypeVec[deviceTypeCount];
    audioStreamDescriptor->newDeviceDescs_.clear();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioCoreService->WriteCapturerConcurrentMsg(audioStreamDescriptor, result);
}

void AudioCoreServicePrivateLoadHearingAidModuleFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    AudioStreamInfo audioStreamInfo;
    std::vector<std::string> networkIdList = {"abc", "networkId"};
    uint32_t roleListCount = GetData<uint32_t>() % networkIdList.size();
    std::string networkId = networkIdList[roleListCount];
    std::string sinkName = "sinkName";
    SourceType sourceType = g_testSourceTypes[GetData<uint32_t>() % g_testSourceTypes.size()];
    audioCoreService->LoadHearingAidModule(deviceType, audioStreamInfo, networkId, sinkName, sourceType);
}

void AudioCoreServicePrivateIsFastAllowedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::vector<std::string> bundleNameList = {"abc", "bundleName"};
    uint32_t count = GetData<uint32_t>() % bundleNameList.size();
    std::string bundleName = bundleNameList[count];
    audioCoreService->IsFastAllowed(bundleName);
}

void AudioCoreServicePrivateSetDefaultOutputDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    uint32_t sessionID = GetData<uint32_t>();
    StreamUsage streamUsage = GetData<StreamUsage>();
    bool isRunning = GetData<bool>();
    bool skipForce = GetData<bool>();
    audioCoreService->policyConfigMananger_.hasEarpiece_ = GetData<bool>();
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    if (audioCoreService->pipeManager_ == nullptr) {
        return;
    }
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    if (pipeInfo == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (streamDesc == nullptr) {
        return;
    }
    streamDesc->sessionId_ = sessionID;
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    audioCoreService->pipeManager_->curPipeList_.push_back(pipeInfo);
    audioCoreService->SetDefaultOutputDevice(deviceType, sessionID, streamUsage, isRunning, skipForce);
}

void AudioCoreServicePrivateIsHeadsetToSpkOrEpFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> oldDesc = std::make_shared<AudioDeviceDescriptor>();
    if (oldDesc == nullptr) {
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> newDesc = std::make_shared<AudioDeviceDescriptor>();
    if (newDesc == nullptr) {
        return;
    }
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    oldDesc->deviceType_ = DeviceTypeVec[deviceTypeCount];
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    newDesc->deviceType_ = DeviceTypeVec[deviceTypeCount];
    audioCoreService->IsHeadsetToSpkOrEp(oldDesc, newDesc);
}

void AudioCoreServicePrivateSleepForSwitchDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    if (streamDesc == nullptr) {
        return;
    }
    streamDesc->oldDeviceDescs_.clear();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptor == nullptr) {
        return;
    }
    streamDesc->oldDeviceDescs_.push_back(audioDeviceDescriptor);
    streamDesc->newDeviceDescs_.clear();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor1 = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptor1 == nullptr) {
        return;
    }
    audioDeviceDescriptor1->networkId_ = "networkId";
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    audioDeviceDescriptor1->deviceType_ = DeviceTypeVec[deviceTypeCount];
    streamDesc->newDeviceDescs_.push_back(audioDeviceDescriptor1);

    AudioStreamDeviceChangeReasonExt::ExtEnum extEnum = GetData<AudioStreamDeviceChangeReasonExt::ExtEnum>();
    AudioStreamDeviceChangeReasonExt reason(extEnum);
    audioCoreService->SleepForSwitchDevice(streamDesc, reason);
}

void AudioCoreServicePrivateResetNearlinkDeviceStateFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    if (deviceDesc == nullptr) {
        return;
    }
    deviceDesc->deviceType_ = DEVICE_TYPE_NEARLINK;
    deviceDesc->macAddress_ = "00:00:00:00:00:00";
    deviceDesc->deviceId_ = GetData<int32_t>();
    audioCoreService->audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;
    std::vector<std::string> insertList = {"macAddress1", "macAddress2", "macAddress3"};
    uint32_t insertListCount = GetData<uint32_t>() % insertList.size();
    audioCoreService->audioActiveDevice_.currentActiveDevice_.macAddress_ = insertList[insertListCount];

    audioCoreService->ResetNearlinkDeviceState(deviceDesc);

    deviceDesc->deviceType_ = DEVICE_TYPE_NEARLINK_IN;
    audioCoreService->audioActiveDevice_.currentActiveInputDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;
    insertListCount = GetData<uint32_t>() % insertList.size();
    audioCoreService->audioActiveDevice_.currentActiveInputDevice_.macAddress_ = insertList[insertListCount];
    
    audioCoreService->ResetNearlinkDeviceState(deviceDesc);
}

void AudioCoreServicePrivateUpdateRouteForCollaborationFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    if (audioCoreService == nullptr) {
        return;
    }
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    InternalDeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    AudioCollaborativeService::GetAudioCollaborativeService().isCollaborativeStateEnabled_ = true;
    audioCoreService->UpdateRouteForCollaboration(deviceType);
}

TestFuncs g_testFuncs[] = {
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
    AudioCoreServicePrivateSelectA2dpTypeFuzzTest,
    AudioCoreServicePrivateActivateNearlinkDeviceFuzzTest,
    AudioCoreServicePrivateSwitchActiveHearingAidDeviceFuzzTest,
    AudioCoreServiceUpdateModemRouteFuzzTest,
    AudioCoreServicePrivateUpdateOffloadStateFuzzTest,
    AudioCoreServicePrivateNotifyRouteUpdateFuzzTest,
    AudioCoreServicePrivateFetchRendererPipesAndExecuteFuzzTest2,
    AudioCoreServicePrivateUpdateRemoteOffloadModuleNameFuzzTest,
    AudioCoreServicePrivateGetRealPidFuzzTest,
    AudioCoreServicePrivateSetWakeUpAudioCapturerFromAudioServerFuzzTest,
    AudioCoreServicePrivateWriteCapturerConcurrentEventFuzzTest,
    AudioCoreServicePrivateLogCapturerConcurrentResultFuzzTest,
    AudioCoreServicePrivateWriteCapturerConcurrentMsgFuzzTest,
    AudioCoreServicePrivateLoadHearingAidModuleFuzzTest,
    AudioCoreServicePrivateIsFastAllowedFuzzTest,
    AudioCoreServicePrivateSetDefaultOutputDeviceFuzzTest,
    AudioCoreServicePrivateIsHeadsetToSpkOrEpFuzzTest,
    AudioCoreServicePrivateSleepForSwitchDeviceFuzzTest,
    AudioCoreServicePrivateResetNearlinkDeviceStateFuzzTest,
    AudioCoreServicePrivateUpdateRouteForCollaborationFuzzTest,
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

    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[g_count % len]();
        g_count++;
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }
    g_count = g_count == len ? 0 : g_count;
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
