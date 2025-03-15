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
#include "data_share_observer_callback.h"
#include "audio_spatialization_service.h"


namespace OHOS {
namespace AudioStandard {
namespace {
static const int32_t MEDIA_SERVICE_UID = 1013;
const int32_t DATA_LINK_CONNECTED = 11;
static constexpr int64_t WAIT_LOAD_DEFAULT_DEVICE_TIME_MS = 200; // 200ms
static constexpr int32_t RETRY_TIMES = 25;
const uint32_t FIRST_SESSIONID = 100000;
constexpr uint32_t MAX_VALID_SESSIONID = UINT32_MAX - FIRST_SESSIONID;
static const int VOLUME_LEVEL_DEFAULT_SIZE = 3;
static const char* SETTINGS_DATA_BASE_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
static const char* SETTINGS_DATA_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
static const char* AUDIO_SERVICE_PKG = "audio_manager_service";
static const char* PREDICATES_STRING = "settings.general.device_name";
static const char* CONFIG_AUDIO_MONO_KEY = "master_mono";
static const char* CHECK_FAST_BLOCK_PREFIX = "Is_Fast_Blocked_For_AppName#";
static const int32_t BLUETOOTH_FETCH_RESULT_DEFAULT = 0;
static const int32_t BLUETOOTH_FETCH_RESULT_CONTINUE = 1;
static const int32_t BLUETOOTH_FETCH_RESULT_ERROR = 2;
static const uint32_t BT_BUFFER_ADJUSTMENT_FACTOR = 50;
}

static const std::vector<std::string> SourceNames = {
    std::string(PRIMARY_MIC),
    std::string(BLUETOOTH_MIC),
    std::string(USB_MIC),
    std::string(PRIMARY_WAKEUP),
    std::string(FILE_SOURCE)
};
static const char* CONFIG_AUDIO_BALANACE_KEY = "master_balance";
bool AudioCoreService::isBtListenerRegistered = false;
#ifdef BLUETOOTH_ENABLE
static sptr<IStandardAudioService> g_btProxy = nullptr;
mutex g_btProxyMutex;
#endif

static std::string GetEncryptAddr(const std::string &addr)
{
    const int32_t START_POS = 6;
    const int32_t END_POS = 13;
    const int32_t ADDRESS_STR_LEN = 17;
    if (addr.empty() || addr.length() != ADDRESS_STR_LEN) {
        return std::string("");
    }
    std::string tmp = "**:**:**:**:**:**";
    std::string out = addr;
    for (int i = START_POS; i <= END_POS; i++) {
        out[i] = tmp[i];
    }
    return out;
}

AudioCoreService::AudioCoreService() :
    audioPolicyServerHandler_(DelayedSingleton<AudioPolicyServerHandler>::GetInstance()),
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
    audioPipeSelector_(AudioPipeSelector::GetPipeSelector()),
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

    policyConfigMananger_.Init();
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

int32_t AudioCoreService::CreateRendererClient(
    std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t &audioFlag, uint32_t &sessionId)
{
    if (streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION) {
        audioFlag = AUDIO_FLAG_NORMAL;
        sessionId = GenerateSessionId();
        AUDIO_INFO_LOG("Modem communication, sessionId %{public}u", sessionId);
        pipeManager_->SetModemCommunicationId(sessionId);
        return SUCCESS;
    }
    streamDesc->oldDeviceDescs_ = streamDesc->newDeviceDescs_;
    // Select device
    int32_t ret = SUCCESS;
    streamDesc->newDeviceDescs_ =
        audioRouterCenter_.FetchOutputDevices(streamDesc->rendererInfo_.streamUsage, GetRealUid(streamDesc));
    for (auto device : streamDesc->newDeviceDescs_) {
        AUDIO_INFO_LOG("Device type %{public}d", device->deviceType_);
    }
    {
        // handle a2dp
        std::string encryptMacAddr = 
            GetEncryptAddr(streamDesc->newDeviceDescs_.front()->macAddress_);
        int32_t bluetoothFetchResult = BluetoothDeviceFetchOutputHandle(streamDesc->newDeviceDescs_.front(),
            AudioStreamDeviceChangeReason::UNKNOWN, encryptMacAddr);
        if (bluetoothFetchResult == BLUETOOTH_FETCH_RESULT_CONTINUE ||
            bluetoothFetchResult == BLUETOOTH_FETCH_RESULT_ERROR) {
            return ERROR;
        }
    }
    SetPlaybackStreamFlag(streamDesc);
    AUDIO_INFO_LOG("Will use audio flag: %{public}u", streamDesc->audioFlag_);

    // Fetch pipe
    ret = FetchRendererPipeAndExecute(streamDesc, sessionId, audioFlag);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "FetchPipeAndExecute failed");

    return SUCCESS;
}

int32_t AudioCoreService::CreateCapturerClient(
    std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t &audioFlag, uint32_t &sessionId)
{
    streamDesc->oldDeviceDescs_ = streamDesc->newDeviceDescs_;
    // Select device
    int32_t ret = SUCCESS;
    std::shared_ptr<AudioDeviceDescriptor> inputDeviceDesc =
        audioRouterCenter_.FetchInputDevice(streamDesc->capturerInfo_.sourceType, GetRealUid(streamDesc));
    streamDesc->newDeviceDescs_.clear();
    streamDesc->newDeviceDescs_.push_back(inputDeviceDesc);
    AUDIO_INFO_LOG("New stream device type %{public}d", inputDeviceDesc->deviceType_);

    {
        // handle a2dp
        if (streamDesc->newDeviceDescs_[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
            BluetoothScoFetch(streamDesc);
        }
    }

    SetRecordStreamFlag(streamDesc);
    AUDIO_INFO_LOG("Will use audio flag: %{public}u", streamDesc->audioFlag_);

    // Fetch pipe
    ret = FetchCapturerPipeAndExecute(streamDesc, audioFlag, sessionId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "FetchPipeAndExecute failed");

    return SUCCESS;
}


void AudioCoreService::BluetoothScoFetch(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    Trace trace("AudioCoreService::BluetoothScoFetch");
    shared_ptr<AudioDeviceDescriptor> desc = streamDesc->newDeviceDescs_[0];
    int32_t ret;
    if (Util::IsScoSupportSource(streamDesc->capturerInfo_.sourceType)) {
        int32_t activeRet = Bluetooth::AudioHfpManager::SetActiveHfpDevice(desc->macAddress_);
        if (activeRet != SUCCESS) {
            AUDIO_ERR_LOG("Active hfp device failed, retrigger fetch input device");
            desc->exceptionFlag_ = true;
            audioDeviceManager_.UpdateDevicesListInfo(
                std::make_shared<AudioDeviceDescriptor>(*desc), EXCEPTION_FLAG_UPDATE);
            SelectInputDeviceAndRoute();
        }
        ret = ScoInputDeviceFetchedForRecongnition(true, desc->macAddress_, desc->connectState_);
    } else {
        ret = HandleScoInputDeviceFetched(streamDesc);
    }
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("sco [%{public}s] is not connected yet",
            GetEncryptAddr(desc->macAddress_).c_str());
    }
}

int32_t AudioCoreService::ScoInputDeviceFetchedForRecongnition(bool handleFlag, const std::string &address,
    ConnectState connectState)
{
    AUDIO_INFO_LOG("handleflag %{public}d, address %{public}s, connectState %{public}d",
        handleFlag, address.c_str(), connectState);
    if (handleFlag && connectState != DEACTIVE_CONNECTED) {
        return SUCCESS;
    }
    Bluetooth::BluetoothRemoteDevice device = Bluetooth::BluetoothRemoteDevice(address);
    return Bluetooth::AudioHfpManager::HandleScoWithRecongnition(handleFlag, device);
}

int32_t AudioCoreService::HandleScoInputDeviceFetched(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
#ifdef BLUETOOTH_ENABLE
    AUDIO_INFO_LOG("In");
    shared_ptr<AudioDeviceDescriptor> desc = streamDesc->newDeviceDescs_[0];
    int32_t ret = Bluetooth::AudioHfpManager::SetActiveHfpDevice(desc->macAddress_);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Active hfp device failed, retrigger fetch input device");
        desc->exceptionFlag_ = true;
        audioDeviceManager_.UpdateDevicesListInfo(
            std::make_shared<AudioDeviceDescriptor>(*desc), EXCEPTION_FLAG_UPDATE);
        SelectInputDeviceAndRoute();
        return ERROR;
    }
    AUDIO_INFO_LOG("desc->connectState_ %{public}d", desc->connectState_);
    if (desc->connectState_ == DEACTIVE_CONNECTED || !audioSceneManager_.IsSameAudioScene()) {
        AUDIO_INFO_LOG("In2");
        Bluetooth::AudioHfpManager::ConnectScoWithAudioScene(audioSceneManager_.GetAudioScene(true));
        return SUCCESS;
    }
#endif
    return SUCCESS;
}

bool AudioCoreService::IsStreamSupportLowpower(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    Trace trace("IsStreamSupportLowpower");
    if (pipeManager_->PcmOffloadSessionCount() > 0) {
        AUDIO_INFO_LOG("PIPE_TYPE_OFFLOAD already exist.");
        return false;
    }
    if (!streamDesc->rendererInfo_.isOffloadAllowed) {
        AUDIO_INFO_LOG("normal stream beacuse renderInfo not support offload.");
        return false;
    }
    if (streamDesc->streamInfo_.channels < MONO || streamDesc->streamInfo_.channels > STEREO) {
        AUDIO_INFO_LOG("normal stream beacuse channels.");
        return false;
    }

    if (streamDesc->rendererInfo_.streamUsage != STREAM_USAGE_MUSIC &&
        streamDesc->rendererInfo_.streamUsage != STREAM_USAGE_AUDIOBOOK) {
        AUDIO_INFO_LOG("normal stream beacuse streamUsage.");
        return false;
    }
    
    AudioSpatializationState spatialState =
        AudioSpatializationService::GetAudioSpatializationService().GetSpatializationState();
    bool effectOffloadFlag = AudioServerProxy::GetInstance().GetEffectOffloadEnabledProxy();
    if (spatialState.spatializationEnabled && !effectOffloadFlag) {
        AUDIO_INFO_LOG("spatialization effect in arm, Skipped.");
        return false;
    }

    if (streamDesc->newDeviceDescs_[0]->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        // a2dp offload
        return true;
    }

    if (streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_SPEAKER &&
        streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_USB_HEADSET) {
            AUDIO_INFO_LOG("normal stream, deviceType: %{public}d", streamDesc->newDeviceDescs_[0]->deviceType_);
            return false;
        }
    return true;
}

bool IsStreamSupportMultiChannel(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    Trace trace("IsStreamSupportMultiChannel");
    AUDIO_INFO_LOG("In");

    if (streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_SPEAKER &&
        streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP) {
            AUDIO_INFO_LOG("normal stream, deviceType: %{public}d", streamDesc->newDeviceDescs_[0]->deviceType_);
            return false;
    }
    if (streamDesc->streamInfo_.channels <= STEREO) {
        AUDIO_INFO_LOG("normal stream beacuse channels.");
        return false;
    }
    // The multi-channel algorithm needs to be supported in the dsp
    return AudioServerProxy::GetInstance().GetEffectOffloadEnabledProxy();
}

bool IsStreamSupportDirect(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    Trace trace("IsStreamSupportDirect");
    if (streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_WIRED_HEADSET &&
        streamDesc->newDeviceDescs_[0]->deviceType_ != DEVICE_TYPE_USB_HEADSET) {
            AUDIO_INFO_LOG("normal stream, deviceType: %{public}d",
                streamDesc->newDeviceDescs_[0]->deviceType_);
            return false;
        }
    if (streamDesc->rendererInfo_.streamUsage != STREAM_USAGE_MUSIC ||
        streamDesc->streamInfo_.samplingRate < SAMPLE_RATE_48000 ||
        streamDesc->streamInfo_.format < SAMPLE_S24LE ||
        streamDesc->rendererInfo_.pipeType != PIPE_TYPE_DIRECT_MUSIC) {
            AUDIO_INFO_LOG("normal stream because stream info");
            return false;
        }
    if (streamDesc->streamInfo_.samplingRate > SAMPLE_RATE_192000) {
        AUDIO_INFO_LOG("sample rate over 192k");
        return false;
    }
    return true;
}

void AudioCoreService::SetPlaybackStreamFlag(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    AUDIO_INFO_LOG("deviceType: %{public}d", streamDesc->newDeviceDescs_.front()->deviceType_);
    // fast/normal has done in audioRendererPrivate
    if (streamDesc->rendererInfo_.originalFlag == AUDIO_FLAG_FORCED_NORMAL) {
        streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
        AUDIO_INFO_LOG("Forced normal");
        return;
    }

    if (streamDesc->newDeviceDescs_.back()->deviceType_ == DEVICE_TYPE_REMOTE_CAST ||
        streamDesc->newDeviceDescs_.back()->networkId_ == "REMOTE_NETWORK_ID") {
        streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_NORMAL; // remote cast/remote device select normal
        AUDIO_INFO_LOG("remote device/cast set normal");
        return;
    }

    if (streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
        streamDesc->rendererInfo_.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION) {
        std::string sinkPortName =
            AudioPolicyUtils::GetInstance().GetSinkPortName(streamDesc->newDeviceDescs_.front()->deviceType_);
        AUDIO_INFO_LOG("sinkPortName %{public}s", sinkPortName.c_str());
        // in plan: if has two voip, return normal
        streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_VOIP;
        AUDIO_INFO_LOG("sinkPortName: %{public}s, audioFlag_ %{public}d", sinkPortName.c_str(), streamDesc->audioFlag_);
        return;
    }
    AUDIO_INFO_LOG("rendererFlag: %{public}d", streamDesc->rendererInfo_.rendererFlags);
    switch (streamDesc->rendererInfo_.originalFlag)
    {
        case AUDIO_FLAG_MMAP:
            streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_FAST;
            return;
        case AUDIO_FLAG_VOIP_FAST:
            streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_VOIP;
            return;
        case AUDIO_FLAG_VOIP_DIRECT:
            streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_VOIP;
            return;
        default:
            break;
    }

    if (IsStreamSupportDirect(streamDesc)) {
        streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_HD;
        return;
    }
    if (IsStreamSupportLowpower(streamDesc)) {
        streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
        return;
    }
    if (IsStreamSupportMultiChannel(streamDesc)) {
        streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_MULTICHANNEL;
        return;
    }
    streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    AUDIO_INFO_LOG("StreamDesc flag use default - NORMAL");
}

void AudioCoreService::SetRecordStreamFlag(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    // fast/normal has done in audioCapturerPrivate
    if (streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
        // in plan: if has two voip, return normal
        streamDesc->audioFlag_ = AUDIO_INPUT_FLAG_VOIP;
        AUDIO_INFO_LOG("streamDesc->audioFlag_ %{public}d", streamDesc->audioFlag_);
        return;
    }
    if (streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_REMOTE_CAST) {
        streamDesc->audioFlag_ = AUDIO_INPUT_FLAG_NORMAL;
        AUDIO_WARNING_LOG("record normal stream due to remotecast");
        return;
    }

    if (streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_WAKEUP) {
        streamDesc->audioFlag_ = AUDIO_INPUT_FLAG_WAKEUP;
    }
    switch (streamDesc->capturerInfo_.capturerFlags)
    {
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


int32_t AudioCoreService::StartClient(uint32_t sessionId)
{
    AUDIO_INFO_LOG("In, session %{public}u", sessionId);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = pipeManager_->GetStreamDescById(sessionId);
    if (streamDesc == nullptr) {
        AUDIO_ERR_LOG("Cannot find session %{public}u", sessionId);
        return ERROR;
    }

    if (streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK) {
        std::vector<std::pair<DeviceType, DeviceFlag>> activeDevices;
        if (streamDesc->newDeviceDescs_.size() == 2) {
            std::string firstSinkName =
                AudioPolicyUtils::GetInstance().GetSinkName(streamDesc->newDeviceDescs_[0], streamDesc->sessionId_);
            std::string secondSinkName =
                AudioPolicyUtils::GetInstance().GetSinkName(streamDesc->newDeviceDescs_[1], streamDesc->sessionId_);
            AUDIO_INFO_LOG("firstSinkName %{public}s, secondSinkName %{public}s",
                firstSinkName.c_str(), secondSinkName.c_str());
            if (firstSinkName == secondSinkName) {
                activeDevices.push_back(
                    make_pair(streamDesc->newDeviceDescs_[0]->deviceType_, DeviceFlag::OUTPUT_DEVICES_FLAG));
                activeDevices.push_back(
                    make_pair(streamDesc->newDeviceDescs_[1]->deviceType_, DeviceFlag::OUTPUT_DEVICES_FLAG));
            }
        } else {
            std::string firstSinkName =
                AudioPolicyUtils::GetInstance().GetSinkName(streamDesc->newDeviceDescs_[0], streamDesc->sessionId_);
            AUDIO_INFO_LOG("firstSinkName %{public}s", firstSinkName.c_str());
            if (firstSinkName == "primary") {
                activeDevices.push_back(
                    make_pair(streamDesc->newDeviceDescs_[0]->deviceType_, DeviceFlag::OUTPUT_DEVICES_FLAG));
            }
        }
        if (activeDevices.size() != 0) {
            audioActiveDevice_.UpdateActiveDevicesRoute(activeDevices);
        }
    } else {
        audioActiveDevice_.UpdateActiveDeviceRoute(
            streamDesc->newDeviceDescs_[0]->deviceType_, DeviceFlag::INPUT_DEVICES_FLAG);
    }

    pipeManager_->StartClient(sessionId);
    return SUCCESS;
}

int32_t AudioCoreService::PauseClient(uint32_t sessionId)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = pipeManager_->GetStreamDescById(sessionId);
    if (streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK) {
        audioActiveDevice_.UpdateActiveDeviceRoute(
            streamDesc->newDeviceDescs_[0]->deviceType_, DeviceFlag::OUTPUT_DEVICES_FLAG);
    } else {
        audioActiveDevice_.UpdateActiveDeviceRoute(
            streamDesc->newDeviceDescs_[0]->deviceType_, DeviceFlag::INPUT_DEVICES_FLAG);
    }
    audioActiveDevice_.UpdateActiveDeviceRoute(DEVICE_TYPE_SPEAKER, DeviceFlag::OUTPUT_DEVICES_FLAG);

    pipeManager_->PauseClient(sessionId);
    return SUCCESS;
}

int32_t AudioCoreService::StopClient(uint32_t sessionId)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = pipeManager_->GetStreamDescById(sessionId);
    if (streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK) {
        audioActiveDevice_.UpdateActiveDeviceRoute(
            streamDesc->newDeviceDescs_[0]->deviceType_, DeviceFlag::OUTPUT_DEVICES_FLAG);
    } else {
        audioActiveDevice_.UpdateActiveDeviceRoute(
            streamDesc->newDeviceDescs_[0]->deviceType_, DeviceFlag::INPUT_DEVICES_FLAG);
    }
    audioActiveDevice_.UpdateActiveDeviceRoute(DEVICE_TYPE_SPEAKER, DeviceFlag::OUTPUT_DEVICES_FLAG);

    pipeManager_->StopClient(sessionId);
    return SUCCESS;
}

int32_t AudioCoreService::ReleaseClient(uint32_t sessionId)
{
    AUDIO_INFO_LOG("Release session %{public}u", sessionId);
    if (sessionId == pipeManager_->GetModemCommunicationId()) {
        AUDIO_INFO_LOG("Modem communication, sessionId %{public}u", sessionId);
        pipeManager_->ResetModemCommunicationId();
        return SUCCESS;
    }
    pipeManager_->RemoveClient(sessionId);
    RemoveUnusedPipe();

    return SUCCESS;
}

int32_t AudioCoreService::SetAudioScene(AudioScene audioScene)
{
    audioSceneManager_.SetAudioScenePre(audioScene);

    SelectDeviceAndRoute(AudioStreamDeviceChangeReasonExt::ExtEnum::SET_AUDIO_SCENE);

    int32_t result = audioSceneManager_.SetAudioSceneAfter(audioScene, audioA2dpOffloadFlag_.GetA2dpOffloadFlag());
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED, "failed [%{public}d]", result);

    if (audioScene == AUDIO_SCENE_PHONE_CALL) {
        // Make sure the STREAM_VOICE_CALL volume is set before the calling starts.
        audioVolumeManager_.SetVoiceCallVolume(audioVolumeManager_.GetSystemVolumeLevel(STREAM_VOICE_CALL));
    } else {
        audioVolumeManager_.SetVoiceRingtoneMute(false);
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

int32_t AudioCoreService::SetDeviceActive(InternalDeviceType deviceType, bool active, const int32_t pid)
{
    int32_t ret = audioActiveDevice_.SetDeviceActive(deviceType, active, pid);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetDeviceActive failed");

    SelectDeviceAndRoute(AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE);

    audioCapturerSession_.ReloadSourceForDeviceChange(audioActiveDevice_.GetCurrentInputDevice(),
        audioActiveDevice_.GetCurrentOutputDevice(), "SetDevcieActive");
    return SUCCESS;
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
            audioRouterCenter_.FetchOutputDevices(rendererInfo.streamUsage, -1);
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
        if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO && desc->isEnable_) {
            activeDeviceDescriptors.push_back(make_shared<AudioDeviceDescriptor>(*desc));
        }
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

int32_t AudioCoreService::SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address)
{
    AUDIO_WARNING_LOG("Device type[%{public}d] flag[%{public}d] address[%{public}s]",
        deviceType, active, GetEncryptAddr(address).c_str());
    CHECK_AND_RETURN_RET_LOG(deviceType != DEVICE_TYPE_NONE, ERR_DEVICE_NOT_SUPPORTED, "Invalid device");

    int32_t ret = audioActiveDevice_.SetCallDeviceActive(deviceType, active, address);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetCallDeviceActive failed");
    ret = SelectDeviceAndRoute(AudioStreamDeviceChangeReason::OVERRODE);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SelectDeviceAndRoute failed");

    return SUCCESS;
}

std::vector<shared_ptr<AudioDeviceDescriptor>> AudioCoreService::GetAvailableDevices(AudioDeviceUsage usage)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    audioDeviceDescriptors = audioDeviceManager_.GetAvailableDevicesByUsage(usage);
    return audioDeviceDescriptors;
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

int32_t AudioCoreService::UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    HandleAudioCaptureState(mode, streamChangeInfo);

    int32_t ret = streamCollector_.UpdateTracker(mode, streamChangeInfo);

    const auto &rendererState = streamChangeInfo.audioRendererChangeInfo.rendererState;
    if (rendererState == RENDERER_PREPARED || rendererState == RENDERER_NEW || rendererState == RENDERER_INVALID) {
        return ret; // only update tracker in new and prepared
    }

    audioDeviceCommon_.UpdateTracker(mode, streamChangeInfo, rendererState);

    if (audioA2dpOffloadManager_) {
        audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream(audioActiveDevice_.GetCurrentOutputDeviceType());
    }

    SendA2dpConnectedWhileRunning(rendererState, streamChangeInfo.audioRendererChangeInfo.sessionId);
    return ret;
}

void AudioCoreService::HandleAudioCaptureState(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    if (mode == AUDIO_MODE_RECORD &&
        (streamChangeInfo.audioCapturerChangeInfo.capturerState == CAPTURER_RELEASED ||
         streamChangeInfo.audioCapturerChangeInfo.capturerState == CAPTURER_STOPPED)) {
        if (Util::IsScoSupportSource(streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sourceType)) {
            audioDeviceCommon_.BluetoothScoDisconectForRecongnition();
            Bluetooth::AudioHfpManager::ClearRecongnitionStatus();
        }
        audioMicrophoneDescriptor_.RemoveAudioCapturerMicrophoneDescriptorBySessionID(
            streamChangeInfo.audioCapturerChangeInfo.sessionId);
    }
}

void AudioCoreService::RegisteredTrackerClientDied(pid_t uid)
{
    UpdateDefaultOutputDeviceWhenStopping(static_cast<int32_t>(uid));

    audioMicrophoneDescriptor_.RemoveAudioCapturerMicrophoneDescriptor(static_cast<int32_t>(uid));
    streamCollector_.RegisteredTrackerClientDied(static_cast<int32_t>(uid));

    audioDeviceCommon_.ClientDiedDisconnectScoNormal();
    audioDeviceCommon_.ClientDiedDisconnectScoRecognition();

    if (!streamCollector_.ExistStreamForPipe(PIPE_TYPE_OFFLOAD)) {
        audioOffloadStream_.DynamicUnloadOffloadModule();
    }

    if (!streamCollector_.ExistStreamForPipe(PIPE_TYPE_MULTICHANNEL)) {
        audioOffloadStream_.UnloadMchModule();
    }
}

void AudioCoreService::UpdateDefaultOutputDeviceWhenStopping(int32_t uid)
{
    std::vector<uint32_t> sessionIDSet = streamCollector_.GetAllRendererSessionIDForUID(uid);
    for (const auto &sessionID : sessionIDSet) {
        audioDeviceManager_.UpdateDefaultOutputDeviceWhenStopping(sessionID);
        audioDeviceManager_.RemoveSelectedDefaultOutputDevice(sessionID);
    }
    SelectOutputDeviceAndRoute();
}

// No lock -- Should lock or not?
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

#ifdef BLUETOOTH_ENABLE
const sptr<IStandardAudioService> AudioCoreService::RegisterBluetoothDeathCallback()
{
    lock_guard<mutex> lock(g_btProxyMutex);
    if (g_btProxy == nullptr) {
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr,
            "get sa manager failed");
        sptr<IRemoteObject> object = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
        CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr,
            "get audio service remote object failed");
        g_btProxy = iface_cast<IStandardAudioService>(object);
        CHECK_AND_RETURN_RET_LOG(g_btProxy != nullptr, nullptr,
            "get audio service proxy failed");

        // register death recipent
        sptr<AudioServerDeathRecipient> asDeathRecipient =
            new(std::nothrow) AudioServerDeathRecipient(getpid(), getuid());
        if (asDeathRecipient != nullptr) {
            asDeathRecipient->SetNotifyCb([] (pid_t pid, pid_t uid) {
                AudioCoreService::BluetoothServiceCrashedCallback(pid, uid);
            });
            bool result = object->AddDeathRecipient(asDeathRecipient);
            if (!result) {
                AUDIO_ERR_LOG("failed to add deathRecipient");
            }
        }
    }
    sptr<IStandardAudioService> gasp = g_btProxy;
    return gasp;
}

void AudioCoreService::BluetoothServiceCrashedCallback(pid_t pid, pid_t uid)
{
    AUDIO_INFO_LOG("Bluetooth sa crashed, will restore proxy in next call");
    lock_guard<mutex> lock(g_btProxyMutex);
    g_btProxy = nullptr;
    isBtListenerRegistered = false;
    Bluetooth::AudioA2dpManager::DisconnectBluetoothA2dpSink();
    Bluetooth::AudioA2dpManager::DisconnectBluetoothA2dpSource();
    Bluetooth::AudioHfpManager::DisconnectBluetoothHfpSink();
}
#endif

void AudioCoreService::RegisterBluetoothListener()
{
#ifdef BLUETOOTH_ENABLE
    AUDIO_INFO_LOG("Enter");
    Bluetooth::RegisterDeviceObserver(deviceStatusListener_->deviceObserver_);
    if (isBtListenerRegistered) {
        AUDIO_INFO_LOG("audio policy service already register bt listerer, return");
        return;
    }
    Bluetooth::AudioA2dpManager::RegisterBluetoothA2dpListener();
    Bluetooth::AudioHfpManager::RegisterBluetoothScoListener();
    isBtListenerRegistered = true;
    const sptr<IStandardAudioService> gsp = RegisterBluetoothDeathCallback();
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

int32_t AudioCoreService::SelectDeviceAndRoute(const AudioStreamDeviceChangeReasonExt reason)
{
    return SelectOutputDeviceAndRoute(reason) && SelectInputDeviceAndRoute();
}

void AudioCoreService::CheckModemScene(const AudioStreamDeviceChangeReasonExt reason)
{
    if (pipeManager_->GetModemCommunicationId() != 0) {
        AUDIO_INFO_LOG("Modem communication id %{public}u", pipeManager_->GetModemCommunicationId());
        vector<std::shared_ptr<AudioDeviceDescriptor>> descs =
            audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_VOICE_MODEM_COMMUNICATION, -1);
        AUDIO_INFO_LOG("Size %{public}zu", descs.size());
        if (descs.size() != 0) {
            AUDIO_INFO_LOG("Update route %{public}d", descs.front()->deviceType_);
            if (descs.front()->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
                int32_t ret = HandleScoOutputDeviceFetched(descs.front(), reason);
                AUDIO_INFO_LOG("HandleScoOutputDeviceFetched %{public}d", ret);
            }
            audioActiveDevice_.UpdateActiveDeviceRoute(descs.front()->deviceType_, DeviceFlag::OUTPUT_DEVICES_FLAG);
        }
    }
}

int32_t AudioCoreService::SelectOutputDeviceAndRoute(const AudioStreamDeviceChangeReasonExt reason)
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> outputStreamDescs = pipeManager_->GetAllOutputStreamDescs();
    AUDIO_INFO_LOG("Output stream size: %{public}zu", outputStreamDescs.size());

    CheckModemScene(reason);

    if (outputStreamDescs.empty()) {
        AUDIO_PRERELEASE_LOGI("when no stream in");
        vector<std::shared_ptr<AudioDeviceDescriptor>> descs =
            audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_MEDIA, -1);
        CHECK_AND_RETURN_RET_LOG(!descs.empty(), ERROR, "descs is empty");
        AudioDeviceDescriptor tmpOutputDeviceDesc = audioActiveDevice_.GetCurrentOutputDevice();
        if (descs.front()->deviceType_ == DEVICE_TYPE_NONE || IsSameDevice(descs.front(), tmpOutputDeviceDesc)) {
            AUDIO_DEBUG_LOG("output device is not change");
            return SUCCESS;
        }
        audioActiveDevice_.SetCurrentOutputDevice(*descs.front());
        AUDIO_DEBUG_LOG("currentActiveDevice %{public}d", audioActiveDevice_.GetCurrentOutputDeviceType());
        audioVolumeManager_.SetVolumeForSwitchDevice(descs.front()->deviceType_);
        if (descs.front()->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
            SwitchActiveA2dpDevice(std::make_shared<AudioDeviceDescriptor>(*descs.front()));
        }
        OnPreferredOutputDeviceUpdated(audioActiveDevice_.GetCurrentOutputDevice());
    }

    for (auto streamDesc : outputStreamDescs) {
        streamDesc->oldDeviceDescs_ = streamDesc->newDeviceDescs_;
        streamDesc->newDeviceDescs_ =
            audioRouterCenter_.FetchOutputDevices(streamDesc->rendererInfo_.streamUsage, GetRealUid(streamDesc));
        AUDIO_INFO_LOG("DeviceType %{public}d", streamDesc->newDeviceDescs_[0]->deviceType_);

        // handle a2dp
        std::string encryptMacAddr =
            GetEncryptAddr(streamDesc->newDeviceDescs_.front()->macAddress_);
        int32_t bluetoothFetchResult =
            BluetoothDeviceFetchOutputHandle(streamDesc->newDeviceDescs_.front(), reason, encryptMacAddr);
        if (bluetoothFetchResult == BLUETOOTH_FETCH_RESULT_CONTINUE ||
            bluetoothFetchResult == BLUETOOTH_FETCH_RESULT_ERROR) {
            continue;
        }
        if (streamDesc->newDeviceDescs_.front()->deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET) {
            audioEcManager_.ActivateArmDevice(
                streamDesc->newDeviceDescs_.front()->macAddress_, streamDesc->newDeviceDescs_.front()->deviceRole_);
        }
        SetPlaybackStreamFlag(streamDesc);
        AUDIO_INFO_LOG("Will use audio flag: %{public}u", streamDesc->audioFlag_);
    }

    int32_t ret = FetchRendererPipesAndExecute(outputStreamDescs, reason);
    return ret;
}

int32_t AudioCoreService::BluetoothDeviceFetchOutputHandle(shared_ptr<AudioDeviceDescriptor> desc,
    const AudioStreamDeviceChangeReasonExt reason, std::string encryptMacAddr)
{
    if (desc == nullptr) {
        AUDIO_ERR_LOG("Fetch output device is null");
        return BLUETOOTH_FETCH_RESULT_CONTINUE;
    }
    if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        AUDIO_INFO_LOG("A2dp device");
        int32_t ret = ActivateA2dpDeviceWhenDescEnabled(desc, reason);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("Activate a2dp [%{public}s] failed", encryptMacAddr.c_str());
            return BLUETOOTH_FETCH_RESULT_ERROR;
        }
    } else if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        int32_t ret = HandleScoOutputDeviceFetched(desc, reason);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("sco [%{public}s] is not connected yet", encryptMacAddr.c_str());
            return BLUETOOTH_FETCH_RESULT_ERROR;
        }
    }
    return BLUETOOTH_FETCH_RESULT_DEFAULT;
}

int32_t AudioCoreService::HandleScoOutputDeviceFetched(
    shared_ptr<AudioDeviceDescriptor> &desc, const AudioStreamDeviceChangeReasonExt reason)
{
    AUDIO_INFO_LOG("In");
    Trace trace("AudioCoreService::HandleScoOutputDeviceFetched");
#ifdef BLUETOOTH_ENABLE
    int32_t ret = Bluetooth::AudioHfpManager::SetActiveHfpDevice(desc->macAddress_);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Active hfp device failed, retrigger fetch output device.");
        desc->exceptionFlag_ = true;
        audioDeviceManager_.UpdateDevicesListInfo(
            std::make_shared<AudioDeviceDescriptor>(*desc), EXCEPTION_FLAG_UPDATE);
        SelectOutputDeviceAndRoute(reason);
        return ERROR;
    }
    if (desc->connectState_ == DEACTIVE_CONNECTED || !audioSceneManager_.IsSameAudioScene()) {
        Bluetooth::AudioHfpManager::ConnectScoWithAudioScene(audioSceneManager_.GetAudioScene(true));
        return SUCCESS;
    }
#endif
    AUDIO_INFO_LOG("out");
    return SUCCESS;
}

int32_t AudioCoreService::ActivateA2dpDeviceWhenDescEnabled(shared_ptr<AudioDeviceDescriptor> desc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    AUDIO_INFO_LOG("Desc isEnabled %{public}d", desc->isEnable_);
    if (desc->isEnable_) {
        return ActivateA2dpDevice(desc, reason);
    }
    return SUCCESS;
}

int32_t AudioCoreService::ActivateA2dpDevice(std::shared_ptr<AudioDeviceDescriptor> desc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    Trace trace("AudioCoreService::ActiveA2dpDevice");
    int32_t ret = SwitchActiveA2dpDevice(desc);
    AUDIO_INFO_LOG("ActivateA2dpDevice ret : %{public}d", ret);
    // In plan: re-try when failed
    return ret;
}

int32_t AudioCoreService::SwitchActiveA2dpDevice(std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor)
{
    CHECK_AND_RETURN_RET_LOG(audioA2dpDevice_.CheckA2dpDeviceExist(deviceDescriptor->macAddress_),
        ERR_INVALID_PARAM, "Target A2DP device doesn't exist.");
    int32_t result = ERROR;
#ifdef BLUETOOTH_ENABLE
    std::string lastActiveA2dpDevice = audioActiveDevice_.GetActiveBtDeviceMac();
    audioActiveDevice_.SetActiveBtDeviceMac(deviceDescriptor->macAddress_);
    DeviceType lastDevice = audioPolicyManager_.GetActiveDevice();
    audioPolicyManager_.SetActiveDevice(DEVICE_TYPE_BLUETOOTH_A2DP);

    if (Bluetooth::AudioA2dpManager::GetActiveA2dpDevice() == deviceDescriptor->macAddress_ &&
        audioIOHandleMap_.CheckIOHandleExist(BLUETOOTH_SPEAKER)) {
        AUDIO_WARNING_LOG("A2dp device [%{public}s] is already active",
            GetEncryptAddr(deviceDescriptor->macAddress_).c_str());
        return SUCCESS;
    }

    result = Bluetooth::AudioA2dpManager::SetActiveA2dpDevice(deviceDescriptor->macAddress_);
    if (result != SUCCESS) {
        audioActiveDevice_.SetActiveBtDeviceMac(lastActiveA2dpDevice);
        audioPolicyManager_.SetActiveDevice(lastDevice);
        AUDIO_ERR_LOG("Active [%{public}s] failed, using original [%{public}s] device",
            GetEncryptAddr(audioActiveDevice_.GetActiveBtDeviceMac()).c_str(),
            GetEncryptAddr(lastActiveA2dpDevice).c_str());
        return result;
    }

    AudioStreamInfo audioStreamInfo = {};
    audioActiveDevice_.GetActiveA2dpDeviceStreamInfo(DEVICE_TYPE_BLUETOOTH_A2DP, audioStreamInfo);
    std::string networkId = audioActiveDevice_.GetCurrentOutputDeviceNetworkId();
    std::string sinkName = AudioPolicyUtils::GetInstance().GetSinkPortName(
        audioActiveDevice_.GetCurrentOutputDeviceType());
    result = LoadA2dpModule(DEVICE_TYPE_BLUETOOTH_A2DP, audioStreamInfo, networkId, sinkName, SOURCE_TYPE_INVALID);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED, "LoadA2dpModule failed %{public}d", result);
#endif
    return result;
}

int32_t AudioCoreService::LoadA2dpModule(DeviceType deviceType, const AudioStreamInfo &audioStreamInfo,
    std::string networkId, std::string sinkName, SourceType sourceType)
{
    std::list<AudioModuleInfo> moduleInfoList;
    bool ret = policyConfigMananger_.GetModuleListByType(ClassType::TYPE_A2DP, moduleInfoList);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_OPERATION_FAILED, "A2dp module is not exist in the configuration file");

    // not load bt_a2dp_fast and bt_hdap, maybe need fix
    int32_t loadRet = AudioServerProxy::GetInstance().LoadHdiAdapterProxy(HDI_DEVICE_MANAGER_TYPE_BLUETOOTH, "bt_a2dp");
    if (loadRet) {
        AUDIO_ERR_LOG("load adapter failed");
    }
    for (auto &moduleInfo : moduleInfoList) {
        DeviceRole configRole = moduleInfo.role == "source" ? INPUT_DEVICE : OUTPUT_DEVICE;
        DeviceRole deviceRole = deviceType == DEVICE_TYPE_BLUETOOTH_A2DP ? OUTPUT_DEVICE : INPUT_DEVICE;
        AUDIO_INFO_LOG("Load a2dp module [%{public}s], load role[%{public}d], config role[%{public}d]",
            moduleInfo.name.c_str(), deviceRole, configRole);
        if (configRole != deviceRole) {continue;}
        if (audioIOHandleMap_.CheckIOHandleExist(moduleInfo.name) == false) {
            AUDIO_INFO_LOG("A2dp device connects for the first time");
            // a2dp device connects for the first time
            GetA2dpModuleInfo(moduleInfo, audioStreamInfo, sourceType);
            uint32_t paIndex = 0;
            AudioIOHandle ioHandle = audioPolicyManager_.OpenAudioPort(moduleInfo, paIndex);
            CHECK_AND_RETURN_RET_LOG(ioHandle != OPEN_PORT_FAILURE, ERR_OPERATION_FAILED,
                "OpenAudioPort failed %{public}d", ioHandle);
            audioIOHandleMap_.AddIOHandleInfo(moduleInfo.name, ioHandle);

            std::shared_ptr<AudioPipeInfo> pipeInfo_ = std::make_shared<AudioPipeInfo>();
            pipeInfo_->id_ = ioHandle;
            pipeInfo_->paIndex_ = paIndex;
            if (moduleInfo.role == "sink") {
                pipeInfo_->pipeRole_ = PIPE_ROLE_OUTPUT;
                pipeInfo_->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
            } else {
                pipeInfo_->pipeRole_ = PIPE_ROLE_INPUT;
                pipeInfo_->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
            }
            pipeInfo_->adapterName_ = "a2dp";
            pipeInfo_->moduleInfo_ = moduleInfo;
            pipeInfo_->pipeAction_ = PIPE_ACTION_DEFAULT;
            pipeManager_->AddAudioPipeInfo(pipeInfo_);
            AUDIO_INFO_LOG("Add PipeInfo %{public}u in loada2dp.", pipeInfo_->id_);
        } else {
            // At least one a2dp device is already connected. A new a2dp device is connecting.
            // Need to reload a2dp module when switching to a2dp device.
            int32_t result = ReloadA2dpAudioPort(moduleInfo, deviceType, audioStreamInfo, networkId, sinkName,
                sourceType);
            CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "ReloadA2dpAudioPort failed %{public}d", result);
        }
    }

    return SUCCESS;
}

int32_t AudioCoreService::ReloadA2dpAudioPort(AudioModuleInfo &moduleInfo, DeviceType deviceType,
    const AudioStreamInfo &audioStreamInfo, std::string networkId, std::string sinkName,
    SourceType sourceType)
{
    AUDIO_INFO_LOG("Switch device from a2dp to another a2dp, reload a2dp module");
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        audioIOHandleMap_.MuteDefaultSinkPort(networkId, sinkName);
    }

    // Firstly, unload the existing a2dp sink or source.
    std::string portName = BLUETOOTH_SPEAKER;
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP_IN) {
        portName = BLUETOOTH_MIC;
    }
    AudioIOHandle activateDeviceIOHandle;
    audioIOHandleMap_.GetModuleIdByKey(portName, activateDeviceIOHandle);
    int32_t result = audioPolicyManager_.CloseAudioPort(activateDeviceIOHandle);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result,
        "CloseAudioPort failed %{public}d", result);

    // Load a2dp sink or source module again with the configuration of active a2dp device.
    GetA2dpModuleInfo(moduleInfo, audioStreamInfo, sourceType);
    uint32_t paIndex = 0;
    AudioIOHandle ioHandle = audioPolicyManager_.OpenAudioPort(moduleInfo, paIndex);
    CHECK_AND_RETURN_RET_LOG(ioHandle != OPEN_PORT_FAILURE, ERR_OPERATION_FAILED,
        "OpenAudioPort failed %{public}d", ioHandle);
    audioIOHandleMap_.AddIOHandleInfo(moduleInfo.name, ioHandle);

    std::shared_ptr<AudioPipeInfo> pipeInfo_ = std::make_shared<AudioPipeInfo>();
    pipeInfo_->id_ = ioHandle;
    pipeInfo_->paIndex_ = paIndex;
    if (moduleInfo.role == "sink") {
        pipeInfo_->pipeRole_ = PIPE_ROLE_OUTPUT;
        pipeInfo_->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    } else {
        pipeInfo_->pipeRole_ = PIPE_ROLE_INPUT;
        pipeInfo_->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    }
    pipeInfo_->adapterName_ = "a2dp";
    pipeInfo_->moduleInfo_ = moduleInfo;
    pipeInfo_->pipeAction_ = PIPE_ACTION_DEFAULT;
    pipeManager_->AddAudioPipeInfo(pipeInfo_);
    AUDIO_INFO_LOG("Add PipeInfo %{public}u in reloada2dp.", pipeInfo_->id_);
    return SUCCESS;
}

void AudioCoreService::GetA2dpModuleInfo(AudioModuleInfo &moduleInfo, const AudioStreamInfo& audioStreamInfo,
    SourceType sourceType)
{
    uint32_t bufferSize = audioStreamInfo.samplingRate *
        AudioPolicyUtils::GetInstance().PcmFormatToBytes(audioStreamInfo.format) *
        audioStreamInfo.channels / BT_BUFFER_ADJUSTMENT_FACTOR;
    AUDIO_INFO_LOG("a2dp rate: %{public}d, format: %{public}d, channel: %{public}d",
        audioStreamInfo.samplingRate, audioStreamInfo.format, audioStreamInfo.channels);
    moduleInfo.channels = to_string(audioStreamInfo.channels);
    moduleInfo.rate = to_string(audioStreamInfo.samplingRate);
    moduleInfo.format = AudioPolicyUtils::GetInstance().ConvertToHDIAudioFormat(audioStreamInfo.format);
    moduleInfo.bufferSize = to_string(bufferSize);
    if (moduleInfo.role != "source") {
        moduleInfo.renderInIdleState = "1";
        moduleInfo.sinkLatency = "0";
    }
    // audioEcManager_.UpdateStreamEcAndMicRefInfo(moduleInfo, sourceType);
}

bool AudioCoreService::IsSameDevice(shared_ptr<AudioDeviceDescriptor> &desc, AudioDeviceDescriptor &deviceInfo)
{
    if (desc->networkId_ == deviceInfo.networkId_ && desc->deviceType_ == deviceInfo.deviceType_ &&
        desc->macAddress_ == deviceInfo.macAddress_ && desc->connectState_ == deviceInfo.connectState_) {
        AUDIO_INFO_LOG("Enter");
        if (deviceInfo.IsAudioDeviceDescriptor()) {
            return true;
        }
        BluetoothOffloadState state = audioA2dpOffloadFlag_.GetA2dpOffloadFlag();
        if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP &&
            // switch to A2dp
            ((deviceInfo.a2dpOffloadFlag_ == A2DP_OFFLOAD && state != A2DP_OFFLOAD) ||
            // switch to A2dp offload
            (deviceInfo.a2dpOffloadFlag_ != A2DP_OFFLOAD && state == A2DP_OFFLOAD))) {
            return false;
        }
        if (IsUsb(desc->deviceType_)) {
            return desc->deviceRole_ == deviceInfo.deviceRole_;
        }
        return true;
    } else {
        return false;
    }
}

int32_t AudioCoreService::FetchRendererPipesAndExecute(
    std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs, const AudioStreamDeviceChangeReasonExt reason)
{
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos = audioPipeSelector_->FetchPipesAndExecute(streamDescs);
    uint32_t audioFlag;
    for (auto pipeInfo : pipeInfos) {
        AUDIO_INFO_LOG("Pipe name: %{public}s, device class: %{public}s, action: %{public}d",
            pipeInfo->moduleInfo_.adapterName.c_str(), pipeInfo->moduleInfo_.className.c_str(), pipeInfo->pipeAction_);
        if (pipeInfo->pipeAction_ == PIPE_ACTION_UPDATE) {
            ProcessOutputPipeUpdate(pipeInfo, audioFlag);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_NEW) {
            ProcessOutputPipeNew(pipeInfo, audioFlag);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_DEFAULT) {
            // Do nothing
        }
    }
    pipeManager_->UpdateRendererPipeInfos(pipeInfos);
    RemoveUnusedPipe();
    return SUCCESS;
}

int32_t AudioCoreService::SelectInputDeviceAndRoute()
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> inputStreamDescs = pipeManager_->GetAllInputStreamDescs();
    for (auto streamDesc : inputStreamDescs) {
        streamDesc->oldDeviceDescs_ = streamDesc->newDeviceDescs_;
        streamDesc->newDeviceDescs_.clear();
        std::shared_ptr<AudioDeviceDescriptor> inputDeviceDesc =
            audioRouterCenter_.FetchInputDevice(streamDesc->capturerInfo_.sourceType, GetRealUid(streamDesc));
        streamDesc->newDeviceDescs_.push_back(inputDeviceDesc);
        AUDIO_INFO_LOG("device type: %{public}d", inputDeviceDesc->deviceType_);
        SetRecordStreamFlag(streamDesc);
    }

    int32_t ret = FetchCapturerPipesAndExecute(inputStreamDescs);
    return ret;
}

int32_t AudioCoreService::FetchCapturerPipesAndExecute(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs)
{
    uint32_t audioFlag;
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos = audioPipeSelector_->FetchPipesAndExecute(streamDescs);
    for (auto pipeInfo : pipeInfos) {
        AUDIO_INFO_LOG("Pipe name: %{public}s, class: %{public}s, route: %{public}u, action: %{public}d",
            pipeInfo->moduleInfo_.adapterName.c_str(), pipeInfo->moduleInfo_.className.c_str(),
            pipeInfo->routeFlag_, pipeInfo->pipeAction_);
        if (pipeInfo->pipeAction_ == PIPE_ACTION_UPDATE) {
            ProcessInputPipeUpdate(pipeInfo, audioFlag);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_NEW) { // new
            ProcessInputPipeNew(pipeInfo, audioFlag);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_DEFAULT) { // DEFAULT
            // Do nothing
        }
    }
    pipeManager_->UpdateCapturerPipeInfos(pipeInfos);
    RemoveUnusedPipe();
    return SUCCESS;
}

int32_t AudioCoreService::FetchRendererPipeAndExecute(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    uint32_t &sessionId, uint32_t &audioFlag, const AudioStreamDeviceChangeReasonExt reason)
{

    if (sessionId == 0) {
        streamDesc->sessionId_ = GenerateSessionId();
        sessionId = streamDesc->sessionId_;
        AUDIO_INFO_LOG("New sessionId: %{public}u", sessionId);
    }

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos = audioPipeSelector_->FetchPipeAndExecute(streamDesc);

    uint32_t sinkId = HDI_INVALID_ID;
    for (auto pipeInfo : pipeInfos) {
        AUDIO_INFO_LOG("Pipe name: %{public}s, class: %{public}s, route: %{public}u, action: %{public}d",
            pipeInfo->moduleInfo_.adapterName.c_str(), pipeInfo->moduleInfo_.className.c_str(),
            pipeInfo->routeFlag_, pipeInfo->pipeAction_);
        if (pipeInfo->pipeAction_ == PIPE_ACTION_UPDATE) {
            ProcessOutputPipeUpdate(pipeInfo, audioFlag);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_NEW) { // new
            ProcessOutputPipeNew(pipeInfo, audioFlag);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_DEFAULT) { // DEFAULT
            // Do nothing
        }
    }
    RemoveUnusedPipe();
    return SUCCESS;
}

void AudioCoreService::ProcessOutputPipeNew(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &flag)
{
    uint32_t paIndex = 0;
    uint32_t id = OpenNewAudioPortAndRoute(pipeInfo, paIndex);
    CHECK_AND_RETURN_LOG(id != HDI_INVALID_ID, "Invalid sink");
    pipeInfo->id_ = id;
    pipeInfo->paIndex_ = paIndex;

    for (auto desc : pipeInfo->streamDescriptors_) {
        AUDIO_INFO_LOG("Stream id: %{public}u, action: %{public}d", desc->sessionId_, desc->streamAction_);
        switch (desc->streamAction_) {
            case AUDIO_STREAM_ACTION_NEW:
                flag = desc->routeFlag_;
                break;
            case AUDIO_STREAM_ACTION_MOVE:
                MoveToNewOutputDevice(desc, AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
                break;
            case AUDIO_STREAM_ACTION_RECREATE:
                TriggerRecreateRendererStreamCallback(desc->appInfo_.appPid,
                    desc->sessionId_, desc->routeFlag_);
                break;
            default:
                break;
        }
    }
    pipeManager_->AddAudioPipeInfo(pipeInfo);
}

void AudioCoreService::ProcessOutputPipeUpdate(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &flag)
{
    for (auto desc : pipeInfo->streamDescriptors_) {
        AUDIO_INFO_LOG("Stream id: %{public}u, action: %{public}d", desc->sessionId_, desc->streamAction_);
        switch (desc->streamAction_) {
            case AUDIO_STREAM_ACTION_NEW:
                flag = desc->routeFlag_;
                break;
            case AUDIO_STREAM_ACTION_DEFAULT:
            case AUDIO_STREAM_ACTION_MOVE:
                MoveToNewOutputDevice(desc);
                break;
            case AUDIO_STREAM_ACTION_RECREATE:
                TriggerRecreateRendererStreamCallback(desc->appInfo_.appPid,
                    desc->sessionId_, desc->routeFlag_);
                break;
            default:
                break;
        }
    }
    pipeManager_->UpdateAudioPipeInfo(pipeInfo);
}

int32_t AudioCoreService::FetchCapturerPipeAndExecute(std::shared_ptr<AudioStreamDescriptor> streamDesc,
    uint32_t &audioFlag, uint32_t &sessionId)
{
    if (sessionId == 0) {
        streamDesc->sessionId_ = GenerateSessionId();
        sessionId = streamDesc->sessionId_;
        AUDIO_INFO_LOG("New sessionId: %{public}u", sessionId);
    }

    if (streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE) {
        AUDIO_INFO_LOG("Playback capturer, return");
        audioFlag = AUDIO_INPUT_FLAG_NORMAL;
        return SUCCESS;
    }
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos = audioPipeSelector_->FetchPipeAndExecute(streamDesc);
    uint32_t sinkId = HDI_INVALID_ID;
    for (auto pipeInfo : pipeInfos) {
        AUDIO_INFO_LOG("Pipe name: %{public}s, class: %{public}s, route: %{public}u, action: %{public}d",
            pipeInfo->moduleInfo_.adapterName.c_str(), pipeInfo->moduleInfo_.className.c_str(),
            pipeInfo->routeFlag_, pipeInfo->pipeAction_);
        if (pipeInfo->pipeAction_ == PIPE_ACTION_UPDATE) {
            ProcessInputPipeUpdate(pipeInfo, audioFlag);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_NEW) { // new
            ProcessInputPipeNew(pipeInfo, audioFlag);
        } else if (pipeInfo->pipeAction_ == PIPE_ACTION_DEFAULT) { // DEFAULT
            // Do nothing
        }
    }
    RemoveUnusedPipe();
    return SUCCESS;
}

void AudioCoreService::ProcessInputPipeNew(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &flag)
{
    uint32_t paIndex = 0;
    uint32_t sourceId = OpenNewAudioPortAndRoute(pipeInfo, paIndex);
    CHECK_AND_RETURN_LOG(sourceId != HDI_INVALID_ID, "Invalid sink");
    pipeInfo->id_ = sourceId;
    pipeInfo->paIndex_ = paIndex;

    for (auto desc : pipeInfo->streamDescriptors_) {
        AUDIO_INFO_LOG("Pipe name: %{public}s, class: %{public}s, route: %{public}u, action: %{public}d",
            pipeInfo->moduleInfo_.adapterName.c_str(), pipeInfo->moduleInfo_.className.c_str(),
            pipeInfo->routeFlag_, pipeInfo->pipeAction_);
        switch (desc->streamAction_) {
            case AUDIO_STREAM_ACTION_NEW:
                flag = desc->routeFlag_;
                break;
            case AUDIO_STREAM_ACTION_DEFAULT:
            case AUDIO_STREAM_ACTION_MOVE:
                MoveToNewInputDevice(desc);
                break;
            case AUDIO_STREAM_ACTION_RECREATE:
                TriggerRecreateCapturerStreamCallback(desc->appInfo_.appPid,
                    desc->sessionId_, desc->routeFlag_);
                break;
            default:
                break;
        }
    }
    pipeManager_->AddAudioPipeInfo(pipeInfo);
}

void AudioCoreService::ProcessInputPipeUpdate(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &flag)
{
    for (auto desc : pipeInfo->streamDescriptors_) {
        AUDIO_INFO_LOG("Stream id: %{public}u, action: %{public}d", desc->sessionId_, desc->streamAction_);
        switch (desc->streamAction_) {
            case AUDIO_STREAM_ACTION_NEW:
                flag = desc->routeFlag_;
                break;
            case AUDIO_STREAM_ACTION_DEFAULT:
            case AUDIO_STREAM_ACTION_MOVE:
                MoveToNewInputDevice(desc);
                break;
            case AUDIO_STREAM_ACTION_RECREATE:
                TriggerRecreateCapturerStreamCallback(desc->appInfo_.appPid,
                    desc->sessionId_, desc->routeFlag_);
                break;
            default:
                break;
        }
    }
    pipeManager_->UpdateAudioPipeInfo(pipeInfo);
}

void AudioCoreService::RemoveUnusedPipe()
{
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos = pipeManager_->GetUnusedPipe();
    for (auto pipeInfo : pipeInfos) {
        AUDIO_INFO_LOG("Close hid port id %{public}u", pipeInfo->id_);
        audioPolicyManager_.CloseAudioPort(pipeInfo->id_, pipeInfo->paIndex_);
        pipeManager_->RemoveAudioPipeInfo(pipeInfo);
    }
}

std::atomic<uint32_t> g_sessionId = {FIRST_SESSIONID}; // begin at 100000

uint32_t AudioCoreService::GenerateSessionId()
{
    uint32_t sessionId = g_sessionId++;
    AUDIO_INFO_LOG("sessionId:%{public}d", sessionId);
    if (g_sessionId > MAX_VALID_SESSIONID) {
        AUDIO_WARNING_LOG("sessionId is too large, reset it!");
        g_sessionId = FIRST_SESSIONID;
    }
    return sessionId;
}

void AudioCoreService::MoveToNewOutputDevice(
    std::shared_ptr<AudioStreamDescriptor> streamDesc, const AudioStreamDeviceChangeReasonExt reason)
{
    AUDIO_INFO_LOG("session %{public}u", streamDesc->sessionId_);
    Trace trace("AudioCoreService::MoveToNewOutputDevice");
    DeviceType oldDeviceType = DEVICE_TYPE_NONE;
    bool isNeedTriggerCallback = true;
    if (streamDesc->oldDeviceDescs_.size() == 0) {
        AUDIO_INFO_LOG("Move session, [][]->[%{public}d][%{public}s], reason %{public}d",
            streamDesc->newDeviceDescs_.front()->deviceType_,
            GetEncryptAddr(streamDesc->newDeviceDescs_.front()->macAddress_).c_str(), static_cast<int32_t>(reason));
    } else {
        oldDeviceType = streamDesc->oldDeviceDescs_.front()->deviceType_;
        if (streamDesc->oldDeviceDescs_.front()->IsSameDeviceDesc(streamDesc->newDeviceDescs_.front())) {
            isNeedTriggerCallback = false;
        }

        AUDIO_INFO_LOG("Move session %{public}u, [%{public}d][%{public}s]->[%{public}d][%{public}s], reason %{public}d",
            streamDesc->sessionId_, streamDesc->oldDeviceDescs_.front()->deviceType_,
            GetEncryptAddr(streamDesc->oldDeviceDescs_.front()->macAddress_).c_str(),
            streamDesc->newDeviceDescs_.front()->deviceType_,
            GetEncryptAddr(streamDesc->newDeviceDescs_.front()->macAddress_).c_str(), static_cast<int32_t>(reason));
    }

    std::vector<SinkInput> sinkInputs;
    audioPolicyManager_.GetAllSinkInputs(sinkInputs);
    std::vector<SinkInput> targetSinkInputs = audioOffloadStream_.FilterSinkInputs(streamDesc->sessionId_, sinkInputs);
    
    if (isNeedTriggerCallback && audioPolicyServerHandler_) {
        audioPolicyServerHandler_->SendRendererDeviceChangeEvent(streamDesc->appInfo_.appPid,
            streamDesc->sessionId_, streamDesc->newDeviceDescs_.front(), reason);
    }

    AudioPolicyUtils::GetInstance().UpdateEffectDefaultSink(streamDesc->newDeviceDescs_.front()->deviceType_);

    auto ret = (streamDesc->newDeviceDescs_.front()->networkId_ == LOCAL_NETWORK_ID)
        ? MoveToLocalOutputDevice(targetSinkInputs, streamDesc->newDeviceDescs_.front())
        : MoveToRemoteOutputDevice(targetSinkInputs, streamDesc->newDeviceDescs_.front());
    if (ret != SUCCESS) {
        AudioPolicyUtils::GetInstance().UpdateEffectDefaultSink(oldDeviceType);
        AUDIO_ERR_LOG("Move sink input %{public}d to device %{public}d failed!",
            streamDesc->sessionId_, streamDesc->newDeviceDescs_.front()->deviceType_);
        return;
    }

    if (policyConfigMananger_.GetUpdateRouteSupport() &&
        streamDesc->newDeviceDescs_.front()->networkId_ == LOCAL_NETWORK_ID && !reason.isSetAudioScene()) {
        UpdateOutputRoute(streamDesc);
    }

    
    std::string newSinkName = AudioPolicyUtils::GetInstance().GetSinkName(streamDesc->newDeviceDescs_.front(),
        streamDesc->sessionId_);
    audioVolumeManager_.SetVolumeForSwitchDevice(streamDesc->newDeviceDescs_.front()->deviceType_, newSinkName);

    streamCollector_.UpdateRendererDeviceInfo(streamDesc->newDeviceDescs_.front());
}

int32_t AudioCoreService::MoveToRemoteOutputDevice(std::vector<SinkInput> sinkInputIds,
    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor)
{
    AUDIO_INFO_LOG("Start for [%{public}zu] sink-inputs", sinkInputIds.size());

    std::string networkId = remoteDeviceDescriptor->networkId_;
    DeviceRole deviceRole = remoteDeviceDescriptor->deviceRole_;
    DeviceType deviceType = remoteDeviceDescriptor->deviceType_;

    // check: networkid
    CHECK_AND_RETURN_RET_LOG(networkId != LOCAL_NETWORK_ID, ERR_INVALID_OPERATION,
        "failed: not a remote device.");

    uint32_t sinkId = -1; // invalid sink id, use sink name instead.
    std::string moduleName = AudioPolicyUtils::GetInstance().GetRemoteModuleName(networkId, deviceRole);
    AUDIO_ERR_LOG("moduleName %{public}s", moduleName.c_str());

    AudioIOHandle moduleId;
    if (audioIOHandleMap_.GetModuleIdByKey(moduleName, moduleId)) {
        (void)moduleId; // mIOHandle is module id, not equal to sink id.
    } else {
        AUDIO_ERR_LOG("no such device.");
        if (!isOpenRemoteDevice) {
            AUDIO_INFO_LOG("directly return");
            return ERR_INVALID_PARAM;
        } else {
            return OpenRemoteAudioDevice(networkId, deviceRole, deviceType, remoteDeviceDescriptor);
        }
    }
    int32_t res = AudioServerProxy::GetInstance().CheckRemoteDeviceStateProxy(networkId, deviceRole, true);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, ERR_OPERATION_FAILED, "remote device state is invalid!");

    // start move.
    for (size_t i = 0; i < sinkInputIds.size(); i++) {
        int32_t ret = audioPolicyManager_.MoveSinkInputByIndexOrName(sinkInputIds[i].paStreamId, sinkId, moduleName);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "move [%{public}d] failed", sinkInputIds[i].streamId);
        audioRouteMap_.AddRouteMapInfo(sinkInputIds[i].uid, moduleName, sinkInputIds[i].pid);
    }

    if (deviceType != DeviceType::DEVICE_TYPE_DEFAULT) {
        AUDIO_WARNING_LOG("Not defult type[%{public}d] on device:[%{public}s]",
            deviceType, GetEncryptStr(networkId).c_str());
    }
    isCurrentRemoteRenderer_ = true;
    return SUCCESS;
}

void AudioCoreService::MoveToNewInputDevice(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    Trace trace("AudioCoreService::MoveToNewInputDevice");
    std::vector<SourceOutput> targetSourceOutputs = FilterSourceOutputs(streamDesc->sessionId_);

    AUDIO_INFO_LOG("Move session %{public}u, [%{public}d][%{public}s]-->[%{public}d][%{public}s]",
        streamDesc->sessionId_, streamDesc->oldDeviceDescs_.front()->deviceType_,
        GetEncryptAddr(streamDesc->oldDeviceDescs_.front()->macAddress_).c_str(),
        streamDesc->newDeviceDescs_.front()->deviceType_,
        GetEncryptAddr(streamDesc->newDeviceDescs_.front()->macAddress_).c_str());
    
    // MoveSourceOuputByIndexName
    auto ret = (streamDesc->newDeviceDescs_.front()->networkId_ == LOCAL_NETWORK_ID)
        ? MoveToLocalInputDevice(targetSourceOutputs, streamDesc->newDeviceDescs_.front())
        : MoveToRemoteInputDevice(targetSourceOutputs, streamDesc->newDeviceDescs_.front());
    CHECK_AND_RETURN_LOG((ret == SUCCESS), "Move source output %{public}d to device %{public}d failed!",
        streamDesc->sessionId_, streamDesc->newDeviceDescs_.front()->deviceType_);


    if (policyConfigMananger_.GetUpdateRouteSupport() &&
        streamDesc->newDeviceDescs_.front()->networkId_ == LOCAL_NETWORK_ID) {
        audioActiveDevice_.UpdateActiveDeviceRoute(streamDesc->newDeviceDescs_.front()->deviceType_,
            DeviceFlag::INPUT_DEVICES_FLAG, streamDesc->newDeviceDescs_.front()->deviceName_);
        streamCollector_.UpdateCapturerDeviceInfo(streamDesc->newDeviceDescs_.front());
    }
}

int32_t AudioCoreService::MoveToLocalInputDevice(std::vector<SourceOutput> sourceOutputs,
    std::shared_ptr<AudioDeviceDescriptor> localDeviceDescriptor)
{
    AUDIO_DEBUG_LOG("Start");
    // check
    CHECK_AND_RETURN_RET_LOG(LOCAL_NETWORK_ID == localDeviceDescriptor->networkId_, ERR_INVALID_OPERATION,
        "failed: not a local device.");
    // start move.
    uint32_t sourceId = -1; // invalid source id, use source name instead.
    std::string sourceName = AudioPolicyUtils::GetInstance().GetSourcePortName(localDeviceDescriptor->deviceType_);
    for (size_t i = 0; i < sourceOutputs.size(); i++) {
        int32_t ret = audioPolicyManager_.MoveSourceOutputByIndexOrName(sourceOutputs[i].paStreamId,
            sourceId, sourceName);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR,
            "move [%{public}d] to local failed", sourceOutputs[i].paStreamId);
    }

    return SUCCESS;
}

int32_t AudioCoreService::MoveToRemoteInputDevice(std::vector<SourceOutput> sourceOutputs,
    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor)
{
    AUDIO_INFO_LOG("Start");

    std::string networkId = remoteDeviceDescriptor->networkId_;
    DeviceRole deviceRole = remoteDeviceDescriptor->deviceRole_;
    DeviceType deviceType = remoteDeviceDescriptor->deviceType_;

    // check: networkid
    CHECK_AND_RETURN_RET_LOG(networkId != LOCAL_NETWORK_ID, ERR_INVALID_OPERATION,
        "failed: not a remote device.");

    uint32_t sourceId = -1; // invalid sink id, use sink name instead.
    std::string moduleName = AudioPolicyUtils::GetInstance().GetRemoteModuleName(networkId, deviceRole);

    AudioIOHandle moduleId;
    if (audioIOHandleMap_.GetModuleIdByKey(moduleName, moduleId)) {
        (void)moduleId; // mIOHandle is module id, not equal to sink id.
    } else {
        AUDIO_ERR_LOG("no such device.");
        if (!isOpenRemoteDevice) {
            return ERR_INVALID_PARAM;
        } else {
            return OpenRemoteAudioDevice(networkId, deviceRole, deviceType, remoteDeviceDescriptor);
        }
    }
    int32_t res = AudioServerProxy::GetInstance().CheckRemoteDeviceStateProxy(networkId, deviceRole, true);
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, ERR_OPERATION_FAILED, "remote device state is invalid!");

    // start move.
    for (size_t i = 0; i < sourceOutputs.size(); i++) {
        int32_t ret = audioPolicyManager_.MoveSourceOutputByIndexOrName(sourceOutputs[i].paStreamId,
            sourceId, moduleName);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR,
            "move [%{public}d] failed", sourceOutputs[i].paStreamId);
    }

    if (deviceType != DeviceType::DEVICE_TYPE_DEFAULT) {
        AUDIO_DEBUG_LOG("Not defult type[%{public}d] on device:[%{public}s]",
            deviceType, GetEncryptStr(networkId).c_str());
    }
    return SUCCESS;
}

int32_t AudioCoreService::OpenRemoteAudioDevice(std::string networkId, DeviceRole deviceRole, DeviceType deviceType,
    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor)
{
    // open the test device. We should open it when device is online.
    std::string moduleName = AudioPolicyUtils::GetInstance().GetRemoteModuleName(networkId, deviceRole);
    AudioModuleInfo remoteDeviceInfo = AudioPolicyUtils::GetInstance().ConstructRemoteAudioModuleInfo(networkId,
        deviceRole, deviceType);
    
    auto ret = AudioServerProxy::GetInstance().LoadHdiAdapterProxy(HDI_DEVICE_MANAGER_TYPE_REMOTE, networkId);
    if (ret) {
        AUDIO_ERR_LOG("load adapter fail");
    }
    audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleName, remoteDeviceInfo);

    // If device already in list, remove it else do not modify the list.
    audioConnectedDevice_.DelConnectedDevice(networkId, deviceType);
    AudioPolicyUtils::GetInstance().UpdateDisplayName(remoteDeviceDescriptor);
    audioConnectedDevice_.AddConnectedDevice(remoteDeviceDescriptor);
    audioMicrophoneDescriptor_.AddMicrophoneDescriptor(remoteDeviceDescriptor);
    return SUCCESS;
}

inline std::string PrintSourceOutput(SourceOutput sourceOutput)
{
    std::stringstream value;
    value << "streamId:[" << sourceOutput.streamId << "] ";
    value << "streamType:[" << sourceOutput.streamType << "] ";
    value << "uid:[" << sourceOutput.uid << "] ";
    value << "pid:[" << sourceOutput.pid << "] ";
    value << "statusMark:[" << sourceOutput.statusMark << "] ";
    value << "deviceSourceId:[" << sourceOutput.deviceSourceId << "] ";
    value << "startTime:[" << sourceOutput.startTime << "]";
    return value.str();
}

std::vector<SourceOutput> AudioCoreService::FilterSourceOutputs(int32_t sessionId)
{
    std::vector<SourceOutput> targetSourceOutputs = {};
    std::vector<SourceOutput> sourceOutputs = GetSourceOutputs();

    for (size_t i = 0; i < sourceOutputs.size(); i++) {
        AUDIO_DEBUG_LOG("sourceOutput[%{public}zu]:%{public}s", i, PrintSourceOutput(sourceOutputs[i]).c_str());
        if (sessionId == sourceOutputs[i].streamId) {
            targetSourceOutputs.push_back(sourceOutputs[i]);
        }
    }
    return targetSourceOutputs;
}

std::vector<SourceOutput> AudioCoreService::GetSourceOutputs()
{
    std::vector<SourceOutput> sourceOutputs;
    {
        std::unordered_map<std::string, AudioIOHandle> mapCopy = AudioIOHandleMap::GetInstance().GetCopy();
        if (std::any_of(mapCopy.cbegin(), mapCopy.cend(), [](const auto &pair) {
                return std::find(SourceNames.cbegin(), SourceNames.cend(), pair.first) != SourceNames.cend();
            })) {
            sourceOutputs = audioPolicyManager_.GetAllSourceOutputs();
        }
    }
    return sourceOutputs;
}

void AudioCoreService::UpdateOutputRoute(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    StreamUsage streamUsage = streamDesc->rendererInfo_.streamUsage;
    InternalDeviceType deviceType = streamDesc->newDeviceDescs_.front()->deviceType_;
    AUDIO_INFO_LOG("update route, streamUsage:%{public}d, 1st devicetype:%{public}d", streamUsage, deviceType);
    if (Util::IsRingerOrAlarmerStreamUsage(streamUsage) && IsRingerOrAlarmerDualDevicesRange(deviceType) &&
        !VolumeUtils::IsPCVolumeEnable()) {
        if (!SelectRingerOrAlarmDevices(streamDesc)) {
            audioActiveDevice_.UpdateActiveDeviceRoute(deviceType, DeviceFlag::OUTPUT_DEVICES_FLAG);
        }

        AudioRingerMode ringerMode = audioPolicyManager_.GetRingerMode();
        if (ringerMode != RINGER_MODE_NORMAL &&
            IsRingerOrAlarmerDualDevicesRange(streamDesc->newDeviceDescs_.front()->getType()) &&
            streamDesc->newDeviceDescs_.front()->getType() != DEVICE_TYPE_SPEAKER) {
            audioPolicyManager_.SetStreamMute(STREAM_RING, false, streamUsage);
            audioVolumeManager_.SetRingerModeMute(false);
            if (audioPolicyManager_.GetSystemVolumeLevel(STREAM_RING) <
                audioPolicyManager_.GetMaxVolumeLevel(STREAM_RING) / VOLUME_LEVEL_DEFAULT_SIZE) {
                audioPolicyManager_.SetDoubleRingVolumeDb(STREAM_RING,
                    audioPolicyManager_.GetMaxVolumeLevel(STREAM_RING) / VOLUME_LEVEL_DEFAULT_SIZE);
            }
        } else {
            audioVolumeManager_.SetRingerModeMute(true);
        }
        shouldUpdateDeviceDueToDualTone_ = true;
    } else {
        if (enableDualHalToneState_) {
            AUDIO_INFO_LOG("disable dual hal tone for not ringer/alarm.");
            UpdateDualToneState(false, enableDualHalToneSessionId_);
        }
        audioVolumeManager_.SetRingerModeMute(true);
        audioActiveDevice_.UpdateActiveDeviceRoute(deviceType, DeviceFlag::OUTPUT_DEVICES_FLAG);
        shouldUpdateDeviceDueToDualTone_ = false;
    }
}

void AudioCoreService::OnPreferredOutputDeviceUpdated(const AudioDeviceDescriptor& deviceDescriptor)
{
    AUDIO_INFO_LOG("In");
    Trace trace("AudioCoreService::OnPreferredOutputDeviceUpdated:" + std::to_string(deviceDescriptor.deviceType_));

    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendPreferredOutputDeviceUpdated();
    }
    spatialDeviceMap_.insert(make_pair(deviceDescriptor.macAddress_, deviceDescriptor.deviceType_));

    if (deviceDescriptor.macAddress_ !=
        AudioSpatializationService::GetAudioSpatializationService().GetCurrentDeviceAddress()) {
        AudioServerProxy::GetInstance().UpdateEffectBtOffloadSupportedProxy(false);
    }
    AudioPolicyUtils::GetInstance().UpdateEffectDefaultSink(deviceDescriptor.deviceType_);
    AudioSpatializationService::GetAudioSpatializationService().UpdateCurrentDevice(deviceDescriptor.macAddress_);
}

void AudioCoreService::OnPreferredInputDeviceUpdated(DeviceType deviceType, std::string networkId)
{
    AUDIO_INFO_LOG("OnPreferredInputDeviceUpdated Start");

    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendPreferredInputDeviceUpdated();
    }
}


bool AudioCoreService::IsRingerOrAlarmerDualDevicesRange(const InternalDeviceType &deviceType)
{
    switch (deviceType) {
        case DEVICE_TYPE_SPEAKER:
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            return true;
        default:
            return false;
    }
}

bool AudioCoreService::SelectRingerOrAlarmDevices(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc->newDeviceDescs_.size() > 0 &&
        streamDesc->newDeviceDescs_.size() <= AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT, false,
        "audio devices not in range for ringer or alarmer.");
    const int32_t sessionId = streamDesc->sessionId_;
    const StreamUsage streamUsage = streamDesc->rendererInfo_.streamUsage;
    bool allDevicesInDualDevicesRange = true;
    std::vector<std::pair<InternalDeviceType, DeviceFlag>> activeDevices;
    for (size_t i = 0; i < streamDesc->newDeviceDescs_.size(); i++) {
        if (IsRingerOrAlarmerDualDevicesRange(streamDesc->newDeviceDescs_[i]->deviceType_)) {
            activeDevices.push_back(make_pair(streamDesc->newDeviceDescs_[i]->deviceType_,
            DeviceFlag::OUTPUT_DEVICES_FLAG));
            AUDIO_INFO_LOG("select ringer/alarm devices devicetype[%{public}zu]:%{public}d",
                i, streamDesc->newDeviceDescs_[i]->deviceType_);
        } else {
            allDevicesInDualDevicesRange = false;
            break;
        }
    }

    AUDIO_INFO_LOG("select ringer/alarm sessionId:%{public}d, streamUsage:%{public}d", sessionId, streamUsage);
    if (!streamDesc->newDeviceDescs_.empty() && allDevicesInDualDevicesRange) {
        if (streamDesc->newDeviceDescs_.size() == AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT &&
            AudioPolicyUtils::GetInstance().GetSinkName(*streamDesc->newDeviceDescs_.front(), sessionId) !=
            AudioPolicyUtils::GetInstance().GetSinkName(*streamDesc->newDeviceDescs_.back(), sessionId)) {
            AUDIO_INFO_LOG("set dual hal tone, reset primary sink to default before.");
            audioActiveDevice_.UpdateActiveDeviceRoute(DEVICE_TYPE_SPEAKER, DeviceFlag::OUTPUT_DEVICES_FLAG);
            if (enableDualHalToneState_ && enableDualHalToneSessionId_ != sessionId) {
                AUDIO_INFO_LOG("sesion changed, disable old dual hal tone.");
                UpdateDualToneState(false, enableDualHalToneSessionId_);
            }

            if ((audioPolicyManager_.GetRingerMode() != RINGER_MODE_NORMAL && streamUsage != STREAM_USAGE_ALARM) ||
                (VolumeUtils::IsPCVolumeEnable() && audioVolumeManager_.GetStreamMute(STREAM_MUSIC))) {
                AUDIO_INFO_LOG("no normal ringer mode and no alarm, dont dual hal tone.");
                return false;
            }
            UpdateDualToneState(true, sessionId);
        } else {
            audioActiveDevice_.UpdateActiveDevicesRoute(activeDevices);
        }
        return true;
    }
    return false;
}

void AudioCoreService::UpdateDualToneState(const bool &enable, const int32_t &sessionId)
{
    AUDIO_INFO_LOG("update dual tone state, enable:%{public}d, sessionId:%{public}d", enable, sessionId);
    enableDualHalToneState_ = enable;
    if (enableDualHalToneState_) {
        enableDualHalToneSessionId_ = sessionId;
    }
    Trace trace("AudioDeviceCommon::UpdateDualToneState sessionId:" + std::to_string(sessionId));
    auto ret = AudioServerProxy::GetInstance().UpdateDualToneStateProxy(enable, sessionId);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Failed to update the dual tone state for sessionId:%{public}d", sessionId);
}

int32_t AudioCoreService::MoveToLocalOutputDevice(std::vector<SinkInput> sinkInputIds,
    std::shared_ptr<AudioDeviceDescriptor> localDeviceDescriptor)
{
    AUDIO_INFO_LOG("Start for [%{public}zu] sink-inputs", sinkInputIds.size());
    // check
    CHECK_AND_RETURN_RET_LOG(LOCAL_NETWORK_ID == localDeviceDescriptor->networkId_,
        ERR_INVALID_OPERATION, "failed: not a local device.");

    // start move.
    uint32_t sinkId = -1; // invalid sink id, use sink name instead.
    for (size_t i = 0; i < sinkInputIds.size(); i++) {
        AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;

        std::string sinkName = pipeManager_->GetAdapterNameBySessionId(sinkInputIds[i].streamId);
        AUDIO_INFO_LOG("Session %{public}d, sinkName %{public}s", sinkInputIds[i].streamId, sinkName.c_str());
        if (sinkName == BLUETOOTH_SPEAKER) {
            std::string activePort = BLUETOOTH_SPEAKER;
            audioPolicyManager_.SuspendAudioDevice(activePort, false);
        }
        AUDIO_INFO_LOG("move for session [%{public}d], portName %{public}s pipeType %{public}d",
            sinkInputIds[i].streamId, sinkName.c_str(), pipeType);
        int32_t ret = audioPolicyManager_.MoveSinkInputByIndexOrName(sinkInputIds[i].paStreamId, sinkId, sinkName);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR,
            "move [%{public}d] to local failed", sinkInputIds[i].streamId);
        audioRouteMap_.AddRouteMapInfo(sinkInputIds[i].uid, LOCAL_NETWORK_ID, sinkInputIds[i].pid);
    }

    isCurrentRemoteRenderer_ = false;
    return SUCCESS;
}

void AudioCoreService::UpdateDeviceInfo(std::shared_ptr<AudioDeviceDescriptor> oldDeviceDesc,
    const std::shared_ptr<AudioDeviceDescriptor> newDeviceDesc, bool hasBTPermission, bool hasSystemPermission)
{
    oldDeviceDesc->deviceType_ = newDeviceDesc->deviceType_;
    oldDeviceDesc->deviceRole_ = newDeviceDesc->deviceRole_;
    oldDeviceDesc->deviceId_ = newDeviceDesc->deviceId_;
    oldDeviceDesc->channelMasks_ = newDeviceDesc->channelMasks_;
    oldDeviceDesc->channelIndexMasks_ = newDeviceDesc->channelIndexMasks_;
    oldDeviceDesc->displayName_ = newDeviceDesc->displayName_;
    oldDeviceDesc->connectState_ = newDeviceDesc->connectState_;

    if (oldDeviceDesc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        oldDeviceDesc->a2dpOffloadFlag_ = audioA2dpOffloadFlag_.GetA2dpOffloadFlag();
    }

    if (hasBTPermission) {
        oldDeviceDesc->deviceName_ = newDeviceDesc->deviceName_;
        oldDeviceDesc->macAddress_ = newDeviceDesc->macAddress_;
        oldDeviceDesc->deviceCategory_ = newDeviceDesc->deviceCategory_;
    } else {
        oldDeviceDesc->deviceName_ = "";
        oldDeviceDesc->macAddress_ = "";
        oldDeviceDesc->deviceCategory_ = CATEGORY_DEFAULT;
    }

    oldDeviceDesc->isLowLatencyDevice_ = HasLowLatencyCapability(oldDeviceDesc->deviceType_,
        newDeviceDesc->networkId_ != LOCAL_NETWORK_ID);

    if (hasSystemPermission) {
        oldDeviceDesc->networkId_ = newDeviceDesc->networkId_;
        oldDeviceDesc->volumeGroupId_ = newDeviceDesc->volumeGroupId_;
        oldDeviceDesc->interruptGroupId_ = newDeviceDesc->interruptGroupId_;
    } else {
        oldDeviceDesc->networkId_ = "";
        oldDeviceDesc->volumeGroupId_ = GROUP_ID_NONE;
        oldDeviceDesc->interruptGroupId_ = GROUP_ID_NONE;
    }
    oldDeviceDesc->audioStreamInfo_.samplingRate = newDeviceDesc->audioStreamInfo_.samplingRate;
    oldDeviceDesc->audioStreamInfo_.encoding = newDeviceDesc->audioStreamInfo_.encoding;
    oldDeviceDesc->audioStreamInfo_.format = newDeviceDesc->audioStreamInfo_.format;
    oldDeviceDesc->audioStreamInfo_.channels = newDeviceDesc->audioStreamInfo_.channels;
}

bool AudioCoreService::HasLowLatencyCapability(DeviceType deviceType, bool isRemote)
{
    // Distributed devices are low latency devices
    if (isRemote) {
        return true;
    }

    switch (deviceType) {
        case DeviceType::DEVICE_TYPE_EARPIECE:
        case DeviceType::DEVICE_TYPE_SPEAKER:
        case DeviceType::DEVICE_TYPE_WIRED_HEADSET:
        case DeviceType::DEVICE_TYPE_WIRED_HEADPHONES:
        case DeviceType::DEVICE_TYPE_USB_HEADSET:
        case DeviceType::DEVICE_TYPE_DP:
            return true;

        case DeviceType::DEVICE_TYPE_BLUETOOTH_SCO:
        case DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP:
            return false;
        default:
            return false;
    }
}

void AudioCoreService::TriggerRecreateRendererStreamCallback(int32_t callerPid, int32_t sessionId,
    uint32_t routeFlag, const AudioStreamDeviceChangeReasonExt::ExtEnum reason)
{
    Trace trace("AudioDeviceCommon::TriggerRecreateRendererStreamCallback");
    AUDIO_INFO_LOG("Trigger recreate renderer stream, pid: %{public}d, sessionId: %{public}d, flag: %{public}d",
        callerPid, sessionId, routeFlag);
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendRecreateRendererStreamEvent(callerPid, sessionId, routeFlag, reason);
    } else {
        AUDIO_WARNING_LOG("No audio policy server handler");
    }
}

void AudioCoreService::TriggerRecreateCapturerStreamCallback(int32_t callerPid, int32_t sessionId,
    uint32_t routeFlag)
{
    Trace trace("AudioCoreService::TriggerRecreateCapturerStreamCallback");
    AUDIO_INFO_LOG("Trigger recreate capturer stream, pid: %{public}d, sessionId: %{public}d, flag: %{public}d",
        callerPid, sessionId, routeFlag);
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendRecreateCapturerStreamEvent(callerPid, sessionId, routeFlag,
            AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
    } else {
        AUDIO_WARNING_LOG("No audio policy server handler");
    }
}

uint32_t AudioCoreService::OpenNewAudioPortAndRoute(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &paIndex)
{
    AUDIO_INFO_LOG("Flag: %{public}u, role: %{public}s", pipeInfo->routeFlag_, pipeInfo->moduleInfo_.role.c_str());
    uint32_t id = OPEN_PORT_FAILURE;
    if (pipeInfo->streamDescriptors_.front()->newDeviceDescs_.front()->deviceType_ == DEVICE_TYPE_REMOTE_CAST) {
        // DEVICE_TYPE_REMOTE_CAST no need to open audioport
        id = pipeInfo->streamDescriptors_.front()->sessionId_;
    } else {
        id = audioPolicyManager_.OpenAudioPort(pipeInfo, paIndex);
    }
    CHECK_AND_RETURN_RET_LOG(id != OPEN_PORT_FAILURE, ERR_OPERATION_FAILED, "OpenAudioPort failed %{public}d", id);
    audioIOHandleMap_.AddIOHandleInfo(pipeInfo->moduleInfo_.name, id);
    AUDIO_INFO_LOG("Get HDI id: %{public}u, paIndex %{public}u", id, paIndex);
    return id;
}

bool AudioCoreService::IsPaRoute(uint32_t routeFlag)
{
    if (routeFlag & AUDIO_OUTPUT_FLAG_DIRECT ||
        routeFlag & AUDIO_OUTPUT_FLAG_FAST ||
        routeFlag & AUDIO_INPUT_FLAG_FAST) {
        return false;
    }
    return true;
}

int32_t AudioCoreService::GetRealUid(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    if (streamDesc->callerUid_ == MEDIA_SERVICE_UID) {
        return streamDesc->appInfo_.appUid;
    }
    return streamDesc->callerUid_;
}

// device status listsener inner
void AudioCoreService::OnDeviceStatusUpdated(DeviceType devType, bool isConnected, const std::string& macAddress,
    const std::string& deviceName, const AudioStreamInfo& streamInfo, DeviceRole role, bool hasPair)
{
    // Pnp device status update
    audioDeviceStatus_.OnDeviceStatusUpdated(devType, isConnected, macAddress, deviceName, streamInfo, role, hasPair);
}

void AudioCoreService::OnDeviceStatusUpdated(AudioDeviceDescriptor &updatedDesc, bool isConnected)
{
    // Bluetooth device status updated
    DeviceType devType = updatedDesc.deviceType_;
    string macAddress = updatedDesc.macAddress_;
    string deviceName = updatedDesc.deviceName_;
    bool isActualConnection = (updatedDesc.connectState_ != VIRTUAL_CONNECTED);
    AUDIO_INFO_LOG("Device connection is actual connection: %{public}d", isActualConnection);

    AudioStreamInfo streamInfo = {};
#ifdef BLUETOOTH_ENABLE
    if (devType == DEVICE_TYPE_BLUETOOTH_A2DP && isActualConnection && isConnected) {
        int32_t ret = Bluetooth::AudioA2dpManager::GetA2dpDeviceStreamInfo(macAddress, streamInfo);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "Get a2dp device stream info failed!");
    }
    if (devType == DEVICE_TYPE_BLUETOOTH_A2DP_IN && isActualConnection && isConnected) {
        int32_t ret = Bluetooth::AudioA2dpManager::GetA2dpInDeviceStreamInfo(macAddress, streamInfo);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "Get a2dp input device stream info failed!");
    }
    if (isConnected && isActualConnection
        && devType == DEVICE_TYPE_BLUETOOTH_SCO
        && updatedDesc.deviceCategory_ != BT_UNWEAR_HEADPHONE
        && !audioDeviceManager_.GetScoState()) {
        Bluetooth::AudioHfpManager::SetActiveHfpDevice(macAddress);
    }
#endif
    audioDeviceStatus_.OnDeviceStatusUpdated(updatedDesc, devType,
        macAddress, deviceName, isActualConnection, streamInfo, isConnected);
}

void AudioCoreService::OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop)
{
    // Distributed devices status update
    audioDeviceStatus_.OnDeviceStatusUpdated(statusInfo, isStop);
}

void AudioCoreService::OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status)
{
    CHECK_AND_RETURN_LOG(devType != DEVICE_TYPE_NONE, "devType is none type");
    audioDeviceStatus_.OnMicrophoneBlockedUpdate(devType, status);
}

void AudioCoreService::OnPnpDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected)
{
    audioDeviceStatus_.OnPnpDeviceStatusUpdated(desc, isConnected);
}

void AudioCoreService::OnDeviceConfigurationChanged(DeviceType deviceType, const std::string &macAddress,
    const std::string &deviceName, const AudioStreamInfo &streamInfo)
{
    audioDeviceStatus_.OnDeviceConfigurationChanged(deviceType, macAddress, deviceName, streamInfo);
}

int32_t AudioCoreService::OnServiceConnected(AudioServiceIndex serviceIndex)
{
    return audioDeviceStatus_.OnServiceConnected(serviceIndex);
}

void AudioCoreService::OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress)
{
    audioDeviceStatus_.OnForcedDeviceSelected(devType, macAddress);
}
// device status listener end 

std::vector<sptr<MicrophoneDescriptor>> AudioCoreService::GetAvailableMicrophones()
{
    return audioMicrophoneDescriptor_.GetAvailableMicrophones();
}

std::vector<sptr<MicrophoneDescriptor>> AudioCoreService::GetAudioCapturerMicrophoneDescriptors(int32_t sessionId)
{
    return audioMicrophoneDescriptor_.GetAudioCapturerMicrophoneDescriptors(sessionId);
}

void AudioCoreService::OnReceiveBluetoothEvent(const std::string macAddress, const std::string deviceName)
{
    audioDeviceManager_.OnReceiveBluetoothEvent(macAddress, deviceName);
    audioConnectedDevice_.SetDisplayName(macAddress, deviceName);
}

int32_t AudioCoreService::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    return audioRecoveryDevice_.SelectOutputDevice(audioRendererFilter, selectedDesc);
}

int32_t AudioCoreService::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    return audioRecoveryDevice_.SelectInputDevice(audioCapturerFilter, selectedDesc);
}

int32_t AudioCoreService::GetCurrentRendererChangeInfos(vector<shared_ptr<AudioRendererChangeInfo>>
    &audioRendererChangeInfos, bool hasBTPermission, bool hasSystemPermission)
{
    int32_t status = streamCollector_.GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    CHECK_AND_RETURN_RET_LOG(status == SUCCESS, status,
        "AudioPolicyServer:: Get renderer change info failed");

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

void AudioCoreService::UpdateRendererInfoWhenNoPermission(const shared_ptr<AudioRendererChangeInfo> &audioRendererChangeInfos,
    bool hasSystemPermission)
{
    if (!hasSystemPermission) {
        audioRendererChangeInfos->clientUID = 0;
        audioRendererChangeInfos->rendererState = RENDERER_INVALID;
    }
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
    audioDeviceCommon_.OnPreferredOutputDeviceUpdated(curOutputDeviceDesc);
    AUDIO_DEBUG_LOG("Success");
}

void AudioCoreService::SendA2dpConnectedWhileRunning(const RendererState &rendererState, const uint32_t &sessionId)
{
    if ((rendererState == RENDERER_RUNNING) && (audioA2dpOffloadManager_ != nullptr) &&
        !audioA2dpOffloadManager_->IsA2dpOffloadConnecting(sessionId)) {
        AUDIO_INFO_LOG("Notify client not to block.");
        std::thread sendConnectedToClient(&AudioCoreService::UpdateSessionConnectionState, this, sessionId,
            DATA_LINK_CONNECTED);
        sendConnectedToClient.detach();
    }
}

void AudioCoreService::UpdateSessionConnectionState(const int32_t &sessionID, const int32_t &state)
{
    AudioServerProxy::GetInstance().UpdateSessionConnectionStateProxy(sessionID, state);
}

void AudioCoreService::UpdateTrackerDeviceChange(const vector<std::shared_ptr<AudioDeviceDescriptor>> &desc)
{
    AUDIO_INFO_LOG("Start");

    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    for (std::shared_ptr<AudioDeviceDescriptor> deviceDesc : desc) {
        if (deviceDesc->deviceRole_ == OUTPUT_DEVICE) {
            DeviceType type = curOutputDeviceType;
            std::string macAddress = audioActiveDevice_.GetCurrentOutputDeviceMacAddr();
            auto itr = audioConnectedDevice_.CheckExistOutputDevice(type, macAddress);
            if (itr != nullptr) {
                AudioDeviceDescriptor outputDevice(AudioDeviceDescriptor::DEVICE_INFO);
                audioDeviceCommon_.UpdateDeviceInfo(outputDevice, itr, true, true);
                streamCollector_.UpdateTracker(AUDIO_MODE_PLAYBACK, outputDevice);
            }
        }

        if (deviceDesc->deviceRole_ == INPUT_DEVICE) {
            DeviceType type = audioActiveDevice_.GetCurrentInputDeviceType();
            auto itr = audioConnectedDevice_.CheckExistInputDevice(type);
            if (itr != nullptr) {
                AudioDeviceDescriptor inputDevice(AudioDeviceDescriptor::DEVICE_INFO);
                audioDeviceCommon_.UpdateDeviceInfo(inputDevice, itr, true, true);
                audioMicrophoneDescriptor_.UpdateAudioCapturerMicrophoneDescriptor(itr->deviceType_);
                streamCollector_.UpdateTracker(AUDIO_MODE_RECORD, inputDevice);
            }
        }
    }
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

void AudioCoreService::SetDisplayName(const std::string &deviceName, bool isLocalDevice)
{
    audioConnectedDevice_.SetDisplayName(deviceName, isLocalDevice);
}

int32_t AudioCoreService::TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason)
{
    SelectOutputDeviceAndRoute(reason);
    SelectInputDeviceAndRoute();

    // update a2dp offload
    audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream();
    return SUCCESS;
}

// Call directly - no lock
void AudioCoreService::NotifyAccountsChanged(const int &id)
{
    audioPolicyManager_.NotifyAccountsChanged(id);
    RegisterDataObserver();
    SubscribeAccessibilityConfigObserver();
}

void AudioCoreService::RegisterDataObserver()
{
    std::string devicesName = "";
    int32_t ret = AudioPolicyUtils::GetInstance().GetDeviceNameFromDataShareHelper(devicesName);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "RegisterDataObserver get devicesName failed");
    eventEntry_->SetDisplayName(devicesName, true);
    RegisterNameMonitorHelper();
}

void AudioCoreService::SubscribeAccessibilityConfigObserver()
{
#ifdef ACCESSIBILITY_ENABLE
    RegisterAccessibilityMonitorHelper();
    AUDIO_INFO_LOG("Subscribe accessibility config observer successfully");
#endif
}

void AudioCoreService::RegisterAccessibilityMonitorHelper()
{
    RegisterAccessiblilityBalance();
    RegisterAccessiblilityMono();
}

void AudioCoreService::RegisterAccessiblilityBalance()
{
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    AudioSettingObserver::UpdateFunc updateFuncBalance = [&](const std::string &key) {
        AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
        float balance = 0;
        int32_t ret = settingProvider.GetFloatValue(CONFIG_AUDIO_BALANACE_KEY, balance, "secure");
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "get balance value failed");
        if (balance < -1.0f || balance > 1.0f) {
            AUDIO_WARNING_LOG("audioBalance value is out of range [-1.0, 1.0]");
        } else {
            OnAudioBalanceChanged(balance);
        }
    };
    sptr observer = settingProvider.CreateObserver(CONFIG_AUDIO_BALANACE_KEY, updateFuncBalance);
    ErrCode ret = settingProvider.RegisterObserver(observer, "secure");
    if (ret != ERR_OK) {
        AUDIO_ERR_LOG("RegisterObserver balance failed");
    }
}

void AudioCoreService::OnAudioBalanceChanged(float audioBalance)
{
    AUDIO_DEBUG_LOG("audioBalance = %{public}f", audioBalance);
    AudioServerProxy::GetInstance().SetAudioBalanceValueProxy(audioBalance);
}

bool AudioCoreService::GetFastControlParam()
{
    int32_t fastControlFlag = 1; // default 1, set isFastControlled_ true
    GetSysPara("persist.multimedia.audioflag.fastcontrolled", fastControlFlag);
    if (fastControlFlag == 0) {
        isFastControlled_ = false;
    }
    return isFastControlled_;
}

void AudioCoreService::RegisterAccessiblilityMono()
{
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    AudioSettingObserver::UpdateFunc updateFuncMono = [&](const std::string &key) {
        AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
        int32_t value = 0;
        ErrCode ret = settingProvider.GetIntValue(CONFIG_AUDIO_MONO_KEY, value, "secure");
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "get mono value failed");
        OnMonoAudioConfigChanged(value != 0);
    };
    sptr observer = settingProvider.CreateObserver(CONFIG_AUDIO_MONO_KEY, updateFuncMono);
    ErrCode ret = settingProvider.RegisterObserver(observer, "secure");
    if (ret != ERR_OK) {
        AUDIO_ERR_LOG("RegisterObserver mono failed");
    }
}

void AudioCoreService::OnMonoAudioConfigChanged(bool audioMono)
{
    AUDIO_DEBUG_LOG("audioMono = %{public}s", audioMono? "true": "false");
    AudioServerProxy::GetInstance().SetAudioMonoStateProxy(audioMono);
}

void AudioCoreService::RegisterNameMonitorHelper()
{
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper
        = AudioPolicyUtils::GetInstance().CreateDataShareHelperInstance();
    CHECK_AND_RETURN_LOG(dataShareHelper != nullptr, "dataShareHelper is NULL");

    auto uri = std::make_shared<Uri>(std::string(SETTINGS_DATA_BASE_URI) + "&key=" + PREDICATES_STRING);
    sptr<AAFwk::DataAbilityObserverStub> settingDataObserver = std::make_unique<DataShareObserverCallBack>().release();
    dataShareHelper->RegisterObserver(*uri, settingDataObserver);

    dataShareHelper->Release();
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

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioCoreService::GetExcludedDevices(
    AudioDeviceUsage audioDevUsage)
{
    return audioStateManager_.GetExcludedDevices(audioDevUsage);
}

bool AudioCoreService::ConnectServiceAdapter()
{
    return audioPolicyManager_.ConnectServiceAdapter();
}

bool AudioCoreService::GetVolumeGroupInfos(std::vector<sptr<VolumeGroupInfo>> &infos)
{
    return audioVolumeManager_.GetVolumeGroupInfosNotWait(infos);
}

void AudioCoreService::StoreDistributedRoutingRoleInfo(
    const std::shared_ptr<AudioDeviceDescriptor> descriptor, CastType type)
{
    distributedRoutingInfo_.descriptor = descriptor;
    distributedRoutingInfo_.type = type;
}

void AudioCoreService::ConfigDistributedRoutingRole(
    const std::shared_ptr<AudioDeviceDescriptor> descriptor, CastType type)
{
    StoreDistributedRoutingRoleInfo(descriptor, type);
    SelectDeviceAndRoute(AudioStreamDeviceChangeReason::OVERRODE);
}

int32_t AudioCoreService::GetSystemVolumeLevel(AudioStreamType streamType)
{
    return audioVolumeManager_.GetSystemVolumeLevel(streamType);
}

float AudioCoreService::GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel,
    DeviceType deviceType) const
{
    return audioPolicyManager_.GetSystemVolumeInDb(volumeType, volumeLevel, deviceType);
}

int32_t AudioCoreService::SetRingerMode(AudioRingerMode ringMode)
{
    int32_t result = audioPolicyManager_.SetRingerMode(ringMode);
    if (result == SUCCESS) {
        if (Util::IsRingerAudioScene(audioSceneManager_.GetAudioScene(true))) {
            AUDIO_INFO_LOG("fetch output device after switch new ringmode.");
            SelectOutputDeviceAndRoute();
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

int32_t AudioCoreService::SetDefaultOutputDevice(const DeviceType deviceType, const uint32_t sessionID,
    const StreamUsage streamUsage, bool isRunning)
{
    CHECK_AND_RETURN_RET_LOG(policyConfigMananger_.GetHasEarpiece(), ERR_NOT_SUPPORTED, "the device has no earpiece");
    int32_t ret = audioDeviceManager_.SetDefaultOutputDevice(deviceType, sessionID, streamUsage, isRunning);
    if (ret == NEED_TO_FETCH) {
        SelectOutputDeviceAndRoute(AudioStreamDeviceChangeReasonExt::ExtEnum::SET_DEFAULT_OUTPUT_DEVICE);
        return SUCCESS;
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

std::string AudioCoreService::GetAdapterNameBySessionId(uint32_t sessionId)
{
    AUDIO_INFO_LOG("SessionId %{public}u", sessionId);
    std::string adapterName = pipeManager_->GetAdapterNameBySessionId(sessionId);
    return adapterName;
}

int32_t AudioCoreService::GetProcessDeviceInfoBySessionId(uint32_t sessionId, AudioDeviceDescriptor &deviceInfo)
{
    AUDIO_INFO_LOG("SessionId %{public}u", sessionId);
    deviceInfo = AudioDeviceDescriptor(pipeManager_->GetProcessDeviceInfoBySessionId(sessionId));
    return SUCCESS;
}

// Inner class - EventEntry
AudioCoreService::EventEntry::EventEntry(std::shared_ptr<AudioCoreService> coreService) : coreService_(coreService) {}

void AudioCoreService::EventEntry::RegiestCoreService()
{
    AUDIO_INFO_LOG("In");
    coreService_->SetAudioServerProxy();
    sptr<CoreServiceProviderWrapper> wrapper = new(std::nothrow) CoreServiceProviderWrapper(this);
    CHECK_AND_RETURN_LOG(wrapper != nullptr, "Get null CoreServiceProviderWrapper");
    sptr<IRemoteObject> object = wrapper->AsObject();
    CHECK_AND_RETURN_LOG(object != nullptr, "AsObject is nullptr");

    int32_t ret = AudioServerProxy::GetInstance().RegiestCoreServiceProviderProxy(object);
    AUDIO_INFO_LOG("Result:%{public}d", ret);
}

int32_t AudioCoreService::EventEntry::CreateRendererClient(
    std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t &flag, uint32_t &sessionId)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->CreateRendererClient(streamDesc, flag, sessionId);
    return SUCCESS;
}

int32_t AudioCoreService::EventEntry::CreateCapturerClient(
    std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t &flag, uint32_t &sessionId)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->CreateCapturerClient(streamDesc, flag, sessionId);
    return SUCCESS;
}

int32_t AudioCoreService::EventEntry::UpdateSessionOperation(uint32_t sessionId, SessionOperation operation)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    switch (operation) {
        case SESSION_OPERATION_START:
            return coreService_->StartClient(sessionId);
        case SESSION_OPERATION_PAUSE:
            return coreService_->PauseClient(sessionId);
        case SESSION_OPERATION_STOP:
            return coreService_->StopClient(sessionId);
        case SESSION_OPERATION_RELEASE:
            return coreService_->ReleaseClient(sessionId);
        default:
            return SUCCESS;
    }
}

std::string AudioCoreService::EventEntry::GetAdapterNameBySessionId(uint32_t sessionId)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetAdapterNameBySessionId(sessionId);
}

int32_t AudioCoreService::EventEntry::GetProcessDeviceInfoBySessionId(
    uint32_t sessionId, AudioDeviceDescriptor &deviceInfo)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetProcessDeviceInfoBySessionId(sessionId, deviceInfo);
}

uint32_t AudioCoreService::EventEntry::GenerateSessionId()
{
    return coreService_->GenerateSessionId();
}

int32_t AudioCoreService::EventEntry::SetDefaultOutputDevice(const DeviceType deviceType, const uint32_t sessionID,
    const StreamUsage streamUsage, bool isRunning)
{
    int32_t ret = coreService_->SetDefaultOutputDevice(deviceType, sessionID, streamUsage, isRunning);
    return ret;
}

// device status listener
void AudioCoreService::EventEntry::OnDeviceStatusUpdated(
    DeviceType devType, bool isConnected, const std::string& macAddress,
    const std::string& deviceName, const AudioStreamInfo& streamInfo, DeviceRole role, bool hasPair)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnDeviceStatusUpdated(devType, isConnected, macAddress, deviceName, streamInfo, role, hasPair);
}

void AudioCoreService::EventEntry::OnDeviceStatusUpdated(AudioDeviceDescriptor &updatedDesc, bool isConnected)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnDeviceStatusUpdated(updatedDesc, isConnected);
}

void AudioCoreService::EventEntry::OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnDeviceStatusUpdated(statusInfo, isStop);
}

void AudioCoreService::EventEntry::OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status)
{
    CHECK_AND_RETURN_LOG(devType != DEVICE_TYPE_NONE, "devType is none type");
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnMicrophoneBlockedUpdate(devType, status);
}

void AudioCoreService::EventEntry::OnPnpDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnPnpDeviceStatusUpdated(desc, isConnected);
}

void AudioCoreService::EventEntry::OnDeviceConfigurationChanged(DeviceType deviceType, const std::string &macAddress,
    const std::string &deviceName, const AudioStreamInfo &streamInfo)
{
    coreService_->OnDeviceConfigurationChanged(deviceType, macAddress, deviceName, streamInfo);
}

void AudioCoreService::EventEntry::OnServiceConnected(AudioServiceIndex serviceIndex)
{
    AUDIO_INFO_LOG("[module_load]::OnServiceConnected for [%{public}d]", serviceIndex);
    CHECK_AND_RETURN_LOG(serviceIndex >= HDI_SERVICE_INDEX && serviceIndex <= AUDIO_SERVICE_INDEX, "invalid index");

    // If audio service or hdi service is not ready, donot load default modules
    std::lock_guard<std::mutex> lock(coreService_->serviceFlagMutex_);
    coreService_->serviceFlag_.set(serviceIndex, true);
    if (coreService_->serviceFlag_.count() != MIN_SERVICE_COUNT) {
        AUDIO_INFO_LOG("[module_load]::hdi service or audio service not up. Cannot load default module now");
        return;
    }

    std::unique_lock<std::shared_mutex> serviceLock(eventMutex_);
    int32_t ret = coreService_->OnServiceConnected(serviceIndex);
    serviceLock.unlock();
    if (ret == SUCCESS) {
#ifdef USB_ENABLE
        AudioUsbManager::GetInstance().Init(this);
#endif
        coreService_->audioEffectService_.SetMasterSinkAvailable();
    }
    // RegisterBluetoothListener() will be called when bluetooth_host is online
    // load hdi-effect-model
    LoadHdiEffectModel();
    AudioServerProxy::GetInstance().NotifyAudioPolicyReady();
}

void AudioCoreService::EventEntry::OnServiceDisconnected(AudioServiceIndex serviceIndex)
{
    AUDIO_WARNING_LOG("Service index [%{public}d]", serviceIndex);
}

void AudioCoreService::EventEntry::OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnForcedDeviceSelected(devType, macAddress);
}

int32_t AudioCoreService::EventEntry::SetAudioScene(AudioScene audioScene)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->SetAudioScene(audioScene);
    return SUCCESS;
}

bool AudioCoreService::EventEntry::IsArmUsbDevice(const AudioDeviceDescriptor &deviceDesc)
{
    std::shared_lock<std::shared_mutex> lock(eventMutex_);
    return coreService_->IsArmUsbDevice(deviceDesc);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioCoreService::EventEntry::GetDevices(DeviceFlag deviceFlag)
{
    std::shared_lock<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetDevices(deviceFlag);
}

int32_t AudioCoreService::EventEntry::SetDeviceActive(InternalDeviceType deviceType, bool active, const int32_t pid)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->SetDeviceActive(deviceType, active, pid);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioCoreService::EventEntry::GetPreferredOutputDeviceDescriptors(
    AudioRendererInfo &rendererInfo, std::string networkId)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetPreferredOutputDeviceDescInner(rendererInfo, networkId);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioCoreService::EventEntry::GetPreferredInputDeviceDescriptors(
    AudioCapturerInfo &captureInfo, std::string networkId)
{
    std::shared_lock<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetPreferredInputDeviceDescInner(captureInfo, networkId);
}

std::shared_ptr<AudioDeviceDescriptor> AudioCoreService::EventEntry::GetActiveBluetoothDevice()
{
    std::shared_lock<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetActiveBluetoothDevice();
}

void AudioCoreService::EventEntry::OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnDeviceInfoUpdated(desc, command);
}

int32_t AudioCoreService::EventEntry::SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->SetCallDeviceActive(deviceType, active, address);
    return SUCCESS;
}

std::vector<shared_ptr<AudioDeviceDescriptor>> AudioCoreService::EventEntry::GetAvailableDevices(AudioDeviceUsage usage)
{
    std::shared_lock<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetAvailableDevices(usage);
}

int32_t AudioCoreService::EventEntry::RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
    const sptr<IRemoteObject> &object, const int32_t apiVersion)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->RegisterTracker(mode, streamChangeInfo, object, apiVersion);
}

int32_t AudioCoreService::EventEntry::UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->UpdateTracker(mode, streamChangeInfo);
}

void AudioCoreService::EventEntry::RegisteredTrackerClientDied(pid_t uid)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->RegisteredTrackerClientDied(uid);
}

bool AudioCoreService::EventEntry::ConnectServiceAdapter()
{
    bool ret = coreService_->ConnectServiceAdapter();
    CHECK_AND_RETURN_RET_LOG(ret, false, "Error in connecting to audio service adapter");

    OnServiceConnected(AudioServiceIndex::AUDIO_SERVICE_INDEX);

    return true;
}

vector<sptr<MicrophoneDescriptor>> AudioCoreService::EventEntry::GetAvailableMicrophones()
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetAvailableMicrophones();
}

vector<sptr<MicrophoneDescriptor>> AudioCoreService::EventEntry::GetAudioCapturerMicrophoneDescriptors(
    int32_t sessionId)
{
    std::shared_lock<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetAudioCapturerMicrophoneDescriptors(sessionId);
}

void AudioCoreService::EventEntry::OnReceiveBluetoothEvent(const std::string macAddress, const std::string deviceName)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnReceiveBluetoothEvent(macAddress, deviceName);
}

int32_t AudioCoreService::EventEntry::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    Trace trace("AudioCoreService::SelectOutputDevice");
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->SelectOutputDevice(audioRendererFilter, selectedDesc);
}

int32_t AudioCoreService::EventEntry::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    Trace trace("AudioCoreService::SelectInputDevice");
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->SelectInputDevice(audioCapturerFilter, selectedDesc);
}

int32_t AudioCoreService::EventEntry::GetCurrentRendererChangeInfos(vector<shared_ptr<AudioRendererChangeInfo>>
    &audioRendererChangeInfos, bool hasBTPermission, bool hasSystemPermission)
{
    std::shared_lock<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetCurrentRendererChangeInfos(audioRendererChangeInfos, hasBTPermission,
        hasSystemPermission);
}

void AudioCoreService::EventEntry::NotifyRemoteRenderState(
    std::string networkId, std::string condition, std::string value)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->NotifyRemoteRenderState(networkId, condition, value);
}

int32_t AudioCoreService::EventEntry::OnCapturerSessionAdded(uint64_t sessionID, SessionInfo sessionInfo,
    AudioStreamInfo streamInfo)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->OnCapturerSessionAdded(sessionID, sessionInfo, streamInfo);
}

void AudioCoreService::EventEntry::OnCapturerSessionRemoved(uint64_t sessionID)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnCapturerSessionRemoved(sessionID);
}

void AudioCoreService::EventEntry::SetDisplayName(const std::string &deviceName, bool isLocalDevice)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->SetDisplayName(deviceName, isLocalDevice);
}

int32_t AudioCoreService::EventEntry::TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->TriggerFetchDevice(reason);
}

#ifdef HAS_FEATURE_INNERCAPTURER
void AudioCoreService::LoadModernInnerCapSink()
{
    AUDIO_INFO_LOG("Start");
    AudioModuleInfo moduleInfo = {};
    moduleInfo.lib = "libmodule-inner-capturer-sink.z.so";
    moduleInfo.name = INNER_CAPTURER_SINK;

    moduleInfo.format = "s16le";
    moduleInfo.channels = "2"; // 2 channel
    moduleInfo.rate = "48000";
    moduleInfo.bufferSize = "3840"; // 20ms

    audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
}
#endif

void AudioCoreService::EventEntry::LoadHdiEffectModel()
{
    return AudioServerProxy::GetInstance().LoadHdiEffectModelProxy();
}

std::vector<sptr<VolumeGroupInfo>> AudioCoreService::EventEntry::GetVolumeGroupInfos()
{
    std::vector<sptr<VolumeGroupInfo>> infos = {};
    for (int32_t i = 0; i < RETRY_TIMES; i++) {
        std::shared_lock<std::shared_mutex> lock(eventMutex_);
        if (coreService_->GetVolumeGroupInfos(infos)) {
            return infos;
        } else {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_LOAD_DEFAULT_DEVICE_TIME_MS));
        }
    }
    AUDIO_ERR_LOG("timeout");
    return infos;
}

void AudioCoreService::EventEntry::FetchOutputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo,
    const AudioStreamDeviceChangeReasonExt reason)
{

}

void AudioCoreService::EventEntry::FetchInputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo)
{

}

int32_t AudioCoreService::EventEntry::ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    Trace trace("AudioCoreService::EventEntry::ExcludeOutputDevices");
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

int32_t AudioCoreService::EventEntry::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioCoreService::EventEntry::GetExcludedDevices(
    AudioDeviceUsage audioDevUsage)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetExcludedDevices(audioDevUsage);
}

int32_t AudioCoreService::EventEntry::GetPreferredOutputStreamType(AudioRendererInfo &rendererInfo,
    const std::string &bundleName)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetPreferredOutputStreamType(rendererInfo, bundleName);
}

int32_t AudioCoreService::EventEntry::GetPreferredInputStreamType(AudioCapturerInfo &capturerInfo)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetPreferredInputStreamType(capturerInfo);
}

} // namespace AudioStandard
} // namespace OHOS
