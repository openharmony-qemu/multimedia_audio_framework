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
#ifndef REMOTE_FAST_AUDIO_RENDER_SINK_AIUT_H
#define REMOTE_FAST_AUDIO_RENDER_SINK_AIUT_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mock_remote_i_audio_render.h"
#include "sink/remote_fast_audio_render_sink.h"

namespace OHOS {
namespace AudioStandard {

class RemoteFastAudioRenderSinkAiut : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void);
    void TearDown(void);

protected:
    std::shared_ptr<RemoteFastAudioRenderSink> sink_;
    IAudioSinkAttr attr_ = {};
};

} // namespace AudioStandard
} // namespace OHOS

#endif // REMOTE_FAST_AUDIO_RENDER_SINK_AIUT_H
