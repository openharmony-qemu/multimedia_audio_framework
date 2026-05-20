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

#ifndef AUDIO_LOOPBACK_WRAPPER_H
#define AUDIO_LOOPBACK_WRAPPER_H

#include <memory>
#include <mutex>

#include "audio_errors.h"
#include "audio_loopback.h"
#include "audio_loopback_private.h"
#include "audio_policy_manager.h"
#include "iloopback.h"
#include "loopback_callback_stub.h"

namespace OHOS {
namespace AudioStandard {

class LoopbackCallback : public LoopbackCallbackStub {
public:
    explicit LoopbackCallback(const std::shared_ptr<ILoopbackCB> &cb) : cb_(cb) {}
    ~LoopbackCallback() = default;

    int32_t OnCommandResult(int32_t command, bool success, int32_t errorCode) override
    {
        auto cb = cb_.lock();
        if (cb != nullptr) {
            cb->OnCommandResult(command, success, errorCode);
        }
        return SUCCESS;
    }

    int32_t OnStatusChange(int32_t status, int32_t reason) override
    {
        auto cb = cb_.lock();
        if (cb != nullptr) {
            cb->OnStatusChange(status, reason);
        }
        return SUCCESS;
    }
private:
    std::weak_ptr<ILoopbackCB> cb_;
};

class AudioLoopbackWrapper : public AudioLoopback, public ILoopbackCB,
    public std::enable_shared_from_this<AudioLoopbackWrapper> {
public:
    AudioLoopbackWrapper(AudioLoopbackMode mode, LoopbackType type, const AppInfo &appInfo);
    ~AudioLoopbackWrapper();

    int32_t Init();

    bool Enable(bool enable) override;
    AudioLoopbackStatus GetStatus() override;
    int32_t SetVolume(float volume) override;
    float GetVolume() override;
    int32_t SetAudioLoopbackCallback(const std::shared_ptr<AudioLoopbackCallback> &callback) override;
    int32_t RemoveAudioLoopbackCallback() override;
    bool SetReverbPreset(AudioLoopbackReverbPreset preset) override;
    AudioLoopbackReverbPreset GetReverbPreset() override;
    bool SetEqualizerPreset(AudioLoopbackEqualizerPreset preset) override;
    AudioLoopbackEqualizerPreset GetEqualizerPreset() override;
    std::vector<AudioDevicePair> GetSupportedDevicePairs() override;
    AudioDevicePair GetPreferredDevicePair() override;

    void OnCommandResult(int command, bool success, int errorCode) override;
    void OnStatusChange(int status, int reason) override;

    void SetDebugKey(uint32_t key) { debugKey_ = key; }
    uint32_t GetDebugKey() const override { return debugKey_; }

private:
    AudioLoopbackMode mode_;
    LoopbackType type_;
    AppInfo appInfo_;

    sptr<ILoopback> loopbackProxy_ = nullptr;
    sptr<LoopbackCallback> callbackStub_ = nullptr;
    std::shared_ptr<AudioLoopbackPrivate> private_;

    std::shared_ptr<AudioLoopbackCallback> userCallback_;
    std::mutex callbackMutex_;
    uint32_t debugKey_ = 0;

    int32_t CreateLoopbackIpc();
    int32_t EnableNormal(bool enable);
    int32_t EnableHandler(bool enable);
    int32_t EnableControl(bool enable);
};

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_LOOPBACK_WRAPPER_H