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
#ifndef ST_AUDIO_BACKGROUND_MANAGER_H
#define ST_AUDIO_BACKGROUND_MANAGER_H

#include <bitset>
#include <string>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include "audio_stream_collector.h"
#include "common_event_manager.h"

#include  "background_task_listener.h"
#include "continuous_task_callback_info.h"
#include "background_task_subscriber.h"

namespace OHOS {
namespace AudioStandard {

struct AppState
{
    bool isFreeze = false;
    bool isBack = false;
    bool hasSession = false;
    bool hasBackTask = false;
    bool isBinder = false;
    bool isSystem = false;
};

class AudioBackgroundManager {
public:
    static AudioBackgroundManager& GetInstance()
    {
        static AudioBackgroundManager instance;
        return instance;
    }

    int32_t SetQueryAllowedPlaybackCallback(const sptr<IRemoteObject> &object);
    int32_t SetBackgroundMuteCallback(const sptr<IRemoteObject> &object);
    bool IsAllowedPlayback(const int32_t &uid, const int32_t &pid);
    void SubscribeBackgroundTask();
    void NotifyAppStateChange(const int32_t uid, const int32_t pid, AppIsBackState state);
    void NotifyBackgroundTaskStateChange(const int32_t uid, const int32_t pid, bool hasBackgroundTask);
    int32_t NotifySessionStateChange(const int32_t uid, const int32_t pid, const bool hasSession);
    void HandleSessionStateChange(const int32_t uid, const int32_t pid);
    int32_t NotifyFreezeStateChange(const std::set<int32_t> &pidList, const bool isFreeze);
    int32_t ResetAllProxy();
    void HandleFreezeStateChange(const int32_t pid, bool isFreeze);

private:
    AudioBackgroundManager() : streamCollector_(AudioStreamCollector::GetAudioStreamCollector()) {}
    ~AudioBackgroundManager() {}

    void InsertIntoAppStatesMap(int32_t pid, AppState appState);
    void DeleteFromMap(int32_t pid);
    bool FindKeyInMap(int32_t pid);

    std::map<int32_t, AppState> appStatesMap_;
    AudioStreamCollector& streamCollector_;

    sptr<IStandardAudioPolicyManagerListener> isAllowedPlaybackListener_;
    sptr<IStandardAudioPolicyManagerListener> backgroundMuteListener_;
    std::shared_ptr<BackgroundTaskListener> backgroundTaskListener_;

    std::mutex appStatesMapMutex_;
};
}
}
#endif