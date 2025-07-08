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

#ifndef ST_AUDIO_ZONE_CLIENT_MANAGER_H
#define ST_AUDIO_ZONE_CLIENT_MANAGER_H

#include "istandard_audio_zone_client.h"
#include "i_audio_zone_event_dispatcher.h"
#include "audio_policy_server_handler.h"

namespace OHOS {
namespace AudioStandard {
class AudioZoneClientManager : public IAudioZoneEventDispatcher {
public:
    explicit AudioZoneClientManager(std::shared_ptr<AudioPolicyServerHandler> handler);
    virtual ~AudioZoneClientManager() = default;

    static AudioZoneClientManager &GetInstance();
    int32_t RegisterAudioZoneClient(pid_t clientPid, sptr<IStandardAudioZoneClient> client);
    void UnRegisterAudioZoneClient(pid_t clientPid);
    bool IsRegisterAudioZoneClient(pid_t clientPid);
    
    void DispatchEvent(std::shared_ptr<AudioZoneEvent> event) override;

    void SendZoneAddEvent(pid_t clientPid, std::shared_ptr<AudioZoneDescriptor> descriptor);
    void SendZoneRemoveEvent(pid_t clientPid, int32_t zoneId);
    void SendZoneChangeEvent(pid_t clientPid, std::shared_ptr<AudioZoneDescriptor> descriptor,
        AudioZoneChangeReason reason);
    void SendZoneInterruptEvent(pid_t clientPid, int32_t zoneId, const std::string &deviceTag,
        std::list<std::pair<AudioInterrupt, AudioFocuState>> interrupts,
        AudioZoneInterruptReason reason);

    int32_t SetSystemVolumeLevel(const pid_t clientPid, const int32_t zoneId,
        const AudioVolumeType volumeType, const int32_t volumeLevel, const int32_t volumeFlag = 0);
    int32_t GetSystemVolumeLevel(const pid_t clientPid, const int32_t zoneId, AudioVolumeType volumeType);

private:
    std::unordered_map<pid_t, sptr<IStandardAudioZoneClient>> clients_;
    std::mutex clientMutex_;
    std::shared_ptr<AudioPolicyServerHandler> handler_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_ZONE_CLIENT_MANAGER_H