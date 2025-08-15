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

#include "audio_affinity_manager.h"
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

AudioPolicyService::~AudioPolicyService()
{
    AUDIO_INFO_LOG("~AudioPolicyService()");
    Deinit();
}

bool AudioPolicyService::LoadAudioPolicyConfig()
{
    bool ret = audioConfigManager_.Init();
    if (!ret) {
        AudioPolicyUtils::GetInstance().WriteServiceStartupError("Audio Policy Config Load Configuration failed");
        audioCapturerSession_.SetConfigParserFlag();
    }
    CHECK_AND_RETURN_RET_LOG(ret, false, "Audio Policy Config Load Configuration failed");
    audioCapturerSession_.SetConfigParserFlag();
    isFastControlled_ = getFastControlParam();
    if (!ret) {
        AudioPolicyUtils::GetInstance().WriteServiceStartupError("Audio Config Parse failed");
    }
    CHECK_AND_RETURN_RET_LOG(ret, false, "Audio Config Parse failed");
    SetDefaultAdapterEnable(audioConfigManager_.GetDefaultAdapterEnable());
    return ret;
}

bool AudioPolicyService::Init(void)
{
    audioPolicyManager_.Init();
    audioEffectService_.EffectServiceInit();
    audioDeviceManager_.ParseDeviceXml();
    audioAffinityManager_.ParseAffinityXml();
#ifdef AUDIO_WIRED_DETECT
    audioPnpServer_.init();
#endif
    audioGlobalConfigManager_.ParseGlobalConfigXml();

    bool ret = LoadAudioPolicyConfig();
    if (!ret) {
        return ret;
    }

#ifdef FEATURE_DTMF_TONE
    ret = audioToneManager_.LoadToneDtmfConfig();
    CHECK_AND_RETURN_RET_LOG(ret, false, "Audio Tone Load Configuration failed");
#endif

    CreateRecoveryThread();
    std::string versionType = OHOS::system::GetParameter("const.logsystem.versiontype", "commercial");
    AudioDump::GetInstance().SetVersionType(versionType);

    int32_t ecEnableState = system::GetBoolParameter("const.multimedia.audio.fwk_ec.enable", 0);
    int32_t micRefEnableState = system::GetBoolParameter("const.multimedia.audio.fwk_pnr.enable", 0);

    audioEcManager_.Init(ecEnableState, micRefEnableState);
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
    audioPnpServer_.StopPnpServer();
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
    audioCapturerSession_.DeInit();
    return;
}

void SafeVolumeEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    if (eventReceiver_ == nullptr) {
        AUDIO_ERR_LOG("eventReceiver_ is nullptr.");
        return;
    }
    eventReceiver_(eventData);
}

void AudioPolicyService::SubscribeSafeVolumeEvent()
{
    AUDIO_INFO_LOG("enter.");
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(AUDIO_RESTORE_VOLUME_EVENT);
    matchingSkills.AddEvent(AUDIO_INCREASE_VOLUME_EVENT);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto commonSubscribePtr = std::make_shared<SafeVolumeEventSubscriber>(subscribeInfo,
        std::bind(&AudioPolicyService::OnReceiveEvent, this, std::placeholders::_1));
    if (commonSubscribePtr == nullptr) {
        AUDIO_ERR_LOG("commonSubscribePtr is nullptr");
        return;
    }
    EventFwk::CommonEventManager::SubscribeCommonEvent(commonSubscribePtr);
}

void AudioPolicyService::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    audioVolumeManager_.OnReceiveEvent(eventData);
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
    if (status > 0) {
        streamCollector_.UpdateCapturerInfoMuteStatus(uid, setMute);
    }
    return status;
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

AudioStreamInfo AudioPolicyService::GetFastStreamInfo()
{
    AudioStreamInfo streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    streamInfo.format = audioConfigManager_.GetFastFormat();

    // change to SAMPLE_S16LE for bluetooth
    if (streamInfo.format == SAMPLE_S32LE) {
        DeviceType deviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
        streamInfo.format = deviceType == DEVICE_TYPE_BLUETOOTH_A2DP ? SAMPLE_S16LE : SAMPLE_S32LE;
    }
    AUDIO_INFO_LOG("Fast format is %{public}d", streamInfo.format);
    return streamInfo;
}

bool AudioPolicyService::IsAbsVolumeSupported()
{
    return audioPolicyManager_.IsAbsVolumeScene();
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::GetDevices(DeviceFlag deviceFlag)
{
    return audioDeviceLock_.GetDevices(deviceFlag);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::GetPreferredInputDeviceDescriptors(
    AudioCapturerInfo &captureInfo, std::string networkId)
{
    return audioDeviceLock_.GetPreferredInputDeviceDescriptors(captureInfo, networkId);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::GetPreferredOutputDeviceDescInner(
    AudioRendererInfo &rendererInfo, std::string networkId)
{
    return audioDeviceCommon_.GetPreferredOutputDeviceDescInner(rendererInfo, networkId);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::GetPreferredInputDeviceDescInner(
    AudioCapturerInfo &captureInfo, std::string networkId)
{
    return audioDeviceCommon_.GetPreferredInputDeviceDescInner(captureInfo, networkId);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::GetOutputDevice(
    sptr<AudioRendererFilter> audioRendererFilter)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceList = {};
    if (audioRendererFilter->uid != -1) {
        shared_ptr<AudioDeviceDescriptor> preferredDesc =
            audioAffinityManager_.GetRendererDevice(audioRendererFilter->uid);
        std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*preferredDesc);
        deviceList.push_back(devDesc);
    }
    return deviceList;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyService::GetInputDevice(
    sptr<AudioCapturerFilter> audioCapturerFilter)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceList = {};
    if (audioCapturerFilter->uid != -1) {
        shared_ptr<AudioDeviceDescriptor> preferredDesc =
            audioAffinityManager_.GetCapturerDevice(audioCapturerFilter->uid);
        std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*preferredDesc);
        deviceList.push_back(devDesc);
    }
    return deviceList;
}

shared_ptr<AudioDeviceDescriptor> AudioPolicyService::GetActiveOutputDeviceDescriptor()
{
    return make_shared<AudioDeviceDescriptor>(audioActiveDevice_.GetCurrentOutputDevice());
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
        audioA2dpOffloadManager_->UpdateA2dpOffloadFlagForAllStream(sessionIDToSpatializationEnableMap, spatialDevice);
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

void AudioPolicyService::OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress)
{
    audioDeviceLock_.OnForcedDeviceSelected(devType, macAddress);
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

    return status;
}

void AudioPolicyService::UpdateDescWhenNoBTPermission(vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    for (std::shared_ptr<AudioDeviceDescriptor> &desc : deviceDescs) {
        CHECK_AND_CONTINUE_LOG(desc != nullptr, "Device is nullptr, continue");
        if ((desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) || (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO)) {
            std::shared_ptr<AudioDeviceDescriptor> copyDesc = std::make_shared<AudioDeviceDescriptor>(desc);
            copyDesc->deviceName_ = "";
            copyDesc->macAddress_ = "";
            desc = copyDesc;
        }
    }
}

bool AudioPolicyService::getFastControlParam()
{
    int32_t fastControlFlag = 1; // default 1, set isFastControlled_ true
    GetSysPara("persist.multimedia.audioflag.fastcontrolled", fastControlFlag);
    if (fastControlFlag == 0) {
        isFastControlled_ = false;
    }
    return isFastControlled_;
}

int32_t AudioPolicyService::GetPreferredOutputStreamType(AudioRendererInfo &rendererInfo,
    const std::string &bundleName)
{
    // Use GetPreferredOutputDeviceDescriptors instead of currentActiveDevice, if prefer != current, recreate stream
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> preferredDeviceList =
        audioDeviceLock_.GetPreferredOutputDeviceDescriptors(rendererInfo, LOCAL_NETWORK_ID);
    if (preferredDeviceList.size() == 0) {
        return AUDIO_FLAG_NORMAL;
    }

    int32_t flag = audioDeviceCommon_.GetPreferredOutputStreamTypeInner(rendererInfo.streamUsage,
        preferredDeviceList[0]->deviceType_, rendererInfo.rendererFlags, preferredDeviceList[0]->networkId_,
        rendererInfo.samplingRate);
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
    AudioSamplingRate samplingRate = config.streamInfo.samplingRate;
    AudioStreamInfo targetStreamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO};
    if (config.audioMode == AUDIO_MODE_PLAYBACK) {
        if (config.rendererInfo.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
            config.rendererInfo.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION) {
            AudioRendererInfo rendererInfo = config.rendererInfo;
            std::vector<std::shared_ptr<AudioDeviceDescriptor>> preferredDeviceList =
                (lockFlag ? audioDeviceLock_.GetPreferredOutputDeviceDescriptors(rendererInfo, LOCAL_NETWORK_ID)
                          : audioDeviceCommon_.GetPreferredOutputDeviceDescInner(rendererInfo, LOCAL_NETWORK_ID));
            int32_t type = audioDeviceCommon_.GetPreferredOutputStreamTypeInner(rendererInfo.streamUsage,
                preferredDeviceList[0]->deviceType_, rendererInfo.originalFlag, preferredDeviceList[0]->networkId_,
                samplingRate);
            deviceInfo.deviceRole_ = OUTPUT_DEVICE;
            return GetVoipDeviceInfo(config, deviceInfo, type, preferredDeviceList);
        }
        AudioDeviceDescriptor curOutputDeviceDesc = audioActiveDevice_.GetCurrentOutputDevice();
        deviceInfo.deviceId_ = curOutputDeviceDesc.deviceId_;
        deviceInfo.networkId_ = curOutputDeviceDesc.networkId_;
        deviceInfo.deviceType_ = curOutputDeviceDesc.deviceType_;
        deviceInfo.deviceRole_ = OUTPUT_DEVICE;
        targetStreamInfo.format = curOutputDeviceDesc.deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP ?
            audioConfigManager_.GetFastFormat() : SAMPLE_S16LE;
        CHECK_AND_RETURN_RET_LOG(IsDevicePlaybackSupported(config, deviceInfo), ERROR, "device not support playback");
    } else {
        if (config.capturerInfo.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
            AudioCapturerInfo capturerInfo = config.capturerInfo;
            std::vector<std::shared_ptr<AudioDeviceDescriptor>> preferredDeviceList =
                (lockFlag ? GetPreferredInputDeviceDescriptors(capturerInfo, LOCAL_NETWORK_ID)
                          : audioDeviceCommon_.GetPreferredInputDeviceDescInner(capturerInfo, LOCAL_NETWORK_ID));
            int32_t type = audioDeviceCommon_.GetPreferredInputStreamTypeInner(capturerInfo.sourceType,
                preferredDeviceList[0]->deviceType_, capturerInfo.originalFlag, preferredDeviceList[0]->networkId_,
                samplingRate);
            deviceInfo.deviceRole_ = INPUT_DEVICE;
            return GetVoipDeviceInfo(config, deviceInfo, type, preferredDeviceList);
        }
        deviceInfo.deviceId_ = audioActiveDevice_.GetCurrentInputDevice().deviceId_;
        deviceInfo.networkId_ = audioActiveDevice_.GetCurrentInputDevice().networkId_;
        deviceInfo.deviceRole_ = INPUT_DEVICE;
        deviceInfo.deviceType_ = audioActiveDevice_.GetCurrentInputDeviceType();
    }

    // todo
    // check process in routerMap, return target device for it
    // put the currentActiveDevice_ in deviceinfo, so it can create with current using device.
    // genarate the unique deviceid?
    deviceInfo.audioStreamInfo_ = { targetStreamInfo };
    deviceInfo.deviceName_ = "mmap_device";
    audioRouteMap_.GetNetworkIDInFastRouterMap(config.appInfo.appUid, deviceInfo.deviceRole_, deviceInfo.networkId_);
    deviceInfo.a2dpOffloadFlag_ = GetA2dpOffloadFlag();
    return SUCCESS;
}

int32_t AudioPolicyService::GetVoipDeviceInfo(const AudioProcessConfig &config, AudioDeviceDescriptor &deviceInfo,
    int32_t type, std::vector<std::shared_ptr<AudioDeviceDescriptor>> &preferredDeviceList)
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

int32_t AudioPolicyService::DynamicUnloadModule(const AudioPipeType pipeType)
{
    switch (pipeType) {
        case PIPE_TYPE_OFFLOAD:
            audioOffloadStream_.DynamicUnloadOffloadModule();
            break;
        case PIPE_TYPE_MULTICHANNEL:
            return audioOffloadStream_.UnloadMchModule();
        default:
            AUDIO_WARNING_LOG("not supported for pipe type %{public}d", pipeType);
            break;
    }
    return SUCCESS;
}

int32_t AudioPolicyService::GetMaxRendererInstances()
{
    return audioConfigManager_.GetMaxRendererInstances();
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
}

int32_t AudioPolicyService::GetHardwareOutputSamplingRate(const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    int32_t rate = 48000;

    CHECK_AND_RETURN_RET_LOG(desc != nullptr, -1, "desc is null!");

    bool ret = audioConnectedDevice_.IsConnectedOutputDevice(desc);
    CHECK_AND_RETURN_RET(ret, -1);

    std::unordered_map<ClassType, std::list<AudioModuleInfo>> deviceClassInfo = {};
    audioConfigManager_.GetDeviceClassInfo(deviceClassInfo);
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

int32_t AudioPolicyService::OffloadStartPlaying(const std::vector<int32_t> &sessionIds)
{
#ifdef BLUETOOTH_ENABLE
    if (audioA2dpOffloadManager_) {
        return audioA2dpOffloadManager_->OffloadStartPlaying(sessionIds);
    }
    AUDIO_WARNING_LOG("Null audioA2dpOffloadManager");
#endif
    return SUCCESS;
}

int32_t AudioPolicyService::OffloadStopPlaying(const std::vector<int32_t> &sessionIds)
{
#ifdef BLUETOOTH_ENABLE
    if (audioA2dpOffloadManager_) {
        return audioA2dpOffloadManager_->OffloadStopPlaying(sessionIds);
    }
    AUDIO_WARNING_LOG("Null audioA2dpOffloadManager");
#endif
    return SUCCESS;
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
    AudioDeviceDescriptor curOutputDevice = audioActiveDevice_.GetCurrentOutputDevice();
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

void AudioPolicyService::LoadHdiEffectModel()
{
    return AudioServerProxy::GetInstance().LoadHdiEffectModelProxy();
}

int32_t AudioPolicyService::GetSupportedAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray)
{
    AudioEffectPropertyArrayV3 effectPropertyArray = {};
    GetSupportedEffectProperty(effectPropertyArray);
    for (auto &effectItem : effectPropertyArray.property) {
        effectItem.flag = RENDER_EFFECT_FLAG;
        propertyArray.property.push_back(effectItem);
    }
    AudioEffectPropertyArrayV3 enhancePropertyArray = {};
    GetSupportedEnhanceProperty(enhancePropertyArray);
    for (auto &enhanceItem : enhancePropertyArray.property) {
        enhanceItem.flag = CAPTURE_EFFECT_FLAG;
        propertyArray.property.push_back(enhanceItem);
    }
    return AUDIO_OK;
}

void AudioPolicyService::GetSupportedEffectProperty(AudioEffectPropertyArrayV3 &propertyArray)
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
            return AudioEffectPropertyV3{p.first, p.second, RENDER_EFFECT_FLAG};
        });
    return;
}

void AudioPolicyService::GetSupportedEnhanceProperty(AudioEffectPropertyArrayV3 &propertyArray)
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
            return AudioEffectPropertyV3{p.first, p.second, CAPTURE_EFFECT_FLAG};
        });
    return;
}

int32_t AudioPolicyService::CheckSupportedAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray,
    const EffectFlag& flag)
{
    AudioEffectPropertyArrayV3 supportPropertyArray;
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

int32_t AudioPolicyService::SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray)
{
    int32_t ret = AUDIO_OK;
    AudioEffectPropertyArrayV3 effectPropertyArray = {};
    AudioEffectPropertyArrayV3 enhancePropertyArray = {};
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
        AudioEffectPropertyArrayV3 oldPropertyArray = {};
        ret = GetAudioEnhanceProperty(oldPropertyArray);
        CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "get audio enhance property fail");
        ret = AudioServerProxy::GetInstance().SetAudioEffectPropertyProxy(enhancePropertyArray,
            audioActiveDevice_.GetCurrentInputDeviceType());
        CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "set audio enhance property fail");
        audioCapturerSession_.ReloadSourceForEffect(oldPropertyArray, enhancePropertyArray);
    }
    if (effectPropertyArray.property.size() > 0) {
        ret = AudioServerProxy::GetInstance().SetAudioEffectPropertyProxy(effectPropertyArray);
        CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "set audio effect property fail");
    }
    return ret;
}

int32_t AudioPolicyService::GetAudioEnhanceProperty(AudioEffectPropertyArrayV3 &propertyArray)
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

int32_t AudioPolicyService::GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray)
{
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        return audioPolicyManager_.GetAudioEffectProperty(propertyArray);
    } else {
        return AudioServerProxy::GetInstance().GetAudioEffectPropertyProxy(propertyArray);
    }
}

int32_t AudioPolicyService::GetSupportedAudioEffectProperty(AudioEffectPropertyArray &propertyArray)
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
            return AudioEffectProperty{p.first, p.second};
        });
    return AUDIO_OK;
}

int32_t AudioPolicyService::GetSupportedAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray)
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
            return AudioEnhanceProperty{p.first, p.second};
        });
    return AUDIO_OK;
}

int32_t AudioPolicyService::SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray)
{
    AudioEffectPropertyArray supportPropertyArray;
    std::vector<AudioEffectProperty>::iterator oIter;
    (void)GetSupportedAudioEffectProperty(supportPropertyArray);
    for (auto &item : propertyArray.property) {
        oIter = std::find(supportPropertyArray.property.begin(), supportPropertyArray.property.end(), item);
        CHECK_AND_RETURN_RET_LOG(oIter != supportPropertyArray.property.end(),
            ERR_INVALID_PARAM, "set audio effect property not valid %{public}s:%{public}s",
            item.effectClass.c_str(), item.effectProp.c_str());
    }
    return AudioServerProxy::GetInstance().SetAudioEffectPropertyProxy(propertyArray);
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

int32_t AudioPolicyService::SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray)
{
    AudioEnhancePropertyArray supportPropertyArray;
    std::vector<AudioEnhanceProperty>::iterator oIter;
    (void)GetSupportedAudioEnhanceProperty(supportPropertyArray);
    for (auto &item : propertyArray.property) {
        oIter = std::find(supportPropertyArray.property.begin(), supportPropertyArray.property.end(), item);
        CHECK_AND_RETURN_RET_LOG(oIter != supportPropertyArray.property.end(),
            ERR_INVALID_PARAM, "set audio enhance property not valid %{public}s:%{public}s",
            item.enhanceClass.c_str(), item.enhanceProp.c_str());
    }
    AudioEnhancePropertyArray oldPropertyArray = {};
    int32_t ret = AudioServerProxy::GetInstance().GetAudioEnhancePropertyProxy(oldPropertyArray);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "get audio enhance property fail");

    ret = AudioServerProxy::GetInstance().SetAudioEnhancePropertyProxy(propertyArray,
        audioActiveDevice_.GetCurrentInputDeviceType());
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, ret, "set audio enhance property fail");

    audioCapturerSession_.ReloadSourceForEffect(oldPropertyArray, propertyArray);
    return ret;
}

int32_t AudioPolicyService::GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray)
{
    return AudioServerProxy::GetInstance().GetAudioEnhancePropertyProxy(propertyArray);
}

BluetoothOffloadState AudioPolicyService::GetA2dpOffloadFlag()
{
    if (audioA2dpOffloadManager_) {
        return audioA2dpOffloadManager_->GetA2dpOffloadFlag();
    }
    return NO_A2DP_DEVICE;
}

void AudioPolicyService::SetDefaultAdapterEnable(bool isEnable)
{
    return AudioServerProxy::GetInstance().SetDefaultAdapterEnableProxy(isEnable);
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

int32_t AudioPolicyService::UnloadModernInnerCapSink(int32_t innerCapId)
{
    AUDIO_INFO_LOG("Start");
    std::string name = INNER_CAPTURER_SINK;
    name += std::to_string(innerCapId);

    audioIOHandleMap_.ClosePortAndEraseIOHandle(name);
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

int32_t AudioPolicyService::CaptureConcurrentCheck(const uint32_t &sessionID)
{
    return AudioCoreService::GetCoreService()->CaptureConcurrentCheck(sessionID);
}
} // namespace AudioStandard
} // namespace OHOS
