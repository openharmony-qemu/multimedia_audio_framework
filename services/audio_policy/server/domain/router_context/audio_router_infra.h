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

#ifndef AUDIO_ROUTER_INFRA_H
#define AUDIO_ROUTER_INFRA_H

#include <memory>
#include <map>
#include <list>
#include <vector>
#include <mutex>
#include "audio_app_router_context.h"
#include "audio_select_device_info.h"
#include "audio_device_simple_descriptor.h"
#include "audio_source_type.h"
#include "audio_stream_info.h"

namespace OHOS {
namespace AudioStandard {

class AudioRouterInfra {
public:
    static AudioRouterInfra& GetInstance()
    {
        static AudioRouterInfra instance;
        return instance;
    }

    void RefreshSelectDevice(SelectDeviceType type, int32_t uid, 
        const std::shared_ptr<AudioDeviceSimpleDescriptor>& device);
    void ClearAllSelectSelectDevice(SelectDeviceType type);

    AudioSelectDeviceInfo GetSystemSelectDevice(SelectDeviceType type);
    AudioSelectDeviceInfo GetAppSelectDevice(SelectDeviceType type, int32_t uid);
    AudioSelectDeviceInfo GetHighestPrioritySelectDevice(SelectDeviceType type);
    AudioSelectDeviceInfo GetStreamSelectDevice(int32_t uid, int32_t streamId, SelectDeviceType type);

    void UpdatePreferredInputCategory(int32_t uid, BluetoothAndNearlinkPreferredRecordCategory category);
    BluetoothAndNearlinkPreferredRecordCategory GetPreferredInputCategory(int32_t uid);
    BluetoothAndNearlinkPreferredRecordCategory GetHighestPriorityPreferredInputCategory();

    void UpdateInputStreamState(int32_t uid, uint32_t streamId, SourceType sourceType, StreamState state);
    void UpdateOutputStreamState(int32_t uid, uint32_t streamId, StreamUsage streamUsage, StreamState state);

    bool HasRunningSourceType(const std::vector<SourceType>& sourceTypes);
    bool HasHighestPriorityRunningSourceType(const std::vector<SourceType>& sourceTypes);
    bool HasRunningStreamUsage(const std::vector<StreamUsage>& streamUsages);

    void UpdateAppForegroundState(int32_t uid, bool isForeground);
    void OnAppDied(int32_t uid);

    void Lock();
    void Unlock();

    std::vector<SourceType> GetHighestPriorityRunningSourceTypes();
    std::vector<StreamUsage> GetHighestPriorityRunningStreamUsages();
    SourceType GetStreamSourceType(int32_t uid, uint32_t streamId);
    StreamUsage GetStreamStreamUsage(int32_t uid, uint32_t streamId);
    SourceType GetFirstAppRunningSourceType(int32_t uid);

    void SetCallOwnerUid(int32_t uid);
    int32_t GetCallOwnerUid() const;
    bool IsInCall() const;

private:
    AudioRouterInfra() = default;
    ~AudioRouterInfra() = default;
    AudioRouterInfra(const AudioRouterInfra&) = delete;
    AudioRouterInfra& operator=(const AudioRouterInfra&) = delete;

    void RefreshSystemSelectDevice(SelectDeviceType type, int32_t uid, 
        const std::shared_ptr<AudioDeviceSimpleDescriptor>& device);
    void RefreshAppSelectDevice(SelectDeviceType type, int32_t uid, 
        const std::shared_ptr<AudioDeviceSimpleDescriptor>& device);
    void ClearSystemSelectDevice(SelectDeviceType type);
    void ClearAllAppSelectDevice(SelectDeviceType type);
    void AdjustInputPriority(int32_t uid, bool hasRunningStream, bool isForeground, bool isSelfSelecting);
    void AdjustOutputPriority(int32_t uid, bool hasRunningStream, bool isForeground, bool isSelfSelecting);
    bool HasRunningInputStream(int32_t uid);
    bool HasRunningOutputStream(int32_t uid);
    int64_t GetCurrentTimeMs() const;

    std::recursive_mutex mutex_;
    std::map<int32_t, std::shared_ptr<struct AudioAppRouterContext>> appContexts_;
    AudioAppSelectDevice systemSelectDevice_;
    std::list<int32_t> inputPriorityList_;
    std::list<int32_t> outputPriorityList_;
};

} 
} 
#endif 
