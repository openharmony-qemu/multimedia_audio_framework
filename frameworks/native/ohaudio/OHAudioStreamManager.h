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

#ifndef OH_AUDIO_STREAM_MANAGER_H
#define OH_AUDIO_STREAM_MANAGER_H

#include "audio_common_log.h"
#include "native_audio_stream_manager.h"
#include "native_audio_common.h"
#include "audio_stream_manager.h"

namespace OHOS {
namespace AudioStandard {

class OHAudioStreamManager {
public:
    ~OHAudioStreamManager();

    static OHAudioStreamManager* GetInstance()
    {
        if (!ohAudioStreamManager_) {
            ohAudioStreamManager_ = new OHAudioStreamManager();
        }
        return ohAudioStreamManager_;
    }

    OH_AudioStream_DirectPlaybackMode GetDirectPlaybackSupport(AudioStreamInfo streamInfo, StreamUsage usage);
    
    bool IsAcousticEchoCancelerSupported(SourceType sourceType);

    bool IsFastPlaybackSupported(AudioStreamInfo &streamInfo, StreamUsage usage);
    bool IsFastRecordingSupported(AudioStreamInfo &streamInfo, SourceType source);

private:
    OHAudioStreamManager();
    static OHAudioStreamManager *ohAudioStreamManager_;
    AudioStreamManager *audioStreamManager_ = AudioStreamManager::GetInstance();
};
OHAudioStreamManager* OHAudioStreamManager::ohAudioStreamManager_ = nullptr;

} // namespace AudioStandard
} // namespace OHOS
#endif // OH_AUDIO_STREAM_MANAGER_H
