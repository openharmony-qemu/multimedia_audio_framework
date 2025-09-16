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

#ifndef ST_AUDIO_USR_SELECT_MANAGER_H
#define ST_AUDIO_USR_SELECT_MANAGER_H

#include "audio_system_manager.h"

#include "audio_device_manager.h"
#include "audio_stream_descriptor.h"

#include "ipc_skeleton.h"

#include <shared_mutex>
#include <unordered_map>
#include <list>

namespace OHOS {
namespace AudioStandard {
typedef std::shared_ptr<AudioDeviceDescriptor> AudioDevicePtr;

enum UpdateType {
    START_CLIENT,
    APP_SELECT,
    SYSTEM_SELECT,
    APP_PREFER,
    STOP_CLIENT,
    RELEASE_CLIENT,
};

struct RecordDeviceInfo {
    int32_t uid_{-1};
    SourceType sourceType_{SourceType::SOURCE_TYPE_INVALID};
    AudioDevicePtr selectedDevice_{std::make_shared<AudioDeviceDescriptor>()};
    AudioDevicePtr activeSelectedDevice_{std::make_shared<AudioDeviceDescriptor>()};
    BluetoothAndNearlinkPreferredRecordCategory appPreferredCategory_{
        BluetoothAndNearlinkPreferredRecordCategory::PREFERRED_NONE};
};

class AudioUsrSelectManager {
public:
    static AudioUsrSelectManager& GetAudioUsrSelectManager()
    {
        static AudioUsrSelectManager audioUsrSelectManager;
        return audioUsrSelectManager;
    }

    // Set media render device selected by the user
    bool SelectInputDeviceByUid(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor, int32_t uid);
    std::shared_ptr<AudioDeviceDescriptor> GetSelectedInputDeviceByUid(int32_t uid);
    BluetoothAndNearlinkPreferredRecordCategory GetPreferBluetoothAndNearlinkRecordByUid(int32_t uid);
    std::shared_ptr<AudioDeviceDescriptor> GetCapturerDevice(int32_t uid, SourceType sourceType);
    void UpdateRecordDeviceInfo(UpdateType updateType, RecordDeviceInfo info);
    void UpdateAppIsBackState(int32_t uid, AppIsBackState appState);

private:
    AudioUsrSelectManager() {};
    ~AudioUsrSelectManager() {};

    std::shared_ptr<AudioDeviceDescriptor> JudgeFinalSelectDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc,
        SourceType sourceType, BluetoothAndNearlinkPreferredRecordCategory category);
    std::shared_ptr<AudioDeviceDescriptor> GetPreferDevice();
    int32_t GetIdFromRecordDeviceInfoList(int32_t uid);
    void UpdateRecordDeviceInfoForStartInner(int32_t index, RecordDeviceInfo info);
    void UpdateRecordDeviceInfoForSelectInner(int32_t index, RecordDeviceInfo info);
    void UpdateRecordDeviceInfoForPreferInner(int32_t index, RecordDeviceInfo info);
    void UpdateRecordDeviceInfoForStopInner(int32_t index);

    std::mutex mutex_;
    std::vector<RecordDeviceInfo> recordDeviceInfoList_;
    std::map<int32_t, AppIsBackState> appIsBackStatesMap_;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_USR_SELECT_MANAGER_H
