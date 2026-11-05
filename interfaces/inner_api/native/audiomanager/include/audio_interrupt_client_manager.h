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

#ifndef AUDIO_INTERRUPT_CLIENT_MANAGER_H
#define AUDIO_INTERRUPT_CLIENT_MANAGER_H
#include "audio_interrupt_types.h"

namespace OHOS {
namespace AudioStandard {
class AudioManagerInterruptCallbackImpl : public AudioInterruptCallback {
public:
    explicit AudioManagerInterruptCallbackImpl();
    virtual ~AudioManagerInterruptCallbackImpl();
    
    /**
     * Called when an interrupt is received.
     *
     * @param interruptAction Indicates the InterruptAction information needed by client.
     * For details, refer InterruptAction struct in audio_info.h
     * @since 7
    */
    void OnInterrupt(const InterruptEventInternal &interruptEvent) override;
    void SaveCallback(const std::weak_ptr<AudioManagerCallback> &callback);
private:
    std::weak_ptr<AudioManagerCallback> callback_;
    std::shared_ptr<AudioManagerCallback> cb_;
};

class AudioInterruptClientManager {
public:
    static AudioInterruptClientManager &GetInstance();

    /**
     * @brief Get audio focus info
     *
     * @return Returns success or not
     * @since 10
     */
    int32_t GetAudioFocusInfoList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList);

    /**
     * @brief Register callback to listen audio focus info change event
     *
     * @return Returns success or not
     * @since 10
     */
    int32_t RegisterFocusInfoChangeCallback(const std::shared_ptr<AudioFocusInfoChangeCallback> &callback);

    /**
     * @brief Unregister callback to listen audio focus info change event
     *
     * @return Returns success or not
     * @since 10
     */
    int32_t UnregisterFocusInfoChangeCallback(
        const std::shared_ptr<AudioFocusInfoChangeCallback> &callback = nullptr);

    /**
     * @brief Activate audio Interrupt
     *
     * @param audioInterrupt audioInterrupt
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t ActivateAudioInterrupt(AudioInterrupt &audioInterrupt);

    /**
     * @brief Set App Concurrency Mode
     *
     * @param appUid app Uid
     * @param mode concurrency Mode
     * @return Returns {@link SUCCESS} if seting is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 20
     */
    int32_t SetAppConcurrencyMode(const int32_t appUid, const int32_t mode);

    /**
     * @brief Set duck passthrough mode
     *
     * @param enable True to enable duck passthrough mode, false to disable.
     * @return Returns {@link SUCCESS} if successful; returns an error code otherwise.
     * @since 26.0.0
     */
    int32_t SetDuckPassthroughMode(bool enable);

    /**
     * @brief Set the duck factor used when sending DUCK interrupt callbacks.
     *
     * @param duckFactor The duck volume factor percentage, range [0, 100].
     * @return Returns {@link SUCCESS} if successful; returns an error code otherwise.
     * @since 26.0.0
     */
    int32_t SetDuckFactor(int32_t duckFactor);

    /**
     * @brief Start voip stream by uid
     *
     * @param appUid app Uid
     * @return Returns {@link SUCCESS} if successful; returns an error code otherwise.
     * @since 26.1.0
     */
    int32_t StartVoipStreamByUid(const int32_t appUid);

    /**
     * @brief Deactivactivate audio Interrupt
     *
     * @param audioInterrupt audioInterrupt
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt) const;

    /**
     * @brief registers the Interrupt callback listener
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t SetAudioManagerInterruptCallback(const std::shared_ptr<AudioManagerCallback> &callback);

    /**
     * @brief Unregisters the Interrupt callback listener
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t UnsetAudioManagerInterruptCallback();

    /**
     * @brief Request audio focus
     *
     * @param audioInterrupt audioInterrupt
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t RequestAudioFocus(const AudioInterrupt &audioInterrupt);

    /**
     * @brief Abandon audio focus
     *
     * @param audioInterrupt audioInterrupt
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 8
     */
    int32_t AbandonAudioFocus(const AudioInterrupt &audioInterrupt);

    /**
     * @brief Request independent interrupt
     *
     * @param focusType focus type
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 8
     */
    bool RequestIndependentInterrupt(FocusType focusType);

    /**
     * @brief Abandon independent interrupt
     *
     * @param focusType focus type
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 8
     */
    bool AbandonIndependentInterrupt(FocusType focusType);

    /**
     * @brief inject interruption event.
     *
     * @param networkId networkId.
     * @param event Indicates the InterruptEvent information needed by client.
     * For details, refer InterruptEvent struct in audio_interrupt_info.h
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     * @since 12
     */
    int32_t InjectInterruption(const std::string networkId, InterruptEvent &event);

    int32_t SetAudioInterruptCallback(const uint32_t sessionID, const std::shared_ptr<AudioInterruptCallback> &callback,
        uint32_t clientUid, const int32_t zoneID);
    int32_t UnsetAudioInterruptCallback(const int32_t zoneId, const uint32_t sessionId);
private:
    AudioInterruptClientManager() = default;
    ~AudioInterruptClientManager() = default;

    std::shared_ptr<AudioInterruptCallback> audioInterruptCallback_ = nullptr;
    std::shared_ptr<AudioFocusInfoChangeCallback> audioFocusInfoCallback_ = nullptr;
    std::mutex audioFocusInfoCallbackMutex_;
    std::mutex audioInterruptCallbackMutex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_INTERRUPT_CLIENT_MANAGER_H
