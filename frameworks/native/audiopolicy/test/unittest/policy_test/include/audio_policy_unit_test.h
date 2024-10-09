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

#ifndef AUDIO_POLICY_UNIT_TEST_H
#define AUDIO_POLICY_UNIT_TEST_H

#include "gtest/gtest.h"
#include "audio_info.h"
#define private public
#include "audio_policy_manager.h"
#undef private
#include "audio_policy_proxy.h"
#include "audio_stream_manager.h"
#include "audio_group_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioManagerDeviceChangeCallbackTest : public AudioManagerDeviceChangeCallback {
    virtual void OnDeviceChange(const DeviceChangeAction &deviceChangeAction) {}
};

class AudioRendererStateChangeCallbackTest : public AudioRendererStateChangeCallback {
    virtual void OnRendererStateChange(
        const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) {}
};

class AudioCapturerStateChangeCallbackTest : public AudioCapturerStateChangeCallback {
    virtual void OnCapturerStateChange(
        const std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos) {}
};

class AudioRingerModeCallbackTest : public AudioRingerModeCallback {
public:
    virtual ~AudioRingerModeCallbackTest() = default;
    /**
     * Called when ringer mode is updated.
     *
     * @param ringerMode Indicates the updated ringer mode value.
     * For details, refer RingerMode enum in audio_info.h
     */
    virtual void OnRingerModeUpdated(const AudioRingerMode &ringerMode) {};
};

class AudioManagerMicStateChangeCallbackTest : public AudioManagerMicStateChangeCallback {
public:
    virtual ~AudioManagerMicStateChangeCallbackTest() = default;
    /**
     * Called when the microphone state changes
     *
     * @param micStateChangeEvent Microphone Status Information.
     */
    virtual void OnMicStateUpdated(const MicStateChangeEvent &micStateChangeEvent) {};
};

class AudioClientTrackerTest : public AudioClientTracker {
public:
    virtual ~AudioClientTrackerTest() = default;
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

class AudioPolicyUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
    static void InitAudioPolicyProxy(std::shared_ptr<AudioPolicyProxy> &audioPolicyProxy);
    static void GetIRemoteObject(sptr<IRemoteObject> &object);
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_POLICY_UNIT_TEST_H
