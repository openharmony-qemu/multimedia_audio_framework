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

#ifndef MOCK_AUDIO_CLIENT_TRACKER_H
#define MOCK_AUDIO_CLIENT_TRACKER_H

#include "audio_stream_manager.h"
#include "gmock/gmock.h"

namespace OHOS {
namespace AudioStandard {

class MockAudioClientTracker : public AudioClientTracker {
public:
    MockAudioClientTracker() = default;
    virtual ~MockAudioClientTracker() = default;

    MOCK_METHOD(void, MuteStreamImpl, (const StreamSetStateEventInternal& streamSetStateEventInternal), (override));
    MOCK_METHOD(void, UnmuteStreamImpl, (const StreamSetStateEventInternal& streamSetStateEventInternal), (override));
    MOCK_METHOD(void, PausedStreamImpl, (const StreamSetStateEventInternal& streamSetStateEventInternal), (override));
    MOCK_METHOD(void, ResumeStreamImpl, (const StreamSetStateEventInternal& streamSetStateEventInternal), (override));

    MOCK_METHOD(void, SetLowPowerVolumeImpl, (float volume), (override));
    MOCK_METHOD(void, GetLowPowerVolumeImpl, (float& volume), (override));

    MOCK_METHOD(void, SetOffloadModeImpl, (int32_t state, bool isAppBack), (override));
    MOCK_METHOD(void, UnsetOffloadModeImpl, (), (override));

    MOCK_METHOD(void, GetSingleStreamVolumeImpl, (float& volume), (override));
};
} // namespace AudioStandard
} // namespace OHOS
#endif // MOCK_AUDIO_CLIENT_TRACKER_H