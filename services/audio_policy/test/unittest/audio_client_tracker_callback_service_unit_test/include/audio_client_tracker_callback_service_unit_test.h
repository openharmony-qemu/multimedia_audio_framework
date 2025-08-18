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

#ifndef AUDIO_CLIENT_TRACKER_CALLBACK_SERVICE_UNIT_TEST_H
#define AUDIO_CLIENT_TRACKER_CALLBACK_SERVICE_UNIT_TEST_H

#include "audio_client_tracker_callback_service.h"
#include "audio_stream_manager.h"
#include "standard_client_tracker_stub.h"
#include "gmock/gmock.h"

namespace OHOS {
namespace AudioStandard {
class AudioClientTrackerCallbackServiceUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
};

class MockAudioClientTracker : public AudioClientTracker {
public:
    MockAudioClientTracker() = default;
    virtual ~MockAudioClientTracker() = default;

    void MuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override
    {
        (void)streamSetStateEventInternal;
        hasMuteStreamImplMonitor_ = true;
    }

    void UnmuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override
    {
        (void)streamSetStateEventInternal;
        hasUnmuteStreamImplMonitor_ = true;
    }

    void PausedStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override
    {
        (void)streamSetStateEventInternal;
        hasPausedStreamImplMonitor_ = true;
    }

    void ResumeStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) override
    {
        (void)streamSetStateEventInternal;
        hasResumeStreamImplMonitor_ = true;
    }

    void SetLowPowerVolumeImpl(float volume) override
    {
        (void)volume;
        hasSetLowPowerVolumeImplMonitor_ = true;
    }

    void GetLowPowerVolumeImpl(float &volume) override
    {
        (void)volume;
        hasGetLowPowerVolumeImplMonitor_ = true;
    }

    void SetOffloadModeImpl(int32_t state, bool isAppBack) override
    {
        (void)state;
        (void)isAppBack;
        hasSetOffloadModeImplMonitor_ = true;
    }

    void UnsetOffloadModeImpl() override
    {
        hasUnsetOffloadModeImplMonitor_ = true;
    }

    void GetSingleStreamVolumeImpl(float &volume) override
    {
        (void)volume;
        hasGetSingleStreamVolumeImplMonitor_ = true;
    }

    bool GetMuteStreamImplMonitor() const
    {
        return hasMuteStreamImplMonitor_;
    }

    bool GetUnmuteStreamImplMonitor() const
    {
        return hasUnmuteStreamImplMonitor_;
    }

    bool GetPausedStreamImplMonitor() const
    {
        return hasPausedStreamImplMonitor_;
    }

    bool GetResumeStreamImplMonitor() const
    {
        return hasResumeStreamImplMonitor_;
    }

    bool GetSetLowPowerVolumeImplMonitor() const
    {
        return hasSetLowPowerVolumeImplMonitor_;
    }

    bool GetGetLowPowerVolumeImplMonitor() const
    {
        return hasGetLowPowerVolumeImplMonitor_;
    }

    bool GetSetOffloadModeImplMonitor() const
    {
        return hasSetOffloadModeImplMonitor_;
    }

    bool GetUnsetOffloadModeImplMonitor() const
    {
        return hasUnsetOffloadModeImplMonitor_;
    }

    bool GetGetSingleStreamVolumeImplMonitor() const
    {
        return hasGetSingleStreamVolumeImplMonitor_;
    }

private:
    bool hasMuteStreamImplMonitor_{false};
    bool hasUnmuteStreamImplMonitor_{false};
    bool hasPausedStreamImplMonitor_{false};
    bool hasResumeStreamImplMonitor_{false};
    bool hasSetLowPowerVolumeImplMonitor_{false};
    bool hasGetLowPowerVolumeImplMonitor_{false};
    bool hasSetOffloadModeImplMonitor_{false};
    bool hasUnsetOffloadModeImplMonitor_{false};
    bool hasGetSingleStreamVolumeImplMonitor_{false};
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_CLIENT_TRACKER_CALLBACK_SERVICE_UNIT_TEST_H