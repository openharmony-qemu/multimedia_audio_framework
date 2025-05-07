/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#ifndef ST_AUDIO_CONCURRENCY_SERVICE_H
#define ST_AUDIO_CONCURRENCY_SERVICE_H
#include <mutex>

#include "iremote_object.h"

#include "audio_policy_log.h"
#include "audio_concurrency_callback.h"
#include "i_audio_concurrency_event_dispatcher.h"
#include "audio_concurrency_parser.h"
#include "audio_policy_server_handler.h"

namespace OHOS {
namespace AudioStandard {

class AudioConcurrencyService : public std::enable_shared_from_this<AudioConcurrencyService>,
                                public IAudioConcurrencyEventDispatcher {
public:
    AudioConcurrencyService()
    {
        AUDIO_INFO_LOG("ctor");
    }
    virtual ~AudioConcurrencyService()
    {
        AUDIO_ERR_LOG("dtor");
    }
    void Init();
    void DispatchConcurrencyEventWithSessionId(uint32_t sessionID) override;
    int32_t SetAudioConcurrencyCallback(const uint32_t sessionID, const sptr<IRemoteObject> &object);
    int32_t UnsetAudioConcurrencyCallback(const uint32_t sessionID);
    void SetCallbackHandler(std::shared_ptr<AudioPolicyServerHandler> handler);
    int32_t ActivateAudioConcurrency(AudioPipeType incomingPipeType,
        const std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos,
        const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos);
    int32_t ActivateAudioConcurrencyExt(AudioPipeType incomingPipeType);
    int32_t ActivateOffloadConcurrencyExt();
    int32_t ActivateFastConcurrencyExt();
private:
    // Inner class for death handler
    class AudioConcurrencyDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit AudioConcurrencyDeathRecipient(
            const std::shared_ptr<AudioConcurrencyService> &service, uint32_t sessionID);
        virtual ~AudioConcurrencyDeathRecipient() = default;

        DISALLOW_COPY_AND_MOVE(AudioConcurrencyDeathRecipient);

        void OnRemoteDied(const wptr<IRemoteObject> &remote);

    private:
        const std::weak_ptr<AudioConcurrencyService> service_;
        const uint32_t sessionID_;
    };
    // Inner class for callback
    class AudioConcurrencyClient {
    public:
        explicit AudioConcurrencyClient(const std::shared_ptr<AudioConcurrencyCallback> &callback,
            const sptr<IRemoteObject> &object, const sptr<AudioConcurrencyDeathRecipient> &deathRecipient,
            uint32_t sessionID);
        virtual ~AudioConcurrencyClient();

        DISALLOW_COPY_AND_MOVE(AudioConcurrencyClient);

        void OnConcedeStream();

    private:
        const std::shared_ptr<AudioConcurrencyCallback> callback_;
        const sptr<IRemoteObject> object_;
        sptr<AudioConcurrencyDeathRecipient> deathRecipient_;
        const uint32_t sessionID_;
    };

private:
    bool CheckFastActivatedState();
    bool CheckOffloadActivatedState();
    bool fastActivated_ = false;
    int64_t lastFastActivedTime_ = 0;
    bool offloadActivated_ = false;
    int64_t lastOffloadActivedTime_ = 0;
    std::map<int32_t /*sessionId*/, std::shared_ptr<AudioConcurrencyClient>> concurrencyClients_ = {};
    std::map<std::pair<AudioPipeType, AudioPipeType>, ConcurrencyAction> concurrencyCfgMap_ = {};
    std::shared_ptr<AudioPolicyServerHandler> handler_;
    std::mutex cbMapMutex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif