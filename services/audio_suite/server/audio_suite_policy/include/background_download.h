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
    BackgroundDownload(std::function<void()> callback);
    ~BackgroundDownload();

    int32_t Init();
    int32_t Release();
    bool AreConditionsMet();

    // 队列管理接口
    void AddToPendingQueue(const std::string& version);
    size_t GetPendingQueueSize();
    bool PopNextPendingVersion(std::string& version);

private:
    class EventSubscriber : public EventFwk::CommonEventSubscriber {
    public:
        EventSubscriber(BackgroundDownload& monitor, const EventFwk::CommonEventSubscribeInfo& subscribeInfo);
        void OnReceiveEvent(const EventFwk::CommonEventData& data) override;

    private:
        BackgroundDownload& monitor_;
    };

    bool IsWifiConnected();
    bool IsScreenLocked();
    bool IsBatteryConditionMet();
    void HandleEvent(const EventFwk::CommonEventData& data);

    std::mutex mutex_;
    std::atomic<bool> isInitialized_;
    std::function<void()> conditionMetCallback_;
    std::shared_ptr<EventSubscriber> subscriber_;

    // 队列相关成员
    std::queue<std::string> pendingDownloadQueue_;
    std::mutex queueMutex_;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif