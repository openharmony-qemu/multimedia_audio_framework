/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef AUDIO_SASDK_H
#define AUDIO_SASDK_H

namespace OHOS {
namespace AudioStandard {

enum SaSdkAudioVolumeType {
    /**
     * Indicates audio streams of voices in calls.
     */
    SASDK_STREAM_VOICE_CALL = 0,
    /**
     * Indicates audio streams for music.
     */
    SASDK_STREAM_MUSIC = 1,
    /**
     * Indicates audio streams for ringtones.
     */
    SASDK_STREAM_RING = 2,
};

/**
 * @brief The AudioSaSdk class is an abstract definition of audio sasdk abilities.
 */
class AudioSaSdk {
public:
    static AudioSaSdk *GetInstance();

    AudioSaSdk();

    ~AudioSaSdk();

    /**
     * @brief Is stream active.
     *
     * @param volumeType audio volume type.
     * @return Returns <b>true</b> if the rendering is successfully started; returns <b>false</b> otherwise.
     * @since 11
     */
    bool IsStreamActive(SaSdkAudioVolumeType volumeType);
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SASDK_H
