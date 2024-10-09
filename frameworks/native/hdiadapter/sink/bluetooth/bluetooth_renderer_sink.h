/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_RENDERER_SINK_H
#define BLUETOOTH_RENDERER_SINK_H

#include "audio_info.h"
#include "i_audio_renderer_sink.h"

namespace OHOS {
namespace AudioStandard {
class BluetoothRendererSink : public IMmapAudioRendererSink {
public:
    static BluetoothRendererSink *GetInstance(void);
    static IMmapAudioRendererSink *GetMmapInstance(void);

    BluetoothRendererSink() = default;
    ~BluetoothRendererSink() = default;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // BLUETOOTH_RENDERER_SINK_H
