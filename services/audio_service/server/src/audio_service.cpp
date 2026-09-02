/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioService"
#endif

#include "audio_service.h"
#include "audio_stream_common.h"

#include <unistd.h>
#include <algorithm>
#include <thread>

#include "ipc_skeleton.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "audio_utils.h"
#include "i_hpae_manager.h"
#include "policy_handler.h"
#include "core_service_handler.h"
#include "ipc_stream_in_server.h"
#include "stream_dfx_manager.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "source/i_audio_capture_source.h"
#include "audio_volume.h"
#include "audio_performance_monitor.h"
#include "privacy_kit.h"
#include "media_monitor_manager.h"
#ifdef HAS_FEATURE_INNERCAPTURER
#include "playback_capturer_manager.h"
#endif
#include "audio_resource_service.h"
#include "stream_dfx_manager.h"
#include "audio_privacy_manager_in_server.h"

namespace OHOS {
namespace AudioStandard {

static const uint32_t UPDATE_MUTE_CONTROL_SET_TIME_OUT = 15; // 15s
static const uint32_t BLOCK_HIBERNATE_CALLBACK_IN_MS = 5000; // 5s
static const uint32_t RECHECK_SINK_STATE_IN_US = 300000; // 300ms
static const int32_t MEDIA_SERVICE_UID = 1013;
static const int32_t INVALID_APP_UID = -1;
static const int32_t INVALID_APP_CREATED_AUDIO_STREAM_NUM = 0;
static const int32_t UID_AUDIO = 1041;
namespace {
static inline const std::unordered_set<SourceType> specialSourceTypeSet_ = {
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VIRTUAL_CAPTURE,
    SOURCE_TYPE_REMOTE_CAST
};
static inline const std::unordered_set<AudioStreamType> workgroupSupportStreamTypeSet_ {
    STREAM_MUSIC,
    STREAM_MOVIE,
    STREAM_SPEECH,
    STREAM_NAVIGATION,
    STREAM_VOICE_COMMUNICATION,
};
const size_t MAX_FG_LIST_SIZE = 10;
}

AudioService *AudioService::GetInstance()
{
    static AudioService AudioService;

    return &AudioService;
}

AudioService::AudioService()
{
    AUDIO_INFO_LOG("AudioService()");
}

AudioService::~AudioService()
{
    AUDIO_INFO_LOG("~AudioService()");
}

int32_t AudioService::ReportWakeupEvent(const AudioWakeupTrackInfo &audioWakeupTrackInfo)
{
    CHECK_AND_RETURN_RET_LOG(wakeupEventCallback_ != nullptr, ERR_OPERATION_FAILED,
        "Wakeup event callback is not registered");
    return wakeupEventCallback_(audioWakeupTrackInfo);
}

void AudioService::RegisterWakeupEventCallback(WakeupEventCallback callback)
{
    wakeupEventCallback_ = std::move(callback);
}

#ifdef SUPPORT_LOW_LATENCY
void AudioService::SetEndpointMuteForSwitchDevice(const std::string &deviceClass, bool mute)
{
    AUDIO_INFO_LOG("SetEndpointMuteForSwitchDevice deviceClass:%{public}s mute:%{public}d",
        deviceClass.c_str(), mute);
    
    // 调用 HPAE 层方法
    HPAE::IHpaeManager::GetHpaeManager().SetSinkMuteForSwitchDevice(deviceClass, mute);
}
#endif

void AudioService::DisableLoopback()
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_LOG(deviceManager != nullptr, "local device manager is nullptr");
    deviceManager->SetAudioParameter("primary", AudioParamKey::NONE, "", "Karaoke_enable=disable");
}

sptr<IpcStreamInServer> AudioService::GetIpcStream(const AudioProcessConfig &config, int32_t &ret)
{
    Trace trace("AudioService::GetIpcStream");
#ifdef HAS_FEATURE_INNERCAPTURER
    if (!isRegisterCapturerFilterListened_) {
        AUDIO_INFO_LOG("isRegisterCapturerFilterListened_ is false");
        PlaybackCapturerManager::GetInstance()->RegisterCapturerFilterListener(this);
        isRegisterCapturerFilterListened_ = true;
    }
#endif
    // in plan: GetDeviceInfoForProcess(config) and stream limit check
    // in plan: call GetProcessDeviceInfo to load inner-cap-sink
    sptr<IpcStreamInServer> ipcStreamInServer = IpcStreamInServer::Create(config, ret);
    if (ipcStreamInServer == nullptr && config.audioMode == AUDIO_MODE_PLAYBACK) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(config.appInfo.appUid),
            PLAY_CREATE_OPERATION_FAILED, "IpcStreamInServer create failed", false);
        AUDIO_ERR_LOG("IpcStreamInServer::Create failed, ret: %{public}d", ret);
    }

    // in plan: Put playback into list, check if EnableInnerCap is need.
    if (ipcStreamInServer != nullptr && config.audioMode == AUDIO_MODE_PLAYBACK) {
        uint32_t sessionId = 0;
        std::shared_ptr<RendererInServer> renderer = ipcStreamInServer->GetRenderer();
        if (renderer != nullptr && renderer->GetSessionId(sessionId) == SUCCESS) {
            CHECK_AND_RETURN_RET_LOG(InsertRenderer(sessionId, renderer) == SUCCESS, nullptr, "InsertRenderer fail.");
#ifdef HAS_FEATURE_INNERCAPTURER
            CheckInnerCapForRenderer(sessionId, renderer);
#endif
            CheckRenderSessionMuteState(sessionId, renderer);
            CoreServiceHandler::GetInstance().UpdateSessionOperation(sessionId, SESSION_OPERATION_CREATE);
        }
    }
    if (ipcStreamInServer != nullptr && config.audioMode == AUDIO_MODE_RECORD) {
        uint32_t sessionId = 0;
        std::shared_ptr<CapturerInServer> capturer = ipcStreamInServer->GetCapturer();
        if (capturer != nullptr && capturer->GetSessionId(sessionId) == SUCCESS) {
            CHECK_AND_RETURN_RET_LOG(InsertCapturer(sessionId, capturer) == SUCCESS, nullptr, "InsertCapturer fail.");
            CheckCaptureSessionMuteState(sessionId, capturer);
            CoreServiceHandler::GetInstance().UpdateSessionOperation(sessionId, SESSION_OPERATION_CREATE);
        }
    }

    return ipcStreamInServer;
}

void AudioService::UpdateMuteControlSet(uint32_t sessionId, bool muteFlag)
{
    int32_t xCollieFlag = (1 | 2); // flag 1 generate log file,flag 2 die when timeout, restart server
    AudioXCollie audioXCollie("AudioService::UpdateMuteControlSet", UPDATE_MUTE_CONTROL_SET_TIME_OUT,
        nullptr, nullptr, xCollieFlag);
    if (sessionId < MIN_STREAMID || sessionId > MAX_STREAMID) {
        AUDIO_WARNING_LOG("Invalid sessionid %{public}u", sessionId);
        return;
    }
    std::lock_guard<std::mutex> lock(mutedSessionsMutex_);
    if (muteFlag) {
        mutedSessions_.insert(sessionId);
        return;
    }
    if (mutedSessions_.find(sessionId) != mutedSessions_.end()) {
        mutedSessions_.erase(sessionId);
    } else {
        AUDIO_WARNING_LOG("Session id %{public}u not in the set", sessionId);
    }
}

void AudioService::RemoveIdFromMuteControlSet(uint32_t sessionId)
{
    std::lock_guard<std::mutex> mutedSessionsLock(mutedSessionsMutex_);
    if (mutedSessions_.find(sessionId) != mutedSessions_.end()) {
        mutedSessions_.erase(sessionId);
    } else {
        AUDIO_WARNING_LOG("Session id %{public}u not in the set", sessionId);
    }
}

void AudioService::CheckRenderSessionMuteState(uint32_t sessionId, std::shared_ptr<RendererInServer> renderer)
{
    std::unique_lock<std::mutex> mutedSessionsLock(mutedSessionsMutex_);
    if (mutedSessions_.find(sessionId) != mutedSessions_.end() || IsMuteSwitchStream(sessionId)) {
        mutedSessionsLock.unlock();
        AUDIO_INFO_LOG("Session %{public}u is in control", sessionId);
        renderer->SetNonInterruptMute(true);
    }
}

void AudioService::CheckCaptureSessionMuteState(uint32_t sessionId, std::shared_ptr<CapturerInServer> capturer)
{
    std::unique_lock<std::mutex> mutedSessionsLock(mutedSessionsMutex_);
    if (mutedSessions_.find(sessionId) != mutedSessions_.end() || IsMuteSwitchStream(sessionId)) {
        mutedSessionsLock.unlock();
        AUDIO_INFO_LOG("Session %{public}u is in control", sessionId);
        capturer->SetNonInterruptMute(true);
    }
}

bool AudioService::IsMuteSwitchStream(uint32_t sessionId)
{
    std::lock_guard<std::mutex> muteSwitchStreamLock(muteSwitchStreamSetMutex_);
    if (muteSwitchStreams_.count(sessionId)) {
        AUDIO_INFO_LOG("find session %{public}u in muteSwitchStreams_", sessionId);
        muteSwitchStreams_.erase(sessionId);
        return true;
    }
    return false;
}

int32_t AudioService::InsertRenderer(uint32_t sessionId, std::shared_ptr<RendererInServer> renderer)
{
    std::unique_lock<std::mutex> lock(rendererMapMutex_);
    AUDIO_INFO_LOG("Insert renderer:%{public}u into map", sessionId);
    CHECK_AND_RETURN_RET_LOG(!allRendererMap_.count(sessionId), ERROR,
        "%{public}u already insert.", sessionId);
    allRendererMap_[sessionId] = renderer;
    return SUCCESS;
}

void AudioService::SaveForegroundList(std::vector<std::string> list)
{
    std::lock_guard<std::mutex> lock(foregroundSetMutex_);
    if (list.size() > MAX_FG_LIST_SIZE) {
        AUDIO_ERR_LOG("invalid list size %{public}zu", list.size());
        return;
    }

    foregroundSet_.clear();
    foregroundUidSet_.clear();
    for (auto &item : list) {
        AUDIO_WARNING_LOG("Add for hap: %{public}s", item.c_str());
        foregroundSet_.insert(item);
    }
}

bool AudioService::MatchForegroundList(const std::string &bundleName, uint32_t uid)
{
    std::lock_guard<std::mutex> lock(foregroundSetMutex_);
    if (foregroundSet_.find(bundleName) != foregroundSet_.end()) {
        AUDIO_WARNING_LOG("find hap %{public}s in list!", bundleName.c_str());
        if (uid != 0) {
            foregroundUidSet_.insert(uid);
        }
        return true;
    }
    return false;
}

bool AudioService::InForegroundList(uint32_t uid)
{
    std::lock_guard<std::mutex> lock(foregroundSetMutex_);
    if (foregroundUidSet_.find(uid) != foregroundUidSet_.end()) {
        AUDIO_INFO_LOG("find hap %{public}d in list!", uid);
        return true;
    }
    return false;
}


void AudioService::SaveRenderWhitelist(std::vector<std::string> list)
{
    std::lock_guard<std::mutex> lock(renderWhitelistMutex_);

    renderWhitelist_.clear();
    for (auto &item : list) {
        AUDIO_INFO_LOG("Add for hap: %{public}s", item.c_str());
        renderWhitelist_.insert(item);
    }
}

bool AudioService::InRenderWhitelist(const std::string bundleName)
{
    std::lock_guard<std::mutex> lock(renderWhitelistMutex_);
    CHECK_AND_RETURN_RET_LOG(renderWhitelist_.find(bundleName) != renderWhitelist_.end(),
        false, "hap %{public}s not in list", bundleName.c_str());

    AUDIO_INFO_LOG("find hap %{public}s in list!", bundleName.c_str());
    return true;
}

bool AudioService::UpdateForegroundState(uint32_t appTokenId, bool isActive)
{
    // UpdateForegroundState 200001000 to active
    std::string str = "UpdateForegroundState " + std::to_string(appTokenId) + (isActive ? "to active" : "to deactive");
    Trace trace(str);
    WatchTimeout guard(str);
    int32_t res = OHOS::Security::AccessToken::PrivacyKit::SetHapWithFGReminder(appTokenId, isActive);
    AUDIO_INFO_LOG("res is %{public}d for %{public}s", res, str.c_str());
    return res;
}

void AudioService::DumpForegroundList(std::string &dumpString)
{
    std::lock_guard<std::mutex> lock(foregroundSetMutex_);
    std::stringstream temp;
    temp << "DumpForegroundList:\n";
    int32_t index = 0;
    for (auto item : foregroundSet_) {
        temp << "    " <<  std::to_string(index++) << ": " <<  item << "\n";
    }
    dumpString = temp.str();
}

int32_t AudioService::GetStandbyStatus(uint32_t sessionId, bool &isStandby, int64_t &enterStandbyTime)
{
    // for normal renderer.
    std::unique_lock<std::mutex> lockRender(rendererMapMutex_);
    if (allRendererMap_.count(sessionId)) {
        std::shared_ptr<RendererInServer> render = allRendererMap_[sessionId].lock();
        if (render == nullptr) {
            return ERR_INVALID_PARAM;
        }
        return render->GetStandbyStatus(isStandby, enterStandbyTime);
    }
    lockRender.unlock();

    // not found target sessionId
    return ERR_INVALID_PARAM;
}

void AudioService::RemoveRenderer(uint32_t sessionId, bool isSwitchStream)
{
    std::unique_lock<std::mutex> lock(rendererMapMutex_);
    AUDIO_INFO_LOG("Renderer:%{public}u will be removed.", sessionId);
    if (!allRendererMap_.count(sessionId)) {
        AUDIO_WARNING_LOG("Renderer in not in map!");
        return;
    }
    allRendererMap_.erase(sessionId);
    if (!isSwitchStream) {
        RemoveIdFromMuteControlSet(sessionId);
    }
    AudioPerformanceMonitor::GetInstance().DeleteSilenceMonitor(sessionId);
}

int32_t AudioService::InsertCapturer(uint32_t sessionId, std::shared_ptr<CapturerInServer> capturer)
{
    std::unique_lock<std::mutex> lock(capturerMapMutex_);
    AUDIO_INFO_LOG("Insert capturer:%{public}u into map", sessionId);
    CHECK_AND_RETURN_RET_LOG(!allCapturerMap_.count(sessionId), ERROR,
        "%{public}u already insert.", sessionId);
    allCapturerMap_[sessionId] = capturer;
    return SUCCESS;
}

void AudioService::RemoveCapturer(uint32_t sessionId, bool isSwitchStream)
{
    std::unique_lock<std::mutex> lock(capturerMapMutex_);
    AUDIO_INFO_LOG("Capturer: %{public}u will be removed.", sessionId);
    if (!allCapturerMap_.count(sessionId)) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "Capturer in not in map", false);
        AUDIO_WARNING_LOG("Capturer in not in map!");
        return;
    }
    allCapturerMap_.erase(sessionId);
    {
        std::unique_lock<std::mutex> muteStatelock(muteStateMapMutex_);
        muteStateCallbacks_.erase(sessionId);
    }

    if (!isSwitchStream) {
        RemoveIdFromMuteControlSet(sessionId);
    }
}

void AudioService::AddFilteredRender(int32_t innerCapId, std::shared_ptr<RendererInServer> renderer)
{
    if (!filteredRendererMap_.count(innerCapId)) {
        std::vector<std::weak_ptr<RendererInServer>> renders;
        filteredRendererMap_[innerCapId] = renders;
    }
    filteredRendererMap_[innerCapId].push_back(renderer);
}

#ifdef HAS_FEATURE_INNERCAPTURER
void AudioService::CheckInnerCapForRenderer(uint32_t sessionId, std::shared_ptr<RendererInServer> renderer)
{
    CHECK_AND_RETURN_LOG(renderer != nullptr, "renderer is null.");

    std::unique_lock<std::mutex> lock(rendererMapMutex_);

    // inner-cap not working
    {
        std::lock_guard<std::mutex> lock(workingConfigsMutex_);
        if (workingConfigs_.size() == 0) {
            return;
        }
    }
    // in plan: check if meet with the workingConfig_
    std::set<int32_t> captureIds;
    if (ShouldBeInnerCap(renderer->processConfig_, captureIds)) {
        for (auto innerCapId : captureIds) {
            AddFilteredRender(innerCapId, renderer);
            renderer->EnableInnerCap(innerCapId); // for debug
        }
    }
}

InnerCapFilterPolicy AudioService::GetInnerCapFilterPolicy(int32_t innerCapId)
{
    if (!workingConfigs_.count(innerCapId)) {
        AUDIO_ERR_LOG("error, invalid innerCapId");
        return POLICY_INVALID;
    }
    auto usagesSize = workingConfigs_[innerCapId].filterOptions.usages.size();
    auto pidsSize = workingConfigs_[innerCapId].filterOptions.pids.size();
    if (usagesSize == 0 && pidsSize == 0) {
        AUDIO_ERR_LOG("error, invalid usages and pids");
        return POLICY_INVALID;
    }
    if (usagesSize > 0 && pidsSize == 0) {
        AUDIO_INFO_LOG("usages only");
        return POLICY_USAGES_ONLY;
    }
    return POLICY_USAGES_AND_PIDS;
}

template<typename T>
bool isFilterMatched(const std::vector<T> &params, T param, FilterMode mode)
{
    bool isFound = std::count(params.begin(), params.end(), param) != 0;
    return (mode == FilterMode::INCLUDE && isFound) || (mode == FilterMode::EXCLUDE && !isFound);
}

bool AudioService::CanCapturePrivateStream(const AudioPlaybackCaptureConfig &config) const
{
    return config.filterOptions.capturePrivatePrivacyStreams;
}

bool AudioService::ShouldBeInnerCap(const AudioProcessConfig &rendererConfig, int32_t innerCapId)
{
    std::lock_guard<std::mutex> lock(workingConfigsMutex_);
    if (innerCapId == 0 || !workingConfigs_.count(innerCapId)) {
        return false;
    }
    const auto &config = workingConfigs_[innerCapId];
    bool canBeCaptured = rendererConfig.privacyType == AudioPrivacyType::PRIVACY_TYPE_PUBLIC ||
        (rendererConfig.privacyType == AudioPrivacyType::PRIVACY_TYPE_PRIVATE && CanCapturePrivateStream(config));
    if (!canBeCaptured) {
        AUDIO_INFO_LOG("%{public}d privacyType %{public}d can not be captured by innerCapId %{public}d",
            rendererConfig.appInfo.appPid, rendererConfig.privacyType, innerCapId);
        return false;
    }
    return CheckShouldCap(rendererConfig, innerCapId);
}

bool AudioService::ShouldBeInnerCap(const AudioProcessConfig &rendererConfig, std::set<int32_t> &beCapIds)
{
    bool ret = false;
    std::lock_guard<std::mutex> lock(workingConfigsMutex_);
    for (auto& filter : workingConfigs_) {
        bool canBeCaptured = rendererConfig.privacyType == AudioPrivacyType::PRIVACY_TYPE_PUBLIC ||
            (rendererConfig.privacyType == AudioPrivacyType::PRIVACY_TYPE_PRIVATE &&
            CanCapturePrivateStream(filter.second));
        if (!canBeCaptured) {
            continue;
        }
        if (CheckShouldCap(rendererConfig, filter.first)) {
            ret = true;
            beCapIds.insert(filter.first);
        }
    }
    return ret;
}

bool AudioService::CheckShouldCap(const AudioProcessConfig &rendererConfig, int32_t innerCapId)
{
    InnerCapFilterPolicy filterPolicy = GetInnerCapFilterPolicy(innerCapId);
    if (workingConfigs_[innerCapId].filterOptions.playbackCaptureUid != PLAYBACKCAPTURER_RECORD_ALL) {
        CHECK_AND_RETURN_RET_LOG(
            (workingConfigs_[innerCapId].filterOptions.playbackCaptureUid == rendererConfig.appInfo.appUid),
            false, "only %{public}d need be recorded, current %{public}d is invalid",
            workingConfigs_[innerCapId].filterOptions.playbackCaptureUid, rendererConfig.appInfo.appUid);
    }
    bool res = false;
    switch (filterPolicy) {
        case POLICY_INVALID:
            return false;
        case POLICY_USAGES_ONLY:
            res = isFilterMatched(workingConfigs_[innerCapId].filterOptions.usages,
                rendererConfig.rendererInfo.streamUsage, workingConfigs_[innerCapId].filterOptions.usageFilterMode);
            break;
        case POLICY_USAGES_AND_PIDS:
            res = isFilterMatched(workingConfigs_[innerCapId].filterOptions.usages,
                rendererConfig.rendererInfo.streamUsage,
                workingConfigs_[innerCapId].filterOptions.usageFilterMode) &&
                isFilterMatched(workingConfigs_[innerCapId].filterOptions.pids, rendererConfig.appInfo.appPid,
                workingConfigs_[innerCapId].filterOptions.pidFilterMode);
            break;
        default:
            break;
    }
    AUDIO_INFO_LOG("pid:%{public}d usage:%{public}d result:%{public}s capId:%{public}d",
        rendererConfig.appInfo.appPid, rendererConfig.rendererInfo.streamUsage, res ? "true" : "false", innerCapId);
    return res;
}
#endif

bool AudioService::ShouldBeDualTone(const AudioProcessConfig &config, const std::string &dupSinkName)
{
    // for dup device scene, use dmsdp speaker
    if (dupSinkName != "Speaker") {
        return true;
    }

    CHECK_AND_RETURN_RET_LOG(Util::IsRingerOrAlarmerStreamUsage(config.rendererInfo.streamUsage), false,
        "Wrong usage ,should not be dualtone");
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    bool ret = PolicyHandler::GetInstance().GetProcessDeviceInfo(config, false, deviceInfo);
    if (!ret) {
        AUDIO_WARNING_LOG("GetProcessDeviceInfo from audio policy server failed!");
        return false;
    }
    if (config.audioMode != AUDIO_MODE_PLAYBACK) {
        AUDIO_WARNING_LOG("No playback mode!");
        return false;
    }
    AUDIO_INFO_LOG("Get DeviceInfo from policy server success, deviceType: %{public}d, "
        "supportLowLatency: %{public}d", deviceInfo.deviceType_, deviceInfo.isLowLatencyDevice_);
    if (deviceInfo.deviceType_ == DEVICE_TYPE_WIRED_HEADSET || deviceInfo.deviceType_ == DEVICE_TYPE_WIRED_HEADPHONES ||
        deviceInfo.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP || deviceInfo.deviceType_ == DEVICE_TYPE_USB_HEADSET ||
        deviceInfo.deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET || deviceInfo.deviceType_ == DEVICE_TYPE_REMOTE_CAST ||
        (deviceInfo.deviceType_ == DEVICE_TYPE_SPEAKER && deviceInfo.networkId_ != std::string(LOCAL_NETWORK_ID)) ||
        deviceInfo.deviceType_ == DEVICE_TYPE_HEARING_AID) {
        switch (config.rendererInfo.streamUsage) {
            case STREAM_USAGE_ALARM:
            case STREAM_USAGE_VOICE_RINGTONE:
            case STREAM_USAGE_RINGTONE:
                AUDIO_WARNING_LOG("Should DualTone.");
                return true;
            default:
                return false;
        }
    }
    return false;
}

#ifdef HAS_FEATURE_INNERCAPTURER
int32_t AudioService::OnInitInnerCapList(int32_t innerCapId)
{
    AUDIO_INFO_LOG("workingInnerCapId_ is %{public}d", innerCapId);

    // strong ref to prevent destruct before unlock
    std::vector<std::shared_ptr<RendererInServer>> renderers;

    {
        std::unique_lock<std::mutex> lock(rendererMapMutex_);
        for (auto it = allRendererMap_.begin(); it != allRendererMap_.end(); it++) {
            std::shared_ptr<RendererInServer> renderer = it->second.lock();
            if (renderer == nullptr) {
                AUDIO_WARNING_LOG("Renderer is already released!");
                continue;
            }
            if (ShouldBeInnerCap(renderer->processConfig_, innerCapId)) {
                renderer->EnableInnerCap(innerCapId);
                AddFilteredRender(innerCapId, renderer);
            }
            renderers.push_back(std::move(renderer));
        }
    }

    return SUCCESS;
}

int32_t AudioService::OnUpdateInnerCapList(int32_t innerCapId)
{
    AUDIO_INFO_LOG("workingInnerCapId_ is %{public}d", innerCapId);

    std::unique_lock<std::mutex> lock(rendererMapMutex_);
    if (filteredRendererMap_.count(innerCapId)) {
        for (size_t i = 0; i < filteredRendererMap_[innerCapId].size(); i++) {
            std::shared_ptr<RendererInServer> renderer = filteredRendererMap_[innerCapId][i].lock();
            if (renderer == nullptr) {
                AUDIO_WARNING_LOG("Renderer is already released!");
                continue;
            }
            if (!ShouldBeInnerCap(renderer->processConfig_, innerCapId)) {
                renderer->DisableInnerCap(innerCapId);
            }
        }
        filteredRendererMap_.erase(innerCapId);
    }
    lock.unlock();
    // EnableInnerCap will be called twice as it's already in filteredRendererMap_.
    return OnInitInnerCapList(innerCapId);
}
#endif

int32_t AudioService::EnableDualStreamForNormalStream(const uint32_t sessionId, const std::string &dupSinkName)
{
    AUDIO_INFO_LOG("sessionId is %{public}d", sessionId);
    std::lock_guard<std::mutex> lock(rendererMapMutex_);

    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(allRendererMap_.count(sessionId) > 0, ERR_OPERATION_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(INVALID_APP_UID,
            PLAY_CREATE_INVALID_INDEX, "EnableDualStreamForNormalStream sessionId not in map", false),
        "sessionId %{public}u not in map", sessionId);
    auto weakRenderer = allRendererMap_[sessionId];
    auto renderer = weakRenderer.lock();

    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(renderer != nullptr, ERR_OPERATION_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(INVALID_APP_UID,
            PLAY_CREATE_NULL_POINTER, "EnableDualStreamForNormalStream renderer is null", false),
        "renderer is null");
    CHECK_AND_RETURN_RET(ShouldBeDualTone(renderer->processConfig_, dupSinkName), SUCCESS);

    renderer->EnableDualTone(dupSinkName);
    return SUCCESS;
}

int32_t AudioService::DisableDualStreamForNormalStream(const uint32_t sessionId)
{
    AUDIO_INFO_LOG("disable dual tone, sessionId is %{public}d", sessionId);
    std::lock_guard<std::mutex> lock(rendererMapMutex_);

    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(allRendererMap_.count(sessionId) > 0, ERR_OPERATION_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(INVALID_APP_UID,
            PLAY_CREATE_INVALID_INDEX, "DisableDualStreamForNormalStream sessionId not in map", false),
        "sessionId %{public}u not in map", sessionId);
    auto weakRenderer = allRendererMap_[sessionId];
    auto renderer = weakRenderer.lock();

    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(renderer != nullptr, ERR_OPERATION_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(INVALID_APP_UID,
            PLAY_CREATE_NULL_POINTER, "DisableDualStreamForNormalStream renderer is null", false),
        "renderer is null");
    renderer->DisableDualTone();
    return SUCCESS;
}

// Only one session is working at the same time.
int32_t AudioService::OnCapturerFilterChange(uint32_t sessionId, const AudioPlaybackCaptureConfig &newConfig,
    int32_t innerCapId)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    Trace trace("AudioService::OnCapturerFilterChange");
    // in plan:
    // step 1: if sessionId is not added before, add the sessionId and enbale the filter in allRendererMap_
    // step 2: if sessionId is already in using, this means the config is changed. Check the filtered renderer before,
    // call disable inner-cap for those not meet with the new config, than filter all allRendererMap_.
    bool isOldCap = false;
    {
        std::lock_guard<std::mutex> lock(workingConfigsMutex_);
        if (workingConfigs_.count(innerCapId)) {
            workingConfigs_[innerCapId] = newConfig;
            isOldCap = true;
        } else {
            workingConfigs_[innerCapId] = newConfig;
        }
    }
    if (isOldCap) {
        return OnUpdateInnerCapList(innerCapId);
    } else {
        return OnInitInnerCapList(innerCapId);
    }
    AUDIO_WARNING_LOG("%{public}u is working, comming %{public}u will not work!", innerCapId, sessionId);
    return ERR_OPERATION_FAILED;
#endif
    return SUCCESS;
}

int32_t AudioService::OnCapturerFilterRemove(uint32_t sessionId, int32_t innerCapId)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    {
        std::lock_guard<std::mutex> lock(workingConfigsMutex_);
        if (!workingConfigs_.count(innerCapId)) {
            AUDIO_WARNING_LOG("%{public}u is working, remove %{public}u will not work!", innerCapId, sessionId);
            return SUCCESS;
        }
        workingConfigs_.erase(innerCapId);
    }

    // strong ref to prevent destruct before unlock
    std::vector<std::shared_ptr<RendererInServer>> renderers;

    {
        std::lock_guard<std::mutex> lock(rendererMapMutex_);
        if (filteredRendererMap_.count(innerCapId)) {
            for (size_t i = 0; i < filteredRendererMap_[innerCapId].size(); i++) {
                std::shared_ptr<RendererInServer> renderer = filteredRendererMap_[innerCapId][i].lock();
                if (renderer == nullptr) {
                    AUDIO_WARNING_LOG("Find renderer is already released!");
                    continue;
                }
                renderer->DisableInnerCap(innerCapId);
                renderers.push_back(std::move(renderer));
        }
        AUDIO_INFO_LOG("Filter removed, clear %{public}zu filtered renderer.",
            filteredRendererMap_[innerCapId].size());
        filteredRendererMap_.erase(innerCapId);
        }
    }
#endif
    return SUCCESS;
}

bool AudioService::IsEndpointTypeVoip(const AudioProcessConfig &config, AudioDeviceDescriptor &deviceInfo)
{
    if (config.rendererInfo.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
        config.rendererInfo.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION) {
        return config.rendererInfo.originalFlag == AUDIO_FLAG_VOIP_FAST || deviceInfo.networkId_ != LOCAL_NETWORK_ID;
    }

    if (config.capturerInfo.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
        return config.capturerInfo.originalFlag == AUDIO_FLAG_VOIP_FAST || deviceInfo.networkId_ != LOCAL_NETWORK_ID;
    }
    return false;
}

bool AudioService::GetRemoteDeviceName(const std::string &deviceId, std::string &deviceName)
{
    return PolicyHandler::GetInstance().GetRemoteDeviceName(deviceId, deviceName);
}

#ifdef SUPPORT_LOW_LATENCY
AudioDeviceDescriptor AudioService::GetDeviceInfoForProcess(const AudioProcessConfig &config,
    AudioStreamInfo &streamInfo, bool &isUltraFast, bool isReloadProcess)
{
    // send the config to AudioPolicyServera and get the device info.
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    int32_t ret = CoreServiceHandler::GetInstance().GetProcessDeviceInfoBySessionId(config.originalSessionId,
        deviceInfo, streamInfo, isUltraFast, isReloadProcess);
    if (ret == SUCCESS) {
        AUDIO_INFO_LOG("Get DeviceInfo from policy: deviceType:%{public}d, supportLowLatency:%{public}s"
            " a2dpOffloadFlag:%{public}d", deviceInfo.deviceType_, (deviceInfo.isLowLatencyDevice_ ? "true" : "false"),
            deviceInfo.a2dpOffloadFlag_);
        if (AudioStreamCommon::IsVoipMmap(config.rendererInfo.streamUsage, config.capturerInfo.sourceType)) {
            if (config.streamInfo.samplingRate <= SAMPLE_RATE_16000) {
                AUDIO_INFO_LOG("VoIP 16K");
                streamInfo = {SAMPLE_RATE_16000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO};
            } else {
                AUDIO_INFO_LOG("VoIP 48K");
                streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO};
            }
        } else {
            AUDIO_INFO_LOG("Fast stream use rate:%{public}d format:%{public}d", streamInfo.samplingRate,
                streamInfo.format);
            deviceInfo.deviceName_ = "mmap_device";
        }
        return deviceInfo;
    }

    AUDIO_WARNING_LOG("GetProcessDeviceInfo from audio policy server failed!");
    if (config.audioMode == AUDIO_MODE_RECORD) {
        deviceInfo.deviceId_ = 1;
        deviceInfo.networkId_ = LOCAL_NETWORK_ID;
        deviceInfo.deviceRole_ = INPUT_DEVICE;
        deviceInfo.deviceType_ = DEVICE_TYPE_MIC;
    } else {
        deviceInfo.deviceId_ = 6; // 6 for test
        deviceInfo.networkId_ = LOCAL_NETWORK_ID;
        deviceInfo.deviceRole_ = OUTPUT_DEVICE;
        deviceInfo.deviceType_ = DEVICE_TYPE_SPEAKER;
    }
    streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO,
        CH_LAYOUT_STEREO}; // note: read from xml
    deviceInfo.deviceName_ = "mmap_device";
    return deviceInfo;
}
#endif

int32_t AudioService::NotifyStreamVolumeChanged(AudioStreamType streamType, float volume)
{
    std::lock_guard<std::mutex> lock(processListMutex_);
    int32_t ret = SUCCESS;
    UpdateSystemVolumeForWorkgroup(streamType, volume);
    return ret;
}

void AudioService::Dump(std::string &dumpString)
{
    AUDIO_INFO_LOG("AudioService dump begin");
    {
        std::lock_guard<std::mutex> lock(workingConfigsMutex_);
        for (auto &workingConfig_ : workingConfigs_) {
            AppendFormat(dumpString, "InnerCapid: %s  - InnerCap filter: %s\n",
                std::to_string(workingConfig_.first).c_str(),
                ProcessConfig::DumpInnerCapConfig(workingConfig_.second).c_str());
        }
    }
    // dump normal, voip and direct
    {
        std::lock_guard<std::mutex> lock(rendererMapMutex_);
        for (const auto &item : allRendererMap_) {
            std::shared_ptr<RendererInServer> renderer = item.second.lock();
            if (renderer) {
                renderer->Dump(dumpString);
            }
        }
    }

    // dump appUseNumMap_ and currentRendererStreamCnt_
    {
        std::lock_guard<std::mutex> lock(streamLifeCycleMutex_);
        AppendFormat(dumpString, " - currentRendererStreamCnt is %d\n", currentRendererStreamCnt_);
        for (auto it : appUseNumMap_) {
            AppendFormat(dumpString, "  - appUseNumMap_ appUid: %d\n", it.first);
            AppendFormat(dumpString, "  - appUseNumMap_ appUid created stream: %d\n", it.second);
        }
    }
    PolicyHandler::GetInstance().Dump(dumpString);
    AudioVolume::GetInstance()->Dump(dumpString);
}

float AudioService::GetMaxAmplitude(bool isOutputDevice)
{
    return 0;
}

std::shared_ptr<RendererInServer> AudioService::GetRendererBySessionID(const uint32_t &sessionID)
{
    std::lock_guard<std::mutex> lock(rendererMapMutex_);
    if (allRendererMap_.count(sessionID)) {
        return allRendererMap_[sessionID].lock();
    } else {
        return nullptr;
    }
}

std::shared_ptr<CapturerInServer> AudioService::GetCapturerBySessionID(const uint32_t &sessionID)
{
    std::lock_guard<std::mutex> lock(rendererMapMutex_);
    if (allCapturerMap_.count(sessionID)) {
        return allCapturerMap_[sessionID].lock();
    } else {
        return std::shared_ptr<CapturerInServer>();
    }
}

void AudioService::SetNonInterruptMute(const uint32_t sessionId, const bool muteFlag)
{
    AUDIO_INFO_LOG("SessionId: %{public}u, muteFlag: %{public}d", sessionId, muteFlag);
    std::unique_lock<std::mutex> rendererLock(rendererMapMutex_);
    if (allRendererMap_.count(sessionId)) {
        std::shared_ptr<RendererInServer> renderer = allRendererMap_[sessionId].lock();
        if (renderer == nullptr) {
            AUDIO_ERR_LOG("rendererinserver is null");
            rendererLock.unlock();
            return;
        }
        renderer->SetNonInterruptMute(muteFlag);
        AUDIO_INFO_LOG("allRendererMap_ has sessionId");
        UpdateSessionMuteStatus(sessionId, muteFlag);
        rendererLock.unlock();
        return;
    }
    rendererLock.unlock();
    std::unique_lock<std::mutex> capturerLock(capturerMapMutex_);
    if (allCapturerMap_.count(sessionId)) {
        std::shared_ptr<CapturerInServer> capturer = allCapturerMap_[sessionId].lock();
        if (capturer == nullptr) {
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CALLBACK_ILLEGAL_STATE, "capturerinserver is null", false);
            AUDIO_ERR_LOG("capturerinserver is null");
            capturerLock.unlock();
            return;
        }
        capturer->SetNonInterruptMute(muteFlag);
        AUDIO_INFO_LOG("allCapturerMap_ has sessionId");
        capturerLock.unlock();
        UpdateSessionMuteStatus(sessionId, muteFlag);
        return;
    }
    capturerLock.unlock();
    SetNonInterruptMuteForProcess(sessionId, muteFlag);
}

void AudioService::SetNonInterruptMuteForProcess(const uint32_t sessionId, const bool muteFlag)
{
    AUDIO_INFO_LOG("Cannot find sessionId");
    // when old stream already released and new stream not create yet
    // set muteflag 0 but can not find session in allRendererMap_, allCapturerMap_ and linkedPairedList_
    // need erase it from mutedSessions_ to avoid new stream can not be set unmute
    if (!muteFlag) {
        RemoveIdFromMuteControlSet(sessionId);
    }
    // when old stream already released and new stream not create yet
    // set muteflag 1 but cannot find sessionId in allRendererMap_, allCapturerMap_ and linkedPairedList_
    // this sessionid will not add into mutedSessions_
    // so need save it temporarily, when new stream create, check if new stream need mute
    // if set muteflag 0 again before new stream create, do not mute it
    UpdateSessionMuteStatus(sessionId, muteFlag);
}

void AudioService::UpdateSessionMuteStatus(const uint32_t sessionId, const bool muteFlag)
{
    std::lock_guard<std::mutex> muteSwitchStreamLock(muteSwitchStreamSetMutex_);
    if (muteFlag) {
        muteSwitchStreams_.insert(sessionId);
        AUDIO_INFO_LOG("Insert into muteSwitchStreams_");
    } else if (muteSwitchStreams_.count(sessionId)) {
        muteSwitchStreams_.erase(sessionId);
    }
}

int32_t AudioService::SetOffloadMode(uint32_t sessionId, int32_t state, bool isAppBack)
{
    std::unique_lock<std::mutex> lock(rendererMapMutex_);
    if (!allRendererMap_.count(sessionId)) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(INVALID_APP_UID,
            PLAY_CREATE_INVALID_INDEX, "SetOffloadMode renderer not in map", false);
        AUDIO_WARNING_LOG("Renderer %{public}u is not in map", sessionId);
        return ERR_INVALID_INDEX;
    }
    AUDIO_INFO_LOG("Set offload mode for renderer %{public}u", sessionId);
    std::shared_ptr<RendererInServer> renderer = allRendererMap_[sessionId].lock();
    if (renderer == nullptr) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(INVALID_APP_UID,
            PLAY_CREATE_NULL_POINTER, "SetOffloadMode RendererInServer is nullptr", false);
        AUDIO_WARNING_LOG("RendererInServer is nullptr");
        lock.unlock();
        return ERROR;
    }
    lock.unlock();
    int32_t ret = renderer->SetOffloadMode(state, isAppBack);
    return ret;
}

int32_t AudioService::UnsetOffloadMode(uint32_t sessionId)
{
    std::unique_lock<std::mutex> lock(rendererMapMutex_);
    if (!allRendererMap_.count(sessionId)) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(INVALID_APP_UID,
            PLAY_CREATE_INVALID_INDEX, "UnsetOffloadMode renderer not in map", false);
        AUDIO_WARNING_LOG("Renderer %{public}u is not in map", sessionId);
        return ERR_INVALID_INDEX;
    }
    AUDIO_INFO_LOG("Set offload mode for renderer %{public}u", sessionId);
    std::shared_ptr<RendererInServer> renderer = allRendererMap_[sessionId].lock();
    if (renderer == nullptr) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(INVALID_APP_UID,
            PLAY_CREATE_NULL_POINTER, "UnsetOffloadMode RendererInServer is nullptr", false);
        AUDIO_WARNING_LOG("RendererInServer is nullptr");
        lock.unlock();
        return ERROR;
    }
    lock.unlock();
    int32_t ret = renderer->UnsetOffloadMode();
    return ret;
}

void AudioService::UpdateAudioSinkState(uint32_t sinkId, bool started)
{
    std::unique_lock<std::mutex> lock(allRunningSinksMutex_);
    if (started) {
        CHECK_AND_RETURN_LOG(allRunningSinks_.find(sinkId) == allRunningSinks_.end(),
            "Sink %{public}u already started", sinkId);
        allRunningSinks_.insert(sinkId);
        AUDIO_INFO_LOG("Sink %{public}u started", sinkId);
    } else {
        CHECK_AND_RETURN_LOG(allRunningSinks_.find(sinkId) != allRunningSinks_.end(),
            "Sink %{public}u already stopped or not started", sinkId);
        allRunningSinks_.erase(sinkId);
        AUDIO_INFO_LOG("Sink %{public}u stopped", sinkId);
        if (allRunningSinks_.empty()) {
            allRunningSinksCV_.notify_all();
            AUDIO_INFO_LOG("All sinks stop, continue to hibernate");
        }
    }
    return;
}

void AudioService::CheckHibernateState(bool onHibernate)
{
    std::unique_lock<std::mutex> lock(allRunningSinksMutex_);
    onHibernate_ = onHibernate;
    if (onHibernate) {
        bool ret = true;
        if (allRunningSinks_.empty()) {
            // Sleep for 300ms and recheck to avoid another sink start right after first check.
            AUDIO_INFO_LOG("No running sinks, sleep for 300ms and check again");
            lock.unlock(); // Unlock so that other running sinks can be added
            usleep(RECHECK_SINK_STATE_IN_US); // sleep for 300ms
            lock.lock();
            CHECK_AND_RETURN_LOG(!allRunningSinks_.empty(), "No running sinks, continue to hibernate");
        }
        AUDIO_INFO_LOG("Wait for all sinks to stop");
        ret = allRunningSinksCV_.wait_for(lock, std::chrono::milliseconds(BLOCK_HIBERNATE_CALLBACK_IN_MS),
            [this] {return (allRunningSinks_.empty() || !onHibernate_);});
        if (!ret) {
            AUDIO_ERR_LOG("On hibernate timeout, some sinks still running");
        }
        return;
    } else {
        allRunningSinksCV_.notify_all();
        AUDIO_INFO_LOG("Wake up from hibernate");
    }
}

bool AudioService::GetHibernateState()
{
    std::unique_lock<std::mutex> lock(allRunningSinksMutex_);
    return onHibernate_;
}

int32_t AudioService::UpdateSourceType(SourceType sourceType)
{
    // specialSourceType need not updateaudioroute
    if (specialSourceTypeSet_.count(sourceType) != 0) {
        return SUCCESS;
    }

    uint32_t id = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY);
    std::shared_ptr<IAudioCaptureSource> source = HdiAdapterManager::GetInstance().GetCaptureSource(id);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, ERROR, "source is null");

    return source->UpdateSourceType(sourceType);
}

void AudioService::SetIncMaxRendererStreamCnt(AudioMode audioMode)
{
    if (audioMode == AUDIO_MODE_PLAYBACK) {
        std::lock_guard<std::mutex> lock(streamLifeCycleMutex_);
        currentRendererStreamCnt_++;
    }
}

void AudioService::SetDecMaxRendererStreamCnt()
{
    std::lock_guard<std::mutex> lock(streamLifeCycleMutex_);
    currentRendererStreamCnt_--;
}

void AudioService::SetIncMaxLoopbackStreamCnt(AudioMode audioMode)
{
    if (audioMode == AUDIO_MODE_PLAYBACK) {
        currentLoopbackRendererStreamCnt_.fetch_add(1);
    } else {
        currentLoopbackCapturerStreamCnt_.fetch_add(1);
    }
}

int32_t AudioService::GetCurrentLoopbackStreamCnt(AudioMode audioMode)
{
    if (audioMode == AUDIO_MODE_PLAYBACK) {
        return currentLoopbackRendererStreamCnt_.load();
    } else {
        return currentLoopbackCapturerStreamCnt_.load();
    }
}

void AudioService::SetDecMaxLoopbackStreamCnt(AudioMode audioMode)
{
    if (audioMode == AUDIO_MODE_PLAYBACK) {
        currentLoopbackRendererStreamCnt_.fetch_sub(1);
    } else {
        currentLoopbackCapturerStreamCnt_.fetch_sub(1);
    }
}

void AudioService::CleanAppUseNumMap(int32_t appUid)
{
    std::lock_guard<std::mutex> lock(streamLifeCycleMutex_);
    auto appUseNum = appUseNumMap_.find(appUid);
    if (appUseNum != appUseNumMap_.end()) {
        appUseNumMap_[appUid] = --appUseNum->second;
    }
}

bool AudioService::HasBluetoothEndpoint()
{
    return true;
}

int32_t AudioService::GetCurrentRendererStreamCnt()
{
    std::lock_guard<std::mutex> lock(streamLifeCycleMutex_);
    return currentRendererStreamCnt_;
}

void AudioService::GetAllSinkInputs(std::vector<SinkInput> &sinkInputs)
{
    IStreamManager::GetPlaybackManager(PLAYBACK).GetAllSinkInputs(sinkInputs);
    std::vector<SinkInput> directSinkInputs;
    IStreamManager::GetPlaybackManager(DIRECT_PLAYBACK).GetAllSinkInputs(directSinkInputs);
    sinkInputs.insert(sinkInputs.end(), directSinkInputs.begin(), directSinkInputs.end());
    std::vector<SinkInput> voipSinkInputs;
    IStreamManager::GetPlaybackManager(VOIP_PLAYBACK).GetAllSinkInputs(voipSinkInputs);
    sinkInputs.insert(sinkInputs.end(), voipSinkInputs.begin(), voipSinkInputs.end());
}

// need call with streamLifeCycleMutex_ lock
bool AudioService::IsExceedingMaxStreamCntPerUid(int32_t callingUid, int32_t appUid,
    int32_t maxStreamCntPerUid)
{
    if (callingUid != MEDIA_SERVICE_UID) {
        appUid = callingUid;
    }

    auto appUseNum = appUseNumMap_.find(appUid);
    if (appUseNum != appUseNumMap_.end()) {
        ++appUseNum->second;
    } else {
        int32_t initValue = 1;
        appUseNumMap_.emplace(appUid, initValue);
    }

    if (appUseNumMap_[appUid] > maxStreamCntPerUid) {
        --appUseNumMap_[appUid]; // actual created stream num is stream num decrease one
        int32_t mostAppUid = INVALID_APP_UID;
        int32_t mostAppNum = INVALID_APP_CREATED_AUDIO_STREAM_NUM;
        GetCreatedAudioStreamMostUid(mostAppUid, mostAppNum);
        std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
            Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::AUDIO_STREAM_EXHAUSTED_STATS,
            Media::MediaMonitor::EventType::FAULT_EVENT);
        bean->Add("CLIENT_UID", mostAppUid);
        bean->Add("TIMES", mostAppNum);
        bean->Add("EXCEEDED_SCENE", "SingleApp");
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
        HILOG_COMM_WARN("[IsExceedingMaxStreamCntPerUid]Current audio renderer stream num is greater "
            "than the renderer stream num limit per uid");
        return true;
    }
    return false;
}

void AudioService::GetCreatedAudioStreamMostUid(int32_t &mostAppUid, int32_t &mostAppNum)
{
    std::lock_guard<std::mutex> lock(streamLifeCycleMutex_);
    for (auto it = appUseNumMap_.begin(); it != appUseNumMap_.end(); it++) {
        if (it->second > mostAppNum) {
            mostAppNum = it->second;
            mostAppUid = it->first;
        }
    }
    return;
}

#ifdef HAS_FEATURE_INNERCAPTURER
int32_t AudioService::UnloadModernInnerCapSink(int32_t innerCapId)
{
    return PolicyHandler::GetInstance().UnloadModernInnerCapSink(innerCapId);
}

int32_t AudioService::UnloadModernOffloadCapSource()
{
    return PolicyHandler::GetInstance().UnloadModernOffloadCapSource();
}
#endif

RestoreStatus AudioService::RestoreSession(uint32_t sessionId, RestoreInfo restoreInfo)
{
    {
        std::lock_guard<std::mutex> lock(rendererMapMutex_);
        if (allRendererMap_.find(sessionId) != allRendererMap_.end()) {
            std::shared_ptr<RendererInServer> rendererInServer = allRendererMap_[sessionId].lock();
            CHECK_AND_CALL_FUNC_RETURN_RET_LOG(rendererInServer != nullptr, RESTORE_ERROR,
                StreamDfxManager::GetInstance().SendAudioErrorEvent(INVALID_APP_UID,
                    PLAY_CREATE_OPERATION_FAILED, "RestoreSession rendererInServer is null", false),
                "Session could be released, restore failed");
            return rendererInServer->RestoreSession(restoreInfo);
        }
    }
    {
        std::lock_guard<std::mutex> lock(capturerMapMutex_);
        if (allCapturerMap_.find(sessionId) != allCapturerMap_.end()) {
            std::shared_ptr<CapturerInServer> capturerInServer = allCapturerMap_[sessionId].lock();
            CHECK_AND_CALL_FUNC_RETURN_RET_LOG(capturerInServer != nullptr, RESTORE_ERROR,
                StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                    RECORD_CREATE_ILLEGAL_STATE, "Session could be released, restore failed", false),
                "Session could be released, restore failed");
            return capturerInServer->RestoreSession(restoreInfo);
        }
    }
    StreamDfxManager::GetInstance().SendAudioErrorEvent(INVALID_APP_UID,
        PLAY_CREATE_OPERATION_FAILED, "RestoreSession session not exists", false);
    AUDIO_WARNING_LOG("Session not exists, restore failed");
    return RESTORE_ERROR;
}

void AudioService::SaveAdjustStreamVolumeInfo(float volume, uint32_t sessionId, std::string adjustTime,
    uint32_t code)
{
    AudioVolume::GetInstance()->SaveAdjustStreamVolumeInfo(volume, sessionId, adjustTime, code);
}

void AudioService::RegisterMuteStateChangeCallback(uint32_t sessionId, const MuteStateChangeCallbck &callback)
{
    std::unique_lock<std::mutex> lock(muteStateMapMutex_);
    if (muteStateCallbacks_.count(sessionId) != 0) {
        if (muteStateMap_.count(sessionId) != 0) {
            AUDIO_INFO_LOG("session:%{public}u may start again, invoke callback now", sessionId);
            bool flag = muteStateMap_[sessionId];
            callback(flag);
        } else {
            AUDIO_WARNING_LOG("session:%{public}u mute state update failed...", sessionId);
        }
    }
    muteStateCallbacks_[sessionId] = callback;
}

void AudioService::SetLatestMuteState(const uint32_t sessionId, const bool muteFlag)
{
    std::unique_lock<std::mutex> lock(muteStateMapMutex_);
    if (muteStateCallbacks_.count(sessionId) == 0) {
        AUDIO_ERR_LOG("send mute flag to session:%{public}u failed", sessionId);
        return;
    }
    if (muteFlag) {
        muteStateMap_[sessionId] = muteFlag;
    } else {
        muteStateMap_.erase(sessionId);
    }
    AUDIO_INFO_LOG("session:%{public}u muteflag=%{public}d", sessionId, muteFlag ? 1 : 0);
    muteStateCallbacks_[sessionId](muteFlag);
}

int32_t AudioService::ForceStopAudioStream(StopAudioType audioType)
{
    CHECK_AND_RETURN_RET_LOG(audioType >= STOP_ALL && audioType <= STOP_RECORD, ERR_INVALID_PARAM, "Invalid audioType");
    AUDIO_INFO_LOG("stop audio stream, type:%{public}d", audioType);
    if (audioType == StopAudioType::STOP_ALL || audioType == StopAudioType::STOP_RENDER) {
        std::lock_guard<std::mutex> lock(rendererMapMutex_);
        for (auto &rendererMap : allRendererMap_) {
            std::shared_ptr<RendererInServer> rendererInServer = rendererMap.second.lock();
            CHECK_AND_CONTINUE_LOG(rendererInServer != nullptr, "stream could be released, no need to stop");
            rendererInServer->StopSession();
        }
    }
    if (audioType == StopAudioType::STOP_ALL || audioType == StopAudioType::STOP_RECORD) {
        std::lock_guard<std::mutex> lock(capturerMapMutex_);
        for (auto &capturerMap : allCapturerMap_) {
            std::shared_ptr<CapturerInServer> capturerInServer = capturerMap.second.lock();
            CHECK_AND_CONTINUE_LOG(capturerInServer != nullptr, "stream could be released, no need to stop");
            capturerInServer->StopSession();
        }
    }
    return SUCCESS;
}

float AudioService::GetSystemVolumeForWorkgroup()
{
    std::unique_lock<std::mutex> lock(audioWorkGroupSystemVolumeMutex_);
    return audioWorkGroupSystemVolume_;
}

bool AudioService::IsStreamTypeFitWorkgroup(AudioStreamType streamType)
{
    return (workgroupSupportStreamTypeSet_.find(streamType) !=
        workgroupSupportStreamTypeSet_.end());
}

void AudioService::UpdateSystemVolumeForWorkgroup(AudioStreamType streamType, float volume)
{
    AUDIO_INFO_LOG("[WorkgroupInServer] streamType:%{public}d, systemvolume:%{public}f", streamType, volume);
    if (!IsStreamTypeFitWorkgroup(streamType)) {
        return;
    }
    {
        std::unique_lock<std::mutex> lock(audioWorkGroupSystemVolumeMutex_);
        audioWorkGroupSystemVolume_ = volume;
    }
    std::vector<int32_t> pids = AudioResourceService::GetInstance()->GetProcessesOfAudioWorkgroup();
    for (int32_t pid : pids) {
        RenderersCheckForAudioWorkgroup(pid);
    }
}

void AudioService::RenderersCheckForAudioWorkgroup(int32_t pid)
{
    if (!AudioResourceService::GetInstance()->IsProcessInWorkgroup(pid)) {
        return;
    }
    if (AudioResourceService::GetInstance()->IsProcessHasSystemPermission(pid)) {
        return;
    }

    std::unordered_map<int32_t, std::unordered_map<int32_t, bool>> allRenderPerProcessMap;
    {
        std::unique_lock<std::mutex> lock(rendererMapMutex_);
        for (auto it = allRendererMap_.begin(); it != allRendererMap_.end(); it++) {
            std::shared_ptr<RendererInServer> renderer = it->second.lock();
            if (renderer == nullptr) {
                continue;
            }
            if (renderer->processConfig_.appInfo.appPid != pid) {
                continue;
            }
            if (!IsStreamTypeFitWorkgroup(renderer->processConfig_.streamType)) {
                continue;
            }
            allRenderPerProcessMap[pid][renderer->processConfig_.originalSessionId]
                = renderer->CollectInfosForWorkgroup(GetSystemVolumeForWorkgroup());
        }
    }
    // all processes in workgroup
    for (const auto &outerPair : allRenderPerProcessMap) {
        int32_t pid = outerPair.first;
        const auto &innerMap = outerPair.second;
        bool isAllowed = false;
        // check all renderer info this process
        for (const auto &innerPair : innerMap) {
            if (innerPair.second) {
                // this process allowed if one renderer running and voiced
                isAllowed = true;
                break;
            }
        }
        AudioResourceService::GetInstance()->WorkgroupRendererMonitor(pid, isAllowed);
    }
}

void AudioService::InitAllDupBuffer(int32_t innerCapId)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    AUDIO_INFO_LOG("InitAllDupBuffer, innerCapId: %{public}d", innerCapId);

    std::unique_lock<std::mutex> lock(rendererMapMutex_);
    if (filteredRendererMap_.count(innerCapId)) {
        for (size_t i = 0; i < filteredRendererMap_[innerCapId].size(); i++) {
            std::shared_ptr<RendererInServer> renderer = filteredRendererMap_[innerCapId][i].lock();
            if (renderer == nullptr) {
                AUDIO_WARNING_LOG("Renderer is already released!");
                continue;
            }
            if (ShouldBeInnerCap(renderer->processConfig_, innerCapId)) {
                renderer->InitDupBuffer(innerCapId);
            }
        }
    }
    lock.unlock();
#endif
}

std::shared_ptr<RendererInServer> AudioService::GetRendererInServerBySessionId(const uint32_t sessionId)
{
    std::lock_guard<std::mutex> lock(rendererMapMutex_);
    CHECK_AND_RETURN_RET(allRendererMap_.count(sessionId) > 0, nullptr);
    return allRendererMap_[sessionId].lock();
}

int32_t AudioService::GetPrivacyTypeForNormalStream(const uint32_t sessionId, AudioPrivacyType &privacyType)
{
    auto rendererInServer = GetRendererInServerBySessionId(sessionId);
    CHECK_AND_RETURN_RET(rendererInServer != nullptr, ERROR);
    privacyType = rendererInServer->processConfig_.privacyType;
    return SUCCESS;
}

int32_t AudioService::GetPrivacyType(const uint32_t sessionId, AudioPrivacyType &privacyType)
{
    int32_t ret = GetPrivacyTypeForNormalStream(sessionId, privacyType);
    CHECK_AND_RETURN_RET(ret != SUCCESS, ret);

    StreamDfxManager::GetInstance().SendAudioErrorEvent(INVALID_APP_UID,
        PLAY_CREATE_INVALID_INDEX, "GetPrivacyType sessionId not found", false);
    AUDIO_ERR_LOG("%{public}u not found", sessionId);
    return ERROR;
}

int32_t AudioService::EnableDualStream(const uint32_t sessionId, const std::string &dupSinkName)
{
    std::lock_guard lock(dualStreamMutex_);
    int32_t ret = EnableDualStreamForNormalStream(sessionId, dupSinkName);
    CHECK_AND_RETURN_RET(ret != SUCCESS, ret);

    AUDIO_ERR_LOG("%{public}u failed", sessionId);
    return ERR_OPERATION_FAILED;
}

int32_t AudioService::DisableDualStream(const uint32_t sessionId)
{
    std::lock_guard lock(dualStreamMutex_);
    int32_t ret = DisableDualStreamForNormalStream(sessionId);
    CHECK_AND_RETURN_RET(ret != SUCCESS, ret);

    AUDIO_ERR_LOG("%{public}u failed", sessionId);
    return ERR_OPERATION_FAILED;
}

int32_t AudioService::RequestUserPrivacyAuthority(uint32_t sessionId)
{
    std::shared_ptr<CapturerInServer> capturerInServer = nullptr;
    std::unique_lock<std::mutex> lock(capturerMapMutex_);
    if (allCapturerMap_.count(sessionId)) {
        capturerInServer = allCapturerMap_[sessionId].lock();
    }
    lock.unlock();
    CHECK_AND_RETURN_RET_LOG(capturerInServer != nullptr, ERROR, "sessionid not in allCapturerMap");

    return capturerInServer->RequestUserPrivacyAuthority(sessionId);
}

int32_t AudioService::ReportPlaybackCaptureUserChoice(uint32_t sessionId, bool choice)
{
    std::shared_ptr<CapturerInServer> capturerInServer = nullptr;
    std::unique_lock<std::mutex> lock(capturerMapMutex_);
    if (allCapturerMap_.count(sessionId)) {
        capturerInServer = allCapturerMap_[sessionId].lock();
    }
    lock.unlock();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(capturerInServer != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "sessionid not in allCapturerMap", false),
        "sessionid not in allCapturerMap");
    return capturerInServer->ReportPlaybackCaptureUserChoice(sessionId, choice);
}

void AudioService::SetAncoRecordingConfigChanged(const std::vector<std::string> &bundleNameList)
{
    std::lock_guard<std::mutex> lock(capturerMapMutex_);
    bool isAncoCapturerExist = false;
    for (auto &capturerMap : allCapturerMap_) {
        std::shared_ptr<CapturerInServer> capturerInServer = capturerMap.second.lock();
        CHECK_AND_CONTINUE(capturerInServer != nullptr && capturerInServer->GetAppUid() == UID_AUDIO);
        capturerInServer->HandleMultiAncoCapturer(bundleNameList);
        isAncoCapturerExist = true;
    }
    AUDIO_ERR_LOG("isAncoCapturerExist: %{public}d", isAncoCapturerExist);
    CHECK_AND_CALL_FUNC_RETURN(isAncoCapturerExist,
        AudioPrivacyManagerInServer::GetInstance().UpdateActiveAncoCapturer(bundleNameList));
}

void AudioService::NotifySensitiveRecordPermit(uint32_t sessionId, bool isPermitted)
{
    AUDIO_INFO_LOG("sessionId: %{public}u, isPermitted: %{public}d", sessionId, isPermitted);
    std::unique_lock<std::mutex> capturerLock(capturerMapMutex_);
    if (allCapturerMap_.count(sessionId)) {
        std::shared_ptr<CapturerInServer> capturer = allCapturerMap_[sessionId].lock();
        if (capturer == nullptr) {
            AUDIO_ERR_LOG("capturerinserver is null");
            return;
        }
    capturer->NotifySensitiveRecordPermit(isPermitted);
    return;
    }
    AUDIO_INFO_LOG("Cannot find sessionId %{public}u in allCapturerMap_", sessionId);
}
} // namespace AudioStandard
} // namespace OHOS
