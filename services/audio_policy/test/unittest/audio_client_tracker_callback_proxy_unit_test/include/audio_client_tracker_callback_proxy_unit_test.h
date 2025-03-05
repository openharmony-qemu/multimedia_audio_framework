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

#ifndef AUDIO_CLIENT_TRACKER_CALLBACK_PROXY_UNIT_TEST_H
#define AUDIO_CLIENT_TRACKER_CALLBACK_PROXY_UNIT_TEST_H
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "audio_client_tracker_callback_proxy.h"
#include "audio_client_tracker_callback_stub.h"

namespace OHOS {
namespace AudioStandard {
class AudioClientTrackerTest : public AudioClientTracker {
public:
    virtual ~AudioClientTrackerTest() = default;
    /**
     * Mute Stream was controlled by system application
     *
     * @param streamSetStateEventInternal Contains the set even information.
     */
    virtual void MuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
     /**
     * Unmute Stream was controlled by system application
     *
     * @param streamSetStateEventInternal Contains the set even information.
     */
    virtual void UnmuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
    /**
     * Paused Stream was controlled by system application
     *
     * @param streamSetStateEventInternal Contains the set even information.
     */
    virtual void PausedStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
     /**
     * Resumed Stream was controlled by system application
     *
     * @param streamSetStateEventInternal Contains the set even information.
     */
    virtual void ResumeStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) {};
    virtual void SetLowPowerVolumeImpl(float volume) {};
    virtual void GetLowPowerVolumeImpl(float &volume) {};
    virtual void GetSingleStreamVolumeImpl(float &volume) {};
    virtual void SetOffloadModeImpl(int32_t state, bool isAppBack) {};
    virtual void UnsetOffloadModeImpl() {};
};

class AudioClientTrackerCallbackProxyUnitTest : public testing::Test {
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
} // namespace AudioStandard
} // namespace OHOS
#endif //AUDIO_CLIENT_TRACKER_CALLBACK_PROXY_UNIT_TEST_H
