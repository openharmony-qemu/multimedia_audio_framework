/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <thread>

#include "audio_errors.h"
#include "audio_common_log.h"
#include "audio_utils.h"
#include "policy_handler.h"
#include "core_service_handler.h"
#include "ipc_stream_in_server.h"
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

namespace OHOS {
namespace AudioStandard {

#ifdef SUPPORT_LOW_LATENCY
static const int32_t NORMAL_ENDPOINT_RELEASE_DELAY_TIME_MS = 3000; // 3s
static const uint32_t A2DP_ENDPOINT_RELEASE_DELAY_TIME = 3000; // 3s
static const uint32_t VOIP_ENDPOINT_RELEASE_DELAY_TIME = 200; // 200ms
static const uint32_t VOIP_REC_ENDPOINT_RELEASE_DELAY_TIME = 60; // 60ms
static const uint32_t A2DP_ENDPOINT_RE_CREATE_RELEASE_DELAY_TIME = 200; // 200ms
#endif
static const uint32_t BLOCK_HIBERNATE_CALLBACK_IN_MS = 5000; // 5s
static const uint32_t RECHECK_SINK_STATE_IN_US = 100000; // 100ms
static const int32_t MEDIA_SERVICE_UID = 1013;
static const int32_t RENDERER_STREAM_CNT_PER_UID_LIMIT = 40;
static const int32_t INVALID_APP_UID = -1;
static const int32_t INVALID_APP_CREATED_AUDIO_STREAM_NUM = 0;
namespace {
static inline const std::unordered_set<SourceType> specialSourceTypeSet_ = {
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VIRTUAL_CAPTURE,
    SOURCE_TYPE_REMOTE_CAST
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

#ifdef SUPPORT_LOW_LATENCY
int32_t AudioService::OnProcessRelease(IAudioProcessStream *process, bool isSwitchStream)
{
    std::lock_guard<std::mutex> processListLock(processListMutex_);
    CHECK_AND_RETURN_RET_LOG(process != nullptr, ERROR, "process is nullptr");
    bool isFind = false;
    int32_t ret = ERROR;
    auto paired = linkedPairedList_.begin();
    std::string endpointName;
    bool needRelease = false;
    int32_t delayTime = NORMAL_ENDPOINT_RELEASE_DELAY_TIME_MS;
    while (paired != linkedPairedList_.end()) {
        if ((*paired).first == process) {
            AUDIO_INFO_LOG("SessionId %{public}u", (*paired).first->GetSessionId());
            AudioPerformanceMonitor::GetInstance().DeleteSilenceMonitor(process->GetAudioSessionId());
            auto processConfig = process->GetAudioProcessConfig();
            if (processConfig.audioMode == AUDIO_MODE_PLAYBACK) {
                SetDecMaxRendererStreamCnt();
                CleanAppUseNumMap(processConfig.appInfo.appUid);
            }
            if (processConfig.capturerInfo.isLoopback || processConfig.rendererInfo.isLoopback) {
                SetDecMaxLoopbackStreamCnt(processConfig.audioMode);
                DisableLoopback();
            }
            if (!isSwitchStream) {
                AUDIO_INFO_LOG("is not switch stream, remove from mutedSessions_");
                RemoveIdFromMuteControlSet((*paired).first->GetSessionId());
            }
            ret = UnlinkProcessToEndpoint((*paired).first, (*paired).second);
            if ((*paired).second->GetStatus() == AudioEndpoint::EndpointStatus::UNLINKED) {
                needRelease = true;
                endpointName = (*paired).second->GetEndpointName();
                delayTime = GetReleaseDelayTime((*paired).second, isSwitchStream,
                    processConfig.audioMode == AUDIO_MODE_RECORD);
            }
            linkedPairedList_.erase(paired);
            isFind = true;
            break;
        } else {
            paired++;
        }
    }
    if (isFind) {
        AUDIO_INFO_LOG("find and release process result %{public}d", ret);
    } else {
        AUDIO_INFO_LOG("can not find target process, maybe already released.");
    }
    if (needRelease) {
        ReleaseProcess(endpointName, delayTime);
    }
    return SUCCESS;
}

void AudioService::ReleaseProcess(const std::string endpointName, const int32_t delayTime)
{
    AUDIO_INFO_LOG("Release endpoint [%{public}s] after %{public}d ms", endpointName.c_str(), delayTime);
    {
        std::unique_lock<std::mutex> lock(releaseEndpointMutex_);
        releasingEndpointSet_.insert(endpointName);
    }
    auto releaseMidpointThread = [this, endpointName, delayTime] () {
        std::unique_lock<std::mutex> processListLock(processListMutex_);
        CHECK_AND_RETURN_LOG(endpointList_.count(endpointName), "Can't find endpoint %{public}s", endpointName.c_str());
        if (delayTime != 0) {
            bool ret = releaseEndpointCV_.wait_for(processListLock, std::chrono::milliseconds(delayTime),
                [this, endpointName] {
                std::lock_guard<std::mutex> lock(releaseEndpointMutex_);
                if (releasingEndpointSet_.count(endpointName)) {
                    AUDIO_INFO_LOG("Release endpoint %{public}s", endpointName.c_str());
                    return false;
                }
                AUDIO_INFO_LOG("No need release endpoint: %{public}s", endpointName.c_str());
                return true;
            });

            if (ret) {
                return;
            }
        }
        this->DelayCallReleaseEndpoint(endpointName);
    };
    std::thread releaseEndpointThread(releaseMidpointThread);
    releaseEndpointThread.detach();
}

int32_t AudioService::GetReleaseDelayTime(std::shared_ptr<AudioEndpoint> endpoint, bool isSwitchStream, bool isRecord)
{
    if (endpoint->GetEndpointType() == AudioEndpoint::EndpointType::TYPE_VOIP_MMAP) {
        return isRecord ? VOIP_REC_ENDPOINT_RELEASE_DELAY_TIME : VOIP_ENDPOINT_RELEASE_DELAY_TIME;
    }
    if (endpoint->GetDeviceInfo().deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP) {
        return NORMAL_ENDPOINT_RELEASE_DELAY_TIME_MS;
    }
    // The delay for destruction and reconstruction cannot be set to 0, otherwise there may be a problem:
    // An endpoint exists at check process, but it may be destroyed immediately - during the re-create process
    return isSwitchStream ? A2DP_ENDPOINT_RE_CREATE_RELEASE_DELAY_TIME : A2DP_ENDPOINT_RELEASE_DELAY_TIME;
}
#endif

void AudioService::DisableLoopback()
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_LOG(deviceManager != nullptr, "local device manager is nullptr!");
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

    // in plan: Put playback into list, check if EnableInnerCap is need.
    if (ipcStreamInServer != nullptr && config.audioMode == AUDIO_MODE_PLAYBACK) {
        uint32_t sessionId = 0;
        std::shared_ptr<RendererInServer> renderer = ipcStreamInServer->GetRenderer();
        if (renderer != nullptr && renderer->GetSessionId(sessionId) == SUCCESS) {
            InsertRenderer(sessionId, renderer); // for all renderers
#ifdef HAS_FEATURE_INNERCAPTURER
            CheckInnerCapForRenderer(sessionId, renderer);
#endif
            CheckRenderSessionMuteState(sessionId, renderer);
        }
    }
    if (ipcStreamInServer != nullptr && config.audioMode == AUDIO_MODE_RECORD) {
        uint32_t sessionId = 0;
        std::shared_ptr<CapturerInServer> capturer = ipcStreamInServer->GetCapturer();
        if (capturer != nullptr && capturer->GetSessionId(sessionId) == SUCCESS) {
            InsertCapturer(sessionId, capturer); // for all capturers
            CheckCaptureSessionMuteState(sessionId, capturer);
        }
    }

    return ipcStreamInServer;
}

void AudioService::UpdateMuteControlSet(uint32_t sessionId, bool muteFlag)
{
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

#ifdef SUPPORT_LOW_LATENCY
void AudioService::CheckFastSessionMuteState(uint32_t sessionId, sptr<AudioProcessInServer> process)
{
    std::unique_lock<std::mutex> mutedSessionsLock(mutedSessionsMutex_);
    if (mutedSessions_.find(sessionId) != mutedSessions_.end() || IsMuteSwitchStream(sessionId)) {
        mutedSessionsLock.unlock();
        AUDIO_INFO_LOG("Session %{public}u is in control", sessionId);
        process->SetNonInterruptMute(true);
    }
}
#endif

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

void AudioService::InsertRenderer(uint32_t sessionId, std::shared_ptr<RendererInServer> renderer)
{
    std::unique_lock<std::mutex> lock(rendererMapMutex_);
    AUDIO_INFO_LOG("Insert renderer:%{public}u into map", sessionId);
    allRendererMap_[sessionId] = renderer;
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
    if (renderWhitelist_.find(bundleName) != renderWhitelist_.end()) {
        AUDIO_INFO_LOG("find hap %{public}s in list!", bundleName.c_str());
        return true;
    }
    return false;
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

    // for fast process.
#ifdef SUPPORT_LOW_LATENCY
    std::unique_lock<std::mutex> lockProcess(processListMutex_);
    for (auto paired : linkedPairedList_) {
        sptr<AudioProcessInServer> process = paired.first;
        if (process->GetSessionId() == sessionId) {
            return process->GetStandbyStatus(isStandby, enterStandbyTime);
        }
    }
#endif
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

void AudioService::InsertCapturer(uint32_t sessionId, std::shared_ptr<CapturerInServer> capturer)
{
    std::unique_lock<std::mutex> lock(capturerMapMutex_);
    AUDIO_INFO_LOG("Insert capturer:%{public}u into map", sessionId);
    allCapturerMap_[sessionId] = capturer;
}

void AudioService::RemoveCapturer(uint32_t sessionId, bool isSwitchStream)
{
    std::unique_lock<std::mutex> lock(capturerMapMutex_);
    AUDIO_INFO_LOG("Capturer: %{public}u will be removed.", sessionId);
    if (!allCapturerMap_.count(sessionId)) {
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

bool AudioService::ShouldBeInnerCap(const AudioProcessConfig &rendererConfig, int32_t innerCapId)
{
    bool canBeCaptured = rendererConfig.privacyType == AudioPrivacyType::PRIVACY_TYPE_PUBLIC;
    std::lock_guard<std::mutex> lock(workingConfigsMutex_);
    if (!canBeCaptured || innerCapId == 0 || !workingConfigs_.count(innerCapId)) {
        AUDIO_WARNING_LOG("%{public}d privacy is not public!", rendererConfig.appInfo.appPid);
        return false;
    }
    return CheckShouldCap(rendererConfig, innerCapId);
}

bool AudioService::ShouldBeInnerCap(const AudioProcessConfig &rendererConfig, std::set<int32_t> &beCapIds)
{
    bool canBeCaptured = rendererConfig.privacyType == AudioPrivacyType::PRIVACY_TYPE_PUBLIC;
    if (!canBeCaptured) {
        AUDIO_WARNING_LOG("%{public}d privacy is not public!", rendererConfig.appInfo.appPid);
        return false;
    }
    bool ret = false;
    std::lock_guard<std::mutex> lock(workingConfigsMutex_);
    for (auto& filter : workingConfigs_) {
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

bool AudioService::ShouldBeDualTone(const AudioProcessConfig &config)
{
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
#ifdef SUPPORT_LOW_LATENCY
void AudioService::FilterAllFastProcess()
{
    std::unique_lock<std::mutex> lock(processListMutex_);
    if (linkedPairedList_.size() == 0) {
        return;
    }
    for (auto paired : linkedPairedList_) {
        AudioProcessConfig temp = paired.first->processConfig_;
        std::set<int32_t> captureIds;
        if (temp.audioMode == AUDIO_MODE_PLAYBACK && ShouldBeInnerCap(temp, captureIds)) {
            HandleFastCapture(captureIds, paired.first, paired.second);
        }
    }

    for (auto pair : endpointList_) {
        if (pair.second->GetDeviceRole() == OUTPUT_DEVICE) {
            CheckDisableFastInner(pair.second);
        }
    }
}
#endif

int32_t AudioService::CheckDisableFastInner(std::shared_ptr<AudioEndpoint> endpoint)
{
    std::lock_guard<std::mutex> lock(workingConfigsMutex_);
    for (auto workingConfig : workingConfigs_) {
        if (!endpoint->ShouldInnerCap(workingConfig.first)) {
            endpoint->DisableFastInnerCap(workingConfig.first);
        }
    }
    return SUCCESS;
}

int32_t AudioService::HandleFastCapture(std::set<int32_t> captureIds, sptr<AudioProcessInServer> audioProcessInServer,
    std::shared_ptr<AudioEndpoint> audioEndpoint)
{
    for (auto captureId : captureIds) {
        audioProcessInServer->SetInnerCapState(true, captureId);
        audioEndpoint->EnableFastInnerCap(captureId);
    }
    return SUCCESS;
}

int32_t AudioService::OnInitInnerCapList(int32_t innerCapId)
{
    AUDIO_INFO_LOG("workingInnerCapId_ is %{public}d", innerCapId);
#ifdef SUPPORT_LOW_LATENCY
    FilterAllFastProcess();
#endif

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

int32_t AudioService::EnableDualToneList(uint32_t sessionId)
{
    workingDualToneId_ = sessionId;
    AUDIO_INFO_LOG("EnableDualToneList sessionId is %{public}d", sessionId);
    std::unique_lock<std::mutex> lock(rendererMapMutex_);
    for (auto it = allRendererMap_.begin(); it != allRendererMap_.end(); it++) {
        std::shared_ptr<RendererInServer> renderer = it->second.lock();
        if (renderer == nullptr) {
            AUDIO_WARNING_LOG("Renderer is already released!");
            continue;
        }
        if (ShouldBeDualTone(renderer->processConfig_)) {
            renderer->EnableDualTone();
            filteredDualToneRendererMap_.push_back(renderer);
        }
    }
    return SUCCESS;
}

int32_t AudioService::DisableDualToneList(uint32_t sessionId)
{
    AUDIO_INFO_LOG("disable dual tone, sessionId is %{public}d", sessionId);
    std::unique_lock<std::mutex> lock(rendererMapMutex_);
    for (size_t i = 0; i < filteredDualToneRendererMap_.size(); i++) {
        std::shared_ptr<RendererInServer> renderer = filteredDualToneRendererMap_[i].lock();
        if (renderer == nullptr) {
            AUDIO_WARNING_LOG("Renderer is already released!");
            continue;
        }
        renderer->DisableDualTone();
    }
    filteredDualToneRendererMap_.clear();
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

#ifdef SUPPORT_LOW_LATENCY
    std::unique_lock<std::mutex> lockEndpoint(processListMutex_);
    for (auto pair : endpointList_) {
        if (pair.second->GetDeviceRole() == OUTPUT_DEVICE) {
            pair.second->DisableFastInnerCap(innerCapId);
        }
    }
    lockEndpoint.unlock();
#endif

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

#ifdef SUPPORT_LOW_LATENCY
sptr<AudioProcessInServer> AudioService::GetAudioProcess(const AudioProcessConfig &config)
{
    Trace trace("AudioService::GetAudioProcess for " + std::to_string(config.appInfo.appPid));
    AUDIO_INFO_LOG("GetAudioProcess dump %{public}s", ProcessConfig::DumpProcessConfig(config).c_str());
    AudioStreamInfo audioStreamInfo;
    AudioDeviceDescriptor deviceInfo = GetDeviceInfoForProcess(config, audioStreamInfo);
    std::lock_guard<std::mutex> lock(processListMutex_);
    std::shared_ptr<AudioEndpoint> audioEndpoint = GetAudioEndpointForDevice(deviceInfo, config,
        audioStreamInfo, IsEndpointTypeVoip(config, deviceInfo));
    CHECK_AND_RETURN_RET_LOG(audioEndpoint != nullptr, nullptr, "no endpoint found for the process");

    uint32_t totalSizeInframe = 0;
    uint32_t spanSizeInframe = 0;
    audioEndpoint->GetPreferBufferInfo(totalSizeInframe, spanSizeInframe);

    CHECK_AND_RETURN_RET_LOG(audioStreamInfo.samplingRate > 0, nullptr, "Sample rate in server is invalid.");

    sptr<AudioProcessInServer> process = AudioProcessInServer::Create(config, this);
    CHECK_AND_RETURN_RET_LOG(process != nullptr, nullptr, "AudioProcessInServer create failed.");
    CheckFastSessionMuteState(process->GetSessionId(), process);

    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    int32_t ret = process->ConfigProcessBuffer(totalSizeInframe, spanSizeInframe, audioStreamInfo, buffer);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "ConfigProcessBuffer failed");

    ret = LinkProcessToEndpoint(process, audioEndpoint);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "LinkProcessToEndpoint failed");
    linkedPairedList_.push_back(std::make_pair(process, audioEndpoint));
#ifdef HAS_FEATURE_INNERCAPTURER
    CheckInnerCapForProcess(process, audioEndpoint);
#endif
    return process;
}

void AudioService::ResetAudioEndpoint()
{
    std::lock_guard<std::mutex> lock(processListMutex_);

    std::vector<std::string> audioEndpointNames;
    for (auto paired = linkedPairedList_.begin(); paired != linkedPairedList_.end(); paired++) {
        if (paired->second->GetEndpointType() == AudioEndpoint::TYPE_MMAP) {
            // unlink old link
            if (UnlinkProcessToEndpoint(paired->first, paired->second) != SUCCESS) {
                AUDIO_ERR_LOG("Unlink process to old endpoint failed");
            }
            audioEndpointNames.push_back(paired->second->GetEndpointName());
        }
    }

    // release old endpoint
    for (auto &endpointName : audioEndpointNames) {
        if (endpointList_.count(endpointName) > 0) {
            endpointList_[endpointName]->Release();
            AUDIO_INFO_LOG("Erase endpoint %{public}s from endpointList_", endpointName.c_str());
            endpointList_.erase(endpointName);
        }
    }

    ReLinkProcessToEndpoint();
}

void AudioService::ReLinkProcessToEndpoint()
{
    using LinkPair = std::pair<sptr<AudioProcessInServer>, std::shared_ptr<AudioEndpoint>>;
    std::vector<std::vector<LinkPair>::iterator> errorLinkedPaireds;
    for (auto paired = linkedPairedList_.begin(); paired != linkedPairedList_.end(); paired++) {
        if (paired->second->GetEndpointType() == AudioEndpoint::TYPE_MMAP) {
            AUDIO_INFO_LOG("Session id %{public}u", paired->first->GetSessionId());

            // get new endpoint
            AudioStreamInfo streamInfo;
            const AudioProcessConfig &config = paired->first->processConfig_;
            AudioDeviceDescriptor deviceInfo = GetDeviceInfoForProcess(config, streamInfo, true);
            std::shared_ptr<AudioEndpoint> audioEndpoint = GetAudioEndpointForDevice(deviceInfo, config,
                streamInfo, IsEndpointTypeVoip(config, deviceInfo));
            if (audioEndpoint == nullptr) {
                AUDIO_ERR_LOG("Get new endpoint failed");
                errorLinkedPaireds.push_back(paired);
                continue;
            }
            // link new endpoint
            if (LinkProcessToEndpoint(paired->first, audioEndpoint) != SUCCESS) {
                AUDIO_ERR_LOG("LinkProcessToEndpoint failed");
                errorLinkedPaireds.push_back(paired);
                continue;
            }
            // reset shared_ptr before to new
            paired->second.reset();
            paired->second = audioEndpoint;
#ifdef HAS_FEATURE_INNERCAPTURER
            CheckInnerCapForProcess(paired->first, audioEndpoint);
#endif
        }
    }

    for (auto &paired : errorLinkedPaireds) {
        linkedPairedList_.erase(paired);
    }
}

#ifdef HAS_FEATURE_INNERCAPTURER
void AudioService::CheckInnerCapForProcess(sptr<AudioProcessInServer> process, std::shared_ptr<AudioEndpoint> endpoint)
{
    Trace trace("AudioService::CheckInnerCapForProcess:" + std::to_string(process->processConfig_.appInfo.appPid));
    // inner-cap not working
    std::set<int32_t> captureIds;
    if (ShouldBeInnerCap(process->processConfig_, captureIds)) {
        HandleFastCapture(captureIds, process, endpoint);
    }
}
#endif

int32_t AudioService::LinkProcessToEndpoint(sptr<AudioProcessInServer> process,
    std::shared_ptr<AudioEndpoint> endpoint)
{
    int32_t ret = endpoint->LinkProcessStream(process, !GetHibernateState());
    if (ret != SUCCESS && endpoint->GetLinkedProcessCount() == 0 &&
        endpointList_.count(endpoint->GetEndpointName())) {
        std::string endpointToErase = endpoint->GetEndpointName();
        endpointList_.erase(endpoint->GetEndpointName());
        AUDIO_ERR_LOG("LinkProcessStream failed, erase endpoint %{public}s", endpointToErase.c_str());
        return ERR_OPERATION_FAILED;
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "LinkProcessStream to endpoint %{public}s failed",
        endpoint->GetEndpointName().c_str());

    ret = process->AddProcessStatusListener(endpoint);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "AddProcessStatusListener failed");

    std::unique_lock<std::mutex> lock(releaseEndpointMutex_);
    if (releasingEndpointSet_.count(endpoint->GetEndpointName())) {
        AUDIO_INFO_LOG("LinkProcessToEndpoint find endpoint is releasing, call break.");
        releasingEndpointSet_.erase(endpoint->GetEndpointName());
        releaseEndpointCV_.notify_all();
    }
    return SUCCESS;
}

int32_t AudioService::UnlinkProcessToEndpoint(sptr<AudioProcessInServer> process,
    std::shared_ptr<AudioEndpoint> endpoint)
{
    int32_t ret = endpoint->UnlinkProcessStream(process);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "UnlinkProcessStream failed");

    ret = process->RemoveProcessStatusListener(endpoint);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "RemoveProcessStatusListener failed");

    return SUCCESS;
}

void AudioService::DelayCallReleaseEndpoint(std::string endpointName)
{
    std::lock_guard<std::mutex> lock(releaseEndpointMutex_);
    if (!releasingEndpointSet_.count(endpointName)) {
        AUDIO_DEBUG_LOG("Timeout or not need to release: %{public}s", endpointName.c_str());
        return;
    }
    releasingEndpointSet_.erase(endpointName);
    CHECK_AND_RETURN_LOG(endpointList_.find(endpointName) != endpointList_.end() &&
        endpointList_[endpointName] != nullptr, "Endpoint %{public}s not available, stop call release",
        endpointName.c_str());
    std::shared_ptr<AudioEndpoint> temp = endpointList_[endpointName];
    if (temp->GetStatus() == AudioEndpoint::EndpointStatus::UNLINKED) {
        AUDIO_INFO_LOG("%{public}s not in use anymore, call release!", endpointName.c_str());
        temp->Release();
        temp = nullptr;
        endpointList_.erase(endpointName);
        return;
    }
    AUDIO_WARNING_LOG("%{public}s is not unlinked, stop call release", endpointName.c_str());
    return;
}

AudioDeviceDescriptor AudioService::GetDeviceInfoForProcess(const AudioProcessConfig &config,
    AudioStreamInfo &streamInfo, bool isReloadProcess)
{
    // send the config to AudioPolicyServera and get the device info.
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    int32_t ret = CoreServiceHandler::GetInstance().GetProcessDeviceInfoBySessionId(config.originalSessionId,
        deviceInfo, streamInfo, isReloadProcess);
    if (ret == SUCCESS) {
        AUDIO_INFO_LOG("Get DeviceInfo from policy: deviceType:%{public}d, supportLowLatency:%{public}s"
            " a2dpOffloadFlag:%{public}d", deviceInfo.deviceType_, (deviceInfo.isLowLatencyDevice_ ? "true" : "false"),
            deviceInfo.a2dpOffloadFlag_);
        if (config.rendererInfo.streamUsage == STREAM_USAGE_VOICE_COMMUNICATION ||
            config.rendererInfo.streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION ||
            config.capturerInfo.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
            if (config.streamInfo.samplingRate <= SAMPLE_RATE_16000) {
                AUDIO_INFO_LOG("VoIP 16K");
                streamInfo = {SAMPLE_RATE_16000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO};
            } else {
                AUDIO_INFO_LOG("VoIP 48K");
                streamInfo = {SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO};
            }
        } else {
            AUDIO_INFO_LOG("Fast stream use format:%{public}d", streamInfo.format);
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

void AudioService::CheckBeforeRecordEndpointCreate(bool isRecord)
{
    // release at once to avoid normal fastsource and voip fastsource existing at the same time
    if (isRecord) {
        for (auto &item : endpointList_) {
            if (item.second->GetAudioMode() == AudioMode::AUDIO_MODE_RECORD) {
                std::string endpointName = item.second->GetEndpointName();
                DelayCallReleaseEndpoint(endpointName);
                AUDIO_INFO_LOG("Release endpoint %{public}s change to now", endpointName.c_str());
                break;
            }
        }
    }
}

// must be called with processListMutex_ lock hold
ReuseEndpointType AudioService::GetReuseEndpointType(AudioDeviceDescriptor &deviceInfo,
    const std::string &deviceKey, AudioStreamInfo &streamInfo)
{
    if (endpointList_.find(deviceKey) == endpointList_.end()) {
        return ReuseEndpointType::CREATE_ENDPOINT;
    }
    bool reuse = streamInfo == endpointList_[deviceKey]->GetAudioStreamInfo();
    return reuse ? ReuseEndpointType::REUSE_ENDPOINT : ReuseEndpointType::RECREATE_ENDPOINT;
}

std::shared_ptr<AudioEndpoint> AudioService::GetAudioEndpointForDevice(AudioDeviceDescriptor &deviceInfo,
    const AudioProcessConfig &clientConfig, AudioStreamInfo &streamInfo, bool isVoipStream)
{
    // Create shared stream.
    int32_t endpointFlag = isVoipStream ? AUDIO_FLAG_VOIP_FAST : AUDIO_FLAG_MMAP;
    std::string deviceKey = AudioEndpoint::GenerateEndpointKey(deviceInfo, endpointFlag);
    ReuseEndpointType type = GetReuseEndpointType(deviceInfo, deviceKey, streamInfo);
    std::shared_ptr<AudioEndpoint> endpoint = nullptr;

    switch (type) {
        case ReuseEndpointType::REUSE_ENDPOINT: {
            AUDIO_INFO_LOG("AudioService find endpoint already exist for deviceKey:%{public}s", deviceKey.c_str());
            endpoint = endpointList_[deviceKey];
            break;
        }
        case ReuseEndpointType::RECREATE_ENDPOINT: {
            std::string endpointName = endpointList_[deviceKey]->GetEndpointName();
            AUDIO_INFO_LOG("Release endpoint %{public}s change to now", endpointName.c_str());
            DelayCallReleaseEndpoint(endpointName);
            [[fallthrough]];
        }
        case ReuseEndpointType::CREATE_ENDPOINT: {
            CheckBeforeRecordEndpointCreate(clientConfig.audioMode == AudioMode::AUDIO_MODE_RECORD);
            endpoint = AudioEndpoint::CreateEndpoint(isVoipStream ? AudioEndpoint::TYPE_VOIP_MMAP :
                AudioEndpoint::TYPE_MMAP, endpointFlag, clientConfig, deviceInfo, streamInfo);
            CHECK_AND_RETURN_RET_LOG(endpoint != nullptr, nullptr, "Create mmap AudioEndpoint failed.");
            AUDIO_INFO_LOG("Add endpoint %{public}s to endpointList_", deviceKey.c_str());
            endpointList_[deviceKey] = endpoint;
            break;
        }
        default:
            AUDIO_ERR_LOG("Create mmap AudioEndpoint failed.");
            break;
    }

    return endpoint;
}
#endif

int32_t AudioService::NotifyStreamVolumeChanged(AudioStreamType streamType, float volume)
{
    std::lock_guard<std::mutex> lock(processListMutex_);
    int32_t ret = SUCCESS;
#ifdef SUPPORT_LOW_LATENCY
    for (auto item : endpointList_) {
        if (item.second == nullptr) {
            continue;
        }
        std::string endpointName = item.second->GetEndpointName();
        if (endpointName == item.first) {
            ret = ret != SUCCESS ? ret : item.second->SetVolume(streamType, volume);
        }
    }
#endif
    UpdateSystemVolume(streamType, volume);
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
#ifdef SUPPORT_LOW_LATENCY
    // dump process
    for (auto paired : linkedPairedList_) {
        paired.first->Dump(dumpString);
    }
    // dump endpoint
    for (auto item : endpointList_) {
        AppendFormat(dumpString, "  - Endpoint device id: %s\n", item.first.c_str());
        item.second->Dump(dumpString);
    }
#endif
    // dump voip and direct
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
#ifdef SUPPORT_LOW_LATENCY
    std::lock_guard<std::mutex> lock(processListMutex_);
    if (linkedPairedList_.size() == 0) {
        return 0;
    }

    float fastAudioMaxAmplitude = 0;
    for (auto paired : linkedPairedList_) {
        if (isOutputDevice && (paired.second->GetDeviceRole() == OUTPUT_DEVICE)) {
            float curFastAudioMaxAmplitude = paired.second->GetMaxAmplitude();
            if (curFastAudioMaxAmplitude > fastAudioMaxAmplitude) {
                fastAudioMaxAmplitude = curFastAudioMaxAmplitude;
            }
        }
        if (!isOutputDevice && (paired.second->GetDeviceRole() == INPUT_DEVICE)) {
            float curFastAudioMaxAmplitude = paired.second->GetMaxAmplitude();
            if (curFastAudioMaxAmplitude > fastAudioMaxAmplitude) {
                fastAudioMaxAmplitude = curFastAudioMaxAmplitude;
            }
        }
    }
    return fastAudioMaxAmplitude;
#else
    return 0;
#endif
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
        rendererLock.unlock();
        return;
    }
    rendererLock.unlock();
    std::unique_lock<std::mutex> capturerLock(capturerMapMutex_);
    if (allCapturerMap_.count(sessionId)) {
        std::shared_ptr<CapturerInServer> capturer = allCapturerMap_[sessionId].lock();
        if (capturer == nullptr) {
            AUDIO_ERR_LOG("capturerinserver is null");
            return;
        }
        capturer->SetNonInterruptMute(muteFlag);
        AUDIO_INFO_LOG("allCapturerMap_ has sessionId");
        return;
    }
    capturerLock.unlock();
    SetNonInterruptMuteForProcess(sessionId, muteFlag);
}

void AudioService::SetNonInterruptMuteForProcess(const uint32_t sessionId, const bool muteFlag)
{
#ifdef SUPPORT_LOW_LATENCY
    std::unique_lock<std::mutex> processListLock(processListMutex_);
    for (auto paired : linkedPairedList_) {
        if (paired.first == nullptr) {
            AUDIO_ERR_LOG("processInServer is nullptr");
            return;
        }
        if (paired.first->GetSessionId() == sessionId) {
            AUDIO_INFO_LOG("linkedPairedList_ has sessionId");
            paired.first->SetNonInterruptMute(muteFlag);
            return;
        }
    }
    processListLock.unlock();
#endif
    AUDIO_INFO_LOG("Cannot find sessionId");
    // when old stream already released and new stream not create yet
    // set muteflag 0 but cannot find sessionId in allRendererMap_, allCapturerMap_ and linkedPairedList_
    // need erase it from mutedSessions_ to avoid new stream cannot be set unmute
    if (mutedSessions_.count(sessionId) && !muteFlag) {
        mutedSessions_.erase(sessionId);
    }
    // when old stream already released and new stream not create yet
    // set muteflag 1 but cannot find sessionId in allRendererMap_, allCapturerMap_ and linkedPairedList_
    // this sessionid will not add into mutedSessions_
    // so need save it temporarily, when new stream create, check if new stream need mute
    // if set muteflag 0 again before new stream create, do not mute it
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
        AUDIO_WARNING_LOG("Renderer %{public}u is not in map", sessionId);
        return ERR_INVALID_INDEX;
    }
    AUDIO_INFO_LOG("Set offload mode for renderer %{public}u", sessionId);
    std::shared_ptr<RendererInServer> renderer = allRendererMap_[sessionId].lock();
    if (renderer == nullptr) {
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
        AUDIO_WARNING_LOG("Renderer %{public}u is not in map", sessionId);
        return ERR_INVALID_INDEX;
    }
    AUDIO_INFO_LOG("Set offload mode for renderer %{public}u", sessionId);
    std::shared_ptr<RendererInServer> renderer = allRendererMap_[sessionId].lock();
    if (renderer == nullptr) {
        AUDIO_WARNING_LOG("RendererInServer is nullptr");
        lock.unlock();
        return ERROR;
    }
    int32_t ret = renderer->UnsetOffloadMode();
    lock.unlock();
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
            // Sleep for 100ms and recheck to avoid another sink start right after first check.
            AUDIO_INFO_LOG("No running sinks, sleep for 100ms and check again");
            lock.unlock(); // Unlock so that other running sinks can be added
            usleep(RECHECK_SINK_STATE_IN_US); // sleep for 100ms
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
        currentLoopbackRendererStreamCnt_++;
    } else {
        currentLoopbackCapturerStreamCnt_++;
    }
}

int32_t AudioService::GetCurrentLoopbackStreamCnt(AudioMode audioMode)
{
    if (audioMode == AUDIO_MODE_PLAYBACK) {
        return currentLoopbackRendererStreamCnt_;
    } else {
        return currentLoopbackCapturerStreamCnt_;
    }
}

void AudioService::SetDecMaxLoopbackStreamCnt(AudioMode audioMode)
{
    std::lock_guard<std::mutex> lock(streamLifeCycleMutex_);
    if (audioMode == AUDIO_MODE_PLAYBACK) {
        currentLoopbackRendererStreamCnt_--;
    } else {
        currentLoopbackCapturerStreamCnt_--;
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
#ifdef SUPPORT_LOW_LATENCY
    std::lock_guard<std::mutex> lock(processListMutex_);
    return std::any_of(linkedPairedList_.begin(), linkedPairedList_.end(),
        [](const auto & linkPair) {
            return linkPair.second->GetDeviceInfo().getType() == DEVICE_TYPE_BLUETOOTH_A2DP;
        });
#else
    return true;
#endif
}

int32_t AudioService::GetCurrentRendererStreamCnt()
{
    return currentRendererStreamCnt_;
}

void AudioService::GetAllSinkInputs(std::vector<SinkInput> &sinkInputs)
{
    IStreamManager::GetPlaybackManager(PLAYBACK).GetAllSinkInputs(sinkInputs);
}

void AudioService::SetDefaultAdapterEnable(bool isEnable)
{
    isDefaultAdapterEnable_ = isEnable;
}

bool AudioService::GetDefaultAdapterEnable()
{
    return isDefaultAdapterEnable_;
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

    if (appUseNumMap_[appUid] >= RENDERER_STREAM_CNT_PER_UID_LIMIT) {
        int32_t mostAppUid = INVALID_APP_UID;
        int32_t mostAppNum = INVALID_APP_CREATED_AUDIO_STREAM_NUM;
        GetCreatedAudioStreamMostUid(mostAppUid, mostAppNum);
        std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
            Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::AUDIO_STREAM_EXHAUSTED_STATS,
            Media::MediaMonitor::EventType::FREQUENCY_AGGREGATION_EVENT);
        bean->Add("CLIENT_UID", mostAppUid);
        bean->Add("TIMES", mostAppNum);
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
        AUDIO_WARNING_LOG("Current audio renderer stream num is greater than the renderer stream num limit per uid");
    }

    if (appUseNumMap_[appUid] > maxStreamCntPerUid) {
        --appUseNumMap_[appUid]; // actual created stream num is stream num decrease one
        return true;
    }
    return false;
}

void AudioService::GetCreatedAudioStreamMostUid(int32_t &mostAppUid, int32_t &mostAppNum)
{
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
#endif

RestoreStatus AudioService::RestoreSession(uint32_t sessionId, RestoreInfo restoreInfo)
{
    {
        std::lock_guard<std::mutex> lock(rendererMapMutex_);
        if (allRendererMap_.find(sessionId) != allRendererMap_.end()) {
            std::shared_ptr<RendererInServer> rendererInServer = allRendererMap_[sessionId].lock();
            CHECK_AND_RETURN_RET_LOG(rendererInServer != nullptr, RESTORE_ERROR,
                "Session could be released, restore failed");
            return rendererInServer->RestoreSession(restoreInfo);
        }
    }
    {
        std::lock_guard<std::mutex> lock(capturerMapMutex_);
        if (allCapturerMap_.find(sessionId) != allCapturerMap_.end()) {
            std::shared_ptr<CapturerInServer> capturerInServer = allCapturerMap_[sessionId].lock();
            CHECK_AND_RETURN_RET_LOG(capturerInServer != nullptr, RESTORE_ERROR,
                "Session could be released, restore failed");
            return capturerInServer->RestoreSession(restoreInfo);
        }
    }
#ifdef SUPPORT_LOW_LATENCY
    {
        std::lock_guard<std::mutex> lock(processListMutex_);
        for (auto processEndpointPair : linkedPairedList_) {
            if (processEndpointPair.first->GetSessionId() != sessionId) {
                continue;
            }
            auto audioProcessInServer = processEndpointPair.first;
            CHECK_AND_RETURN_RET_LOG(audioProcessInServer != nullptr, RESTORE_ERROR,
                "Session could be released, restore failed");
            return audioProcessInServer->RestoreSession(restoreInfo);
        }
    }
#endif
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

void AudioService::SetSessionMuteState(const uint32_t sessionId, const bool insert, const bool muteFlag)
{
    std::unique_lock<std::mutex> lock(muteStateMapMutex_);
    if (!insert) {
        muteStateMap_.erase(sessionId);
    } else {
        muteStateMap_[sessionId] = muteFlag;
    }
}

void AudioService::SetLatestMuteState(const uint32_t sessionId, const bool muteFlag)
{
    std::unique_lock<std::mutex> lock(muteStateMapMutex_);
    if (muteStateCallbacks_.count(sessionId) == 0) {
        AUDIO_ERR_LOG("send mute flag to session:%{public}u failed", sessionId);
        return;
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
#ifdef SUPPORT_LOW_LATENCY
    {
        std::lock_guard<std::mutex> lock(processListMutex_);
        for (auto &[audioProcessInServer, audioEndpoint]: linkedPairedList_) {
            CHECK_AND_CONTINUE_LOG(audioProcessInServer && audioEndpoint,
                "stream could be released, no need to stop");
            AudioMode audioMode = audioEndpoint->GetAudioMode();
            bool isNeedStop = (audioType == StopAudioType::STOP_ALL) ||
                (audioMode == AudioMode::AUDIO_MODE_PLAYBACK && audioType == StopAudioType::STOP_RENDER) ||
                (audioMode == AudioMode::AUDIO_MODE_RECORD && audioType == StopAudioType::STOP_RECORD);
            if (isNeedStop) {
                audioProcessInServer->StopSession();
            }
        }
    }
#endif
    return SUCCESS;
}

float AudioService::GetSystemVolume()
{
    std::unique_lock<std::mutex> lock(musicOrVoipSystemVolumeMutex_);
    return musicOrVoipSystemVolume_;
}

void AudioService::UpdateSystemVolume(AudioStreamType streamType, float volume)
{
    AUDIO_INFO_LOG("[WorkgroupInServer] streamType:%{public}d, systemvolume:%{public}f", streamType, volume);
    if ((streamType != STREAM_MUSIC) && (streamType != STREAM_VOICE_COMMUNICATION)) {
        return;
    }
    {
        std::unique_lock<std::mutex> lock(musicOrVoipSystemVolumeMutex_);
        musicOrVoipSystemVolume_ = volume;
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
            if ((renderer->processConfig_.streamType != STREAM_MUSIC) &&
                (renderer->processConfig_.streamType != STREAM_VOICE_COMMUNICATION)) {
                continue;
            }
            allRenderPerProcessMap[pid][renderer->processConfig_.originalSessionId]
                = renderer->CollectInfosForWorkgroup(GetSystemVolume());
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
}
} // namespace AudioStandard
} // namespace OHOS
