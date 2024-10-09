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

#ifndef INTERRUPT_MULTI_RENDERER_TEST_H
#define INTERRUPT_MULTI_RENDERER_TEST_H

#include <cstdint>
#include <cstdio>
#include <memory>
#include "audio_info.h"
#include "audio_renderer.h"

namespace OHOS {
namespace AudioStandard {
class AudioRendererCallbackTestImpl : public AudioRendererCallback {
public:
    void OnInterrupt(const InterruptEvent &interruptEvent) override;
    void OnStateChange(const RendererState state, const StateChangeCmdType __attribute__((unused)) cmdType) override;
    bool isRendererPaused_ = false;
    bool isStopInProgress_ = false;
    bool isRendererStopped_ = false;
    bool isRendererResumed_ = false;
};
class InterruptMultiRendererTest {
public:
    int32_t TestPlayback(int argc, char *argv[]) const;
private:
    AudioSampleFormat GetSampleFormat(int32_t wavSampleFormat) const;
    bool InitRender(const std::unique_ptr<AudioRenderer> &audioRenderer, FILE* &wavFile) const;
    bool StartRender(const std::unique_ptr<AudioRenderer> &audioRenderer) const;
    void WriteBuffer(AudioRenderer* audioRenderer, FILE* wavFile,
                     const std::shared_ptr<AudioRendererCallbackTestImpl> &cb) const;
    int32_t ValidateFile(char *filePath, char path[]) const;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // INTERRUPT_MULTI_RENDERER_TEST_H
