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

#ifndef AUDIO_CLIENT_TRACKER_CALLBCK_SERVICE_H
#define AUDIO_CLIENT_TRACKER_CALLBCK_SERVICE_H

#include "standard_client_tracker_stub.h"
#include "audio_stream_manager.h"

namespace OHOS {
namespace AudioStandard {

class AudioClientTrackerCallbackService : public StandardClientTrackerStub {
public:
    AudioClientTrackerCallbackService();
    virtual ~AudioClientTrackerCallbackService();

    void SetClientTrackerCallback(const std::weak_ptr<AudioClientTracker> &callback);
    void UnsetClientTrackerCallback();

    int32_t MuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override;
    int32_t UnmuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override;
    int32_t PausedStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override;
    int32_t ResumeStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override;

    int32_t SetLowPowerVolumeImpl(float volume) override;
    int32_t GetLowPowerVolumeImpl(float &volume) override;
    int32_t SetOffloadModeImpl(int32_t state, bool isAppBack) override;
    int32_t UnsetOffloadModeImpl() override;
    int32_t GetSingleStreamVolumeImpl(float &volume) override;
private:
    std::mutex clientTrackerMutex_;
    std::weak_ptr<AudioClientTracker> callback_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_CLIENT_TRACKER_CALLBCK_SERVICE_H
