/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef MULTIMEDIA_AUDIO_STREAM_MANAGER_IMPL_H
#define MULTIMEDIA_AUDIO_STREAM_MANAGER_IMPL_H
#include "cj_common_ffi.h"
#include "native/ffi_remote_data.h"
#include "audio_policy_interface.h"
#include "audio_stream_manager.h"
#include "audio_system_manager.h"
#include "multimedia_audio_ffi.h"
#include "multimedia_audio_stream_manager_callback.h"

namespace OHOS {
namespace AudioStandard {
class MMAAudioStreamManagerImpl : public OHOS::FFI::FFIData {
    DECL_TYPE(MMAAudioStreamManagerImpl, OHOS::FFI::FFIData)
public:
    MMAAudioStreamManagerImpl();
    ~MMAAudioStreamManagerImpl();

    bool IsActive(int32_t volumeType);

    CArrI32 GetAudioEffectInfoArray(int32_t usage, int32_t *errorCode);

    CArrAudioRendererChangeInfo GetCurrentRendererChangeInfos(int32_t *errorCode);
    
    CArrAudioCapturerChangeInfo GetAudioCapturerInfoArray(int32_t *errorCode);

    void RegisterCallback(int32_t callbackType, void (*callback)(), int32_t *errorCode);

private:
    int32_t cachedClientId_{};
    AudioStreamManager *streamMgr_{};
    std::shared_ptr<CjAudioCapturerStateChangeCallback> callback_{};
    std::shared_ptr<CjAudioRendererStateChangeCallback> callbackRenderer_{};
};
} // namespace AudioStandard
} // namespace OHOS
#endif // MULTIMEDIA_AUDIO_STREAM_MANAGER_IMPL_H
