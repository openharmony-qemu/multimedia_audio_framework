/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "background_download"
#endif

#include "background_download.h"
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "common_event_support.h"

#if !defined(AUDIOSUITE_SA_UT) && defined(AUDIO_SUITE_ROM_DOWNLOAD_NET_MANAGER)
#include "net_conn_client.h"
#endif

#if !defined(AUDIOSUITE_SA_UT) && defined(AUDIO_SUITE_ROM_DOWNLOAD_BATTERY_MANAGER)
#include "battery_info.h"
#include "battery_srv_client.h"
#endif

#if !defined(AUDIOSUITE_SA_UT) && defined(AUDIO_SUITE_ROM_DOWNLOAD_SCREENLOCK_MANAGER)
#include "screenlock_manager.h"
#endif

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

#if !defined(AUDIOSUITE_SA_UT) && defined(AUDIO_SUITE_ROM_DOWNLOAD_BATTERY_MANAGER)
static constexpr uint32_t BACKGROUND_DOWNLOAD_BATTERY_LEVEL = 50;
#endif

BackgroundDownload::EventSubscriber::EventSubscriber(BackgroundDownload& monitor,
    const EventFwk::CommonEventSubscribeInfo& subscribeInfo)
    : EventFwk::CommonEventSubscriber(subscribeInfo), monitor_(monitor)
{
}

void BackgroundDownload::EventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData& data)
{
    monitor_.HandleEvent(data);
}

BackgroundDownload::BackgroundDownload(std::function<int32_t(const std::string&)> downloadFunction)
    : downloadCallback_(downloadFunction)
{
}

BackgroundDownload::~BackgroundDownload()
{
    Release();
}

int32_t BackgroundDownload::Init()
{
    std::lock_guard<std::mutex> lock(mutex_);

    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_CONN_STATE);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_LOCKED);

    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    subscriber_ = std::make_shared<EventSubscriber>(*this, subscribeInfo);
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_)) {
        AUDIO_ERR_LOG("Failed to subscribe common event ");
        return -1;
    }

    AUDIO_INFO_LOG("BackgroundDownload initialized successfully");
    return 0;
}

void BackgroundDownload::HandleEvent(const EventFwk::CommonEventData& data)
{
    std::queue<std::string> processQueue;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (pendingDownloadQueue_.empty() || downloadCallback_ == nullptr) {
            AUDIO_INFO_LOG("backgroundDownload queue empty or callback null.");
            return;
        }

        std::queue<std::string> remainQueue;
        while (!pendingDownloadQueue_.empty()) {
            std::string featureName = pendingDownloadQueue_.front();
            pendingDownloadQueue_.pop();
            if (AreConditionsMet(featureName)) {
                processQueue.push(featureName);
            } else {
                remainQueue.push(featureName);
            }
        }
        pendingDownloadQueue_ = std::move(remainQueue);

        if (processQueue.empty()) {
            AUDIO_INFO_LOG("backgroundDownload conditions not met for any feature.");
            return;
        }
    }

    AUDIO_INFO_LOG("start process BackgroundDownload.");
    std::queue<std::string> failedQueue;
    while (!processQueue.empty()) {
        std::string featureName = processQueue.front();
        processQueue.pop();
        int32_t ret = downloadCallback_(featureName);
        if (ret != SUCCESS) {
            failedQueue.push(featureName);
        }
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!failedQueue.empty()) {
            pendingDownloadQueue_.push(failedQueue.front());
            failedQueue.pop();
        }
        AUDIO_INFO_LOG("finish process BackgroundDownload, size:%{public}zu", pendingDownloadQueue_.size());
    }
}

void BackgroundDownload::Release()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (subscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
        subscriber_ = nullptr;
        AUDIO_INFO_LOG("Release finish, BackgroundDownload remove subscriber");
    }
}

bool BackgroundDownload::AreConditionsMet(const std::string& featureName)
{
    const auto& config = GetFeatureConfig(featureName);
    bool met = true;
    if (config.requireWifi) {
        met = met && IsWifiConnected();
    }
    if (config.requireCharging) {
        met = met && IsCharging();
    }
    if (config.requireBatteryLevel) {
        met = met && IsBatteryLevelMet();
    }
    if (config.requireScreenLocked) {
        met = met && IsScreenLocked();
    }
    return met;
}

const DownloadConditionConfig& BackgroundDownload::GetFeatureConfig(const std::string& featureName)
{
    static const DownloadConditionConfig defaultConfig;
    static const std::unordered_map<std::string, DownloadConditionConfig> featureConfigs = {
        {"AudioSourceSeparationAlgoRes", DownloadConditionConfig {true, false, false, false}},
        {"RecordDefinition", DownloadConditionConfig {true, false, false, false}},
    };
    auto it = featureConfigs.find(featureName);
    if (it != featureConfigs.end()) {
        return it->second;
    }
    return defaultConfig;
}

bool BackgroundDownload::IsWifiConnected()
{
#if !defined(AUDIOSUITE_SA_UT) && defined(AUDIO_SUITE_ROM_DOWNLOAD_NET_MANAGER)
    NetManagerStandard::NetHandle handle;
    int32_t ret = NetManagerStandard::NetConnClient::GetInstance().GetDefaultNet(handle);
    if (ret != 0) {
        return false;
    }
    NetManagerStandard::NetAllCapabilities netAllCap;
    NetManagerStandard::NetConnClient::GetInstance().GetNetCapabilities(handle, netAllCap);
    const std::set<NetManagerStandard::NetBearType> &types = netAllCap.bearerTypes_;
    return types.count(NetManagerStandard::BEARER_WIFI);
#else
    return true;
#endif
}

bool BackgroundDownload::IsCharging()
{
#if !defined(AUDIOSUITE_SA_UT) && defined(AUDIO_SUITE_ROM_DOWNLOAD_BATTERY_MANAGER)
    auto &batteryService = OHOS::PowerMgr::BatterySrvClient::GetInstance();
    PowerMgr::BatteryChargeState chargeState = batteryService.GetChargingStatus();
    return chargeState == PowerMgr::BatteryChargeState::CHARGE_STATE_ENABLE ||
           chargeState == PowerMgr::BatteryChargeState::CHARGE_STATE_FULL;
#else
    return true;
#endif
}

bool BackgroundDownload::IsBatteryLevelMet()
{
#if !defined(AUDIOSUITE_SA_UT) && defined(AUDIO_SUITE_ROM_DOWNLOAD_BATTERY_MANAGER)
    auto &batteryService = OHOS::PowerMgr::BatterySrvClient::GetInstance();
    uint32_t capacity = static_cast<uint32_t>(batteryService.GetCapacity());
    return capacity > BACKGROUND_DOWNLOAD_BATTERY_LEVEL;
#else
    return true;
#endif
}

bool BackgroundDownload::IsScreenLocked()
{
#if !defined(AUDIOSUITE_SA_UT) && defined(AUDIO_SUITE_ROM_DOWNLOAD_SCREENLOCK_MANAGER)
    auto screenLockMgr = OHOS::ScreenLock::ScreenLockManager::GetInstance();
    return screenLockMgr->IsScreenLocked();
#else
    return true;
#endif
}

int32_t BackgroundDownload::AddFeature(const std::string& featureName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::queue<std::string> tempQueue = pendingDownloadQueue_;
    bool alreadyExists = false;
    while (!tempQueue.empty()) {
        if (tempQueue.front() == featureName) {
            alreadyExists = true;
            break;
        }
        tempQueue.pop();
    }

    if (!alreadyExists) {
        pendingDownloadQueue_.push(featureName);
    }
    AUDIO_INFO_LOG("AddFeature featureName:%{public}s to queue, alreadyExists:%{public}d, size:%{public}zu",
        featureName.c_str(), alreadyExists, pendingDownloadQueue_.size());
    return SUCCESS;
}

void BackgroundDownload::RemoveFeature(const std::string& featureName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::queue<std::string> tempQueue;
    bool found = false;
    
    while (!pendingDownloadQueue_.empty()) {
        const std::string& item = pendingDownloadQueue_.front();
        if (item == featureName) {
            found = true;
        } else {
            tempQueue.push(item);
        }
        pendingDownloadQueue_.pop();
    }
    
    while (!tempQueue.empty()) {
        pendingDownloadQueue_.push(tempQueue.front());
        tempQueue.pop();
    }
    AUDIO_INFO_LOG("featureName:%{public}s already remove from queue, found:%{public}d, size:%{public}zu",
        featureName.c_str(), found, pendingDownloadQueue_.size());
}

bool BackgroundDownload::HasBackgroundDownloadTask()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return !pendingDownloadQueue_.empty();
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS