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

#include "iremote_object.h"

#include "audio_workgroup.h"
#include "audio_common_log.h"
#include "audio_stream_info.h"

namespace OHOS {
namespace AudioStandard {

struct AudioWorkgroupPerProcess {
    std::unordered_map<int32_t, std::shared_ptr<AudioWorkgroup>> groups;
    bool permission;
    bool hasSystemPermission;

    AudioWorkgroupPerProcess()
    {
        groups.clear();
        permission = false;
        hasSystemPermission = false;
    }
    ~AudioWorkgroupPerProcess() {
    }
};

class AudioResourceService {
public:
    static AudioResourceService *GetInstance();
    explicit AudioResourceService();
    ~AudioResourceService();

    int32_t CreateAudioWorkgroup(int32_t pid, const sptr<IRemoteObject> &object);
    int32_t ReleaseAudioWorkgroup(int32_t pid, int32_t workgroupId);
    int32_t AddThreadToGroup(int32_t pid, int32_t workgroupId, int32_t tokenId);
    int32_t RemoveThreadFromGroup(int32_t pid, int32_t workgroupId, int32_t tokenId);
    int32_t StartGroup(int32_t pid, int32_t workgroupId, uint64_t startTime, uint64_t deadlineTime);
    int32_t StopGroup(int32_t pid, int32_t workgroupId);
    void OnWorkgroupRemoteDied(const std::shared_ptr<AudioWorkgroup> &workgroup,
                                const sptr<IRemoteObject> &remoteObj);
    void ReleaseWorkgroupDeathRecipient(const std::shared_ptr<AudioWorkgroup> &workgroup,
                                                         const sptr<IRemoteObject> &remoteObj);
    void WorkgroupRendererMonitor(int32_t pid, const bool isAllowed);
    bool IsProcessInWorkgroup(int32_t pid);
    bool IsProcessHasSystemPermission(int32_t pid);
    void RegisterAudioWorkgroupDeathRecipient(pid_t pid);
    std::vector<int32_t> GetProcessesOfAudioWorkgroup();
    int32_t ImproveAudioWorkgroupPrio(int32_t pid, const std::unordered_map<int32_t, bool> &threads);
    int32_t RestoreAudioWorkgroupPrio(int32_t pid, const std::unordered_map<int32_t, int32_t> &threads);
    void FillAudioWorkgroupCgroupLimit(int32_t pid, std::shared_ptr<AudioWorkgroup>& workgroup);
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
    int32_t AudioWorkgroupCheck(int32_t pid);
    std::shared_ptr<AudioWorkgroup> GetAudioWorkgroup(int32_t pid, int32_t workgroupId);
    int32_t RegisterAudioWorkgroupMonitor(int32_t pid, int32_t groupId, const sptr<IRemoteObject> &object);
    int32_t GetThreadsNumPerProcess(int32_t pid);
    void DumpAudioWorkgroupMap();

    std::mutex workgroupLock_;
    std::unordered_map<int32_t, struct AudioWorkgroupPerProcess> audioWorkgroupMap_;
    std::unordered_map<std::shared_ptr<AudioWorkgroup>,
        std::pair<sptr<IRemoteObject>, sptr<AudioWorkgroupDeathRecipient>>> deathRecipientMap_;
};

} // namespace AudioStandard
} // namespace OHOS
#endif
