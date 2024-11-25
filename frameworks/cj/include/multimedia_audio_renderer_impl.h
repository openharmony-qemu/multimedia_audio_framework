/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MULTIMEDIA_AUDIO_RENDERER_IMPL_H
#define MULTIMEDIA_AUDIO_RENDERER_IMPL_H
#include "cj_common_ffi.h"
#include "native/ffi_remote_data.h"
#include "audio_renderer.h"
#include "audio_stream_manager.h"
#include "multimedia_audio_ffi.h"
#include "multimedia_audio_renderer_callback.h"

namespace OHOS {
namespace AudioStandard {
class MMAAudioRendererImpl : public OHOS::FFI::FFIData {
    DECL_TYPE(MMAAudioRendererImpl, OHOS::FFI::FFIData)
public:
    MMAAudioRendererImpl();

    ~MMAAudioRendererImpl();

    int32_t CreateAudioRenderer(CAudioRendererOptions options);

    int32_t GetState();

    int64_t GetAudioTime(int32_t *errorCode);

    uint32_t GetBufferSize(int32_t *errorCode);

    int32_t Flush();

    int32_t Drain();

    int32_t Pause();

    CArrDeviceDescriptor GetCurrentOutputDevices(int32_t *errorCode);

    double GetSpeed(int32_t *errorCode);

    bool GetSilentModeAndMixWithOthers(int32_t *errorCode);

    double GetVolume(int32_t *errorCode);

    uint32_t GetUnderflowCount(int32_t *errorCode);

    void SetVolumeWithRamp(double volume, int32_t duration, int32_t *errorCode);

    void SetSpeed(double speed, int32_t *errorCode);

    void SetVolume(double volume, int32_t *errorCode);

    void SetSilentModeAndMixWithOthers(bool on, int32_t *errorCode);

    void SetInterruptMode(int32_t mode, int32_t *errorCode);

    void SetChannelBlendMode(int32_t mode, int32_t *errorCode);

    void RegisterCallback(int32_t callbackType, void (*callback)(), int32_t *errorCode);

    void RegisterCallbackWithFrame(int32_t callbackType, void (*callback)(), int64_t frame, int32_t *errorCode);

    int32_t GetAudioEffectMode(int32_t *errorCode);

    void SetAudioEffectMode(int32_t mode, int32_t *errorCode);

    double GetMinStreamVolume(int32_t *errorCode);

    double GetMaxStreamVolume(int32_t *errorCode);

    void Release(int32_t *errorCode);

    uint32_t GetStreamId(int32_t *errorCode);

    void Stop(int32_t *errorCode);

    void Start(int32_t *errorCode);

    CAudioStreamInfo GetStreamInfo(int32_t *errorCode);

    CAudioRendererInfo GetRendererInfo(int32_t *errorCode);

private:
    static constexpr double MIN_VOLUME_IN_DOUBLE = 0.0;
    static constexpr double MAX_VOLUME_IN_DOUBLE = 1.0;
    void RegisterOutputDeviceCallback(int32_t callbackType, void (*callback)(), int32_t *errorCode);
    std::shared_ptr<AudioRenderer> audioRenderer_{};
    std::shared_ptr<CjAudioRendererCallback> callback_{};
    std::shared_ptr<CjRendererPositionCallback> positionCb_{};
    std::shared_ptr<CjRendererPeriodPositionCallback> periodPositionCb_{};
    std::shared_ptr<CjAudioRendererWriteCallback> rendererWriteDataCallback_{};
    std::shared_ptr<CjAudioRendererOutputDeviceChangeCallback> rendererDeviceChangeCallback_{};
    std::shared_ptr<CjAudioRendererOutputDeviceChangeWithInfoCallback> rendererOutputDeviceChangeWithInfoCallback_{};
};
} // namespace AudioStandard
} // namespace OHOS
#endif // MULTIMEDIA_AUDIO_RENDERER_IMPL_H
