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
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <thread>
#include "common_event_manager.h"
#include "common_event_subscriber.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

struct DownloadConditionConfig {
    bool requireWifi = true;
    bool requireCharging = true;
    bool requireBatteryLevel = true;
    bool requireScreenLocked = true;
};

class BackgroundDownload {
public:
    explicit BackgroundDownload(std::function<int32_t(const std::string&)> downloadFunction);
    ~BackgroundDownload();

    int32_t Init();
    static bool AreConditionsMet(const std::string& featureName);
    int32_t AddFeature(const std::string& featureName);
    int32_t RemoveFeature(const std::string& featureName);
    void RestorePendingFeatures(const std::string& featureList);
    void RestoreFromFile(std::shared_ptr<BackgroundDownload> selfPtr = nullptr,
        bool syncProcess = true);
    void PersistOnExit();
    static void ClearPersistedData();

private:
    class EventSubscriber : public EventFwk::CommonEventSubscriber {
    public:
        EventSubscriber(BackgroundDownload& monitor, const EventFwk::CommonEventSubscribeInfo& subscribeInfo);
        void OnReceiveEvent(const EventFwk::CommonEventData& data) override;

    private:
        BackgroundDownload& monitor_;
    };

    static const DownloadConditionConfig& GetFeatureConfig(const std::string& featureName);
    static bool IsWifiConnected();
    static bool IsCharging();
    static bool IsBatteryLevelMet();
    static bool IsScreenLocked();
    void ProcessPendingDownloads();
    void Release();
    void UpdateDownloadStatusPara(const std::string& featureList);
    void PersistPendingQueueLocked();
    std::string BuildXmlContent(const std::string& value);
    std::string ReadPendingTasksFromFile();
    void WritePendingTasksFile(const std::string& featureList);
    static void DeletePendingTasksFile();

    std::mutex mutex_;
    std::unordered_map<std::string, int32_t> processDownloadMap_;
    std::shared_ptr<EventSubscriber> subscriber_ = nullptr;
    std::function<int32_t(const std::string&)> downloadCallback_ = nullptr;
    std::atomic<bool> stopped_{false};
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif