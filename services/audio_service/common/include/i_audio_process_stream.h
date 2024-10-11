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

#ifndef I_AUDIO_PROCESS_STREAM_H
#define I_AUDIO_PROCESS_STREAM_H

#include <memory>

#include "audio_info.h"
#include "oh_audio_buffer.h"

namespace OHOS {
namespace AudioStandard {
class IAudioProcessStream {
public:
    /**
     * Get buffer of client for AudioEndpoint.
    */
    virtual std::shared_ptr<OHAudioBuffer> GetStreamBuffer() = 0;

    virtual AudioStreamInfo GetStreamInfo() = 0;

    virtual uint32_t GetAudioSessionId() = 0;

    virtual AudioStreamType GetAudioStreamType() = 0;

    virtual void SetInnerCapState(bool isInnerCapped) = 0;

    virtual bool GetInnerCapState() = 0;

    virtual AppInfo GetAppInfo() = 0;

    virtual BufferDesc &GetConvertedBuffer() = 0;

    virtual bool GetMuteFlag() = 0;

    virtual ~IAudioProcessStream() = default;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // I_AUDIO_PROCESS_STREAM_H
