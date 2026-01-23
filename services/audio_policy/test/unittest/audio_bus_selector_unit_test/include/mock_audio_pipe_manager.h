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

#ifndef MOCK_AUDIO_PIPE_MANAGER
#define MOCK_AUDIO_PIPE_MANAGER

#include "audio_pipe_manager.h"
#include "gmock/gmock.h"

namespace OHOS {
namespace AudioStandard {

class MockAudioPipeManager : public AudioPipeManager {
public:
    MockAudioPipeManager() = default;
    virtual ~MockAudioPipeManager() = default;
    MOCK_METHOD(const std::vector<std::shared_ptr<AudioPipeInfo>>, GetPipeList, (), (override));
};
} // namespace AudioStandard
} // namespace OHOS

#endif