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

#ifndef MULTIMEDIA_AUDIO_CAPTURER_IMPL_H
#define MULTIMEDIA_AUDIO_CAPTURER_IMPL_H
#include <list>
#include "cj_common_ffi.h"
#include "native/ffi_remote_data.h"
#include "audio_capturer.h"
#include "multimedia_audio_capturer_callback.h"
#include "multimedia_audio_ffi.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
class MMAAudioCapturerImpl : public OHOS::FFI::FFIData {
    DECL_TYPE(MMAAudioCapturerImpl, OHOS::FFI::FFIData)
public:
    MMAAudioCapturerImpl();

    ~MMAAudioCapturerImpl();

    int32_t CreateAudioCapturer(CAudioCapturerOptions options);

    int32_t GetState();

    uint32_t GetStreamId(int32_t *errorCode);

    int64_t GetAudioTime(int32_t *errorCode);

    uint32_t GetBufferSize(int32_t *errorCode);

    uint32_t GetOverflowCount();

    int32_t Start();

    int32_t Stop();

    int32_t Release();

    CAudioCapturerInfo GetCurrentCapturerInfo(int32_t *errorCode);

    CAudioStreamInfo GetStreamInfo(int32_t *errorCode);

    CAudioCapturerChangeInfo GetAudioCapturerChangeInfo(int32_t *errorCode);

    CArrDeviceDescriptor GetInputDevices(int32_t *errorCode);

    void RegisterCallback(int32_t callbackType, void (*callback)(), int32_t *errorCode);

    void RegisterCallbackWithFrame(int32_t callbackType, void (*callback)(), int64_t frame, int32_t *errorCode);

private:
    void RegisterCArrCallback(int32_t callbackType, void (*callback)(), int32_t *errorCode);
    std::shared_ptr<AudioCapturer> audioCapturer_{};
    std::shared_ptr<CjAudioCapturerCallback> callback_{};
    std::shared_ptr<CjCapturerPositionCallback> positionCb_{};
    std::shared_ptr<CjCapturerPeriodPositionCallback> periodPositionCb_{};
    std::shared_ptr<CjAudioCapturerReadCallback> capturerReadDataCb_{};
    std::list<std::shared_ptr<CjAudioCapturerDeviceChangeCallback>> deviceChangeCallbacks_;
    std::list<std::shared_ptr<CjAudioCapturerInfoChangeCallback>> capturerInfoChangeCallbacks_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // MULTIMEDIA_AUDIO_CAPTURER_IMPL_H
