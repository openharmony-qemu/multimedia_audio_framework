/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_SESSION_MANAGER_H
#define ST_AUDIO_SESSION_MANAGER_H

#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioSessionCallback {
public:
    virtual ~AudioSessionCallback() = default;
    /**
     * @brief OnAudioSessionDeactive will be executed when the audio session is deactivated be others.
     *
     * @param deactiveEvent the audio session deactive event info.
     * @since 12
     */
    virtual void OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent) = 0;
};

class AudioSessionManager {
public:
    AudioSessionManager() = default;
    virtual ~AudioSessionManager() = default;

    static AudioSessionManager *GetInstance();

    /**
     * @brief Activate audio session.
     *
     * @param strategy Target audio session strategy.
     * @return Returns {@link SUCCESS} if the operation is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 12
     */
    int32_t ActivateAudioSession(const AudioSessionStrategy &strategy);

    /**
     * @brief Deactivate audio session.
     *
     * @return Returns {@link SUCCESS} if the operation is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 12
     */
    int32_t DeactivateAudioSession();

    /**
     * @brief Query whether the audio session is active.
     *
     * @return Returns <b>true</b> if the audio session is active; returns <b>false</b> otherwise.
     * @since 12
     */
    bool IsAudioSessionActivated();

    /**
     * @brief Set audio session callback.
     *
     * @param audioSessionCallback The audio session callback.
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 12
     */
    int32_t SetAudioSessionCallback(const std::shared_ptr<AudioSessionCallback> &audioSessionCallback);

    /**
     * @brief Unset all audio session callbacks.
     *
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 12
     */
    int32_t UnsetAudioSessionCallback();

    /**
     * @brief Unset audio session callback.
     *
     * @param audioSessionCallback The audio session callback.
     * @return Returns {@link SUCCESS} if callback registration is successful; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     * @since 12
     */
    int32_t UnsetAudioSessionCallback(const std::shared_ptr<AudioSessionCallback> &audioSessionCallback);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_SESSION_MANAGER_H
