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

#ifndef ST_AUDIO_CLIENT_TRACKER_CALLBACK_PROXY_H
#define ST_AUDIO_CLIENT_TRACKER_CALLBACK_PROXY_H

#include "iremote_proxy.h"
#include "audio_stream_manager.h"
#include "i_standard_client_tracker.h"

namespace OHOS {
namespace AudioStandard {
class ClientTrackerCallbackListener : public AudioClientTracker {
public:
    explicit ClientTrackerCallbackListener(const sptr<IStandardClientTracker> &listener);
    virtual ~ClientTrackerCallbackListener();
    DISALLOW_COPY_AND_MOVE(ClientTrackerCallbackListener);

    virtual void MuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override;
    virtual void UnmuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override;
    virtual void PausedStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override;
    virtual void ResumeStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override;

    virtual void SetLowPowerVolumeImpl(float volume) override;
    virtual void GetLowPowerVolumeImpl(float &volume) override;
    virtual void SetOffloadModeImpl(int32_t state, bool isAppBack) override;
    virtual void UnsetOffloadModeImpl() override;
    virtual void GetSingleStreamVolumeImpl(float &volume) override;

private:
    sptr<IStandardClientTracker> listener_ = nullptr;
};

class AudioClientTrackerCallbackProxy : public IRemoteProxy<IStandardClientTracker> {
public:
    explicit AudioClientTrackerCallbackProxy(const sptr<IRemoteObject> &impl);
    virtual ~AudioClientTrackerCallbackProxy() = default;

    virtual void MuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override;
    virtual void UnmuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override;
    virtual void PausedStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override;
    virtual void ResumeStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override;

    virtual void SetLowPowerVolumeImpl(float volume) override;
    virtual void GetLowPowerVolumeImpl(float &volume) override;
    virtual void SetOffloadModeImpl(int32_t state, bool isAppBack) override;
    virtual void UnsetOffloadModeImpl() override;
    virtual void GetSingleStreamVolumeImpl(float &volume) override;

private:
    static inline BrokerDelegator<AudioClientTrackerCallbackProxy> delegator_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_CLIENT_TRACKER_CALLBACK_PROXY_H
