/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef AUDIO_CONCURRENCY_STATE_LISTENER_SERVICE_H
#define AUDIO_CONCURRENCY_STATE_LISTENER_SERVICE_H

#include "standard_concurrency_state_listener_stub.h"
#include "audio_concurrency_callback.h"

namespace OHOS {
namespace AudioStandard {

class AudioConcurrencyStateListenerService : public StandardConcurrencyStateListenerStub {
public:
    AudioConcurrencyStateListenerService();
    virtual ~AudioConcurrencyStateListenerService();

    int32_t OnConcedeStream() override;
    void SetConcurrencyCallback(const std::weak_ptr<AudioConcurrencyCallback> &callback);
private:
    std::weak_ptr<AudioConcurrencyCallback> callback_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_CONCURRENCY_STATE_LISTENER_SERVICE_H
