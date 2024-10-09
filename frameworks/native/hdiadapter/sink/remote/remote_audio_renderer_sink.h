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

#ifndef REMOTE_AUDIO_RENDERER_SINK_H
#define REMOTE_AUDIO_RENDERER_SINK_H

#include "audio_info.h"
#include "i_audio_renderer_sink.h"

namespace OHOS {
namespace AudioStandard {
class RemoteAudioRendererSink : public IRemoteAudioRendererSink {
public:
    static RemoteAudioRendererSink *GetInstance(const std::string &deviceNetworkId);

    RemoteAudioRendererSink() = default;
    ~RemoteAudioRendererSink() = default;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // REMOTE_AUDIO_RENDERER_SINK_H
