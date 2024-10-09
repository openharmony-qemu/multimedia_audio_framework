/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef AUDIO_RENDERER_PROXY_OBJ_H
#define AUDIO_RENDERER_PROXY_OBJ_H

#include "audio_info.h"
#include "audio_renderer.h"
#include "audio_stream_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioRendererProxyObj : public AudioClientTracker {
public:
    virtual ~AudioRendererProxyObj() = default;
    void SaveRendererObj(AudioRenderer *rendererObj);
    void UnsetRendererObj();

    void PausedStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal);
    void ResumeStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal);
    void SetLowPowerVolumeImpl(float volume);
    void GetLowPowerVolumeImpl(float &volume);
    void SetOffloadModeImpl(int32_t state, bool isAppBack);
    void UnsetOffloadModeImpl();
    void GetSingleStreamVolumeImpl(float &volume);
private:
    AudioRenderer *renderer;
    std::mutex mutex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_RENDERER_PROXY_OBJ_H
