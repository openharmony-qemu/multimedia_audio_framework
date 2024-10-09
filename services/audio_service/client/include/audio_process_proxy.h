/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef AUDIO_PROCESS_PROXY_H
#define AUDIO_PROCESS_PROXY_H

#include "message_parcel.h"

#include "i_audio_process.h"

namespace OHOS {
namespace AudioStandard {
class AudioProcessProxy : public IRemoteProxy<IAudioProcess> {
public:
    explicit AudioProcessProxy(const sptr<IRemoteObject> &impl);
    virtual ~AudioProcessProxy();

    // override for AudioProcess
    int32_t ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer) override;

    int32_t GetSessionId(uint32_t &sessionId) override;

    int32_t Start() override;

    int32_t Pause(bool isFlush) override;

    int32_t Resume() override;

    int32_t Stop() override;

    int32_t RequestHandleInfo(bool isAsync = true) override;

    int32_t Release() override;

    int32_t RegisterProcessCb(sptr<IRemoteObject> object) override;

    int32_t RegisterThreadPriority(uint32_t tid, const std::string &bundleName) override;
private:
    static inline BrokerDelegator<AudioProcessProxy> delegator_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PROCESS_PROXY_H
