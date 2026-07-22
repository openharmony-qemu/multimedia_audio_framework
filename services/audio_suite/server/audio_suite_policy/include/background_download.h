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

#ifndef DOWNLOAD_CONDITION_MONITOR_H
#define DOWNLOAD_CONDITION_MONITOR_H

#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include <functional>
#include <queue>
#include "common_event_manager.h"
#include "common_event_subscriber.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class BackgroundDownload {
public:
    BackgroundDownload(std::function<int32_t(const std::string&)> downloadFunction);
    ~BackgroundDownload();

    int32_t Init();
    static bool AreConditionsMet();
    bool HasBackgroundDownloadTask();
    int32_t AddFeature(const std::string& featureName);
    void RemoveFeature(const std::string& featureName);

private:
    class EventSubscriber : public EventFwk::CommonEventSubscriber {
    public:
        EventSubscriber(BackgroundDownload& monitor, const EventFwk::CommonEventSubscribeInfo& subscribeInfo);
        void OnReceiveEvent(const EventFwk::CommonEventData& data) override;

    private:
        BackgroundDownload& monitor_;
    };

    static bool IsWifiConnected();
    static bool IsScreenLocked();
    static bool IsBatteryConditionMet();
    void HandleEvent(const EventFwk::CommonEventData& data);
    void Release();

    std::mutex mutex_;
    std::queue<std::string> pendingDownloadQueue_;
    std::shared_ptr<EventSubscriber> subscriber_ = nullptr;
    std::function<int32_t(const std::string&)> downloadCallback_ = nullptr;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif