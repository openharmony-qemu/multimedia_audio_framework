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

#ifndef AUDIO_DEBUG_MANAGER_H
#define AUDIO_DEBUG_MANAGER_H

#include <cstdint>
#include <memory>

namespace OHOS {
namespace AudioStandard {

class AudioRendererDebugCallback;
class AudioCapturerDebugCallback;
class AudioLoopbackDebugCallback;
class AudioSessionDebugCallback;

class AudioDebugManager {
public:
    static AudioDebugManager &GetInstance();

    virtual int32_t PrintAppAudioDebugInfo(int32_t fd) const = 0;

    virtual int32_t PrintAllAudioRenderersDebugInfo(int32_t fd) const = 0;
    virtual int32_t PrintAllAudioCapturersDebugInfo(int32_t fd) const = 0;
    virtual int32_t PrintAllAudioLoopbacksDebugInfo(int32_t fd) const = 0;

    virtual int32_t RegisterAudioRenderer(uintptr_t rendererKey,
        const std::shared_ptr<AudioRendererDebugCallback> &debugCallback) = 0;
    virtual int32_t UnregisterAudioRenderer(uintptr_t rendererKey) = 0;
    virtual int32_t PrintAudioRendererDebugInfo(uintptr_t rendererKey, int32_t fd) const = 0;

    virtual int32_t RegisterAudioCapturer(uintptr_t capturerKey,
        const std::shared_ptr<AudioCapturerDebugCallback> &debugCallback) = 0;
    virtual int32_t UnregisterAudioCapturer(uintptr_t capturerKey) = 0;
    virtual int32_t PrintAudioCapturerDebugInfo(uintptr_t capturerKey, int32_t fd) const = 0;

    virtual int32_t RegisterAudioLoopback(uint32_t &loopbackKey,
        const std::shared_ptr<AudioLoopbackDebugCallback> &debugCallback) = 0;
    virtual int32_t UnregisterAudioLoopback(uint32_t loopbackKey) = 0;
    virtual int32_t PrintAudioLoopbackDebugInfo(uint32_t loopbackKey, int32_t fd) const = 0;

    virtual int32_t SetAudioSessionCallback(
        const std::shared_ptr<AudioSessionDebugCallback> &debugCallback) = 0;
    virtual int32_t PrintAudioSessionDebugInfo(int32_t fd) const = 0;

    virtual ~AudioDebugManager() = default;

protected:
    AudioDebugManager() = default;

private:
    AudioDebugManager(const AudioDebugManager &) = delete;
    AudioDebugManager &operator=(const AudioDebugManager &) = delete;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_DEBUG_MANAGER_H
