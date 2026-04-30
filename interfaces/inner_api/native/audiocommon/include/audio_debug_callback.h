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

#ifndef AUDIO_DEBUG_CALLBACK_H
#define AUDIO_DEBUG_CALLBACK_H

namespace OHOS {
namespace AudioStandard {

struct AudioRendererDebugInfo;
struct AudioCapturerDebugInfo;
struct AudioLoopbackDebugInfo;
struct AudioSessionDebugInfo;
class AudioRendererDebugCallback {
public:
    virtual ~AudioRendererDebugCallback() = default;
    virtual int32_t GetRendererDebugInfo(AudioRendererDebugInfo &debugInfo) = 0;
};

class AudioCapturerDebugCallback {
public:
    virtual ~AudioCapturerDebugCallback() = default;
    virtual int32_t GetCapturerDebugInfo(AudioCapturerDebugInfo &debugInfo) = 0;
};

class AudioLoopbackDebugCallback {
public:
    virtual ~AudioLoopbackDebugCallback() = default;
    virtual int32_t GetLoopbackDebugInfo(AudioLoopbackDebugInfo &debugInfo) = 0;
};

class AudioSessionDebugCallback {
public:
    virtual ~AudioSessionDebugCallback() = default;
    virtual int32_t GetSessionDebugInfo(AudioSessionDebugInfo &debugInfo) = 0;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_DEBUG_CALLBACK_H
