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

#ifndef AUDIO_DEBUG_MANAGER_PRIVATE_H
#define AUDIO_DEBUG_MANAGER_PRIVATE_H

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "audio_debug_callback.h"
#include "audio_debug_info.h"
#include "audio_debug_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioDebugManagerPrivate final : public AudioDebugManager {
public:
    AudioDebugManagerPrivate() = default;
    ~AudioDebugManagerPrivate() override = default;

    int32_t PrintAppAudioDebugInfo(int32_t fd) const override;

    int32_t PrintAllAudioRenderersDebugInfo(int32_t fd) const override;
    int32_t PrintAllAudioCapturersDebugInfo(int32_t fd) const override;
    int32_t PrintAllAudioLoopbacksDebugInfo(int32_t fd) const override;

    int32_t RegisterAudioRenderer(uintptr_t rendererKey,
        const std::shared_ptr<AudioRendererDebugCallback> &debugCallback) override;
    int32_t UnregisterAudioRenderer(uintptr_t rendererKey) override;
    int32_t PrintAudioRendererDebugInfo(uintptr_t rendererKey, int32_t fd) const override;

    int32_t RegisterAudioCapturer(uintptr_t capturerKey,
        const std::shared_ptr<AudioCapturerDebugCallback> &debugCallback) override;
    int32_t UnregisterAudioCapturer(uintptr_t capturerKey) override;
    int32_t PrintAudioCapturerDebugInfo(uintptr_t capturerKey, int32_t fd) const override;

    int32_t RegisterAudioLoopback(uint32_t &loopbackKey,
        const std::shared_ptr<AudioLoopbackDebugCallback> &debugCallback) override;
    int32_t UnregisterAudioLoopback(uint32_t loopbackKey) override;
    int32_t PrintAudioLoopbackDebugInfo(uint32_t loopbackKey, int32_t fd) const override;

    int32_t SetAudioSessionCallback(
        const std::shared_ptr<AudioSessionDebugCallback> &debugCallback) override;
    int32_t PrintAudioSessionDebugInfo(int32_t fd) const override;

private:
    static void LogAudioDebugInfo(const std::string &debugInfo);
    static int32_t WriteAudioDebugInfo(int32_t fd, const std::string &debugInfo);
    static int32_t PrintfAudioDebugInfo(int32_t fd, const std::string &debugInfo);

    int32_t GetAudioRendererDebugInfo(uintptr_t rendererKey, AudioRendererDebugInfo &debugInfo) const;
    static std::string ParserAudioRendererDebugInfo(const AudioRendererDebugInfo &debugInfo);

    int32_t GetAudioCapturerDebugInfo(uintptr_t capturerKey, AudioCapturerDebugInfo &debugInfo) const;
    static std::string ParserAudioCapturerDebugInfo(const AudioCapturerDebugInfo &debugInfo);

    int32_t GetAudioLoopbackDebugInfo(uint32_t loopbackKey, AudioLoopbackDebugInfo &debugInfo) const;
    static std::string ParserAudioLoopbackDebugInfo(const AudioLoopbackDebugInfo &debugInfo);

    int32_t GetAudioSessionDebugInfo(AudioSessionDebugInfo &debugInfo) const;
    static std::string ParserAudioSessionDebugInfo(const AudioSessionDebugInfo &debugInfo);

    uint32_t GenerateDebugKey();

private:
    mutable std::mutex rendererMutex_;
    mutable std::mutex capturerMutex_;
    mutable std::mutex loopbackMutex_;
    mutable std::mutex sessionMutex_;
    std::map<uintptr_t, std::shared_ptr<AudioRendererDebugCallback>> audioRendererDebugCallbacks_;
    std::map<uintptr_t, std::shared_ptr<AudioCapturerDebugCallback>> audioCapturerDebugCallbacks_;
    std::map<uint32_t, std::shared_ptr<AudioLoopbackDebugCallback>> audioLoopbackDebugCallbacks_;
    std::shared_ptr<AudioSessionDebugCallback> audioSessionDebugCallback_;
    std::atomic<uint32_t> debugKeyCounter_ = DEBUG_KEY_START;
    static constexpr uint32_t DEBUG_KEY_START = 1000;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_DEBUG_MANAGER_PRIVATE_H
