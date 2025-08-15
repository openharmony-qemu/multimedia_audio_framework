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
#ifndef LOG_TAG
#define LOG_TAG "AudioCoreService"
#endif

#include "audio_core_service.h"
#include "system_ability.h"
#include "audio_server_proxy.h"
#include "audio_policy_utils.h"
#include "iservice_registry.h"
#include "hdi_adapter_info.h"
#include "audio_usb_manager.h"
#include "audio_spatialization_service.h"
#include "audio_zone_service.h"
#include "audio_bundle_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace {
const size_t SELECT_DEVICE_HISTORY_LIMIT = 10;
const uint32_t FIRST_SESSIONID = 100000;
static const char* CHECK_FAST_BLOCK_PREFIX = "Is_Fast_Blocked_For_AppName#";
static const int32_t BLUETOOTH_FETCH_RESULT_DEFAULT = 0;
static const int32_t BLUETOOTH_FETCH_RESULT_CONTINUE = 1;
static const int32_t BLUETOOTH_FETCH_RESULT_ERROR = 2;
}

static bool IsRemoteOffloadActive(uint32_t remoteOffloadStreamPropSize, int32_t streamUsage)
{
    CHECK_AND_RETURN_RET_LOG(remoteOffloadStreamPropSize != 0 && streamUsage == STREAM_USAGE_MUSIC, false,
        "Use normal for remote device or remotecast");
    AUDIO_INFO_LOG("remote offload active, music use offload");
    return true;
}

bool AudioCoreService::isBtListenerRegistered = false;
bool AudioCoreService::isBtCrashed = false;
#ifdef BLUETOOTH_ENABLE
mutex g_btProxyMutex;
#endif

AudioCoreService::AudioCoreService()
    : audioPolicyServerHandler_(DelayedSingleton<AudioPolicyServerHandler>::GetInstance()),
      audioActiveDevice_(AudioActiveDevice::GetInstance()),
      audioSceneManager_(AudioSceneManager::GetInstance()),
      audioVolumeManager_(AudioVolumeManager::GetInstance()),
      audioCapturerSession_(AudioCapturerSession::GetInstance()),
      audioDeviceManager_(AudioDeviceManager::GetAudioDeviceManager()),
      audioConnectedDevice_(AudioConnectedDevice::GetInstance()),
      audioDeviceStatus_(AudioDeviceStatus::GetInstance()),
      audioEffectService_(AudioEffectService::GetAudioEffectService()),
      audioMicrophoneDescriptor_(AudioMicrophoneDescriptor::GetInstance()),
      audioRecoveryDevice_(AudioRecoveryDevice::GetInstance()),
      audioRouterCenter_(AudioRouterCenter::GetAudioRouterCenter()),
      streamCollector_(AudioStreamCollector::GetAudioStreamCollector()),
      audioStateManager_(AudioStateManager::GetAudioStateManager()),
      audioDeviceCommon_(AudioDeviceCommon::GetInstance()),
      audioOffloadStream_(AudioOffloadStream::GetInstance()),
      audioA2dpOffloadFlag_(AudioA2dpOffloadFlag::GetInstance()),
      audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()),
      audioRouteMap_(AudioRouteMap::GetInstance()),
      audioIOHandleMap_(AudioIOHandleMap::GetInstance()),
      audioA2dpDevice_(AudioA2dpDevice::GetInstance()),
      audioEcManager_(AudioEcManager::GetInstance()),
      policyConfigMananger_(AudioPolicyConfigManager::GetInstance()),
      audioAffinityManager_(AudioAffinityManager::GetAudioAffinityManager()),
      sleAudioDeviceManager_(SleAudioDeviceManager::GetInstance()),
      audioPipeSelector_(AudioPipeSelector::GetPipeSelector()),
      audioSessionService_(AudioSessionService::GetAudioSessionService()),
      pipeManager_(AudioPipeManager::GetPipeManager())
{
    AUDIO_INFO_LOG("Ctor");
}

AudioCoreService::~AudioCoreService()
{
    AUDIO_INFO_LOG("Dtor");
}

std::shared_ptr<AudioCoreService> AudioCoreService::GetCoreService()
{
    static std::shared_ptr<AudioCoreService> instance = std::make_shared<AudioCoreService>();
    return instance;
}

void AudioCoreService::Init()
{
    serviceFlag_.reset();
    eventEntry_ = std::make_shared<EventEntry>(shared_from_this());

    audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();
    if (audioA2dpOffloadManager_ != nullptr) {
        audioA2dpOffloadManager_->Init();
    }
    audioVolumeManager_.Init(audioPolicyServerHandler_);
    audioDeviceCommon_.Init(audioPolicyServerHandler_);
    audioRecoveryDevice_.Init(audioA2dpOffloadManager_);

    audioDeviceStatus_.Init(audioA2dpOffloadManager_, audioPolicyServerHandler_);
    audioCapturerSession_.Init(audioA2dpOffloadManager_);

    deviceStatusListener_ = std::make_shared<DeviceStatusListener>(*eventEntry_); // shared_ptr.get() -> *
    isFastControlled_ = GetFastControlParam();
    // Register device status listener
    int32_t status = deviceStatusListener_->RegisterDeviceStatusListener();
    if (status != SUCCESS) {
        AudioPolicyUtils::GetInstance().WriteServiceStartupError("Register for device status events failed");
        AUDIO_ERR_LOG("Register for device status events failed");
    }
}

void AudioCoreService::DeInit()
{
    // Remove device status listener
    deviceStatusListener_->UnRegisterDeviceStatusListener();
    if (isBtListenerRegistered) {
        UnregisterBluetoothListener();
    }
}

void AudioCoreService::SetCallbackHandler(std::shared_ptr<AudioPolicyServerHandler> handler)
{
    audioPolicyServerHandler_ = handler;
}

std::shared_ptr<AudioCoreService::EventEntry> AudioCoreService::GetEventEntry()
{
    return eventEntry_;
}

void AudioCoreService::DumpPipeManager(std::string &dumpString)
{
    if (pipeManager_ != nullptr) {
        pipeManager_->Dump(dumpString);
    }
}

int32_t AudioCoreService::CreateRendererClient(
    std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t &audioFlag, uint32_t &sessionId, std::string &networkId)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERR_NULL_POINTER, "stream desc is nullptr");
    if (sessionId == 0) {
        streamDesc->sessionId_ = GenerateSessionId();
        sessionId = streamDesc->sessionId_;
        AUDIO_INFO_LOG("Generate session id %{public}u for stream", sessionId);
    }

    // Modem stream need special process, because there are no real hdi output or input in fwk.
    // Input also need to be handled because capturer won't be created, only has renderer.
    bool isModemStream = false;
    if (streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION) {
        AUDIO_INFO_LOG("Modem communication renderer create, sessionId %{public}u", sessionId);
        isModemStream = true;
        audioFlag = AUDIO_FLAG_NORMAL;
        AddSessionId(sessionId);
        pipeManager_->AddModemCommunicationId(sessionId, streamDesc);
    } else if (streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_RINGTONE ||
        streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION) {
        std::string bundleName = AudioBundleManager::GetBundleNameFromUid(streamDesc->appInfo_.appUid);
        Bluetooth::AudioHfpManager::AddVirtualCallBundleName(bundleName, streamDesc->sessionId_);
    }

    AUDIO_INFO_LOG("[DeviceFetchStart] for stream %{public}d", sessionId);
    streamDesc->oldDeviceDescs_ = streamDesc->newDeviceDescs_;
    streamDesc->newDeviceDescs_ =
        audioRouterCenter_.FetchOutputDevices(streamDesc->rendererInfo_.streamUsage,
            GetRealUid(streamDesc), "CreateRendererClient");
    CHECK_AND_RETURN_RET_LOG(streamDesc->newDeviceDescs_.size() > 0 && streamDesc->newDeviceDescs_.front() != nullptr,
        ERR_NULL_POINTER, "Invalid deviceDesc");
    AUDIO_INFO_LOG("[DeviceFetchInfo] device %{public}s for stream %{public}d",
        streamDesc->GetNewDevicesTypeString().c_str(), sessionId);

    if (isModemStream) {
        return SUCCESS;
    }

    ActivateOutputDevice(streamDesc);

    // Bluetooth may be inactive (paused ringtone stream at Speaker switches to A2dp)
    std::string encryptMacAddr = GetEncryptAddr(streamDesc->newDeviceDescs_.front()->macAddress_);
    int32_t bluetoothFetchResult = BluetoothDeviceFetchOutputHandle(streamDesc,
        AudioStreamDeviceChangeReason::UNKNOWN, encryptMacAddr);
    CHECK_AND_RETURN_RET(bluetoothFetchResult == BLUETOOTH_FETCH_RESULT_DEFAULT, ERR_OPERATION_FAILED);

    UpdatePlaybackStreamFlag(streamDesc, true);
    AUDIO_INFO_LOG("Target audioFlag 0x%{public}x for stream %{public}d",
        streamDesc->audioFlag_, sessionId);

    // Fetch pipe
    audioActiveDevice_.UpdateStreamDeviceMap("CreateRendererClient");
    int32_t ret = FetchRendererPipeAndExecute(streamDesc, sessionId, audioFlag);
    networkId = streamDesc->newDeviceDescs_.front()->networkId_;
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "FetchPipeAndExecute failed");
    AddSessionId(sessionId);
    return SUCCESS;
}

int32_t AudioCoreService::CreateCapturerClient(
    std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t &audioFlag, uint32_t &sessionId)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERR_INVALID_PARAM, "streamDesc is nullptr");
    
    AUDIO_INFO_LOG("[DeviceFetchStart] for stream %{public}d", sessionId);
    streamDesc->oldDeviceDescs_ = streamDesc->newDeviceDescs_;
    std::shared_ptr<AudioDeviceDescriptor> inputDeviceDesc =
        audioRouterCenter_.FetchInputDevice(streamDesc->capturerInfo_.sourceType,
        GetRealUid(streamDesc), sessionId);
    CHECK_AND_RETURN_RET_LOG(inputDeviceDesc != nullptr, ERR_INVALID_PARAM, "inputDeviceDesc is nullptr");
    streamDesc->newDeviceDescs_.clear();
    streamDesc->newDeviceDescs_.push_back(inputDeviceDesc);
    AUDIO_INFO_LOG("[DeviceFetchInfo] device %{public}s for stream %{public}d",
        streamDesc->GetNewDevicesTypeString().c_str(), sessionId);

    UpdateRecordStreamFlag(streamDesc);
    AUDIO_INFO_LOG("Target audioFlag 0x%{public}x for stream %{public}d",
        streamDesc->audioFlag_, sessionId);

    // Fetch pipe
    int32_t ret = FetchCapturerPipeAndExecute(streamDesc, audioFlag, sessionId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "FetchPipeAndExecute failed");
    AddSessionId(sessionId);
    return SUCCESS;
}

bool AudioCoreService::IsStreamSupportMultiChannel(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    Trace trace("IsStreamSupportMultiChannel");

    // MultiChannel: Speaker, A2dp offload
    if (streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_SPEAKER &&
        (streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP ||
        streamDesc->newDeviceDescs_[0]->a2dpOffloadFlag_ != A2DP_OFFLOAD)) {
        AUDIO_INFO_LOG("normal stream, deviceType: %{public}d", streamDesc->newDeviceDescs_[0]->deviceType_);
        return false;
    }
    if (streamDesc->streamInfo_.channels <= STEREO ||
        (streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_MOVIE &&
         streamDesc->rendererInfo_.originalFlag == AUDIO_FLAG_PCM_OFFLOAD)) {
        return false;
    }
    // The multi-channel algorithm needs to be supported in the dsp
    bool isSupported = AudioServerProxy::GetInstance().GetEffectOffloadEnabledProxy();
    AUDIO_INFO_LOG("effect offload enable is %{public}d", isSupported);
    return isSupported;
}

bool AudioCoreService::IsStreamSupportDirect(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    Trace trace("IsStreamSupportDirect");
    if (streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_WIRED_HEADSET &&
        streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_USB_HEADSET &&
        streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_NEARLINK) {
            return false;
        }
    if (streamDesc->rendererInfo_.streamUsage != STREAM_USAGE_MUSIC ||
        streamDesc->streamInfo_.samplingRate < SAMPLE_RATE_48000 ||
        streamDesc->streamInfo_.format < SAMPLE_S24LE) {
            AUDIO_INFO_LOG("normal stream because stream info");
            return false;
        }
    if (streamDesc->streamInfo_.samplingRate > SAMPLE_RATE_192000) {
        AUDIO_INFO_LOG("sample rate over 192k");
        return false;
    }
    return true;
}

bool AudioCoreService::IsForcedNormal(std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    const auto &rendererInfo = streamDesc->rendererInfo_;
    if (rendererInfo.originalFlag == AUDIO_FLAG_FORCED_NORMAL ||
        rendererInfo.rendererFlags == AUDIO_FLAG_FORCED_NORMAL) {
        streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
        return true;
    }
    if (rendererInfo.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION &&
        rendererInfo.samplingRate != SAMPLE_RATE_48000) {
        streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
        return true;
    }
    return false;
}

void AudioCoreService::UpdatePlaybackStreamFlag(std::shared_ptr<AudioStreamDescriptor> &streamDesc, bool isCreateProcess)
{
    CHECK_AND_RETURN_LOG(streamDesc, "Input param error");
    // fast/normal has done in audioRendererPrivate
    CHECK_AND_RETURN_LOG(IsForcedNormal(streamDesc) == false, "Forced normal cases");

    if (streamDesc->newDeviceDescs_.back()->deviceType_ == DEVICE_TYPE_REMOTE_CAST ||
        streamDesc->newDeviceDescs_.back()->networkId_ != LOCAL_NETWORK_ID) {
        auto remoteOffloadStreamPropSize = policyConfigMananger_.GetStreamPropInfoSize("remote",
            "offload_distributed_output");
        streamDesc->audioFlag_ = IsRemoteOffloadActive(remoteOffloadStreamPropSize,
            streamDesc->rendererInfo_.streamUsage) ? AUDIO_OUTPUT_FLAG_LOWPOWER : AUDIO_OUTPUT_FLAG_NORMAL;
        return;
    }

    if (streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
        streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION) {
        std::string sinkPortName =
            AudioPolicyUtils::GetInstance().GetSinkPortName(streamDesc->newDeviceDescs_.front()->deviceType_);
        // in plan: if has two voip, return normal
        streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_VOIP;
        AUDIO_INFO_LOG("sinkPortName %{public}s, audioFlag 0x%{public}x",
            sinkPortName.c_str(), streamDesc->audioFlag_);
        return;
    }
    HandlePlaybackStreamInA2dp(streamDesc, isCreateProcess);
    switch (streamDesc->rendererInfo_.originalFlag) {
        case AUDIO_FLAG_MMAP:
            streamDesc->audioFlag_ =
                IsFastAllowed(streamDesc->bundleName_) ? AUDIO_OUTPUT_FLAG_FAST : AUDIO_OUTPUT_FLAG_NORMAL;
            return;
        case AUDIO_FLAG_VOIP_FAST:
            streamDesc->audioFlag_ =
                IsFastAllowed(streamDesc->bundleName_) ? AUDIO_OUTPUT_FLAG_VOIP : AUDIO_OUTPUT_FLAG_NORMAL;
            return;
        case AUDIO_FLAG_VOIP_DIRECT:
            streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_VOIP;
            return;
        default:
            break;
    }
    streamDesc->audioFlag_ = SetFlagForSpecialStream(streamDesc, isCreateProcess);
}

AudioFlag AudioCoreService::SetFlagForSpecialStream(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    bool isCreateProcess)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr && streamDesc->newDeviceDescs_.size() > 0 &&
        streamDesc->newDeviceDescs_[0] != nullptr, AUDIO_OUTPUT_FLAG_NORMAL, "Invalid stream desc");

    if (IsStreamSupportDirect(streamDesc)) {
        return AUDIO_OUTPUT_FLAG_HD;
    }
    if (IsStreamSupportLowpower(streamDesc)) {
        return AUDIO_OUTPUT_FLAG_LOWPOWER;
    }
    if (IsStreamSupportMultiChannel(streamDesc)) {
        return AUDIO_OUTPUT_FLAG_MULTICHANNEL;
    }
    return AUDIO_OUTPUT_FLAG_NORMAL;
}

void AudioCoreService::UpdateRecordStreamFlag(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    if (streamDesc->capturerInfo_.originalFlag == AUDIO_FLAG_FORCED_NORMAL ||
        streamDesc->capturerInfo_.capturerFlags == AUDIO_FLAG_FORCED_NORMAL) {
        streamDesc->audioFlag_ = AUDIO_INPUT_FLAG_NORMAL;
        AUDIO_INFO_LOG("Forced normal cases");
        return;
    }

    // fast/normal has done in audioCapturerPrivate
    if (streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
        // in plan: if has two voip, return normal
        streamDesc->audioFlag_ = AUDIO_INPUT_FLAG_VOIP;
        AUDIO_INFO_LOG("Use voip");
        return;
    }
    if (streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_REMOTE_CAST) {
        streamDesc->audioFlag_ = AUDIO_INPUT_FLAG_NORMAL;
        AUDIO_WARNING_LOG("Use normal for remotecast");
        return;
    }

    if (streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_WAKEUP) {
        streamDesc->audioFlag_ = AUDIO_INPUT_FLAG_WAKEUP;
    }
    switch (streamDesc->capturerInfo_.capturerFlags) {
        case AUDIO_FLAG_MMAP:
            streamDesc->audioFlag_ = AUDIO_INPUT_FLAG_FAST;
            return;
        case AUDIO_FLAG_VOIP_FAST:
            streamDesc->audioFlag_ = AUDIO_INPUT_FLAG_VOIP_FAST;
            return;
        default:
            break;
    }
    // In plan: streamDesc to audioFlag;
    streamDesc->audioFlag_ = AUDIO_FLAG_NONE;
}

void AudioCoreService::CheckAndSetCurrentOutputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc, int32_t sessionId)
{
    CHECK_AND_RETURN_LOG(desc != nullptr, "desc is null");
    CHECK_AND_RETURN_LOG(!IsSameDevice(desc, audioActiveDevice_.GetCurrentOutputDevice()), "same device");
    audioActiveDevice_.SetCurrentOutputDevice(*(desc));
    std::string sinkName = AudioPolicyUtils::GetInstance().GetSinkName(desc, sessionId);
    if (audioDeviceManager_.IsDeviceConnected(desc)) {
        audioVolumeManager_.SetVolumeForSwitchDevice(*(desc), sinkName);
    }
    OnPreferredOutputDeviceUpdated(audioActiveDevice_.GetCurrentOutputDevice(),
        AudioStreamDeviceChangeReason::STREAM_PRIORITY_CHANGED);
}

void AudioCoreService::CheckAndSetCurrentInputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    CHECK_AND_RETURN_LOG(desc != nullptr, "desc is null");
    CHECK_AND_RETURN_LOG(!IsSameDevice(desc, audioActiveDevice_.GetCurrentInputDevice()),
        "current input device is same as new device");
    audioActiveDevice_.SetCurrentInputDevice(*(desc));
    OnPreferredInputDeviceUpdated(audioActiveDevice_.GetCurrentInputDeviceType(), "");
}

void AudioCoreService::CheckForRemoteDeviceState(std::shared_ptr<AudioDeviceDescriptor> desc)
{
    CHECK_AND_RETURN_LOG(desc != nullptr, "desc is null");
    std::string networkId = desc->networkId_;
    DeviceRole deviceRole = desc->deviceRole_;
    CHECK_AND_RETURN(networkId != LOCAL_NETWORK_ID);
    int32_t res = AudioServerProxy::GetInstance().CheckRemoteDeviceStateProxy(networkId, deviceRole, true);
    CHECK_AND_RETURN_LOG(res == SUCCESS, "remote device state is invalid!");
}

int32_t AudioCoreService::StartClient(uint32_t sessionId)
{
    if (pipeManager_->IsModemCommunicationIdExist(sessionId)) {
        AUDIO_INFO_LOG("Modem communication ring, directly return");
        return SUCCESS;
    }

    std::shared_ptr<AudioStreamDescriptor> streamDesc = pipeManager_->GetStreamDescById(sessionId);
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERR_NULL_POINTER, "Cannot find session %{public}u", sessionId);
    pipeManager_->StartClient(sessionId);

    // A stream set default device special case
    if (audioDeviceManager_.IsSessionSetDefaultDevice(sessionId)) {
        audioDeviceManager_.UpdateDefaultOutputDeviceWhenStarting(sessionId);
        std::vector<std::shared_ptr<AudioStreamDescriptor>> outputDescs = pipeManager_->GetAllOutputStreamDescs();
        for (auto &desc : outputDescs) {
            CHECK_AND_CONTINUE_LOG(desc != nullptr, "desc is null");
            desc->newDeviceDescs_ = audioRouterCenter_.FetchOutputDevices(desc->rendererInfo_.streamUsage,
                GetRealUid(desc), "StartClient");
        }
    }

    CHECK_AND_RETURN_RET_LOG(!streamDesc->newDeviceDescs_.empty(), ERR_INVALID_PARAM, "newDeviceDescs_ is empty");
    if (streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK) {
        int32_t outputRet = ActivateOutputDevice(streamDesc);
        CHECK_AND_RETURN_RET_LOG(outputRet == SUCCESS, outputRet, "Activate output device failed");
        CheckAndSetCurrentOutputDevice(streamDesc->newDeviceDescs_.front(), streamDesc->sessionId_);
        std::vector<std::pair<DeviceType, DeviceFlag>> activeDevices;
        if (policyConfigMananger_.GetUpdateRouteSupport()) {
            UpdateOutputRoute(streamDesc);
        }
    } else {
        int32_t inputRet = ActivateInputDevice(streamDesc);
        CHECK_AND_RETURN_RET_LOG(inputRet == SUCCESS, inputRet, "Activate input device failed");
        CheckAndSetCurrentInputDevice(streamDesc->newDeviceDescs_.front());
        audioActiveDevice_.UpdateActiveDeviceRoute(
            streamDesc->newDeviceDescs_[0]->deviceType_, DeviceFlag::INPUT_DEVICES_FLAG,
            streamDesc->newDeviceDescs_[0]->deviceName_, streamDesc->newDeviceDescs_[0]->networkId_);
        streamCollector_.UpdateCapturerDeviceInfo(streamDesc->newDeviceDescs_.front());
    }
    streamDesc->startTimeStamp_ = ClockTime::GetCurNano();
    sleAudioDeviceManager_.UpdateSleStreamTypeCount(streamDesc);

    CheckForRemoteDeviceState(streamDesc->newDeviceDescs_.front());
    return SUCCESS;
}

int32_t AudioCoreService::PauseClient(uint32_t sessionId)
{
    pipeManager_->PauseClient(sessionId);
    return SUCCESS;
}

int32_t AudioCoreService::StopClient(uint32_t sessionId)
{
    pipeManager_->StopClient(sessionId);
    return SUCCESS;
}

int32_t AudioCoreService::ReleaseClient(uint32_t sessionId, SessionOperationMsg opMsg)
{
    if (pipeManager_->IsModemCommunicationIdExist(sessionId)) {
        AUDIO_INFO_LOG("Modem communication, sessionId %{public}u", sessionId);
        bool isRemoved = true;
        sleAudioDeviceManager_.UpdateSleStreamTypeCount(pipeManager_->GetModemCommunicationStreamDescById(sessionId),
            isRemoved);
        pipeManager_->RemoveModemCommunicationId(sessionId);
        return SUCCESS;
    }
    pipeManager_->RemoveClient(sessionId);
    audioOffloadStream_.ResetOffloadStatus(sessionId);
    RemoveUnusedPipe();
    if (opMsg == SESSION_OP_MSG_REMOVE_PIPE) {
        RemoveUnusedRecordPipe();
    }
    DeleteSessionId(sessionId);

    return SUCCESS;
}

int32_t AudioCoreService::SetAudioScene(AudioScene audioScene, const int32_t uid, const int32_t pid)
{
    audioSceneManager_.SetAudioScenePre(audioScene);
    audioStateManager_.SetAudioSceneOwnerUid(audioScene == 0 ? 0 : uid);
    AudioScene lastAudioScene = audioSceneManager_.GetLastAudioScene();
    bool isSameScene = audioSceneManager_.IsSameAudioScene();
    int32_t result = audioSceneManager_.SetAudioSceneAfter(audioScene, audioA2dpOffloadFlag_.GetA2dpOffloadFlag());
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED, "failed [%{public}d]", result);
    FetchDeviceAndRoute("SetAudioScene", AudioStreamDeviceChangeReasonExt::ExtEnum::SET_AUDIO_SCENE);

    if (!isSameScene) {
        OnAudioSceneChange(audioScene);
    }
    
    if (audioScene == AUDIO_SCENE_PHONE_CALL) {
        // Make sure the STREAM_VOICE_CALL volume is set before the calling starts.
        audioVolumeManager_.SetVoiceCallVolume(audioVolumeManager_.GetSystemVolumeLevel(STREAM_VOICE_CALL));
    } else {
        audioVolumeManager_.SetVoiceRingtoneMute(false);
    }
    if (lastAudioScene == AUDIO_SCENE_RINGING && audioScene != AUDIO_SCENE_RINGING &&
        audioVolumeManager_.IsAppRingMuted(uid)) {
        audioVolumeManager_.SetAppRingMuted(uid, false); // unmute the STREAM_RING for the app.
    }
    audioCapturerSession_.ReloadSourceForDeviceChange(audioActiveDevice_.GetCurrentInputDevice(),
        audioActiveDevice_.GetCurrentOutputDevice(), "SetAudioScene");
    return SUCCESS;
}

bool AudioCoreService::IsArmUsbDevice(const AudioDeviceDescriptor &deviceDesc)
{
    return audioDeviceManager_.IsArmUsbDevice(deviceDesc);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioCoreService::GetDevices(DeviceFlag deviceFlag)
{
    return audioConnectedDevice_.GetDevicesInner(deviceFlag);
}

int32_t AudioCoreService::SetDeviceActive(InternalDeviceType deviceType, bool active, const int32_t uid)
{
    AUDIO_INFO_LOG("[ADeviceEvent] withlock device %{public}d, active %{public}d from uid %{public}d",
        deviceType, active, uid);
    int32_t ret = audioActiveDevice_.SetDeviceActive(deviceType, active, uid);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetDeviceActive failed");

    FetchDeviceAndRoute("SetDeviceActive", AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE);

    audioCapturerSession_.ReloadSourceForDeviceChange(audioActiveDevice_.GetCurrentInputDevice(),
        audioActiveDevice_.GetCurrentOutputDevice(), "SetDevcieActive");
    return SUCCESS;
}

int32_t AudioCoreService::SetInputDevice(const DeviceType deviceType, const uint32_t sessionID,
    const SourceType sourceType, bool isRunning)
{
    int32_t ret = audioDeviceManager_.SetInputDevice(deviceType, sessionID, sourceType, isRunning);
    if (ret == NEED_TO_FETCH) {
        FetchInputDeviceAndRoute("SetInputDevice");
        return SUCCESS;
    }
    return ret;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioCoreService::GetPreferredOutputDeviceDescInner(
    AudioRendererInfo &rendererInfo, std::string networkId)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceList = {};
    if (rendererInfo.streamUsage <= STREAM_USAGE_UNKNOWN ||
        rendererInfo.streamUsage > STREAM_USAGE_MAX) {
        AUDIO_WARNING_LOG("Invalid usage[%{public}d], return current device.", rendererInfo.streamUsage);
        std::shared_ptr<AudioDeviceDescriptor> devDesc =
            std::make_shared<AudioDeviceDescriptor>(audioActiveDevice_.GetCurrentOutputDevice());
        deviceList.push_back(devDesc);
        return deviceList;
    }
    if (networkId == LOCAL_NETWORK_ID) {
        vector<std::shared_ptr<AudioDeviceDescriptor>> descs =
            audioRouterCenter_.FetchOutputDevices(rendererInfo.streamUsage, -1, "GetPreferredOutputDeviceDescInner");
        for (size_t i = 0; i < descs.size(); i++) {
            std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*descs[i]);
            deviceList.push_back(devDesc);
        }
    } else {
        vector<shared_ptr<AudioDeviceDescriptor>> descs = audioDeviceManager_.GetRemoteRenderDevices();
        for (const auto &desc : descs) {
            std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*desc);
            deviceList.push_back(devDesc);
        }
    }

    return deviceList;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioCoreService::GetPreferredInputDeviceDescInner(
    AudioCapturerInfo &captureInfo, std::string networkId)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceList = {};
    if (captureInfo.sourceType <= SOURCE_TYPE_INVALID ||
        captureInfo.sourceType > SOURCE_TYPE_MAX) {
        std::shared_ptr<AudioDeviceDescriptor> devDesc =
            std::make_shared<AudioDeviceDescriptor>(audioActiveDevice_.GetCurrentInputDevice());
        deviceList.push_back(devDesc);
        return deviceList;
    }

    if (captureInfo.sourceType == SOURCE_TYPE_WAKEUP) {
        std::shared_ptr<AudioDeviceDescriptor> devDesc =
            std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
        devDesc->networkId_ = LOCAL_NETWORK_ID;
        deviceList.push_back(devDesc);
        return deviceList;
    }

    if (networkId == LOCAL_NETWORK_ID) {
        std::shared_ptr<AudioDeviceDescriptor> desc = audioRouterCenter_.FetchInputDevice(captureInfo.sourceType, -1);
        CHECK_AND_RETURN_RET_LOG(desc != nullptr, deviceList, "desc is nullptr");
        if (desc->deviceType_ == DEVICE_TYPE_NONE && (captureInfo.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE ||
            captureInfo.sourceType == SOURCE_TYPE_REMOTE_CAST)) {
            desc->deviceType_ = DEVICE_TYPE_INVALID;
            desc->deviceRole_ = INPUT_DEVICE;
        }
        std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*desc);
        deviceList.push_back(devDesc);
    } else {
        vector<shared_ptr<AudioDeviceDescriptor>> descs = audioDeviceManager_.GetRemoteCaptureDevices();
        for (const auto &desc : descs) {
            std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*desc);
            deviceList.push_back(devDesc);
        }
    }

    return deviceList;
}

int32_t AudioCoreService::GetPreferredOutputStreamType(AudioRendererInfo &rendererInfo,
    const std::string &bundleName)
{
    // Use GetPreferredOutputDeviceDescriptors instead of currentActiveDevice, if prefer != current, recreate stream
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> preferredDeviceList =
        GetPreferredOutputDeviceDescInner(rendererInfo, LOCAL_NETWORK_ID);
    if (preferredDeviceList.size() == 0) {
        return AUDIO_FLAG_NORMAL;
    }

    int32_t flag = AUDIO_FLAG_NORMAL;
    if (isFastControlled_ && (rendererInfo.playerType != PLAYER_TYPE_SOUND_POOL) &&
        (flag == AUDIO_FLAG_MMAP || flag == AUDIO_FLAG_VOIP_FAST)) {
        std::string bundleNamePre = CHECK_FAST_BLOCK_PREFIX + bundleName;
        std::string result = AudioServerProxy::GetInstance().GetAudioParameterProxy(bundleNamePre);
        if (result == "true") {
            AUDIO_INFO_LOG("%{public}s not in fast list", bundleName.c_str());
            return AUDIO_FLAG_NORMAL;
        }
    }
    if (flag == AUDIO_FLAG_VOIP_FAST && audioSceneManager_.GetAudioScene() == AUDIO_SCENE_PHONE_CALL) {
        AUDIO_INFO_LOG("Current scene is phone call, concede incoming voip fast output stream");
        flag = AUDIO_FLAG_NORMAL;
    }
    return flag;
}

int32_t AudioCoreService::GetSessionDefaultOutputDevice(const int32_t callerPid, DeviceType &deviceType)
{
    if (audioSessionService_ == nullptr) {
        AUDIO_ERR_LOG("GetSessionDefaultOutputDevice audioSessionService_ is nullptr!");
        return ERR_UNKNOWN;
    }

    deviceType = audioSessionService_->GetSessionDefaultOutputDevice(callerPid);
    return SUCCESS;
}

int32_t AudioCoreService::SetSessionDefaultOutputDevice(const int32_t callerPid, const DeviceType &deviceType)
{
    CHECK_AND_RETURN_RET_LOG(AudioPolicyConfigManager::GetInstance().GetHasEarpiece(), ERR_NOT_SUPPORTED,
        "the device has no earpiece");

    if (audioSessionService_ == nullptr) {
        AUDIO_ERR_LOG("SetSessionDefaultOutputDevice audioSessionService_ is nullptr!");
        return ERR_UNKNOWN;
    }

    return audioSessionService_->SetSessionDefaultOutputDevice(callerPid, deviceType);
}

int32_t AudioCoreService::GetPreferredInputStreamType(AudioCapturerInfo &capturerInfo)
{
    // Use GetPreferredInputDeviceDescriptors instead of currentActiveDevice, if prefer != current, recreate stream
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> preferredDeviceList =
        GetPreferredInputDeviceDescInner(capturerInfo, LOCAL_NETWORK_ID);
    if (preferredDeviceList.size() == 0) {
        return AUDIO_FLAG_NORMAL;
    }
    int32_t flag = audioDeviceCommon_.GetPreferredInputStreamTypeInner(capturerInfo.sourceType,
        preferredDeviceList[0]->deviceType_,
        capturerInfo.originalFlag, preferredDeviceList[0]->networkId_, capturerInfo.samplingRate);
    if (flag == AUDIO_FLAG_VOIP_FAST && audioSceneManager_.GetAudioScene() == AUDIO_SCENE_PHONE_CALL) {
        AUDIO_INFO_LOG("Current scene is phone call, concede incoming voip fast input stream");
        flag = AUDIO_FLAG_NORMAL;
    }
    return flag;
}

bool AudioCoreService::GetVolumeGroupInfos(std::vector<sptr<VolumeGroupInfo>> &infos)
{
    return audioVolumeManager_.GetVolumeGroupInfosNotWait(infos);
}

std::shared_ptr<AudioDeviceDescriptor> AudioCoreService::GetActiveBluetoothDevice()
{
    std::shared_ptr<AudioDeviceDescriptor> preferredDesc = audioStateManager_.GetPreferredCallRenderDevice();
    if (preferredDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        return preferredDesc;
    }

    std::vector<shared_ptr<AudioDeviceDescriptor>> audioPrivacyDeviceDescriptors =
        audioDeviceManager_.GetCommRenderPrivacyDevices();
    std::vector<shared_ptr<AudioDeviceDescriptor>> activeDeviceDescriptors;

    for (const auto &desc : audioPrivacyDeviceDescriptors) {
        if (desc->deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO || desc->exceptionFlag_ || !desc->isEnable_ ||
            desc->connectState_ == SUSPEND_CONNECTED || AudioPolicyUtils::GetInstance().GetScoExcluded() ||
            audioStateManager_.IsExcludedDevice(AudioDeviceUsage::CALL_OUTPUT_DEVICES, desc)) {
            continue;
        }
        activeDeviceDescriptors.push_back(make_shared<AudioDeviceDescriptor>(*desc));
    }

    uint32_t btDeviceSize = activeDeviceDescriptors.size();
    if (btDeviceSize == 0) {
        activeDeviceDescriptors = audioDeviceManager_.GetCommRenderBTCarDevices();
    }
    btDeviceSize = activeDeviceDescriptors.size();
    if (btDeviceSize == 0) {
        return make_shared<AudioDeviceDescriptor>();
    } else if (btDeviceSize == 1) {
        shared_ptr<AudioDeviceDescriptor> res = std::move(activeDeviceDescriptors[0]);
        return res;
    }

    uint32_t index = 0;
    for (uint32_t i = 1; i < btDeviceSize; ++i) {
        if (activeDeviceDescriptors[i]->connectTimeStamp_ >
            activeDeviceDescriptors[index]->connectTimeStamp_) {
            index = i;
        }
    }
    shared_ptr<AudioDeviceDescriptor> res = std::move(activeDeviceDescriptors[index]);
    return res;
}

void AudioCoreService::OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command)
{
    audioDeviceStatus_.OnDeviceInfoUpdated(desc, command);
}

int32_t AudioCoreService::SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address,
    const int32_t uid)
{
    CHECK_AND_RETURN_RET_LOG(deviceType != DEVICE_TYPE_NONE, ERR_DEVICE_NOT_SUPPORTED, "Invalid device");

    int32_t ret = audioActiveDevice_.SetCallDeviceActive(deviceType, active, address, uid);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetCallDeviceActive failed");
    ret = FetchDeviceAndRoute("SetCallDeviceActive", AudioStreamDeviceChangeReason::OVERRODE);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "FetchDeviceAndRoute failed");
    audioCapturerSession_.ReloadSourceForDeviceChange(audioActiveDevice_.GetCurrentInputDevice(),
        audioActiveDevice_.GetCurrentOutputDevice(), "SetCallDeviceActive");

    return SUCCESS;
}

std::vector<shared_ptr<AudioDeviceDescriptor>> AudioCoreService::GetAvailableDevices(AudioDeviceUsage usage)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    audioDeviceDescriptors = audioDeviceManager_.GetAvailableDevicesByUsage(usage);
    return audioDeviceDescriptors;
}

std::vector<sptr<MicrophoneDescriptor>> AudioCoreService::GetAvailableMicrophones()
{
    return audioMicrophoneDescriptor_.GetAvailableMicrophones();
}

std::vector<sptr<MicrophoneDescriptor>> AudioCoreService::GetAudioCapturerMicrophoneDescriptors(int32_t sessionId)
{
    return audioMicrophoneDescriptor_.GetAudioCapturerMicrophoneDescriptors(sessionId);
}

int32_t AudioCoreService::GetCurrentRendererChangeInfos(vector<shared_ptr<AudioRendererChangeInfo>>
    &audioRendererChangeInfos, bool hasBTPermission, bool hasSystemPermission)
{
    int32_t status = streamCollector_.GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    CHECK_AND_RETURN_RET_LOG(status == SUCCESS, status,
        "AudioPolicyServer Get renderer change info failed");

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices =
        audioConnectedDevice_.GetDevicesInner(OUTPUT_DEVICES_FLAG);
    DeviceType activeDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    DeviceRole activeDeviceRole = OUTPUT_DEVICE;
    std::string activeDeviceMac = audioActiveDevice_.GetCurrentOutputDeviceMacAddr();

    const auto& itr = std::find_if(outputDevices.begin(), outputDevices.end(),
        [&activeDeviceType, &activeDeviceRole, &activeDeviceMac](const std::shared_ptr<AudioDeviceDescriptor> &desc) {
        if ((desc->deviceType_ == activeDeviceType) && (desc->deviceRole_ == activeDeviceRole)) {
            // This A2DP device is not the active A2DP device. Skip it.
            return activeDeviceType != DEVICE_TYPE_BLUETOOTH_A2DP || desc->macAddress_ == activeDeviceMac;
        }
        return false;
    });

    if (itr != outputDevices.end()) {
        size_t rendererInfosSize = audioRendererChangeInfos.size();
        for (size_t i = 0; i < rendererInfosSize; i++) {
            UpdateRendererInfoWhenNoPermission(audioRendererChangeInfos[i], hasSystemPermission);
            audioDeviceCommon_.UpdateDeviceInfo(audioRendererChangeInfos[i]->outputDeviceInfo, *itr,
                hasBTPermission, hasSystemPermission);
        }
    }
    return status;
}

int32_t AudioCoreService::GetCurrentCapturerChangeInfos(
    vector<shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos,
    bool hasBTPermission, bool hasSystemPermission)
{
    int status = streamCollector_.GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    CHECK_AND_RETURN_RET_LOG(status == SUCCESS, status,
        "AudioPolicyServer:: Get capturer change info failed");

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> inputDevices = GetDevices(INPUT_DEVICES_FLAG);
    DeviceType activeDeviceType = audioActiveDevice_.GetCurrentInputDeviceType();
    DeviceRole activeDeviceRole = INPUT_DEVICE;
    for (std::shared_ptr<AudioDeviceDescriptor> desc : inputDevices) {
        if ((desc->deviceType_ == activeDeviceType) && (desc->deviceRole_ == activeDeviceRole)) {
            size_t capturerInfosSize = audioCapturerChangeInfos.size();
            for (size_t i = 0; i < capturerInfosSize; i++) {
                CHECK_AND_CONTINUE(audioCapturerChangeInfos[i] != nullptr);
                UpdateCapturerInfoWhenNoPermission(audioCapturerChangeInfos[i], hasSystemPermission);
                CHECK_AND_CONTINUE(audioRouterCenter_.IsConfigRouterStrategy(
                    audioCapturerChangeInfos[i]->capturerInfo.sourceType));
                audioDeviceCommon_.UpdateDeviceInfo(audioCapturerChangeInfos[i]->inputDeviceInfo, desc,
                    hasBTPermission, hasSystemPermission);
            }
            break;
        }
    }
    return status;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioCoreService::GetExcludedDevices(
    AudioDeviceUsage audioDevUsage)
{
    return audioStateManager_.GetExcludedDevices(audioDevUsage);
}

void AudioCoreService::FetchOutputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo,
    const AudioStreamDeviceChangeReasonExt reason)
{
    AUDIO_WARNING_LOG("Not Supported");
}

void AudioCoreService::FetchInputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo)
{
    AUDIO_WARNING_LOG("Not Supported");
}

int32_t AudioCoreService::ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    return audioRecoveryDevice_.ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

int32_t AudioCoreService::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    return audioRecoveryDevice_.UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

int32_t AudioCoreService::RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
    const sptr<IRemoteObject> &object, const int32_t apiVersion)
{
    if (mode == AUDIO_MODE_RECORD) {
        audioMicrophoneDescriptor_.AddAudioCapturerMicrophoneDescriptor(
            streamChangeInfo.audioCapturerChangeInfo.sessionId, DEVICE_TYPE_NONE);
        if (apiVersion > 0 && apiVersion < API_11) {
            audioDeviceCommon_.UpdateDeviceInfo(streamChangeInfo.audioCapturerChangeInfo.inputDeviceInfo,
                std::make_shared<AudioDeviceDescriptor>(audioActiveDevice_.GetCurrentInputDevice()), false, false);
        }
    } else if (apiVersion > 0 && apiVersion < API_11) {
        audioDeviceCommon_.UpdateDeviceInfo(streamChangeInfo.audioRendererChangeInfo.outputDeviceInfo,
            std::make_shared<AudioDeviceDescriptor>(audioActiveDevice_.GetCurrentOutputDevice()), false, false);
    }
    return streamCollector_.RegisterTracker(mode, streamChangeInfo, object);
}

void AudioCoreService::SetAudioRouteCallback(uint32_t sessionId, const sptr<IRemoteObject> &object)
{
    CHECK_AND_RETURN_LOG(object != nullptr, "object is nullptr");
    sptr<IStandardAudioPolicyManagerListener> listener = iface_cast<IStandardAudioPolicyManagerListener>(object);
    CHECK_AND_RETURN_LOG(listener != nullptr, "listener is nullptr");
    std::lock_guard<std::mutex> lock(routeUpdateCallbackMutex_);
    routeUpdateCallback_[sessionId] = listener;
}

void AudioCoreService::UnsetAudioRouteCallback(uint32_t sessionId)
{
    std::lock_guard<std::mutex> lock(routeUpdateCallbackMutex_);
    CHECK_AND_RETURN_LOG(routeUpdateCallback_.count(sessionId) != 0, "sessionId not exists");
    routeUpdateCallback_.erase(sessionId);
}

int32_t AudioCoreService::UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    int32_t ret = streamCollector_.UpdateTracker(mode, streamChangeInfo);
    HandleAudioCaptureState(mode, streamChangeInfo);

    const auto &rendererState = streamChangeInfo.audioRendererChangeInfo.rendererState;
    if (rendererState == RENDERER_PREPARED || rendererState == RENDERER_NEW || rendererState == RENDERER_INVALID) {
        return ret; // only update tracker in new and prepared
    }

    const auto &rendererChangeInfo = streamChangeInfo.audioRendererChangeInfo;
    if ((mode == AUDIO_MODE_PLAYBACK) && (rendererChangeInfo.rendererInfo.streamUsage == STREAM_USAGE_RINGTONE ||
        rendererChangeInfo.rendererInfo.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION)) {
        if ((rendererState == RENDERER_STOPPED ||rendererState == RENDERER_RELEASED ||
            rendererState == RENDERER_PAUSED)) {
            Bluetooth::AudioHfpManager::DeleteVirtualCallStream(rendererChangeInfo.sessionId);
        }
    }
    
    UpdateTracker(mode, streamChangeInfo, rendererState);

    if (audioA2dpOffloadManager_) {
        audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream(audioActiveDevice_.GetCurrentOutputDeviceType());
    }

    SendA2dpConnectedWhileRunning(rendererState, streamChangeInfo.audioRendererChangeInfo.sessionId);

    if (mode == AUDIO_MODE_PLAYBACK) {
        CheckOffloadStream(streamChangeInfo);
    }
    return ret;
}

void AudioCoreService::RegisteredTrackerClientDied(pid_t uid, pid_t pid)
{
    UpdateDefaultOutputDeviceWhenStopping(static_cast<int32_t>(uid));
    UpdateInputDeviceWhenStopping(static_cast<int32_t>(uid));

    audioMicrophoneDescriptor_.RemoveAudioCapturerMicrophoneDescriptor(static_cast<int32_t>(uid));
    streamCollector_.RegisteredTrackerClientDied(static_cast<int32_t>(uid), static_cast<int32_t>(pid));
    CHECK_AND_RETURN_LOG(pipeManager_ != nullptr, "pipeManager is nullptr");
    std::vector<uint32_t> sessionIds = pipeManager_->GetStreamIdsByUid(uid,
        (AUDIO_OUTPUT_FLAG_FAST | AUDIO_INPUT_FLAG_FAST));
    for (auto sessionId : sessionIds) {
        ReleaseClient(sessionId);
    }
    sessionIds = pipeManager_->GetStreamIdsByUid(uid);
    for (auto sessionId : sessionIds) {
        UnsetAudioRouteCallback(sessionId);
    }
    FetchOutputDeviceAndRoute("RegisteredTrackerClientDied");

    audioDeviceCommon_.ClientDiedDisconnectScoNormal();
    audioDeviceCommon_.ClientDiedDisconnectScoRecognition();
}

bool AudioCoreService::ConnectServiceAdapter()
{
    return audioPolicyManager_.ConnectServiceAdapter();
}

void AudioCoreService::OnReceiveBluetoothEvent(const std::string macAddress, const std::string deviceName)
{
    audioDeviceManager_.OnReceiveBluetoothEvent(macAddress, deviceName);
    audioConnectedDevice_.SetDisplayName(macAddress, deviceName);
}

void AudioCoreService::DumpSelectHistory(std::string &dumpString)
{
    dumpString += "Select device history infos\n";
    std::lock_guard<std::mutex> lock(hisQueueMutex_);
    dumpString += "  - TotalPipeNums: " + std::to_string(selectDeviceHistory_.size()) + "\n\n";
    for (auto &item : selectDeviceHistory_) {
        dumpString += item + "\n";
    }
    dumpString += "\n";
}

void AudioCoreService::RecordSelectDevice(const std::string &selectHistory)
{
    std::lock_guard<std::mutex> lock(hisQueueMutex_);
    if (selectDeviceHistory_.size() < SELECT_DEVICE_HISTORY_LIMIT) {
        selectDeviceHistory_.push_back(selectHistory);
        return;
    }
    while (selectDeviceHistory_.size() >= SELECT_DEVICE_HISTORY_LIMIT) {
        selectDeviceHistory_.pop_front();
    }
    selectDeviceHistory_.push_back(selectHistory);
    return;
}

int32_t AudioCoreService::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    if (!selectedDesc.empty() && selectedDesc[0] != nullptr) {
        // eg. 2025-06-22-21:12:07:666|Uid: 6700 select output device: LOCAL_DEVICE type:2
        std::string selectHistory = GetTime() + "|Uid:" + std::to_string(IPCSkeleton::GetCallingUid()) + " Pid:" +
            std::to_string(IPCSkeleton::GetCallingPid()) + " select output device:" + selectedDesc[0]->networkId_ +
            " type:" + std::to_string(selectedDesc[0]->deviceType_);
        RecordSelectDevice(selectHistory);
    }

    return audioRecoveryDevice_.SelectOutputDevice(audioRendererFilter, selectedDesc);
}

void AudioCoreService::NotifyDistributedOutputChange(const AudioDeviceDescriptor &deviceDesc)
{
    audioDeviceCommon_.NotifyDistributedOutputChange(deviceDesc);
}

int32_t AudioCoreService::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    if (!selectedDesc.empty() && selectedDesc[0] != nullptr) {
        // eg. 2025-06-22-21:12:07:666|Uid: 6700 select input device: LOCAL_DEVICE type:15
        std::string selectHistory = GetTime() + "|Uid:" + std::to_string(IPCSkeleton::GetCallingUid()) + " Pid:" +
            std::to_string(IPCSkeleton::GetCallingPid()) + " select input device:" + selectedDesc[0]->networkId_ +
            " type:" + std::to_string(selectedDesc[0]->deviceType_);
        RecordSelectDevice(selectHistory);
    }

    return audioRecoveryDevice_.SelectInputDevice(audioCapturerFilter, selectedDesc);
}

void AudioCoreService::NotifyRemoteRenderState(std::string networkId, std::string condition, std::string value)
{
    AUDIO_INFO_LOG("device<%{public}s> condition:%{public}s value:%{public}s",
        GetEncryptStr(networkId).c_str(), condition.c_str(), value.c_str());

    vector<SinkInput> sinkInputs;
    audioPolicyManager_.GetAllSinkInputs(sinkInputs);
    vector<SinkInput> targetSinkInputs = {};
    for (auto sinkInput : sinkInputs) {
        if (sinkInput.sinkName == networkId) {
            targetSinkInputs.push_back(sinkInput);
        }
    }
    AUDIO_DEBUG_LOG("move [%{public}zu] of all [%{public}zu]sink-inputs to local.",
        targetSinkInputs.size(), sinkInputs.size());
    std::shared_ptr<AudioDeviceDescriptor> localDevice = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN_LOG(localDevice != nullptr, "Device error: null device.");
    localDevice->networkId_ = LOCAL_NETWORK_ID;
    localDevice->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    localDevice->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;

    int32_t ret;
    AudioDeviceDescriptor curOutputDeviceDesc = audioActiveDevice_.GetCurrentOutputDevice();
    if (localDevice->deviceType_ != curOutputDeviceDesc.deviceType_) {
        AUDIO_WARNING_LOG("device[%{public}d] not active, use device[%{public}d] instead.",
            static_cast<int32_t>(localDevice->deviceType_), static_cast<int32_t>(curOutputDeviceDesc.deviceType_));
        ret = audioDeviceCommon_.MoveToLocalOutputDevice(targetSinkInputs,
            std::make_shared<AudioDeviceDescriptor>(curOutputDeviceDesc));
    } else {
        ret = audioDeviceCommon_.MoveToLocalOutputDevice(targetSinkInputs, localDevice);
    }
    CHECK_AND_RETURN_LOG((ret == SUCCESS), "MoveToLocalOutputDevice failed!");

    // Suspend device, notify audio stream manager that device has been changed.
    ret = audioPolicyManager_.SuspendAudioDevice(networkId, true);
    CHECK_AND_RETURN_LOG((ret == SUCCESS), "SuspendAudioDevice failed!");

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc = {};
    desc.push_back(localDevice);
    UpdateTrackerDeviceChange(desc);
    audioDeviceCommon_.OnPreferredOutputDeviceUpdated(curOutputDeviceDesc,
        AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE);
    AUDIO_DEBUG_LOG("Success");
}

int32_t AudioCoreService::OnCapturerSessionAdded(uint64_t sessionID, SessionInfo sessionInfo,
    AudioStreamInfo streamInfo)
{
    return audioCapturerSession_.OnCapturerSessionAdded(sessionID, sessionInfo, streamInfo);
}

void AudioCoreService::OnCapturerSessionRemoved(uint64_t sessionID)
{
    audioCapturerSession_.OnCapturerSessionRemoved(sessionID);
}

void AudioCoreService::CloseWakeUpAudioCapturer()
{
    audioCapturerSession_.CloseWakeUpAudioCapturer();
}

int32_t AudioCoreService::TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason)
{
    FetchOutputDeviceAndRoute("TriggerFetchDevice", reason);
    FetchInputDeviceAndRoute("TriggerFetchDevice");

    // update a2dp offload
    audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream();
    audioCapturerSession_.ReloadSourceForDeviceChange(audioActiveDevice_.GetCurrentInputDevice(),
        audioActiveDevice_.GetCurrentOutputDevice(), "TriggerFetchDevice");
    return SUCCESS;
}

// No lock
int32_t AudioCoreService::SetAudioDeviceAnahsCallback(const sptr<IRemoteObject> &object)
{
    return deviceStatusListener_->SetAudioDeviceAnahsCallback(object);
}

int32_t AudioCoreService::UnsetAudioDeviceAnahsCallback()
{
    return deviceStatusListener_->UnsetAudioDeviceAnahsCallback();
}

void AudioCoreService::OnUpdateAnahsSupport(std::string anahsShowType)
{
    AUDIO_INFO_LOG("OnUpdateAnahsSupport show type: %{public}s", anahsShowType.c_str());
    deviceStatusListener_->UpdateAnahsPlatformType(anahsShowType);
}

void AudioCoreService::RegisterBluetoothListener()
{
#ifdef BLUETOOTH_ENABLE
    AUDIO_INFO_LOG("Enter");
    Bluetooth::RegisterDeviceObserver(deviceStatusListener_->deviceObserver_);
    if (isBtListenerRegistered) {
        AUDIO_INFO_LOG("audio policy service already register bt listerer, return");
        return;
    }
    if (!isBtCrashed) {
        Bluetooth::AudioA2dpManager::RegisterBluetoothA2dpListener();
        Bluetooth::AudioHfpManager::RegisterBluetoothScoListener();
    }
    isBtListenerRegistered = true;
    isBtCrashed = false;
    RegisterBluetoothDeathCallback();
    AudioPolicyUtils::GetInstance().SetBtConnecting(true);
    Bluetooth::AudioA2dpManager::CheckA2dpDeviceReconnect();
    Bluetooth::AudioHfpManager::CheckHfpDeviceReconnect();
    AudioPolicyUtils::GetInstance().SetBtConnecting(false);
#endif
}

void AudioCoreService::UnregisterBluetoothListener()
{
#ifdef BLUETOOTH_ENABLE
    AUDIO_INFO_LOG("Enter");
    Bluetooth::UnregisterDeviceObserver();
    Bluetooth::AudioA2dpManager::UnregisterBluetoothA2dpListener();
    Bluetooth::AudioHfpManager::UnregisterBluetoothScoListener();
    isBtListenerRegistered = false;
#endif
}

void AudioCoreService::ConfigDistributedRoutingRole(
    const std::shared_ptr<AudioDeviceDescriptor> descriptor, CastType type)
{
    AUDIO_INFO_LOG("[ADeviceEvent] device %{public}d, cast type %{public}d",
        (descriptor != nullptr) ? descriptor->deviceType_ : -1, type);
    StoreDistributedRoutingRoleInfo(descriptor, type);
    FetchDeviceAndRoute("ConfigDistributedRoutingRole", AudioStreamDeviceChangeReason::OVERRODE);
}

int32_t AudioCoreService::SetRingerMode(AudioRingerMode ringMode)
{
    int32_t result = audioPolicyManager_.SetRingerMode(ringMode);
    if (result == SUCCESS) {
        if (Util::IsRingerAudioScene(audioSceneManager_.GetAudioScene(true))) {
            AUDIO_INFO_LOG("[ADeviceEvent] fetch output device after switch new ringmode");
            FetchOutputDeviceAndRoute("SetRingerMode");
            audioCapturerSession_.ReloadSourceForDeviceChange(audioActiveDevice_.GetCurrentInputDevice(),
                audioActiveDevice_.GetCurrentOutputDevice(), "SetRingerMode");
        }
        Volume vol = {false, 1.0f, 0};
        DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
        vol.isMute = (ringMode == RINGER_MODE_NORMAL) ? false : true;
        vol.volumeInt = static_cast<uint32_t>(GetSystemVolumeLevel(STREAM_RING));
        vol.volumeFloat = GetSystemVolumeInDb(STREAM_RING, vol.volumeInt, curOutputDeviceType);
        audioVolumeManager_.SetSharedVolume(STREAM_RING, curOutputDeviceType, vol);
    }
    return result;
}

bool AudioCoreService::IsNoRunningStream(std::vector<std::shared_ptr<AudioStreamDescriptor>> outputStreamDescs)
{
    for (auto streamDesc : outputStreamDescs) {
        if (streamDesc->streamStatus_ == STREAM_STATUS_STARTED) {
            return false;
        }
    }
    return true;
}

int32_t AudioCoreService::FetchOutputDeviceAndRoute(std::string caller, const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, ERROR, "pipeManager_ is nullptr");
    std::vector<std::shared_ptr<AudioStreamDescriptor>> outputStreamDescs = pipeManager_->GetAllOutputStreamDescs();
    AUDIO_INFO_LOG("[DeviceFetchStart] by %{public}s for %{public}zu output streams, in devices %{public}s",
        caller.c_str(), outputStreamDescs.size(), audioDeviceManager_.GetConnDevicesStr().c_str());

    if (outputStreamDescs.empty() && !pipeManager_->IsModemCommunicationIdExist()) {
        return HandleFetchOutputWhenNoRunningStream(reason);
    }
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> modemDescs;
    CheckModemScene(modemDescs, reason);

    for (auto &streamDesc : outputStreamDescs) {
        CHECK_AND_CONTINUE_LOG(streamDesc != nullptr, "Stream desc is nullptr");
        streamDesc->oldDeviceDescs_ = streamDesc->newDeviceDescs_;
        StreamUsage streamUsage = StreamUsage::STREAM_USAGE_INVALID;
        if (audioSessionService_ != nullptr) {
            streamUsage = audioSessionService_->GetAudioSessionStreamUsage(GetRealPid(streamDesc));
        }
        streamUsage = (streamUsage != StreamUsage::STREAM_USAGE_INVALID) ? streamUsage :
            streamDesc->rendererInfo_.streamUsage;
        streamDesc->newDeviceDescs_ =
            audioRouterCenter_.FetchOutputDevices(streamUsage, GetRealUid(streamDesc),
                caller + "FetchOutputDeviceAndRoute");
        AUDIO_INFO_LOG("[AudioSession] streamUsage %{public}d renderer streamUsage %{public}d",
            streamUsage, streamDesc->rendererInfo_.streamUsage);
        AUDIO_INFO_LOG("[DeviceFetchInfo] device %{public}s for stream %{public}d with status %{public}u",
            streamDesc->GetNewDevicesTypeString().c_str(), streamDesc->sessionId_, streamDesc->streamStatus_);
        AUDIO_INFO_LOG("Target audioFlag 0x%{public}x for stream %{public}u",
            streamDesc->audioFlag_, streamDesc->sessionId_);
    }

    audioActiveDevice_.UpdateStreamDeviceMap("FetchOutputDeviceAndRoute");
    int32_t ret = FetchRendererPipesAndExecute(outputStreamDescs, reason);
    UpdateModemRoute(modemDescs);
    if (IsNoRunningStream(outputStreamDescs)) {
        HandleFetchOutputWhenNoRunningStream(reason);
    }
    return ret;
}

int32_t AudioCoreService::FetchInputDeviceAndRoute(std::string caller)
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> inputStreamDescs = pipeManager_->GetAllInputStreamDescs();
    AUDIO_INFO_LOG("[DeviceFetchStart] by %{public}s for %{public}zu input streams, in devices %{public}s",
        caller.c_str(), inputStreamDescs.size(), audioDeviceManager_.GetConnDevicesStr().c_str());

    if (inputStreamDescs.empty()) {
        return HandleFetchInputWhenNoRunningStream();
    }

    bool needUpdateActiveDevice = true;
    bool isUpdateActiveDevice = false;
    for (auto streamDesc : inputStreamDescs) {
        streamDesc->oldDeviceDescs_ = streamDesc->newDeviceDescs_;
        streamDesc->newDeviceDescs_.clear();
        std::shared_ptr<AudioDeviceDescriptor> inputDeviceDesc =
            audioRouterCenter_.FetchInputDevice(streamDesc->capturerInfo_.sourceType, GetRealUid(streamDesc),
                streamDesc->sessionId_);
        CHECK_AND_RETURN_RET_LOG(inputDeviceDesc != nullptr, ERR_INVALID_PARAM, "inputDeviceDesc is nullptr");
        streamDesc->newDeviceDescs_.push_back(inputDeviceDesc);
        AUDIO_INFO_LOG("[DeviceFetchInfo] device %{public}s for stream %{public}d with status %{public}u",
            streamDesc->GetNewDevicesTypeString().c_str(), streamDesc->sessionId_, streamDesc->streamStatus_);

        UpdateRecordStreamFlag(streamDesc);
        if (!HandleInputStreamInRunning(streamDesc)) {
            continue;
        }

        // handle nearlink
        int32_t inputRet = ActivateInputDevice(streamDesc);
        CHECK_AND_RETURN_RET_LOG(inputRet == SUCCESS, inputRet, "Activate input device failed");

        if (needUpdateActiveDevice) {
            isUpdateActiveDevice = UpdateInputDevice(inputDeviceDesc, GetRealUid(streamDesc));
            needUpdateActiveDevice = false;
        }
    }

    int32_t ret = FetchCapturerPipesAndExecute(inputStreamDescs);
    if (isUpdateActiveDevice) {
        OnPreferredInputDeviceUpdated(audioActiveDevice_.GetCurrentInputDeviceType(), ""); // networkId is not used.
    }
    return ret;
}

void AudioCoreService::SetAudioServerProxy()
{
    AUDIO_INFO_LOG("SetAudioServerProxy Start");
    const sptr<IStandardAudioService> gsp = AudioServerProxy::GetInstance().GetAudioServerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "SetAudioServerProxy, Audio Server Proxy is null");
    audioPolicyManager_.SetAudioServerProxy(gsp);
}

DirectPlaybackMode AudioCoreService::GetDirectPlaybackSupport(const AudioStreamInfo &streamInfo,
    const StreamUsage &streamUsage)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs = audioRouterCenter_.FetchOutputDevices(
        streamUsage, getuid(), "GetDirectPlaybackSupport");
    CHECK_AND_RETURN_RET_LOG(!descs.empty(), DIRECT_PLAYBACK_NOT_SUPPORTED, "find output device failed");
    return policyConfigMananger_.GetDirectPlaybackSupport(descs.front(), streamInfo);
}

#ifdef BLUETOOTH_ENABLE
void AudioCoreService::RegisterBluetoothDeathCallback()
{
    lock_guard<mutex> lock(g_btProxyMutex);
    AUDIO_INFO_LOG("Enter");
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_LOG(samgr != nullptr,
        "get sa manager failed");
    sptr<IRemoteObject> object = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
    CHECK_AND_RETURN_LOG(object != nullptr,
        "get audio service remote object failed");
    // register death recipent
    sptr<AudioServerDeathRecipient> asDeathRecipient =
        new(std::nothrow) AudioServerDeathRecipient(getpid(), getuid());
    if (asDeathRecipient != nullptr) {
        asDeathRecipient->SetNotifyCb([] (pid_t pid, pid_t uid) {
            BluetoothServiceCrashedCallback(pid, uid);
        });
        bool result = object->AddDeathRecipient(asDeathRecipient);
        if (!result) {
            AUDIO_ERR_LOG("failed to add deathRecipient");
        }
    }
}

void AudioCoreService::BluetoothServiceCrashedCallback(pid_t pid, pid_t uid)
{
    AUDIO_INFO_LOG("Bluetooth sa crashed, will restore proxy in next call");
    lock_guard<mutex> lock(g_btProxyMutex);
    isBtListenerRegistered = false;
    isBtCrashed = true;
    Bluetooth::AudioA2dpManager::DisconnectBluetoothA2dpSink();
    Bluetooth::AudioA2dpManager::DisconnectBluetoothA2dpSource();
    Bluetooth::AudioHfpManager::DisconnectBluetoothHfpSink();
}
#endif

void AudioCoreService::UpdateStreamPropInfo(const std::string &adapterName, const std::string &pipeName,
    const std::list<DeviceStreamInfo> &deviceStreamInfo, const std::list<std::string> &supportDevices)
{
    policyConfigMananger_.UpdateStreamPropInfo(adapterName, pipeName, deviceStreamInfo, supportDevices);
}

void AudioCoreService::ClearStreamPropInfo(const std::string &adapterName, const std::string &pipeName)
{
    policyConfigMananger_.ClearStreamPropInfo(adapterName, pipeName);
}

uint32_t AudioCoreService::GetStreamPropInfoSize(const std::string &adapterName, const std::string &pipeName)
{
    return policyConfigMananger_.GetStreamPropInfoSize(adapterName, pipeName);
}

int32_t AudioCoreService::CaptureConcurrentCheck(uint32_t sessionId)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = pipeManager_->GetStreamDescById(sessionId);
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERR_NULL_POINTER, "streamDesc is null");
    if (streamDesc->audioMode_ != AUDIO_MODE_RECORD) {
        return ERR_NOT_SUPPORTED;
    }
 
    auto dfxResult = std::make_unique<struct ConcurrentCaptureDfxResult>();
    if (!WriteCapturerConcurrentMsg(streamDesc, dfxResult)) {
        return ERR_INVALID_HANDLE;
    }
    LogCapturerConcurrentResult(dfxResult);
    WriteCapturerConcurrentEvent(dfxResult);
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
