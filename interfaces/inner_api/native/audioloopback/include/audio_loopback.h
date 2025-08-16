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

#ifndef AUDIO_LOOPBACK_H
#define AUDIO_LOOPBACK_H

#include "audio_stream_change_info.h"

namespace OHOS {
namespace AudioStandard {


class AudioLoopbackCallback {
public:
    virtual ~AudioLoopbackCallback() = default;

    /**
     * Called when loopback state is updated.
     *
     * @param state Indicates updated state of the loopback.
     * For details, refer AudioLoopbackStatus enum.
     */
    virtual void OnStatusChange(const AudioLoopbackStatus state,
        const StateChangeCmdType cmdType = CMD_FROM_CLIENT) = 0;
};

/**
 * @brief audio loopback
 * @since 20
 */
class AudioLoopback {
public:
    /**
     * @brief  create loopback instance.
     *
     * @param rendererOptions The audio loopback configuration to be used while creating loopback instance.
     * @param appInfo Originating application's uid and token id can be passed here
     * @return Returns shared pointer to the AudioLoopback object
     * @since 20
    */
    static std::shared_ptr<AudioLoopback> CreateAudioLoopback(AudioLoopbackMode mode,
        const AppInfo &appInfo = AppInfo());

    virtual bool Enable(bool enable) = 0;

    virtual AudioLoopbackStatus GetStatus() = 0;

    virtual int32_t SetVolume(float volume) = 0;

    virtual int32_t SetAudioLoopbackCallback(const std::shared_ptr<AudioLoopbackCallback> &callback) = 0;

    virtual int32_t RemoveAudioLoopbackCallback() = 0;

    virtual bool SetReverbPreset(AudioLoopbackReverbPreset preset) = 0;

    virtual AudioLoopbackReverbPreset GetReverbPreset() = 0;

    virtual bool SetEqualizerPreset(AudioLoopbackEqualizerPreset preset) = 0;

    virtual AudioLoopbackEqualizerPreset GetEqualizerPreset() = 0;

    virtual ~AudioLoopback();
};
} // namespace AudioStandard
} // namespace OHOS
#endif