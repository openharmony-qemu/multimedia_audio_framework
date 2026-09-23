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
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
enum ExcludeDeviceType {
    EXCLUDED = 0,
    UNEXCLUDED = 1,
};

constexpr SelectDeviceType ALL_OUTPUT_TYPES = SelectDeviceType::SELECT_DEVICE_TYPE_NUM;

struct StreamDefaultDeviceEntry {
    int32_t uid = INVALID_UID;
    uint32_t streamId = INVALID_STREAM_ID;
    int64_t timestamp = 0;
    SelectDeviceType type = SelectDeviceType::INVALID_TYPE;
};

struct Selection {
    int32_t uid{INVALID_UID};
    SelectDeviceType type{SelectDeviceType::INVALID_TYPE};
    int32_t deviceId{0};
    uint32_t streamId{INVALID_STREAM_ID};
};

class AudioRouterInfra {
public:
    static AudioRouterInfra& GetInstance()
    {
        static AudioRouterInfra instance;
        return instance;
    }

    void RefreshSelectDevice(SelectDeviceType type, int32_t uid,
        const std::shared_ptr<AudioDeviceSimpleDescriptor>& device, const uint32_t deviceTokenId = 0);
    void ClearAllSelectSelectDevice(SelectDeviceType type,
        const std::shared_ptr<AudioDeviceSimpleDescriptor>& device = nullptr);
    void ClearWirelessSelectDevice(const std::shared_ptr<AudioDeviceSimpleDescriptor> &device);
    void SetRecognitionCaptureDevice(const std::shared_ptr<AudioDeviceSimpleDescriptor>& device);
    AudioSelectDeviceInfo GetSystemSelectDevice(SelectDeviceType type);
    AudioSelectDeviceInfo GetAppSelectDevice(SelectDeviceType type, int32_t uid);
    AudioSelectDeviceInfo GetLatestAppSelectDevice(SelectDeviceType type);
    std::shared_ptr<AudioDeviceSimpleDescriptor> GetRecognitionCaptureDevice();

    void UpdateStreamSelectDevice(int32_t uid, uint32_t streamId,
        const std::shared_ptr<AudioDeviceSimpleDescriptor>& device);
    void UpdateStreamDefaultDevice(int32_t uid, uint32_t streamId, SelectDeviceType type,
        const std::shared_ptr<AudioDeviceSimpleDescriptor>& device);
    std::shared_ptr<AudioDeviceSimpleDescriptor> GetStreamSelectDevice(int32_t uid, uint32_t streamId);
    std::shared_ptr<AudioDeviceSimpleDescriptor> GetStreamDefaultDevice(int32_t uid, uint32_t streamId,
        SelectDeviceType type);
    void RefreshStreamDefaultDevicePriority(int32_t uid, uint32_t streamId, SelectDeviceType type);
    std::shared_ptr<AudioDeviceSimpleDescriptor> GetEffectiveDefaultOutputDevice(SelectDeviceType type);
    void ClearStreamSelectDevice(const std::shared_ptr<AudioDeviceSimpleDescriptor> &device);
    void ClearSystemSelectDevice(SelectDeviceType type,
        const std::shared_ptr<AudioDeviceSimpleDescriptor>& device);
    void SetBtConnecting(bool flag);
    int32_t ErasePreferredDeviceByType(const PreferredType preferredType);

    void UpdatePreferredInputCategory(int32_t uid, BluetoothAndNearlinkPreferredRecordCategory category);
    BluetoothAndNearlinkPreferredRecordCategory GetPreferredInputCategory(int32_t uid);
    BluetoothAndNearlinkPreferredRecordCategory GetHighestPriorityPreferredInputCategory();

    void ExcludeDevice(const std::shared_ptr<AudioDeviceSimpleDescriptor> &device,
        AudioDeviceUsage usage);
    void UnexcludeDevice(const std::shared_ptr<AudioDeviceSimpleDescriptor> &device,
        AudioDeviceUsage usage);
    std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>> GetExcludedDevices(AudioDeviceUsage usage);
    bool IsDeviceExcluded(const std::shared_ptr<AudioDeviceSimpleDescriptor> &device,
        AudioDeviceUsage usage);
    void UnexcludeAllDevice();
    void SetScoExcluded(bool scoExcluded);
    bool GetScoExcluded();

    void UpdateCurrentInputDevice(int32_t uid,
        const std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>>& devices);
    std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>> GetCurrentInputDevice(int32_t uid);
    std::set<int32_t> FindCurrentInputDevice(std::set<DeviceType> &&types);

    void UpdateCurrentOutputDevice(int32_t uid,
        const std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>>& devices);
    std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>> GetCurrentOutputDevice(int32_t uid);
    std::set<int32_t> FindCurrentOutputDevice(std::set<DeviceType> &&types);

    void UpdateInputStreamState(int32_t uid, uint32_t streamId, SourceType sourceType, CapturerState state);
    void UpdateOutputStreamState(int32_t uid, uint32_t streamId, StreamUsage streamUsage,
        RendererState state, bool updatePriority = true);

    int32_t GetHighestInputPriorityApp();
    int32_t GetHighestOutputPriorityApp();
    bool HasHighestPriorityRunningSourceType(const std::vector<SourceType>& sourceTypes);
    std::vector<Selection> GetSelections(const std::set<SelectDeviceType> &types);
    std::vector<SourceType> GetAppRunningSourceTypes(int32_t uid);
    std::vector<StreamUsage> GetAppRunningStreamUsages(int32_t uid);
    SourceType GetStreamSourceType(int32_t uid, uint32_t streamId);
    StreamUsage GetStreamStreamUsage(int32_t uid, uint32_t streamId);
    bool IsStreamRunning(int32_t uid, uint32_t streamId);

    void UpdateAppForegroundState(int32_t uid, bool isForeground);
    void OnAppDied(int32_t uid);
    bool IsSystemUid(int32_t uid) const;
    bool IsPreferredDevice(const std::shared_ptr<AudioDeviceSimpleDescriptor> &desc);
    bool HasValidSelectDevice(AudioSelectDeviceInfo selectInfo);
    bool HasRunningInputStream(int32_t uid);
    bool HasRunningOutputStream(int32_t uid);

    bool IsUidWithExclusiveDevice(int32_t uid, SelectDeviceType type);

    void Lock();
    void Unlock();

    bool HasSkipDeviceTypePermission(uid_t uid);
    bool UpdateSkippedDeviceTypes(bool skipped, const std::vector<DeviceType> &deviceTypes);
    bool IsDeviceTypeSkipped(DeviceType deviceType);
    std::string AppendSkippedDeviceTypes(const std::string &devicesStr);

private:
    AudioRouterInfra() = default;
    ~AudioRouterInfra() = default;
    AudioRouterInfra(const AudioRouterInfra&) = delete;
    AudioRouterInfra& operator=(const AudioRouterInfra&) = delete;

    void RefreshSystemSelectDevice(SelectDeviceType type,
        const std::shared_ptr<AudioDeviceSimpleDescriptor>& device);
    void RefreshAppSelectDevice(SelectDeviceType type, int32_t uid,
        const std::shared_ptr<AudioDeviceSimpleDescriptor>& device, const uint32_t deviceTokenId);
    void ClearAllAppSelectDevice(SelectDeviceType type,
        const std::shared_ptr<AudioDeviceSimpleDescriptor>& device);
    void RemoveFromPriorityList(int32_t uid, uint32_t streamId, SelectDeviceType type);
    void InsertStreamDefaultDeviceFront(const StreamDefaultDeviceEntry &entry);
    // NOTE: AdjustOutputPriority manages outputPriorityList_ for app-level select device priority only.
    // Default device priority now uses streamDefaultDevicePriority_ via RefreshStreamDefaultDevicePriority.
    void AdjustInputPriority(int32_t uid, bool hasRunningStream, bool isForeground, bool isSelfSelecting);
    void AdjustOutputPriority(int32_t uid, bool hasRunningStream, bool isForeground, bool isSelfSelecting);
    int64_t GetCurrentTimeMs() const;
    std::vector<SourceType> GetAppRunningSourceTypesNoLock(int32_t uid);
    bool IsWirelessDevice(const std::shared_ptr<AudioDeviceSimpleDescriptor> &device);
    bool NeedClearWirelessDevice(const std::shared_ptr<AudioDeviceSimpleDescriptor> &left,
        const std::shared_ptr<AudioDeviceSimpleDescriptor> &right);
    void UpdateExclusiveUidNoLock(int32_t uid, SelectDeviceType type, const uint32_t deviceTokenId);
    int32_t GetHighestInputPriorityAppNoLock();
    int32_t GetHighestOutputPriorityAppNoLock();

    void WriteExcludeOutputSysEvents(AudioDeviceUsage audioDevUsage,
        const std::shared_ptr<AudioDeviceSimpleDescriptor> &deviceDesc, ExcludeDeviceType excludeType);
    void WriteSelectOutputSysEvents(
        const std::shared_ptr<AudioDeviceSimpleDescriptor>& device, SelectDeviceType type);

    struct CompareExcludedSimpleDevice {
        bool operator()(const std::shared_ptr<AudioDeviceSimpleDescriptor> &a,
            const std::shared_ptr<AudioDeviceSimpleDescriptor> &b) const
        {
            if (a == nullptr && b == nullptr) {
                return false;
            } else if (a == nullptr) {
                return true;
            } else if (b == nullptr) {
                return false;
            }
            return *a < *b;
        }
    };

    std::recursive_mutex mutex_;
    std::map<int32_t, std::shared_ptr<struct AudioAppRouterContext>> appContexts_;
    AudioAppSelectDevice systemSelectDevice_;
    AudioSelectDeviceInfo recognitionSelectDevice_;
    std::map<std::shared_ptr<AudioDeviceSimpleDescriptor>, AudioDeviceUsage,
        CompareExcludedSimpleDevice> excludedDevices_;
    std::list<int32_t> inputPriorityList_;
    std::list<int32_t> outputPriorityList_;
    std::list<StreamDefaultDeviceEntry> streamDefaultDevicePriority_;
    std::unordered_set<int32_t> exclusiveUids_[static_cast<size_t>(SelectDeviceType::SELECT_DEVICE_TYPE_NUM)];
    std::atomic<bool> isScoExcluded_ {false};
    bool isSkipped_ {false};
    std::vector<DeviceType> skippedDeviceTypes_;
    bool isBTReconnecting_ = false;
};

}
}
#endif
