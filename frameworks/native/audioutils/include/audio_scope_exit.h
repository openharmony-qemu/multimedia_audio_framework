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
#ifndef AUDIO_SCOPE_EXIT_H
#define AUDIO_SCOPE_EXIT_H

#include <functional>

namespace OHOS {
namespace AudioStandard {
class AudioScopeExit {
public:
    AudioScopeExit(std::function<void()> &&func) : func_(std::move(func))
    {}

    void Relase()
    { isReleased_ = true; }

    ~AudioScopeExit()
    {
        if (!isReleased_ && func_) { func_(); }
    }

    AudioScopeExit(const AudioScopeExit &) = delete;
    AudioScopeExit &operator=(const AudioScopeExit &) = delete;
    AudioScopeExit(AudioScopeExit &&) = delete;
    AudioScopeExit &operator=(AudioScopeExit &&) = delete;
private:
    bool isReleased_ = false;
    const std::function<void()> func_{};
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SCOPE_EXIT_H