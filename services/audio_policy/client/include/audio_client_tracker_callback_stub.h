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

#ifndef ST_AUDIO_CLIENT_TRACKER_CALLBACK_STUB_H
#define ST_AUDIO_CLIENT_TRACKER_CALLBACK_STUB_H

#include "audio_stream_manager.h"
#include "i_standard_client_tracker.h"

namespace OHOS {
namespace AudioStandard {
class AudioClientTrackerCallbackStub : public IRemoteStub<IStandardClientTracker> {
public:
    AudioClientTrackerCallbackStub();
    virtual ~AudioClientTrackerCallbackStub();
    virtual int OnRemoteRequest(uint32_t code, MessageParcel &data,
            MessageParcel &reply, MessageOption &option) override;
    void SetClientTrackerCallback(const std::weak_ptr<AudioClientTracker> &callback);

    void PausedStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override;
    void ResumeStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override;

    void SetLowPowerVolumeImpl(float volume) override;
    void GetLowPowerVolumeImpl(float &volume) override;
    void SetOffloadModeImpl(int32_t state, bool isAppBack) override;
    void UnsetOffloadModeImpl() override;
    void GetSingleStreamVolumeImpl(float &volume) override;

    int OffloadRemoteRequest(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
private:
    std::weak_ptr<AudioClientTracker> callback_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_CLIENT_TRACKER_CALLBACK_STUB_H
