/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "ipc_skeleton.h"

#include <shared_mutex>
#include <unordered_map>
#include <deque>

namespace OHOS {
namespace AudioStandard {
typedef std::shared_ptr<AudioDeviceDescriptor> AudioDevicePtr;
typedef std::deque<AudioDevicePtr> AudioDeviceList;

class AudioUsrSelectManager {
public:
    static AudioUsrSelectManager& GetAudioUsrSelectManager()
    {
        static AudioUsrSelectManager audioUsrSelectManager;
        return audioUsrSelectManager;
    }

    // Set media render device selected by the user
    void SelectInputDeviceByUid(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor, int32_t uid);
    std::shared_ptr<AudioDeviceDescriptor> GetSelectedInputDeviceByUid(int32_t uid);
    void ClearSelectedInputDeviceByUid(int32_t uid);
    void PreferBluetoothAndNearlinkRecordByUid(int32_t uid, bool isPreferred);
    bool GetPreferBluetoothAndNearlinkRecordByUid(int32_t uid);

private:
    AudioUsrSelectManager() {};
    ~AudioUsrSelectManager() {};

    std::unordered_map<pid_t, AudioDeviceList> audioUsrSelectMap_;
    std::unordered_map<pid_t, bool> isPreferredBluetoothAndNearlinkRecordMap_;
    std::mutex mutex_;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_USR_SELECT_MANAGER_H
