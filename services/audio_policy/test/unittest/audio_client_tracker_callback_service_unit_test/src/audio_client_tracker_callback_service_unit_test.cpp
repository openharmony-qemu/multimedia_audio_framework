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
#include "audio_client_tracker_callback_service_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

const float VOLUME_TEST = 0.1f;
const int32_t NUM = 10;

void AudioClientTrackerCallbackServiceUnitTest::SetUpTestCase(void) {}
void AudioClientTrackerCallbackServiceUnitTest::TearDownTestCase(void) {}
void AudioClientTrackerCallbackServiceUnitTest::SetUp(void) {}
void AudioClientTrackerCallbackServiceUnitTest::TearDown(void) {}


/**
* @tc.name  : AudioClientTrackerCallbackService_001
* @tc.desc  : MuteStreamImpl callback is null.
* @tc.type: FUNC
* @tc.require: #I5Y4MZ
*/
HWTEST_F(AudioClientTrackerCallbackServiceUnitTest, AudioClientTrackerCallbackService_001, TestSize.Level1)
{
    sptr<AudioClientTrackerCallbackService> callback = new(std::nothrow) AudioClientTrackerCallbackService();
    std::weak_ptr<MockAudioClientTracker> nullWeakTracker;
    callback->SetClientTrackerCallback(nullWeakTracker);
    StreamSetStateEventInternal streamSetStateEventInternal;
    int32_t result = callback->MuteStreamImpl(streamSetStateEventInternal);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : AudioClientTrackerCallbackService_002
* @tc.desc  : MuteStreamImpl
* @tc.type: FUNC
* @tc.require: #I5Y4MZ
*/
HWTEST_F(AudioClientTrackerCallbackServiceUnitTest, AudioClientTrackerCallbackService_002, TestSize.Level1)
{
    sptr<AudioClientTrackerCallbackService> callback = new(std::nothrow) AudioClientTrackerCallbackService();
    auto tracker = std::make_shared<MockAudioClientTracker>();
    std::weak_ptr<MockAudioClientTracker> weakTracker(tracker);
    callback->SetClientTrackerCallback(weakTracker);
    StreamSetStateEventInternal streamSetStateEventInternal;
    int32_t result = callback->MuteStreamImpl(streamSetStateEventInternal);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(tracker->GetMuteStreamImplMonitor(), true);
    callback->UnsetClientTrackerCallback();
    callback = nullptr;
}

/**
* @tc.name  : AudioClientTrackerCallbackService_003
* @tc.desc  : UnmuteStreamImpl callback is null.
* @tc.type: FUNC
* @tc.require: #I5Y4MZ
*/
HWTEST_F(AudioClientTrackerCallbackServiceUnitTest, AudioClientTrackerCallbackService_003, TestSize.Level1)
{
    sptr<AudioClientTrackerCallbackService> callback = new(std::nothrow) AudioClientTrackerCallbackService();
    std::weak_ptr<MockAudioClientTracker> nullWeakTracker;
    callback->SetClientTrackerCallback(nullWeakTracker);
    StreamSetStateEventInternal streamSetStateEventInternal;
    int32_t result = callback->UnmuteStreamImpl(streamSetStateEventInternal);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : AudioClientTrackerCallbackService_004
* @tc.desc  : UnmuteStreamImpl
* @tc.type: FUNC
* @tc.require: #I5Y4MZ
*/
HWTEST_F(AudioClientTrackerCallbackServiceUnitTest, AudioClientTrackerCallbackService_004, TestSize.Level1)
{
    sptr<AudioClientTrackerCallbackService> callback = new(std::nothrow) AudioClientTrackerCallbackService();
    auto tracker = std::make_shared<MockAudioClientTracker>();
    std::weak_ptr<MockAudioClientTracker> weakTracker(tracker);
    callback->SetClientTrackerCallback(weakTracker);
    StreamSetStateEventInternal streamSetStateEventInternal;
    int32_t result = callback->UnmuteStreamImpl(streamSetStateEventInternal);
    EXPECT_EQ(result, SUCCESS);
    callback->UnsetClientTrackerCallback();
    EXPECT_EQ(tracker->GetUnmuteStreamImplMonitor(), true);
    callback = nullptr;
}

/**
* @tc.name  : AudioClientTrackerCallbackService_005
* @tc.desc  : PausedStreamImpl callback is null.
* @tc.type: FUNC
* @tc.require: #I5Y4MZ
*/
HWTEST_F(AudioClientTrackerCallbackServiceUnitTest, AudioClientTrackerCallbackService_005, TestSize.Level1)
{
    sptr<AudioClientTrackerCallbackService> callback = new(std::nothrow) AudioClientTrackerCallbackService();
    std::weak_ptr<MockAudioClientTracker> nullWeakTracker;
    callback->SetClientTrackerCallback(nullWeakTracker);
    StreamSetStateEventInternal streamSetStateEventInternal;
    int32_t result = callback->PausedStreamImpl(streamSetStateEventInternal);
    EXPECT_EQ(result, SUCCESS);
    callback = nullptr;
}

/**
* @tc.name  : AudioClientTrackerCallbackService_006
* @tc.desc  : PausedStreamImpl
* @tc.type: FUNC
* @tc.require: #I5Y4MZ
*/
HWTEST_F(AudioClientTrackerCallbackServiceUnitTest, AudioClientTrackerCallbackService_006, TestSize.Level1)
{
    sptr<AudioClientTrackerCallbackService> callback = new(std::nothrow) AudioClientTrackerCallbackService();
    auto tracker = std::make_shared<MockAudioClientTracker>();
    std::weak_ptr<MockAudioClientTracker> weakTracker(tracker);
    callback->SetClientTrackerCallback(weakTracker);
    StreamSetStateEventInternal streamSetStateEventInternal;
    int32_t result = callback->PausedStreamImpl(streamSetStateEventInternal);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(tracker->GetPausedStreamImplMonitor(), true);
    callback->UnsetClientTrackerCallback();
    callback = nullptr;
}

/**
* @tc.name  : AudioClientTrackerCallbackService_007
* @tc.desc  : SetLowPowerVolumeImpl callback is null.
* @tc.type: FUNC
* @tc.require: #I5Y4MZ
*/
HWTEST_F(AudioClientTrackerCallbackServiceUnitTest, AudioClientTrackerCallbackService_007, TestSize.Level1)
{
    sptr<AudioClientTrackerCallbackService> callback = new(std::nothrow) AudioClientTrackerCallbackService();
    std::weak_ptr<MockAudioClientTracker> nullWeakTracker;
    callback->SetClientTrackerCallback(nullWeakTracker);
    int32_t result = callback->SetLowPowerVolumeImpl(VOLUME);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : AudioClientTrackerCallbackService_008
* @tc.desc  : SetLowPowerVolumeImpl
* @tc.type: FUNC
* @tc.require: #I5Y4MZ
*/
HWTEST_F(AudioClientTrackerCallbackServiceUnitTest, AudioClientTrackerCallbackService_008, TestSize.Level1)
{
    sptr<AudioClientTrackerCallbackService> callback = new(std::nothrow) AudioClientTrackerCallbackService();
    auto tracker = std::make_shared<MockAudioClientTracker>();
    std::weak_ptr<MockAudioClientTracker> weakTracker(tracker);
    callback->SetClientTrackerCallback(weakTracker);
    int32_t result = callback->SetLowPowerVolumeImpl(VOLUME);
    EXPECT_EQ(result, SUCCESS);
    callback->UnsetClientTrackerCallback();
    EXPECT_EQ(tracker->GetSetLowPowerVolumeImplMonitor(), true);
}

/**
* @tc.name  : AudioClientTrackerCallbackService_009
* @tc.desc  : ResumeStreamImpl callback is null.
* @tc.type: FUNC
* @tc.require: #I5Y4MZ
*/
HWTEST_F(AudioClientTrackerCallbackServiceUnitTest, AudioClientTrackerCallbackService_009, TestSize.Level1)
{
    sptr<AudioClientTrackerCallbackService> callback = new(std::nothrow) AudioClientTrackerCallbackService();
    std::weak_ptr<MockAudioClientTracker> nullWeakTracker;
    callback->SetClientTrackerCallback(nullWeakTracker);
    StreamSetStateEventInternal streamSetStateEventInternal;
    int32_t result = callback->ResumeStreamImpl(streamSetStateEventInternal);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : AudioClientTrackerCallbackService_010
* @tc.desc  : ResumeStreamImpl
* @tc.type: FUNC
* @tc.require: #I5Y4MZ
*/
HWTEST_F(AudioClientTrackerCallbackServiceUnitTest, AudioClientTrackerCallbackService_010, TestSize.Level1)
{
    sptr<AudioClientTrackerCallbackService> callback = new(std::nothrow) AudioClientTrackerCallbackService();
    auto tracker = std::make_shared<MockAudioClientTracker>();
    std::weak_ptr<MockAudioClientTracker> weakTracker(tracker);
    callback->SetClientTrackerCallback(weakTracker);
    StreamSetStateEventInternal streamSetStateEventInternal;
    int32_t result = callback->ResumeStreamImpl(streamSetStateEventInternal);
    EXPECT_EQ(result, SUCCESS);
    callback->UnsetClientTrackerCallback();
    EXPECT_EQ(tracker->GetResumeStreamImplMonitor(), true);
}

/**
* @tc.name  : AudioClientTrackerCallbackService_011
* @tc.desc  : SetOffloadModeImpl callback is null.
* @tc.type: FUNC
* @tc.require: #I5Y4MZ
*/
HWTEST_F(AudioClientTrackerCallbackServiceUnitTest, AudioClientTrackerCallbackService_011, TestSize.Level1)
{
    sptr<AudioClientTrackerCallbackService> callback = new(std::nothrow) AudioClientTrackerCallbackService();
    std::weak_ptr<MockAudioClientTracker> nullWeakTracker;
    callback->SetClientTrackerCallback(nullWeakTracker);
    int32_t result = callback->SetOffloadModeImpl(NUM, true);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : AudioClientTrackerCallbackService_012
* @tc.desc  : SetOffloadModeImpl
* @tc.type: FUNC
* @tc.require: #I5Y4MZ
*/
HWTEST_F(AudioClientTrackerCallbackServiceUnitTest, AudioClientTrackerCallbackService_012, TestSize.Level1)
{
    sptr<AudioClientTrackerCallbackService> callback = new(std::nothrow) AudioClientTrackerCallbackService();
    auto tracker = std::make_shared<MockAudioClientTracker>();
    std::weak_ptr<MockAudioClientTracker> weakTracker(tracker);
    callback->SetClientTrackerCallback(weakTracker);
    int32_t result = callback->SetOffloadModeImpl(NUM, true);
    EXPECT_EQ(result, SUCCESS);
    callback->UnsetClientTrackerCallback();
    EXPECT_EQ(tracker->GetSetOffloadModeImplMonitor(), true);
}

/**
* @tc.name  : AudioClientTrackerCallbackService_013
* @tc.desc  : UnsetOffloadModeImpl callback is null.
* @tc.type: FUNC
* @tc.require: #I5Y4MZ
*/
HWTEST_F(AudioClientTrackerCallbackServiceUnitTest, AudioClientTrackerCallbackService_013, TestSize.Level1)
{
    sptr<AudioClientTrackerCallbackService> callback = new(std::nothrow) AudioClientTrackerCallbackService();
    std::weak_ptr<MockAudioClientTracker> nullWeakTracker;
    callback->SetClientTrackerCallback(nullWeakTracker);
    int32_t result = callback->UnsetOffloadModeImpl();
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : AudioClientTrackerCallbackService_014
* @tc.desc  : UnsetOffloadModeImpl
* @tc.type: FUNC
* @tc.require: #I5Y4MZ
*/
HWTEST_F(AudioClientTrackerCallbackServiceUnitTest, AudioClientTrackerCallbackService_014, TestSize.Level1)
{
    sptr<AudioClientTrackerCallbackService> callback = new(std::nothrow) AudioClientTrackerCallbackService();
    auto tracker = std::make_shared<MockAudioClientTracker>();
    std::weak_ptr<MockAudioClientTracker> weakTracker(tracker);
    callback->SetClientTrackerCallback(weakTracker);
    int32_t result = callback->UnsetOffloadModeImpl();
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(tracker->GetUnsetOffloadModeImplMonitor(), true);
    callback->UnsetClientTrackerCallback();
}

/**
* @tc.name  : AudioClientTrackerCallbackService_015
* @tc.desc  : GetLowPowerVolumeImpl callback is null.
* @tc.type: FUNC
* @tc.require: #I5Y4MZ
*/
HWTEST_F(AudioClientTrackerCallbackServiceUnitTest, AudioClientTrackerCallbackService_015, TestSize.Level1)
{
    sptr<AudioClientTrackerCallbackService> callback = new(std::nothrow) AudioClientTrackerCallbackService();
    std::weak_ptr<MockAudioClientTracker> nullWeakTracker;
    callback->SetClientTrackerCallback(nullWeakTracker);
    float ret = 0.0f;
    int32_t result = callback->GetLowPowerVolumeImpl(ret);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : AudioClientTrackerCallbackService_016
* @tc.desc  : GetLowPowerVolumeImpl
* @tc.type: FUNC
* @tc.require: #I5Y4MZ
*/
HWTEST_F(AudioClientTrackerCallbackServiceUnitTest, AudioClientTrackerCallbackService_016, TestSize.Level1)
{
    sptr<AudioClientTrackerCallbackService> callback = new(std::nothrow) AudioClientTrackerCallbackService();
    auto tracker = std::make_shared<MockAudioClientTracker>();
    std::weak_ptr<MockAudioClientTracker> weakTracker(tracker);
    callback->SetClientTrackerCallback(weakTracker);
    float ret = 0.0f;
    int32_t result = callback->GetLowPowerVolumeImpl(ret);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(tracker->GetGetLowPowerVolumeImplMonitor(), true);
    callback->UnsetClientTrackerCallback();
}

/**
* @tc.name  : AudioClientTrackerCallbackService_017
* @tc.desc  : GetSingleStreamVolumeImpl callback is null.
* @tc.type: FUNC
* @tc.require: #I5Y4MZ
*/
HWTEST_F(AudioClientTrackerCallbackServiceUnitTest, AudioClientTrackerCallbackService_017, TestSize.Level1)
{
    sptr<AudioClientTrackerCallbackService> callback = new(std::nothrow) AudioClientTrackerCallbackService();
    std::weak_ptr<MockAudioClientTracker> nullWeakTracker;
    callback->SetClientTrackerCallback(nullWeakTracker);
    float ret = 0.0f;
    int32_t result = callback->GetSingleStreamVolumeImpl(ret);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : AudioClientTrackerCallbackService_018
* @tc.desc  : GetSingleStreamVolumeImpl
* @tc.type: FUNC
* @tc.require: #I5Y4MZ
*/
HWTEST_F(AudioClientTrackerCallbackServiceUnitTest, AudioClientTrackerCallbackService_018, TestSize.Level1)
{
    sptr<AudioClientTrackerCallbackService> callback = new(std::nothrow) AudioClientTrackerCallbackService();
    auto tracker = std::make_shared<MockAudioClientTracker>();
    std::weak_ptr<MockAudioClientTracker> weakTracker(tracker);
    callback->SetClientTrackerCallback(weakTracker);
    float ret = 0.0f;
    int32_t result = callback->GetSingleStreamVolumeImpl(ret);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(tracker->GetGetSingleStreamVolumeImplMonitor(), true);
    callback->UnsetClientTrackerCallback();
}
} // namespace AudioStandard
} // namespace OHOS
