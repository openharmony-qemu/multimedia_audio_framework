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
#ifndef ST_AUDIO_APP_VOLUME_MANAGER_H
#define ST_AUDIO_APP_VOLUME_MANAGER_H

#include <memory>
#include <atomic>
#include "async_action_handler.h"
#include "audio_policy_server_handler.h"

namespace OHOS {
namespace AudioStandard {

class IAudioPolicyInterface;

class AppVolumeManager {
public:
    AppVolumeManager();
    ~AppVolumeManager();

    void SetAsyncActionHandler(std::shared_ptr<AsyncActionHandler> asyncHandler);

    int32_t GetAppVolumeLevel(int32_t appUid, int32_t &volumeLevel);
    int32_t SetAppVolumeLevel(int32_t appUid, int32_t volumeLevel);
    int32_t SetSelfAppVolumeLevelWithCallback(int32_t appUid, int32_t volumeLevel, bool isUpdateUi);
    int32_t SetAppVolumeMuted(int32_t appUid, bool muted);
    int32_t IsAppVolumeMute(int32_t appUid, bool owned, bool &isMute);
    int32_t SetSystemAppVolumePercentage(int32_t appUid, int32_t volumePercentage);
    int32_t GetSystemAppVolumePercentage(int32_t appUid, int32_t &volumePercentage);
    int32_t SetSystemAppVolumePercentageWithCallback(int32_t appUid, int32_t volumePercentage);
    int32_t SetSystemAppVolumeMuted(int32_t appUid, bool muted);
    int32_t IsSystemAppVolumeMuted(int32_t appUid, bool &isMute);
    int32_t SetSystemAppVolumeMutedForUid(int32_t appUid, bool muted);
    int32_t SetAppRingMuted(int32_t appUid, bool muted);
    bool IsAppRingMuted(int32_t appUid);
    void FlushSystemAppVolume();

private:
    void ScheduleSystemAppVolumePersist();

    IAudioPolicyInterface& audioPolicyManager_;
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler_;
    std::shared_ptr<AsyncActionHandler> asyncHandler_;

    std::atomic<bool> systemAppVolumeDirty_ {false};
    std::atomic<bool> systemAppVolumeFlushScheduled_ {false};
};

}
}

#endif
