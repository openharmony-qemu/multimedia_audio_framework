/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioPolicyService"
#endif

#include "audio_policy_service.h"
#include <ability_manager_client.h>
#include <dlfcn.h>
#include "iservice_registry.h"

#include "audio_manager_listener_stub_impl.h"
#include "parameter.h"
#include "parameters.h"
#include "device_init_callback.h"
#include "audio_inner_call.h"
#ifdef FEATURE_DEVICE_MANAGER
#endif

#include "audio_collaborative_service.h"
#include "audio_spatialization_service.h"
#include "audio_converter_parser.h"
#include "media_monitor_manager.h"
#include "client_type_manager.h"
#include "audio_safe_volume_notification.h"
#include "audio_setting_provider.h"
#include "audio_spatialization_service.h"
#include "audio_usb_manager.h"

#include "audio_server_proxy.h"
#include "audio_policy_utils.h"
#include "audio_policy_global_parser.h"
#include "audio_background_manager.h"
#include "audio_core_service.h"
#include "audio_policy_datashare_listener.h"
#include "audio_zone_service.h"
#include "audio_policy_manager_listener.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

namespace {
static const char* CHECK_FAST_BLOCK_PREFIX = "Is_Fast_Blocked_For_AppName#";
static const char* AUDIO_SERVICE_PKG = "audio_manager_service";
}

const int32_t UID_AUDIO = 1041;

mutex g_dataShareHelperMutex;
bool AudioPolicyService::isBtListenerRegistered = false;
bool AudioPolicyService::isBtCrashed = false;
const std::string AINR_FLAG = "ai_voice_noise_suppression_flag";
const std::string AUDIO_SETTING_TABLE_TYPE = "global";
const std::string LIVE_EFFECT_KEY = "live_effect_enable";
const std::string LIVE_EFFECT_TABLE_TYPE = "system";
const std::string LIVE_EFFECT_ON = "NRON";
const int32_t INVALID_VALUE = -1;
static const std::unordered_set<std::string> ANRCategories = {"AINR", "PNR"};
static constexpr int32_t MEDIA_SERVICE_UID = 1013;

AudioPolicyService::~AudioPolicyService()
{
    AUDIO_INFO_LOG("~AudioPolicyService()");
    Deinit();
}

void AudioPolicyService::RecordBootStateTime(AudioPolicyServerBootState state, int64_t curTime)
{
    audioPolicyDump_.RecordBootStateTime(state, curTime);
}

void AudioPolicyService::RecordBootStateTime(BootAnimationState state, int32_t uid, int64_t curTime)
{
    CHECK_AND_RETURN(uid == BOOT_ANIMATION_UID);
    audioPolicyDump_.RecordBootStateTime(state, uid, curTime);
}

bool AudioPolicyService::Init(void)
{
    audioPolicyManager_.Init();
    RecordBootStateTime(AudioPolicyServerBootState::LOAD_AUDIO_POLICY_CONFIG, ClockTime::GetCurMilli());
    audioEffectService_.EffectServiceInit();
    RecordBootStateTime(AudioPolicyServerBootState::LOAD_AUDIO_EFFECT_CONFIG, ClockTime::GetCurMilli());
    audioDeviceManager_.ParseDeviceXml();
    audioGlobalConfigManager_.ParseGlobalConfigXml();
    RecordBootStateTime(AudioPolicyServerBootState::LOAD_AUDIO_POLICY_GLOBAL_CONFIG, ClockTime::GetCurMilli());

#ifdef FEATURE_DTMF_TONE
    bool ret = audioToneManager_.LoadToneDtmfConfig();
    RecordBootStateTime(AudioPolicyServerBootState::LOAD_AUDIO_TONE_DTMF_CONFIG, ClockTime::GetCurMilli());
    CHECK_AND_RETURN_RET_LOG(ret, false, "Audio Tone Load Configuration failed");
#endif

    CreateRecoveryThread();
    std::string versionType = OHOS::system::GetParameter("const.logsystem.versiontype", "commercial");
    AudioDump::GetInstance().SetVersionType(versionType);

    ecEnableState_ = GetEcEnableParam();
    AudioCoreService::GetCoreService()->SetEcEnableState(ecEnableState_);

#ifdef HAS_FEATURE_INNERCAPTURER
    AudioServerProxy::GetInstance().SetInnerCapLimitProxy(audioGlobalConfigManager_.GetCapLimit());
#endif
    return true;
}

void AudioPolicyService::CreateRecoveryThread()
{
    if (RecoveryDevicesThread_ != nullptr) {
        RecoveryDevicesThread_->detach();
    }
    RecoveryDevicesThread_ = std::make_unique<std::thread>([this] {
        audioRecoveryDevice_.RecoverExcludedOutputDevices();
        audioRecoveryDevice_.RecoveryPreferredDevices();
        audioBackgroundManager_.RecoveryAppState();
    });
    pthread_setname_np(RecoveryDevicesThread_->native_handle(), "APSRecovery");
}

void AudioPolicyService::Deinit(void)
{
    AUDIO_WARNING_LOG("Policy service died. closing active ports");
    std::unordered_map<std::string, AudioIOHandle> mapCopy = audioIOHandleMap_.GetCopy();
    std::for_each(mapCopy.begin(), mapCopy.end(), [&](std::pair<std::string, AudioIOHandle> handle) {
        audioPolicyManager_.CloseAudioPort(handle.second);
    });
    audioPolicyManager_.Deinit();
    audioIOHandleMap_.DeInit();
    deviceStatusListener_->UnRegisterDeviceStatusListener();
#ifdef AUDIO_WIRED_DETECT
    AudioPnpServer::GetAudioPnpServer().StopPnpServer();
#endif

    if (isBtListenerRegistered) {
        UnregisterBluetoothListener();
    }

    audioVolumeManager_.DeInit();
    if (RecoveryDevicesThread_ != nullptr && RecoveryDevicesThread_->joinable()) {
        RecoveryDevicesThread_->join();
        RecoveryDevicesThread_.reset();
        RecoveryDevicesThread_ = nullptr;
    }

    audioDeviceCommon_.DeInit();
    audioRecoveryDevice_.DeInit();
    audioDeviceStatus_.DeInit();
    audioDeviceLock_.DeInit();
    return;
}

int32_t AudioPolicyService::SetAppVolumeLevel(int32_t appUid, int32_t volumeLevel)
{
    // update dump appvolume
    audioDeviceLock_.UpdateAppVolume(appUid, volumeLevel);
    return audioVolumeManager_.SetAppVolumeLevel(appUid, volumeLevel);
}

int32_t AudioPolicyService::SetSourceOutputStreamMute(int32_t uid, bool setMute) const
{
    int32_t status = audioPolicyManager_.SetSourceOutputStreamMute(uid, setMute);
    if (status == 0) {
        streamCollector_.UpdateCapturerInfoMuteStatus(uid, setMute);
    }
    return status;
}

int32_t AudioPolicyService::SetSourceOutputStreamMuteByStreamId(int32_t sessionId, bool setMute)
{
    int32_t status = audioPolicyManager_.SetSourceOutputStreamMuteByStreamId(sessionId, setMute);
    return status;
}

int32_t AudioPolicyService::SetAncoSourceOutputStreamMute(int32_t uid, bool setMute) const
{
    streamCollector_.UpdateCapturerInfoMuteStatus(uid, setMute);
    return SUCCESS;
}

std::string AudioPolicyService::GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType)
{
    (void)streamType;

    std::string selectedDevice = audioRouteMap_.GetDeviceInfoByUidAndPid(uid, pid);
    if (selectedDevice == "") {
        return selectedDevice;
    }

    if (LOCAL_NETWORK_ID == selectedDevice) {
        AUDIO_INFO_LOG("uid[%{public}d]-->local.", uid);
        return "";
    }
    // check if connected.
    if (audioConnectedDevice_.CheckDeviceConnected(selectedDevice)) {
        AUDIO_INFO_LOG("result[%{public}s]", selectedDevice.c_str());
        return selectedDevice;
    } else {
        audioRouteMap_.DelRouteMapInfoByKey(uid);
        AUDIO_INFO_LOG("device already disconnected.");
        return "";
    }
}

void AudioPolicyService::RestoreSession(const uint32_t &sessionID, RestoreInfo restoreInfo)
{
    AudioServerProxy::GetInstance().RestoreSessionProxy(sessionID, restoreInfo);
}

DistributedRoutingInfo AudioPolicyService::GetDistributedRoutingRoleInfo()
{
    return distributedRoutingInfo_;
}

int32_t AudioPolicyService::NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo,
    uint32_t sessionId)
{
    int32_t error = SUCCESS;
    audioPolicyServerHandler_->SendCapturerCreateEvent(capturerInfo, streamInfo, sessionId, true, error);
    return error;
}

int32_t AudioPolicyService::NotifyWakeUpCapturerRemoved()
{
    audioPolicyServerHandler_->SendWakeupCloseEvent(false);
    return SUCCESS;
}

bool AudioPolicyService::IsAbsVolumeSupported()
{
    return audioPolicyManager_.IsAbsVolumeScene();
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::GetDevices(DeviceFlag deviceFlag)
{
    return audioDeviceLock_.GetDevices(deviceFlag);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::GetOutputDevice(
    sptr<AudioRendererFilter> audioRendererFilter)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> result;
    auto coreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN_RET_LOG(coreService != nullptr, result, "AudioCoreService is null");
    return coreService->GetPreferredOutputDeviceDescInner(audioRendererFilter->rendererInfo,
        LOCAL_NETWORK_ID, audioRendererFilter->uid, audioRendererFilter->streamId);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::GetInputDevice(
    sptr<AudioCapturerFilter> audioCapturerFilter)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> result;
    auto coreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN_RET_LOG(coreService != nullptr, result, "AudioCoreService is null");
    return coreService->GetPreferredInputDeviceDescInner(audioCapturerFilter->capturerInfo,
        LOCAL_NETWORK_ID, audioCapturerFilter->uid, audioCapturerFilter->streamId);
}

void AudioPolicyService::OnUpdateAnahsSupport(std::string anahsShowType)
{
    AUDIO_INFO_LOG("OnUpdateAnahsSupport show type: %{public}s", anahsShowType.c_str());
    deviceStatusListener_->UpdateAnahsPlatformType(anahsShowType);
}

void AudioPolicyService::OnPnpDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected)
{
    audioDeviceLock_.OnPnpDeviceStatusUpdated(desc, isConnected);
}

void AudioPolicyService::OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status)
{
    CHECK_AND_RETURN_LOG(devType != DEVICE_TYPE_NONE, "devType is none type");
    audioDeviceLock_.OnMicrophoneBlockedUpdate(devType, status);
}

void AudioPolicyService::OnDeviceStatusUpdated(DeviceType devType, bool isConnected, const std::string& macAddress,
    const std::string& deviceName, const AudioStreamInfo& streamInfo, DeviceRole role, bool hasPair)
{
    audioDeviceLock_.OnDeviceStatusUpdated(devType, isConnected, macAddress, deviceName, streamInfo, role, hasPair);
}

void AudioPolicyService::OnDeviceStatusUpdated(AudioDeviceDescriptor &updatedDesc, bool isConnected)
{
    audioDeviceLock_.OnDeviceStatusUpdated(updatedDesc, isConnected);
}

void AudioPolicyService::UpdateA2dpOffloadFlagBySpatialService(
    const std::string& macAddress, std::unordered_map<uint32_t, bool> &sessionIDToSpatializationEnableMap)
{
    DeviceType spatialDevice = audioDeviceCommon_.GetSpatialDeviceType(macAddress);
    if (audioA2dpOffloadManager_) {
        audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForSpatializationChanged(
            sessionIDToSpatializationEnableMap, spatialDevice);
    }
}

void AudioPolicyService::OnDeviceConfigurationChanged(DeviceType deviceType, const std::string &macAddress,
    const std::string &deviceName, const AudioStreamInfo &streamInfo)
{
    audioDeviceLock_.OnDeviceConfigurationChanged(deviceType, macAddress, deviceName, streamInfo);
}

void AudioPolicyService::RegisterRemoteDevStatusCallback()
{
#ifdef FEATURE_DEVICE_MANAGER
    std::shared_ptr<DistributedHardware::DmInitCallback> initCallback = std::make_shared<DeviceInitCallBack>();
    int32_t ret = DistributedHardware::DeviceManager::GetInstance().InitDeviceManager(AUDIO_SERVICE_PKG, initCallback);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Init device manage failed");
    auto callback = std::make_shared<DeviceStatusCallbackImpl>();
    DistributedHardware::DeviceManager::GetInstance().RegisterDevStatusCallback(AUDIO_SERVICE_PKG, "", callback);
    DistributedHardware::DeviceManager::GetInstance().RegisterDevStateCallback(AUDIO_SERVICE_PKG, "", callback);
    AUDIO_INFO_LOG("Done");
#endif
}

void AudioPolicyService::GetAllSinkInputs(std::vector<SinkInput> &sinkInputs)
{
    AudioServerProxy::GetInstance().GetAllSinkInputsProxy(sinkInputs);
}

void AudioPolicyService::RegisterAccessibilityMonitorHelper()
{
    AudioPolicyDataShareListener::RegisterAccessiblilityBalance();
    AudioPolicyDataShareListener::RegisterAccessiblilityMono();
    AudioPolicyDataShareListener::RegisterBroadcast();
    AudioPolicyDataShareListener::RegisterVehiclePriority();
}

void AudioPolicyService::OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop)
{
    audioDeviceLock_.OnDeviceStatusUpdated(statusInfo, isStop);
}

void AudioPolicyService::OnServiceConnected(AudioServiceIndex serviceIndex)
{
    AUDIO_INFO_LOG("Not support, use AudioCoreService");
}

void AudioPolicyService::OnServiceDisconnected(AudioServiceIndex serviceIndex)
{
    AUDIO_WARNING_LOG("Start for [%{public}d]", serviceIndex);
}

void AudioPolicyService::OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress,
    sptr<AudioRendererFilter> filter, const std::string &caller)
{
    audioDeviceLock_.OnForcedDeviceSelected(devType, macAddress, filter);
}

void AudioPolicyService::OnPrivacyDeviceSelected(DeviceType devType, const std::string &macAddress,
    const std::string &caller)
{
    audioDeviceLock_.OnPrivacyDeviceSelected(devType, macAddress);
}

void AudioPolicyService::OnConnectFailed(AudioDeviceDescriptor &desc)
{
    audioDeviceLock_.OnConnectFailed(desc);
}

void AudioPolicyService::OnVehiclePriorityChanged(bool enable)
{
    AudioCoreService::GetCoreService()->GetEventEntry()->OnVehiclePriorityChanged(enable);
}

void AudioPolicyService::LoadEffectLibrary()
{
    // IPC -> audioservice load library
    OriginalEffectConfig oriEffectConfig = {};
    audioEffectService_.GetOriginalEffectConfig(oriEffectConfig);
    vector<Effect> successLoadedEffects;

    bool loadSuccess = AudioServerProxy::GetInstance().LoadAudioEffectLibrariesProxy(oriEffectConfig.libraries,
        oriEffectConfig.effects, successLoadedEffects);
    if (!loadSuccess) {
        AUDIO_ERR_LOG("Load audio effect failed, please check log");
    }

    audioEffectService_.UpdateAvailableEffects(successLoadedEffects);
    audioEffectService_.BuildAvailableAEConfig();

    // Initialize EffectChainManager in audio service through IPC
    SupportedEffectConfig supportedEffectConfig;
    audioEffectService_.GetSupportedEffectConfig(supportedEffectConfig);
    EffectChainManagerParam effectChainManagerParam;
    EffectChainManagerParam enhanceChainManagerParam;
    audioEffectService_.ConstructEffectChainManagerParam(effectChainManagerParam);
    audioEffectService_.ConstructEnhanceChainManagerParam(enhanceChainManagerParam);

    bool ret = AudioServerProxy::GetInstance().CreateEffectChainManagerProxy(supportedEffectConfig.effectChains,
        effectChainManagerParam, enhanceChainManagerParam);
    CHECK_AND_RETURN_LOG(ret, "EffectChainManager create failed");

    audioEffectService_.SetEffectChainManagerAvailable();
    AudioSpatializationService::GetAudioSpatializationService().Init(supportedEffectConfig.effectChains);
    AudioCollaborativeService::GetAudioCollaborativeService().Init(supportedEffectConfig.effectChains);
}

int32_t AudioPolicyService::SetAvailableDeviceChangeCallback(const int32_t clientId, const AudioDeviceUsage usage,
    const sptr<IRemoteObject> &object, bool hasBTPermission)
{
    sptr<IStandardAudioPolicyManagerListener> callback = iface_cast<IStandardAudioPolicyManagerListener>(object);

    if (callback != nullptr) {
        auto cb = std::make_shared<AudioPolicyManagerListenerCallback>(callback);
        CHECK_AND_RETURN_RET_LOG(cb != nullptr, SUCCESS, "AudioPolicyManagerListenerCallback create failed");
        cb->hasBTPermission_ = hasBTPermission;

        if (audioPolicyServerHandler_ != nullptr) {
            audioPolicyServerHandler_->AddAvailableDeviceChangeMap(clientId, usage, cb);
        }
    }

    return SUCCESS;
}

int32_t AudioPolicyService::SetQueryClientTypeCallback(const sptr<IRemoteObject> &object)
{
#ifdef FEATURE_APPGALLERY
    sptr<IStandardAudioPolicyManagerListener> callback = iface_cast<IStandardAudioPolicyManagerListener>(object);

    if (callback != nullptr) {
        ClientTypeManager::GetInstance()->SetQueryClientTypeCallback(callback);
    } else {
        AUDIO_ERR_LOG("Client type callback is null");
    }
#endif
    return SUCCESS;
}

int32_t AudioPolicyService::SetQueryDeviceVolumeBehaviorCallback(const sptr<IRemoteObject> &object)
{
    return audioPolicyManager_.SetQueryDeviceVolumeBehaviorCallback(object);
}

static void UpdateCapturerInfoWhenNoPermission(const shared_ptr<AudioCapturerChangeInfo> &audioCapturerChangeInfos,
    bool hasSystemPermission)
{
    if (!hasSystemPermission) {
        audioCapturerChangeInfos->clientUID = 0;
        audioCapturerChangeInfos->capturerState = CAPTURER_INVALID;
    }
}

int32_t AudioPolicyService::GetCurrentCapturerChangeInfos(vector<shared_ptr<AudioCapturerChangeInfo>>
    &audioCapturerChangeInfos, bool hasBTPermission, bool hasSystemPermission)
{
    int status = streamCollector_.GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    CHECK_AND_RETURN_RET_LOG(status == SUCCESS, status,
        "AudioPolicyServer:: Get capturer change info failed");

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> inputDevices =
        audioConnectedDevice_.GetDevicesInner(INPUT_DEVICES_FLAG);
#ifdef INPUT_FEATURE_INDEPENDENT_MODE
    size_t capturerInfosSize = audioCapturerChangeInfos.size();
    for (size_t i = 0; i < capturerInfosSize; i++) {
        CHECK_AND_CONTINUE(audioCapturerChangeInfos[i] != nullptr);
        CHECK_AND_RETURN_RET_LOG(pipeManager_ != nullptr, status, "pipeManager_ is nullptr");
        std::shared_ptr<AudioStreamDescriptor> streamDesc =
            pipeManager_->GetStreamDescById(audioCapturerChangeInfos[i]->sessionId);
        CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, status, "streamDesc is nullptr");
        DeviceType activeDeviceType = audioActiveDevice_.GetCurrentInputDeviceType(streamDesc->GetRealUid());
        DeviceRole activeDeviceRole = INPUT_DEVICE;
        for (std::shared_ptr<AudioDeviceDescriptor> desc : inputDevices) {
            if ((desc->deviceType_ == activeDeviceType) && (desc->deviceRole_ == activeDeviceRole)) {
                UpdateCapturerInfoWhenNoPermission(audioCapturerChangeInfos[i], hasSystemPermission);
                CHECK_AND_CONTINUE(audioRouterCenter_.IsConfigRouterStrategy(
                    audioCapturerChangeInfos[i]->capturerInfo.sourceType));
                audioDeviceCommon_.UpdateDeviceInfo(audioCapturerChangeInfos[i]->inputDeviceInfo, desc,
                    hasBTPermission, hasSystemPermission);
                break;
            }
        }
    }
#else
    DeviceType activeDeviceType = audioActiveDevice_.GetCurrentInputDeviceType();
    DeviceRole activeDeviceRole = INPUT_DEVICE;
    for (std::shared_ptr<AudioDeviceDescriptor> desc : inputDevices) {
        if ((desc->deviceType_ == activeDeviceType) && (desc->deviceRole_ == activeDeviceRole)) {
            size_t capturerInfosSize = audioCapturerChangeInfos.size();
            for (size_t i = 0; i < capturerInfosSize; i++) {
                UpdateCapturerInfoWhenNoPermission(audioCapturerChangeInfos[i], hasSystemPermission);
                audioDeviceCommon_.UpdateDeviceInfo(audioCapturerChangeInfos[i]->inputDeviceInfo, desc,
                    hasBTPermission, hasSystemPermission);
            }
            break;
        }
    }
#endif
    return status;
}

void AudioPolicyService::UpdateDescWhenNoBTPermission(vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    for (std::shared_ptr<AudioDeviceDescriptor> &desc : deviceDescs) {
        CHECK_AND_CONTINUE_LOG(desc != nullptr, "Device is nullptr, continue");
        if (AudioPolicyUtils::GetInstance().IsWirelessDevice(desc->deviceType_)) {
            std::shared_ptr<AudioDeviceDescriptor> copyDesc = std::make_shared<AudioDeviceDescriptor>(desc);
            copyDesc->deviceName_ = "";
            copyDesc->macAddress_ = "";
            desc = copyDesc;
        }
    }
}

void AudioPolicyService::SetDefaultDeviceLoadFlag(bool isLoad)
{
    audioVolumeManager_.SetDefaultDeviceLoadFlag(isLoad);
}

void AudioPolicyService::RegiestPolicy()
{
    AUDIO_INFO_LOG("Start");
    sptr<PolicyProviderWrapper> wrapper = new(std::nothrow) PolicyProviderWrapper(this);
    CHECK_AND_RETURN_LOG(wrapper != nullptr, "Get null PolicyProviderWrapper");
    sptr<IRemoteObject> object = wrapper->AsObject();
    CHECK_AND_RETURN_LOG(object != nullptr, "RegiestPolicy AsObject is nullptr");

    int32_t ret = AudioServerProxy::GetInstance().RegiestPolicyProviderProxy(object);
    AUDIO_DEBUG_LOG("result:%{public}d", ret);
}

/*
 * lockFlag is use to determinewhether GetPreferredOutputDeviceDescriptor or
*  GetPreferredOutputDeviceDescInner is invoked.
 * If deviceStatusUpdateSharedMutex_ write lock is not invoked at the outer layer, lockFlag can be set to true.
 * When deviceStatusUpdateSharedMutex_ write lock has been invoked, lockFlag must be set to false.
 */

int32_t AudioPolicyService::GetProcessDeviceInfo(const AudioProcessConfig &config, bool lockFlag,
    AudioDeviceDescriptor &deviceInfo)
{
    AUDIO_INFO_LOG("%{public}s", ProcessConfig::DumpProcessConfig(config).c_str());
    auto coreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERROR, "AudioCoreService is null");
    auto eventEntry = coreService->GetEventEntry();
    CHECK_AND_RETURN_RET_LOG(eventEntry != nullptr, ERROR, "EventEntry is null");

    AudioSamplingRate samplingRate = config.streamInfo.samplingRate;
    AudioStreamInfo targetStreamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    int32_t result = SUCCESS;
    if (config.audioMode == AUDIO_MODE_PLAYBACK) {
        result = HandlePlaybackModeDeviceInfo(config, lockFlag, deviceInfo, samplingRate);
    } else {
        result = HandleCaptureModeDeviceInfo(config, lockFlag, deviceInfo, samplingRate);
    }
    if (result != SUCCESS) {
        return result;
    }

    deviceInfo.audioStreamInfo_ = {targetStreamInfo};
    deviceInfo.deviceName_ = "mmap_device";
    audioRouteMap_.GetNetworkIDInFastRouterMap(config.appInfo.appUid, deviceInfo.deviceRole_, deviceInfo.networkId_);
    deviceInfo.a2dpOffloadFlag_ = GetA2dpOffloadFlag();
    return SUCCESS;
}

bool AudioPolicyService::IsVoipStreamUsage(StreamUsage streamUsage)
{
    return streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
           streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION;
}

void AudioPolicyService::SetCommonDeviceInfo(AudioDeviceDescriptor &deviceInfo,
    const AudioDeviceDescriptor &sourceDesc, DeviceRole role)
{
    deviceInfo.deviceId_ = sourceDesc.deviceId_;
    deviceInfo.networkId_ = sourceDesc.networkId_;
    deviceInfo.deviceType_ = sourceDesc.deviceType_;
    deviceInfo.deviceRole_ = role;
}

int32_t AudioPolicyService::HandlePlaybackModeDeviceInfo(const AudioProcessConfig &config, bool lockFlag,
    AudioDeviceDescriptor &deviceInfo, AudioSamplingRate samplingRate)
{
    AudioRendererInfo rendererInfo = config.rendererInfo;
    int32_t uid = GetRealUid(config);
    auto coreService = AudioCoreService::GetCoreService();
    auto eventEntry = coreService->GetEventEntry();
    CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERROR, "AudioCoreService is null");
    CHECK_AND_RETURN_RET_LOG(eventEntry != nullptr, ERROR, "EventEntry is null");

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> preferredDeviceList =
        (lockFlag ? eventEntry->GetPreferredOutputDeviceDescriptors(rendererInfo, uid, LOCAL_NETWORK_ID,
            config.originalSessionId) :
                coreService->GetPreferredOutputDeviceDescInner(rendererInfo, LOCAL_NETWORK_ID, uid,
                    config.originalSessionId));

    CHECK_AND_RETURN_RET_LOG(!preferredDeviceList.empty(), ERROR, "Preferred device list is empty");
    CHECK_AND_RETURN_RET_LOG(preferredDeviceList[0] != nullptr, ERROR, "Preferred device is nullptr");
    if (IsVoipStreamUsage(config.rendererInfo.streamUsage)) {
        return HandleVoipPlaybackDeviceInfo(config, deviceInfo, samplingRate, preferredDeviceList);
    }

    AudioDeviceDescriptor curOutputDeviceDesc = preferredDeviceList[0];
    SetCommonDeviceInfo(deviceInfo, curOutputDeviceDesc, OUTPUT_DEVICE);

    AudioStreamInfo targetStreamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    targetStreamInfo.format = curOutputDeviceDesc.deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP ?
        coreService->GetFastFormat() : SAMPLE_S16LE;
    deviceInfo.audioStreamInfo_ = {targetStreamInfo};

    CHECK_AND_RETURN_RET_LOG(IsDevicePlaybackSupported(config, deviceInfo), ERROR, "device not support playback");
    return SUCCESS;
}

int32_t AudioPolicyService::HandleCaptureModeDeviceInfo(const AudioProcessConfig &config, bool lockFlag,
    AudioDeviceDescriptor &deviceInfo, AudioSamplingRate samplingRate)
{
    AudioCapturerInfo capturerInfo = config.capturerInfo;
    int32_t uid = GetRealUid(config);
    auto coreService = AudioCoreService::GetCoreService();
    auto eventEntry = coreService->GetEventEntry();
    CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERROR, "AudioCoreService is null");
    CHECK_AND_RETURN_RET_LOG(eventEntry != nullptr, ERROR, "EventEntry is null");

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> preferredDeviceList =
        (lockFlag ? eventEntry->GetPreferredInputDeviceDescriptors(capturerInfo, uid, LOCAL_NETWORK_ID,
            config.originalSessionId) :
                coreService->GetPreferredInputDeviceDescInner(capturerInfo, LOCAL_NETWORK_ID, uid,
                    config.originalSessionId));

    CHECK_AND_RETURN_RET_LOG(!preferredDeviceList.empty(), ERROR, "Preferred device list is empty");
    CHECK_AND_RETURN_RET_LOG(preferredDeviceList[0] != nullptr, ERROR, "Preferred device is nullptr");

    if (config.capturerInfo.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
        return HandleVoipCaptureDeviceInfo(config, deviceInfo, samplingRate, preferredDeviceList);
    }

    deviceInfo.deviceId_ = preferredDeviceList[0]->deviceId_;
    deviceInfo.networkId_ = preferredDeviceList[0]->networkId_;
    deviceInfo.deviceRole_ = INPUT_DEVICE;
    deviceInfo.deviceType_ = preferredDeviceList[0]->deviceType_;

    return SUCCESS;
}

int32_t AudioPolicyService::HandleVoipPlaybackDeviceInfo(const AudioProcessConfig &config,
    AudioDeviceDescriptor &deviceInfo, AudioSamplingRate samplingRate,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &preferredDeviceList)
{
    int32_t type = audioDeviceCommon_.GetPreferredOutputStreamTypeInner(config.rendererInfo.streamUsage,
        preferredDeviceList[0]->deviceType_, config.rendererInfo.originalFlag, preferredDeviceList[0]->networkId_,
        samplingRate);

    deviceInfo.deviceRole_ = OUTPUT_DEVICE;
    return GetVoipDeviceInfo(config, deviceInfo, type, preferredDeviceList);
}

int32_t AudioPolicyService::HandleVoipCaptureDeviceInfo(const AudioProcessConfig &config,
    AudioDeviceDescriptor &deviceInfo, AudioSamplingRate samplingRate,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &preferredDeviceList)
{
    int32_t type = audioDeviceCommon_.GetPreferredInputStreamTypeInner(config.capturerInfo.sourceType,
        preferredDeviceList[0]->deviceType_, config.capturerInfo.originalFlag, preferredDeviceList[0]->networkId_,
        samplingRate);

    deviceInfo.deviceRole_ = INPUT_DEVICE;
    return GetVoipDeviceInfo(config, deviceInfo, type, preferredDeviceList);
}

int32_t AudioPolicyService::GetRealUid(const AudioProcessConfig &config)
{
    if (config.callerUid == MEDIA_SERVICE_UID) {
        return config.appInfo.appUid;
    }
    return config.callerUid;
}

int32_t AudioPolicyService::GetRemoteDeviceName(const std::string &deviceId, std::string &deviceName)
{
    return AudioPolicyUtils::GetInstance().GetRemoteDeviceName(deviceId, deviceName);
}

int32_t AudioPolicyService::GetVoipDeviceInfo(const AudioProcessConfig &config, AudioDeviceDescriptor &deviceInfo,
    int32_t type, const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &preferredDeviceList)
{
    if (type == AUDIO_FLAG_NORMAL) {
        AUDIO_WARNING_LOG("Current device %{public}d not support", type);
        return ERROR;
    }
    deviceInfo.deviceId_ = preferredDeviceList[0]->deviceId_;
    deviceInfo.networkId_ = preferredDeviceList[0]->networkId_;
    deviceInfo.deviceType_ = preferredDeviceList[0]->deviceType_;
    deviceInfo.deviceName_ = preferredDeviceList[0]->deviceName_;
    if (config.streamInfo.samplingRate <= SAMPLE_RATE_16000) {
        deviceInfo.audioStreamInfo_ = {{SAMPLE_RATE_16000, ENCODING_PCM, SAMPLE_S16LE, CH_LAYOUT_STEREO}};
    } else {
        deviceInfo.audioStreamInfo_ = {{SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, CH_LAYOUT_STEREO}};
    }
    if (type == AUDIO_FLAG_VOIP_DIRECT) {
        AUDIO_INFO_LOG("Direct VoIP stream, deviceInfo has been updated: deviceInfo.deviceType %{public}d",
            deviceInfo.deviceType_);
        return SUCCESS;
    }
    audioRouteMap_.GetNetworkIDInFastRouterMap(config.appInfo.appUid, deviceInfo.deviceRole_, deviceInfo.networkId_);
    deviceInfo.a2dpOffloadFlag_ = GetA2dpOffloadFlag();
    deviceInfo.isLowLatencyDevice_ = true;
    return SUCCESS;
}

int32_t AudioPolicyService::InitSharedVolume(std::shared_ptr<AudioSharedMemory> &buffer)
{
    return audioVolumeManager_.InitSharedVolume(buffer);
}

void AudioPolicyService::SetParameterCallback(const std::shared_ptr<AudioParameterCallback>& callback)
{
    AUDIO_INFO_LOG("Start");
    sptr<AudioManagerListenerStubImpl> parameterChangeCbStub = new(std::nothrow) AudioManagerListenerStubImpl();
    CHECK_AND_RETURN_LOG(parameterChangeCbStub != nullptr,
        "parameterChangeCbStub null");
    parameterChangeCbStub->SetParameterCallback(callback);

    sptr<IRemoteObject> object = parameterChangeCbStub->AsObject();
    if (object == nullptr) {
        AUDIO_ERR_LOG("listenerStub object is nullptr");
        return;
    }
    AUDIO_DEBUG_LOG("done");
    AudioServerProxy::GetInstance().SetParameterCallbackProxy(object);
}

bool AudioPolicyService::IsSupportInnerCaptureOffload()
{
    return AudioCoreService::GetCoreService()->IsSupportInnerCaptureOffload();
}

bool AudioPolicyService::IsEnhancedRoutingSupported()
{
    return AudioCoreService::GetCoreService()->GetEnhancedRoutingSupported();
}

int32_t AudioPolicyService::GetMaxRendererInstances()
{
    return AudioCoreService::GetCoreService()->GetMaxRendererInstances();
}


void AudioPolicyService::RegisterBluetoothListener()
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
#endif
}

void AudioPolicyService::UnregisterBluetoothListener()
{
#ifdef BLUETOOTH_ENABLE
    AUDIO_INFO_LOG("Enter");
    Bluetooth::UnregisterDeviceObserver();
    Bluetooth::AudioA2dpManager::UnregisterBluetoothA2dpListener();
    Bluetooth::AudioHfpManager::UnregisterBluetoothScoListener();
    isBtListenerRegistered = false;
#endif
}

void AudioPolicyService::SubscribeAccessibilityConfigObserver()
{
#ifdef ACCESSIBILITY_ENABLE
    RegisterAccessibilityMonitorHelper();
    AUDIO_INFO_LOG("Subscribe accessibility config observer successfully");
#endif
}

int32_t AudioPolicyService::QueryEffectManagerSceneMode(SupportedEffectConfig& supportedEffectConfig)
{
    int32_t ret = audioEffectService_.QueryEffectManagerSceneMode(supportedEffectConfig);
    return ret;
}

void AudioPolicyService::RegisterDataObserver()
{
    std::string devicesName = "";
    int32_t ret = AudioPolicyUtils::GetInstance().GetDeviceNameFromDataShareHelper(devicesName);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "RegisterDataObserver get devicesName failed");
    audioConnectedDevice_.SetDisplayName(devicesName, true);
    audioConnectedDevice_.RegisterNameMonitorHelper();
    audioPolicyManager_.RegisterDoNotDisturbStatus();
    audioPolicyManager_.RegisterDoNotDisturbStatusWhiteList();
    audioPolicyManager_.RegisterAppIndividualVolumeEnableObserver();
}

int32_t AudioPolicyService::GetHardwareOutputSamplingRate(const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    int32_t rate = 48000;

    CHECK_AND_RETURN_RET_LOG(desc != nullptr, -1, "desc is null!");

    bool ret = audioConnectedDevice_.IsConnectedOutputDevice(desc);
    CHECK_AND_RETURN_RET(ret, -1);

    std::unordered_map<ClassType, std::list<AudioModuleInfo>> deviceClassInfo = {};
    AudioCoreService::GetCoreService()->GetDeviceClassInfo(deviceClassInfo);

    DeviceType clientDevType = desc->deviceType_;
    for (const auto &device : deviceClassInfo) {
        auto moduleInfoList = device.second;
        for (auto &moduleInfo : moduleInfoList) {
            auto serverDevType = AudioPolicyUtils::GetInstance().GetDeviceType(moduleInfo.name);
            if (clientDevType == serverDevType) {
                rate = atoi(moduleInfo.rate.c_str());
                return rate;
            }
        }
    }

    return rate;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::DeviceFilterByUsageInner(AudioDeviceUsage usage,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>>& descs)
{
    std::vector<shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;

    unordered_map<AudioDevicePrivacyType, list<DevicePrivacyInfo>> devicePrivacyMaps =
        audioDeviceManager_.GetDevicePrivacyMaps();
    for (const auto &dev : descs) {
        CHECK_AND_CONTINUE_LOG(dev != nullptr, "dev is nullptr");
        if (dev->IsRemoteDevice()) {
            audioDeviceDescriptors.push_back(make_shared<AudioDeviceDescriptor>(dev));
            continue;
        }
        for (const auto &devicePrivacy : devicePrivacyMaps) {
            list<DevicePrivacyInfo> deviceInfos = devicePrivacy.second;
            audioDeviceManager_.GetAvailableDevicesWithUsage(usage, deviceInfos, dev, audioDeviceDescriptors);
        }
    }
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptors;
    for (const auto &dec : audioDeviceDescriptors) {
        std::shared_ptr<AudioDeviceDescriptor> tempDec = std::make_shared<AudioDeviceDescriptor>(*dec);
        deviceDescriptors.push_back(move(tempDec));
    }
    return deviceDescriptors;
}

int32_t AudioPolicyService::OffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp)
{
    Trace trace("AudioPolicyService::OffloadGetRenderPosition");
#ifdef BLUETOOTH_ENABLE
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    AUDIO_DEBUG_LOG("GetRenderPosition, deviceType: %{public}d, a2dpOffloadFlag: %{public}d",
        GetA2dpOffloadFlag(), curOutputDeviceType);
    int32_t ret = SUCCESS;
    if (curOutputDeviceType == DEVICE_TYPE_BLUETOOTH_A2DP &&
        audioActiveDevice_.GetCurrentOutputDeviceNetworkId() == LOCAL_NETWORK_ID &&
        GetA2dpOffloadFlag() == A2DP_OFFLOAD) {
        ret = Bluetooth::AudioA2dpManager::GetRenderPosition(delayValue, sendDataSize, timeStamp);
    } else {
        delayValue = 0;
        sendDataSize = 0;
        timeStamp = 0;
    }
    return ret;
#else
    return SUCCESS;
#endif
}

int32_t AudioPolicyService::NearlinkGetRenderPosition(uint32_t &delayValue)
{
    Trace trace("AudioPolicyService::NearlinkGetRenderPosition");
    auto descs = AudioRouterSelectStrategy::GetInstance().FindCurrentOutputDevice({DEVICE_TYPE_NEARLINK});
    AudioDeviceDescriptor curOutputDevice = !descs.empty() && *descs.begin() ?
        *(*descs.begin()) : AudioDeviceDescriptor();
    AUDIO_DEBUG_LOG("GetRenderPosition, deviceType: %{public}d", curOutputDevice.deviceType_);
    int32_t ret = SUCCESS;
    delayValue = 0;

    CHECK_AND_RETURN_RET(curOutputDevice.deviceType_ == DEVICE_TYPE_NEARLINK, ret);

    ret = sleAudioDeviceManager_.GetRenderPosition(curOutputDevice.macAddress_, delayValue);
    return ret;
}

int32_t AudioPolicyService::GetAndSaveClientType(uint32_t uid, const std::string &bundleName)
{
#ifdef FEATURE_APPGALLERY
    ClientTypeManager::GetInstance()->GetAndSaveClientType(uid, bundleName);
#endif
    return SUCCESS;
}

void AudioPolicyService::OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command)
{
    audioDeviceLock_.OnDeviceInfoUpdated(desc, command);
}

void AudioPolicyService::NotifyAccountsChanged(const int &id)
{
    audioPolicyManager_.NotifyAccountsChanged(id);
    RegisterDataObserver();
    SubscribeAccessibilityConfigObserver();
    AudioServerProxy::GetInstance().NotifyAccountsChanged();
}

void AudioPolicyService::MuteMediaWhenAccountsChanged()
{
    audioPolicyManager_.MuteMediaWhenAccountsChanged();
}

void AudioPolicyService::LoadHdiEffectModel()
{
    return AudioServerProxy::GetInstance().LoadHdiEffectModelProxy();
}

int32_t AudioPolicyService::GetSupportedAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    AudioEffectPropertyArray effectPropertyArray = {};
    GetSupportedEffectProperty(effectPropertyArray);
    for (auto &effectItem : effectPropertyArray.property) {
        effectItem.flag = RENDER_EFFECT_FLAG;
        propertyArray.property.push_back(effectItem);
    }
    AudioEffectPropertyArray enhancePropertyArray = {};
    GetSupportedEnhanceProperty(enhancePropertyArray);
    for (auto &enhanceItem : enhancePropertyArray.property) {
        enhanceItem.flag = CAPTURE_EFFECT_FLAG;
        propertyArray.property.push_back(enhanceItem);
    }
    return AUDIO_OK;
}

void AudioPolicyService::GetSupportedEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    std::set<std::pair<std::string, std::string>> mergedSet = {};
    audioEffectService_.AddSupportedAudioEffectPropertyByDevice(DEVICE_TYPE_INVALID, mergedSet);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descriptor = GetDevices(OUTPUT_DEVICES_FLAG);
    for (auto &item : descriptor) {
        audioEffectService_.AddSupportedAudioEffectPropertyByDevice(item->getType(), mergedSet);
    }
    propertyArray.property.reserve(mergedSet.size());
    std::transform(mergedSet.begin(), mergedSet.end(), std::back_inserter(propertyArray.property),
        [](const std::pair<std::string, std::string>& p) {
            return AudioEffectProperty{p.first, p.second, RENDER_EFFECT_FLAG};
        });
    return;
}

void AudioPolicyService::GetSupportedEnhanceProperty(AudioEffectPropertyArray &propertyArray)
{
    std::set<std::pair<std::string, std::string>> mergedSet = {};
    audioEffectService_.AddSupportedAudioEnhancePropertyByDevice(DEVICE_TYPE_INVALID, mergedSet);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descriptor = GetDevices(INPUT_DEVICES_FLAG);
    for (auto &item : descriptor) {
        audioEffectService_.AddSupportedAudioEnhancePropertyByDevice(item->getType(), mergedSet);
    }
    propertyArray.property.reserve(mergedSet.size());
    std::transform(mergedSet.begin(), mergedSet.end(), std::back_inserter(propertyArray.property),
        [](const std::pair<std::string, std::string>& p) {
            return AudioEffectProperty{p.first, p.second, CAPTURE_EFFECT_FLAG};
        });
    return;
}

int32_t AudioPolicyService::CheckSupportedAudioEffectProperty(const AudioEffectPropertyArray &propertyArray,
    const EffectFlag& flag)
{
    AudioEffectPropertyArray supportPropertyArray;
    if (flag == CAPTURE_EFFECT_FLAG) {
        GetSupportedEnhanceProperty(supportPropertyArray);
    } else {
        GetSupportedEffectProperty(supportPropertyArray);
    }
    for (auto &item : propertyArray.property) {
        auto oIter = std::find(supportPropertyArray.property.begin(), supportPropertyArray.property.end(), item);
        CHECK_AND_RETURN_RET_LOG(oIter != supportPropertyArray.property.end(),
            ERR_INVALID_PARAM, "set property not valid name:%{public}s,category:%{public}s,flag:%{public}d",
            item.name.c_str(), item.category.c_str(), item.flag);
    }
    return AUDIO_OK;
}

int32_t AudioPolicyService::SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray)
{
    int32_t ret = AUDIO_OK;
    AudioEffectPropertyArray effectPropertyArray = {};
    AudioEffectPropertyArray enhancePropertyArray = {};
    for (auto &item : propertyArray.property) {
        if (item.flag == CAPTURE_EFFECT_FLAG) {
            enhancePropertyArray.property.push_back(item);
        } else {
            effectPropertyArray.property.push_back(item);
        }
    }
    CHECK_AND_RETURN_RET_LOG(CheckSupportedAudioEffectProperty(enhancePropertyArray, CAPTURE_EFFECT_FLAG) == AUDIO_OK,
        ERR_INVALID_PARAM, "check Audio Enhance property failed");
    CHECK_AND_RETURN_RET_LOG(CheckSupportedAudioEffectProperty(effectPropertyArray, RENDER_EFFECT_FLAG) == AUDIO_OK,
        ERR_INVALID_PARAM, "check Audio Effect property failed");
    if (enhancePropertyArray.property.size() > 0) {
        AudioEffectPropertyArray oldPropertyArray = {};
        ret = GetAudioEnhanceProperty(oldPropertyArray);
        CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "get audio enhance property fail");
        ret = AudioServerProxy::GetInstance().SetAudioEffectPropertyProxy(enhancePropertyArray,
            audioActiveDevice_.GetCurrentInputDeviceType());
        CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "set audio enhance property fail");
        AudioCoreService::GetCoreService()->GetEventEntry()->ReloadSourceForEffect(
            oldPropertyArray, enhancePropertyArray);
    }
    if (effectPropertyArray.property.size() > 0) {
        ret = AudioServerProxy::GetInstance().SetAudioEffectPropertyProxy(effectPropertyArray);
        CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "set audio effect property fail");
    }
    return ret;
}

int32_t AudioPolicyService::GetAudioEnhanceProperty(AudioEffectPropertyArray &propertyArray)
{
    int32_t ret = AUDIO_OK;
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        return audioPolicyManager_.GetAudioEffectProperty(propertyArray);
    } else {
        ret = AudioServerProxy::GetInstance().GetAudioEffectPropertyProxy(propertyArray);
    }
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "get audio enhance property fail");
    auto oIter = propertyArray.property.begin();
    while (oIter != propertyArray.property.end()) {
        if (oIter->flag == RENDER_EFFECT_FLAG) {
            oIter = propertyArray.property.erase(oIter);
        } else {
            oIter++;
        }
    }
    return ret;
}

int32_t AudioPolicyService::GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
{
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        return audioPolicyManager_.GetAudioEffectProperty(propertyArray);
    } else {
        return AudioServerProxy::GetInstance().GetAudioEffectPropertyProxy(propertyArray);
    }
}

BluetoothOffloadState AudioPolicyService::GetA2dpOffloadFlag()
{
    if (audioA2dpOffloadManager_) {
        return audioA2dpOffloadManager_->GetA2dpOffloadFlag();
    }
    return NO_A2DP_DEVICE;
}

int32_t AudioPolicyService::SetSleAudioOperationCallback(const sptr<IRemoteObject> &object)
{
    sptr<IStandardSleAudioOperationCallback> sleAudioOperationCallback =
        iface_cast<IStandardSleAudioOperationCallback>(object);
    CHECK_AND_RETURN_RET_LOG(sleAudioOperationCallback != nullptr, ERROR,
        "sleAudioOperationCallback_ is nullptr");

    sleAudioDeviceManager_.SetSleAudioOperationCallback(sleAudioOperationCallback);

    return SUCCESS;
}

int32_t AudioPolicyService::NotifyCapturerRemoved(uint64_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(audioPolicyServerHandler_ != nullptr, ERROR, "audioPolicyServerHandler_ is nullptr");
    audioPolicyServerHandler_->SendCapturerRemovedEvent(sessionId, false);
    return SUCCESS;
}

void AudioPolicyService::UpdateSpatializationSupported(const std::string macAddress, const bool support)
{
    audioDeviceLock_.UpdateSpatializationSupported(macAddress, support);
}
#ifdef HAS_FEATURE_INNERCAPTURER
int32_t AudioPolicyService::LoadModernInnerCapSink(int32_t innerCapId)
{
    AUDIO_INFO_LOG("Start");
    AudioModuleInfo moduleInfo = {};
    moduleInfo.lib = "libmodule-inner-capturer-sink.z.so";
    std::string name = INNER_CAPTURER_SINK;
    moduleInfo.name = name + std::to_string(innerCapId);

    moduleInfo.format = "s16le";
    moduleInfo.channels = "2"; // 2 channel
    moduleInfo.rate = "48000";
    moduleInfo.bufferSize = "3840"; // 20ms

    audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
    return SUCCESS;
}

int32_t AudioPolicyService::LoadModernOffloadCapSource()
{
    if (audioIOHandleMap_.CheckIOHandleExist(OFFLOAD_CAPTURER_SOURCE)) {
        AUDIO_INFO_LOG("offload capture has loaded!");
        return SUCCESS;
    }
    AUDIO_INFO_LOG("Start load offload capture:");
    AudioModuleInfo moduleInfo = {};
    moduleInfo.name = OFFLOAD_CAPTURER_SOURCE;
    moduleInfo.lib = "libmodule-hdi-source.z.so";
    moduleInfo.format = "s16le";
    moduleInfo.ecFormat = "s16le";
    moduleInfo.ecType = "1";
    moduleInfo.channels = "2"; // 2 channel
    moduleInfo.ecChannels = "2"; // 2 channel
    moduleInfo.rate = "48000";
    moduleInfo.ecSamplingRate = "48000";
    moduleInfo.bufferSize = "3840"; // 20ms

    moduleInfo.className = "offload";
    moduleInfo.offloadEnable = "true";
    moduleInfo.role = "source";
    moduleInfo.sourceType = std::to_string(SourceType::SOURCE_TYPE_OFFLOAD_CAPTURE);

    int32_t result = audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
    return result;
}

int32_t AudioPolicyService::UnloadModernInnerCapSink(int32_t innerCapId)
{
    AUDIO_INFO_LOG("Start");
    std::string name = INNER_CAPTURER_SINK;
    name += std::to_string(innerCapId);

    audioIOHandleMap_.ClosePortAndEraseIOHandle(name);
    return SUCCESS;
}

int32_t AudioPolicyService::UnloadModernOffloadCapSource()
{
    AUDIO_INFO_LOG("Start unload offload capture:");
    audioIOHandleMap_.ClosePortAndEraseIOHandle(OFFLOAD_CAPTURER_SOURCE);
    return SUCCESS;
}
#endif

bool AudioPolicyService::IsDevicePlaybackSupported(const AudioProcessConfig &config,
    const AudioDeviceDescriptor &deviceInfo)
{
    if (audioPolicyServerHandler_ && config.streamInfo.encoding == ENCODING_EAC3 &&
        deviceInfo.deviceType_ != DEVICE_TYPE_HDMI && deviceInfo.deviceType_ != DEVICE_TYPE_LINE_DIGITAL) {
        audioPolicyServerHandler_->SendFormatUnsupportedErrorEvent(ERROR_UNSUPPORTED_FORMAT);
        return false;
    }
    return true;
}

int32_t AudioPolicyService::ClearAudioFocusBySessionID(const int32_t &sessionID)
{
    return AudioZoneService::GetInstance().ClearAudioFocusBySessionID(sessionID);
}

bool AudioPolicyService::CheckVoipAnrOn(std::vector<AudioEffectProperty> &property)
{
    bool ret = false;
    for (const auto &item : property) {
        if (item.name != "voip_up") continue;
        if (ANRCategories.find(item.category) != ANRCategories.end()) {
            ret = true;
            break;
        }
    }
    return ret;
}
 
bool AudioPolicyService::IsIntelligentNoiseReductionEnabledForCurrentDevice(SourceType sourceType)
{
    if (sourceType != SOURCE_TYPE_LIVE && sourceType != SOURCE_TYPE_VOICE_COMMUNICATION) {
        AUDIO_INFO_LOG("SourceType %{public}d IsIntelligentNoiseReductionEnabledForCurrentDevice 0", sourceType);
        return false;
    }
 
    bool ret = false;
    if (sourceType == SOURCE_TYPE_LIVE) {
        std::string paramKey = LIVE_EFFECT_KEY;
        std::string paramValue = "";
        AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
        CHECK_AND_RETURN_RET_LOG(settingProvider.CheckOsAccountReady(), false, "os account not ready");
        settingProvider.GetStringValue(paramKey, paramValue, LIVE_EFFECT_TABLE_TYPE);
        ret = (paramValue == LIVE_EFFECT_ON);
        AUDIO_INFO_LOG("SourceType %{public}d IsIntelligentNoiseReductionEnabledForCurrentDevice %{public}d",
            sourceType, ret);
        return ret;
    }

    if (ecEnableState_) { // is configed according to the product
        AudioEffectPropertyArray propertyArray = {};
        int32_t getPropRet = GetAudioEnhanceProperty(propertyArray);
        CHECK_AND_RETURN_RET_LOG(getPropRet == SUCCESS, false, "get audio enhance property failed, return false");
        ret = CheckVoipAnrOn(propertyArray.property);
    } else {
        AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
        CHECK_AND_RETURN_RET_LOG(settingProvider.CheckOsAccountReady(), false, "os account not ready");
        int32_t flagValue = INVALID_VALUE;
        settingProvider.GetIntValue(AINR_FLAG, flagValue, AUDIO_SETTING_TABLE_TYPE);
        ret = (flagValue == 1);
    }
    AUDIO_INFO_LOG("SourceType %{public}d IsIntelligentNoiseReductionEnabledForCurrentDevice %{public}d",
        sourceType, ret);
    return ret;
}

AudioScene AudioPolicyService::GetAudioSceneFromAllZones()
{
    return AudioZoneService::GetInstance().GetAudioSceneFromAllZones();
}
} // namespace AudioStandard
} // namespace OHOS
