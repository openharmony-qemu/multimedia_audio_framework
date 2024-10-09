/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef AUDIO_INNER_CALL_H
#define AUDIO_INNER_CALL_H

#include <atomic>
#include <iostream>
#include <chrono>
namespace OHOS {
namespace AudioStandard {
class IAudioServerInnerCall {
public:
    virtual int32_t SetSinkRenderEmpty(const std::string &devceClass, int32_t durationUs) = 0;
};
class AudioInnerCall {
public:
    static AudioInnerCall *GetInstance();
    IAudioServerInnerCall *GetIAudioServerInnerCall();
    void RegisterAudioServer(IAudioServerInnerCall *audioServer);
private:
    IAudioServerInnerCall *audioServer_ = nullptr;
    std::atomic<bool> isAudioServerRegistered_ = false;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_INNER_CALL_H
