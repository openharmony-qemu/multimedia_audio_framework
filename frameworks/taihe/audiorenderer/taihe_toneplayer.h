/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef TAIHE_TONEPLAYER_H
#define TAIHE_TONEPLAYER_H

#include "ohos.multimedia.audio.proj.hpp"
#include "ohos.multimedia.audio.impl.hpp"
#include "taihe/runtime.hpp"
#include "securec.h"
#include "audio_errors.h"
#include "audio_log.h"
#include "tone_player.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

const int32_t ARGS_LOAD_MAX = 28;
const int32_t TONE_TYPE_ARR[ARGS_LOAD_MAX] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    100, 101, 102, 103, 104, 106, 107, 108, 200, 201, 203, 204};

class TonePlayerImpl {
public:
    TonePlayerImpl() : tonePlayer_(nullptr) {}
    explicit TonePlayerImpl(std::shared_ptr<OHOS::AudioStandard::TonePlayer> obj);
    ~TonePlayerImpl() = default;

    static TonePlayer CreateTonePlayerWrapper(std::unique_ptr<OHOS::AudioStandard::AudioRendererInfo> rendererInfo);

    void LoadSync(ToneType type);
    void ReleaseSync();
    void StopSync();
    void StartSync();
    std::shared_ptr<OHOS::AudioStandard::TonePlayer> tonePlayer_;

private:
    static std::mutex createMutex_;
    static int32_t isConstructSuccess_;
};
} // namespace ANI::Audio
#endif
