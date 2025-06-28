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

#ifndef AUDIO_RESOURCE_SERVICE_H
#define AUDIO_RESOURCE_SERVICE_H

#include <cstdint>
#include <unordered_map>
#include <memory>
#include <cstdio>
#include <thread>
 
#include "audio_workgroup.h"
#include "audio_common_log.h"

namespace OHOS {
namespace AudioStandard {

class AudioResourceService {
public:
    explicit AudioResourceService();
    ~AudioResourceService();

    int32_t CreateAudioWorkgroup(int32_t pid, const sptr<IRemoteObject> &object);
    int32_t ReleaseAudioWorkgroup(int32_t pid, int32_t workgroupId);
    int32_t AddThreadToGroup(int32_t pid, int32_t workgroupId, int32_t tokenId);
    int32_t RemoveThreadFromGroup(int32_t pid, int32_t workgroupId, int32_t tokenId);
    int32_t StartGroup(int32_t pid, int32_t workgroupId, uint64_t startTime, uint64_t deadlineTime);
    int32_t StopGroup(int32_t pid, int32_t workgroupId);
    AudioWorkgroup *GetAudioWorkgroupPtr(int32_t pid, int32_t workgroupId);
    int32_t CreateAudioWorkgroupCheck(int32_t pid);
    void RegisterAudioWorkgroupDeathRecipient(pid_t pid);
    void OnWorkgroupRemoteDied(const std::shared_ptr<AudioWorkgroup> &workgroup,
                                const sptr<IRemoteObject> &remoteObj);
    void ReleaseWorkgroupDeathRecipient(const std::shared_ptr<AudioWorkgroup> &workgroup,
                                                         const sptr<IRemoteObject> &remoteObj);
 
    // Inner class for death handler
    class AudioWorkgroupDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit AudioWorkgroupDeathRecipient();
        virtual ~AudioWorkgroupDeathRecipient() = default;
        DISALLOW_COPY_AND_MOVE(AudioWorkgroupDeathRecipient);
        void OnRemoteDied(const wptr<IRemoteObject> &remote);
 
        using NotifyCbFunc = std::function<void()>;
        void SetNotifyCb(NotifyCbFunc func);
    private:
        NotifyCbFunc diedCb_ = nullptr;
    };
private:
    std::unordered_map<int32_t, std::unordered_map<int32_t, std::shared_ptr<AudioWorkgroup>>> audioWorkgroupMap;
    std::mutex workgroupLock_;
    std::unordered_map<std::shared_ptr<AudioWorkgroup>,
        std::pair<sptr<IRemoteObject>, sptr<AudioWorkgroupDeathRecipient>>> deathRecipientMap_;
};

} // namespace AudioStandard
} // namespace OHOS
#endif
