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
#include "net_conn_client.h"
#include "audio_suite_log.h"
#include "common_event_support.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

BackgroundDownload::EventSubscriber::EventSubscriber(BackgroundDownload& monitor,
    const EventFwk::CommonEventSubscribeInfo& subscribeInfo)
    : EventFwk::CommonEventSubscriber(subscribeInfo), monitor_(monitor)
{
}

void BackgroundDownload::EventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData& data)
{
    monitor_.HandleEvent(data);
}

BackgroundDownload::BackgroundDownload(std::function<void()> callback)
    : isInitialized_(false), conditionMetCallback_(callback)
{
}

BackgroundDownload::~BackgroundDownload()
{
    Release();
}

int32_t BackgroundDownload::Init()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (isInitialized_) {
        return 0;
    }

    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_CONN_STATE);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_LOCKED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED);

    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    subscriber_ = std::make_shared<EventSubscriber>(*this, subscribeInfo);
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_)) {
        AUDIO_ERR_LOG("Failed to subscribe common event");
        return -1;
    }

    isInitialized_ = true;
    AUDIO_INFO_LOG("BackgroundDownload initialized successfully");
    return 0;
}

void BackgroundDownload::HandleEvent(const EventFwk::CommonEventData& data)
{
    if (AreConditionsMet() && conditionMetCallback_) {
        AUDIO_INFO_LOG("Conditions met after event, triggering download");
        conditionMetCallback_();
    }
}

int32_t BackgroundDownload::Release()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!isInitialized_) {
        return 0;
    }

    if (subscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
        subscriber_ = nullptr;
    }

    isInitialized_ = false;
    return 0;
}

bool BackgroundDownload::AreConditionsMet()
{
    return IsWifiConnected() && IsBatteryConditionMet() && IsScreenLocked();
}

bool BackgroundDownload::IsWifiConnected()
{
    NetManagerStandard::NetHandle handle;
    int32_t ret = NetManagerStandard::NetConnClient::GetInstance().GetDefaultNet(handle);
    if (ret != 0) {
        return false;
    }

    NetManagerStandard::NetAllCapabilities netAllCap;
    NetManagerStandard::NetConnClient::GetInstance().GetNetCapabilities(handle, netAllCap);
    const std::set<NetManagerStandard::NetBearType>& types = netAllCap.bearerTypes_;
    return types.count(NetManagerStandard::BEARER_WIFI);
}

bool BackgroundDownload::IsBatteryConditionMet()
{
    return true;
}

bool BackgroundDownload::IsScreenLocked()
{
    return true;
}

void BackgroundDownload::AddToPendingQueue(const std::string& version)
{
    std::lock_guard<std::mutex> lock(queueMutex_);
    pendingDownloadQueue_.push(version);
    AUDIO_INFO_LOG("Version added to pending queue: %{public}s", version.c_str());
}

size_t BackgroundDownload::GetPendingQueueSize()
{
    std::lock_guard<std::mutex> lock(queueMutex_);
    return pendingDownloadQueue_.size();
}

bool BackgroundDownload::PopNextPendingVersion(std::string& version)
{
    std::lock_guard<std::mutex> lock(queueMutex_);
    if (pendingDownloadQueue_.empty()) {
        return false;
    }
    version = pendingDownloadQueue_.front();
    pendingDownloadQueue_.pop();
    AUDIO_INFO_LOG("Version popped from pending queue: %{public}s", version.c_str());
    return true;
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS