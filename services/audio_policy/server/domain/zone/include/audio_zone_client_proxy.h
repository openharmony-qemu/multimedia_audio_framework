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

#ifndef ST_AUDIO_ZONE_CLIENT_PROXY_H
#define ST_AUDIO_ZONE_CLIENT_PROXY_H

#include "i_standard_audio_zone_client.h"

namespace OHOS {
namespace AudioStandard {
class AudioZoneClientProxy : public IRemoteProxy<IStandardAudioZoneClient> {
public:
    explicit AudioZoneClientProxy(const sptr<IRemoteObject> &impl);
    virtual ~AudioZoneClientProxy();

    void OnAudioZoneAdd(const AudioZoneDescriptor &zoneDescriptor) override;
    void OnAudioZoneRemove(int32_t zoneId) override;
    void OnAudioZoneChange(int32_t zoneId, const AudioZoneDescriptor &zoneDescriptor,
        AudioZoneChangeReason reason) override;
    void OnInterruptEvent(int32_t zoneId,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
        AudioZoneInterruptReason reason) override;
    void OnInterruptEvent(int32_t zoneId, const std::string &deviceTag,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
        AudioZoneInterruptReason reason) override;
    int32_t SetSystemVolume(const int32_t zoneId, const AudioVolumeType volumeType,
        const int32_t volumeLevel, const int32_t volumeFlag) override;
    int32_t GetSystemVolume(int32_t zoneId, AudioVolumeType volumeType) override;

private:
    static inline BrokerDelegator<AudioZoneClientProxy> delegator_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_ZONE_CLIENT_PROXY_H