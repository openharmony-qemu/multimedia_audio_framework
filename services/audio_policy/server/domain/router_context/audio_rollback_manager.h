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
#ifndef AUDIO_ROLLBACK_MANAGER_H
#define AUDIO_ROLLBACK_MANAGER_H

#include "audio_router_infra.h"

namespace OHOS {
namespace AudioStandard {

enum RollbackType {
    NEW_CONNECT_ROLLBACK,
    CONNECT_LOST_ROLLBACK,
};

struct RollbackInfo {
    RollbackType type{NEW_CONNECT_ROLLBACK};
    int32_t deviceId{0};
    std::vector<Selection> selections;
};

struct SelectionRollback {
    int32_t targetId{0};
    Selection selection;
};

class AudioRollbackManager {
public:
    static AudioRollbackManager& GetInstance();

    void OnDeviceNewConnect(const AudioDeviceDescriptor &desc);
    void OnDeviceDisconnect(const AudioDeviceDescriptor &desc);
    void OnConnectStateUpdate(const AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command);

    void SetMediaInputDevice(int32_t uid, uint32_t streamId,
        const std::shared_ptr<AudioDeviceDescriptor> &device);
    void SetMediaOutputDevice(int32_t uid, uint32_t streamId,
        const std::shared_ptr<AudioDeviceDescriptor> &device);
    void SetCallInputDevice(int32_t uid, uint32_t streamId,
        const std::shared_ptr<AudioDeviceDescriptor> &device);
    void SetCallOutputDevice(int32_t uid, uint32_t streamId,
        const std::shared_ptr<AudioDeviceDescriptor> &device, const std::string caller = "");
private:
    AudioRollbackManager() = default;
    AudioRollbackManager(const AudioRollbackManager &oth) = delete;
    AudioRollbackManager(AudioRollbackManager &&src) = delete;
    AudioRollbackManager& operator=(const AudioRollbackManager &oth) = delete;
    AudioRollbackManager& operator=(AudioRollbackManager &&src) = delete;

    void SetSelectionRollback(int32_t uid, SelectDeviceType type,
        uint32_t streamId, const std::shared_ptr<AudioDeviceDescriptor> &device);
    void SetConnectLostRollback(const std::shared_ptr<AudioDeviceDescriptor> &device);
    void RollbackSelection(const Selection &selection);
    bool RollbackSelections(int32_t deviceId, RollbackType type);

    std::mutex mutex_;
    std::vector<SelectionRollback> selectionRollbacks_;
    std::unique_ptr<RollbackInfo> rollbackInfo_;
};

}
}
#endif
