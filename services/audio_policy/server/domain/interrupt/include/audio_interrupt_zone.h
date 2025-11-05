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

#ifndef ST_AUDIO_INTERRUPT_ZONE_H
#define ST_AUDIO_INTERRUPT_ZONE_H

#include <set>
#include <list>
#include <string>
#include <unordered_map>
#include <functional>
#include "audio_interrupt_info.h"
#include "audio_zone_info.h"
#include "audio_interrupt_callback.h"
#include "iaudio_policy_client.h"
#include "i_audio_interrupt_event_dispatcher.h"

namespace OHOS {
namespace AudioStandard {

using GetZoneIdFunc = std::function<int32_t(int32_t uid, const std::string &, const std::string &,
    const StreamUsage &)>;
using AudioFocusList = std::list<std::pair<AudioInterrupt, AudioFocuState>>;
using AudioFocusIterator = std::list<AudioFocusList::iterator>;

typedef struct {
    int32_t zoneId; // Zone ID value should 0 on local device.
    AudioZoneContext context;
    std::set<int32_t> pids; // When Zone ID is 0, there does not need to be a value.
    std::set<uint32_t> interruptCbStreamIdsMap;
    std::set<int32_t> audioPolicyClientProxyCBClientPidMap;
    std::unordered_map<uint32_t /* streamId */, std::shared_ptr<AudioInterruptCallback>> interruptCbsMap;
    std::unordered_map<int32_t /* clientPid */, sptr<IAudioPolicyClient>> audioPolicyClientProxyCBMap;
    AudioFocusList audioFocusInfoList;
} AudioInterruptZone;

class AudioInterruptService;

class AudioInterruptZoneManager {
protected:
    friend class AudioInterruptService;
    AudioInterruptZoneManager();
    virtual ~AudioInterruptZoneManager();

    void InitService(AudioInterruptService *service);
    int32_t CreateAudioInterruptZone(const int32_t zoneId, const AudioZoneContext &context,
        bool checkPermission = true);
    int32_t ReleaseAudioInterruptZone(const int32_t zoneId, GetZoneIdFunc func);
    int32_t MigrateAudioInterruptZone(const int32_t zoneId, GetZoneIdFunc func);
    int32_t InjectInterruptToAudioZone(const int32_t zoneId, const AudioFocusList &interrupts);
    int32_t InjectInterruptToAudioZone(const int32_t zoneId, const std::string &deviceTag,
        const AudioFocusList &interrupts);
    int32_t GetAudioFocusInfoList(const int32_t zoneId, AudioFocusList &focusInfoList);
    int32_t GetAudioFocusInfoList(const int32_t zoneId, const std::string &deviceTag,
        AudioFocusList &focusInfoList);

private:
    bool CheckAudioInterruptZonePermission();
    int32_t FindZoneByPid(int32_t pid);
    void RemoveAudioZoneInterrupts(int32_t zoneId, const AudioFocusIterator &focus);
    void TryActiveAudioFocusForZone(int32_t zoneId, AudioFocusList &activeFocusList);
    void TryResumeAudioFocusForZone(int32_t zoneId);
    AudioFocusIterator QueryAudioFocusFromZone(int32_t zoneId, const std::string &deviceTag);
    void ForceStopAudioFocusInZone(int32_t zoneId, const AudioInterrupt &audioInterrupt);
    void ForceStopAllAudioFocusInZone(std::shared_ptr<AudioInterruptZone> &zone);
    void SendInterruptEventForMigration(const std::pair<AudioInterrupt, AudioFocuState> &audioInterrupt,
        const int32_t toZoneId);

    AudioInterruptService *service_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_INTERRUPT_ZONE_H
