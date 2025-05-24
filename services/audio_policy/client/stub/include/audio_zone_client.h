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

#ifndef AUDIO_ZONE_CLIENT_H
#define AUDIO_ZONE_CLIENT_H

#include "i_standard_audio_zone_client.h"
#include "audio_zone_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioZoneClientStub : public IRemoteStub<IStandardAudioZoneClient> {
public:
    AudioZoneClientStub();
    ~AudioZoneClientStub();

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option) override;

private:
    void HandleAudioZoneAdd(MessageParcel &data, MessageParcel &reply);
    void HandleAudioZoneRemove(MessageParcel &data, MessageParcel &reply);
    void HandleAudioZoneChange(MessageParcel &data, MessageParcel &reply);
    void HandleAudioZoneInterrupt(MessageParcel &data, MessageParcel &reply);
    void HandleAudioZoneDeviceInterrupt(MessageParcel &data, MessageParcel &reply);
    void HandleAudioZoneSetSystemVolume(MessageParcel &data, MessageParcel &reply);
    void HandleAudioZoneGetSystemVolume(MessageParcel &data, MessageParcel &reply);
};

class AudioZoneClient : public AudioZoneClientStub {
public:
    AudioZoneClient();
    ~AudioZoneClient();

    int32_t AddAudioZoneCallback(const std::shared_ptr<AudioZoneCallback> &callback);
    void RemoveAudioZoneCallback();

    int32_t AddAudioZoneChangeCallback(int32_t zoneId, const std::shared_ptr<AudioZoneChangeCallback> &callback);
    void RemoveAudioZoneChangeCallback(int32_t zoneId);

    int32_t AddAudioZoneVolumeProxy(int32_t zoneId, const std::shared_ptr<AudioZoneVolumeProxy> &proxy);
    void RemoveAudioZoneVolumeProxy(int32_t zoneId);

    int32_t AddAudioInterruptCallback(int32_t zoneId, const std::shared_ptr<AudioZoneInterruptCallback> &callback);
    int32_t AddAudioInterruptCallback(int32_t zoneId, const std::string &deviceTag,
        const std::shared_ptr<AudioZoneInterruptCallback> &callback);
    void RemoveAudioInterruptCallback(int32_t zoneId);
    void RemoveAudioInterruptCallback(int32_t zoneId, const std::string &deviceTag);

    void Restore();

private:
        std::shared_ptr<AudioZoneCallback> audioZoneCallback_;
        mutable std::mutex audioZoneCallbackMutex_;
        std::unordered_map<int32_t, std::shared_ptr<AudioZoneChangeCallback>> audioZoneChangeCallbackMap_;
        mutable std::mutex audioZoneChangeMutex_;
        std::unordered_map<int32_t, std::shared_ptr<AudioZoneVolumeProxy>> audioZoneVolumeProxyMap_;
        mutable std::mutex audioZoneVolumeProxyMutex_;
        std::unordered_map<std::string, std::shared_ptr<AudioZoneInterruptCallback>> audioZoneInterruptCallbackMap_;
        mutable std::mutex audioZoneInterruptMutex_;

        std::string GetInterruptKeyId(int32_t zoneId, const std::string &deviceTag);

        void OnAudioZoneAdd(const AudioZoneDescriptor &zoneDescriptor) override;
        void OnAudioZoneRemove(int32_t zoneId) override;
        void OnAudioZoneChange(int32_t zoneId, const AudioZoneDescriptor &zoneDescriptor,
            AudioZoneChangeReason reason) override;
        void OnInterruptEvent(int32_t zoneId, const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
            AudioZoneInterruptReason reason) override;
        void OnInterruptEvent(int32_t zoneId, const std::string &deviceTag,
            const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
            AudioZoneInterruptReason reason) override;
        int32_t SetSystemVolume(const int32_t zoneId, const AudioVolumeType volumeType,
            const int32_t volumeLevel, const int32_t volumeFlag) override;
        int32_t GetSystemVolume(int32_t zoneId, AudioVolumeType volumeType) override;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_ZONE_CLIENT_H