/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "audio_core_service_private_route_selector_unit_test.h"

#include "audio_errors.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static const uint32_t TEST_STREAM_1_SESSION_ID = 100001;

void AudioCoreServicePrivateRouteSelectorUnitTest::SetUp(void)
{
    testCoreService_ = std::make_shared<AudioCoreService>();
    testCoreService_->Init();
}

void AudioCoreServicePrivateRouteSelectorUnitTest::TearDown(void)
{
    testCoreService_ = nullptr;
}

class TestAudioRouteSelector : public AudioRouteSelector {
public:
    TestAudioRouteSelector(int32_t ret, int32_t selectResult) : ret_(ret), selectResult_(selectResult) {}
    ~TestAudioRouteSelector() override = default;

    int32_t OnAudioRouteSelect(const std::shared_ptr<AudioRouteSelectInfo> &routeSelectInfo,
        int32_t &selectResult) override
    {
        lastRouteSelectInfo_ = routeSelectInfo;
        callCount_++;
        selectResult = selectResult_;
        return ret_;
    }

    int32_t ret_;
    int32_t selectResult_;
    int32_t callCount_ = 0;
    std::shared_ptr<AudioRouteSelectInfo> lastRouteSelectInfo_ = nullptr;
};

static AudioPipeSelector::AudioRouteSelectorCallback MakeRouteSelectorCallback(
    const std::shared_ptr<AudioRouteSelector> &selector)
{
    return [selector](const std::shared_ptr<AudioRouteSelectInfo> &routeSelectInfo, int32_t &selectResult) -> int32_t {
        CHECK_AND_RETURN_RET_LOG(selector != nullptr, ERR_OPERATION_FAILED, "selector is nullptr");
        return selector->OnAudioRouteSelect(routeSelectInfo, selectResult);
    };
}

static std::shared_ptr<AudioStreamDescriptor> MakeRendererCreateStreamDesc()
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S32LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;
    streamDesc->streamInfo_.encoding = AudioEncodingType::ENCODING_PCM;
    streamDesc->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MOVIE;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();
    streamDesc->callerUid_ = getuid();
    return streamDesc;
}

static std::shared_ptr<AudioStreamDescriptor> MakeCapturerCreateStreamDesc()
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S32LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;
    streamDesc->streamInfo_.encoding = AudioEncodingType::ENCODING_PCM;
    streamDesc->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();
    streamDesc->callerUid_ = getuid();
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    return streamDesc;
}

/**
 * @tc.name  : AudioCoreServicePrivateRouteSelectorUnitTest_FetchRendererPipeAndExecute_003
 * @tc.number: FetchRendererPipeAndExecute_003
 * @tc.desc  : Test AudioCoreService::FetchRendererPipeAndExecute returns error when route selector rejects create.
 */
HWTEST_F(AudioCoreServicePrivateRouteSelectorUnitTest, FetchRendererPipeAndExecute_003, TestSize.Level1)
{
    ASSERT_NE(testCoreService_, nullptr);

    auto routeSelector = std::make_shared<TestAudioRouteSelector>(
        SUCCESS, AudioRouteSelector::ROUTE_SELECT_RESULT_REJECT_CREATE);
    auto callback = MakeRouteSelectorCallback(routeSelector);
    ASSERT_NE(callback, nullptr);
    EXPECT_EQ(testCoreService_->SetAudioRouteSelectorCallback(callback), SUCCESS);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.front()->deviceRole_ = OUTPUT_DEVICE;
    streamDesc->newDeviceDescs_.front()->networkId_ = LOCAL_NETWORK_ID;
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S16LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;
    streamDesc->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;

    uint32_t sessionId = TEST_STREAM_1_SESSION_ID;
    uint32_t audioFlag = AUDIO_OUTPUT_FLAG_NORMAL;
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
    int32_t ret = testCoreService_->FetchRendererPipeAndExecute(streamDesc, sessionId, audioFlag, reason);

    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    EXPECT_TRUE(streamDesc->GetRouteSelectRejectedFlag());
    EXPECT_EQ(routeSelector->callCount_, 1);
    EXPECT_EQ(testCoreService_->UnsetAudioRouteSelectorCallback(), SUCCESS);
}

/**
 * @tc.name  : AudioCoreServicePrivateRouteSelectorUnitTest_FetchCapturerPipeAndExecute_001
 * @tc.number: FetchCapturerPipeAndExecute_001
 * @tc.desc  : Test AudioCoreService::FetchCapturerPipeAndExecute returns error when route selector rejects create.
 */
HWTEST_F(AudioCoreServicePrivateRouteSelectorUnitTest, FetchCapturerPipeAndExecute_001, TestSize.Level1)
{
    ASSERT_NE(testCoreService_, nullptr);

    auto routeSelector = std::make_shared<TestAudioRouteSelector>(
        SUCCESS, AudioRouteSelector::ROUTE_SELECT_RESULT_REJECT_CREATE);
    auto callback = MakeRouteSelectorCallback(routeSelector);
    ASSERT_NE(callback, nullptr);
    EXPECT_EQ(testCoreService_->SetAudioRouteSelectorCallback(callback), SUCCESS);

    auto streamDesc = MakeCapturerCreateStreamDesc();
    streamDesc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_MIC;
    streamDesc->newDeviceDescs_.front()->deviceRole_ = INPUT_DEVICE;
    streamDesc->newDeviceDescs_.front()->networkId_ = LOCAL_NETWORK_ID;

    uint32_t sessionId = TEST_STREAM_1_SESSION_ID;
    uint32_t audioFlag = AUDIO_INPUT_FLAG_NORMAL;
    int32_t ret = testCoreService_->FetchCapturerPipeAndExecute(streamDesc, audioFlag, sessionId);

    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    EXPECT_TRUE(streamDesc->GetRouteSelectRejectedFlag());
    EXPECT_EQ(routeSelector->callCount_, 1);
    EXPECT_EQ(testCoreService_->UnsetAudioRouteSelectorCallback(), SUCCESS);
}

/**
 * @tc.name  : AudioCoreServicePrivateRouteSelectorUnitTest_CreateRendererClient_RejectCreate_001
 * @tc.number: CreateRendererClient_RejectCreate_001
 * @tc.desc  : Test AudioCoreService::CreateRendererClient does not add session id when route selector rejects create.
 */
HWTEST_F(AudioCoreServicePrivateRouteSelectorUnitTest, CreateRendererClient_RejectCreate_001, TestSize.Level1)
{
    ASSERT_NE(testCoreService_, nullptr);

    auto routeSelector = std::make_shared<TestAudioRouteSelector>(
        SUCCESS, AudioRouteSelector::ROUTE_SELECT_RESULT_REJECT_CREATE);
    auto callback = MakeRouteSelectorCallback(routeSelector);
    ASSERT_NE(callback, nullptr);
    EXPECT_EQ(testCoreService_->SetAudioRouteSelectorCallback(callback), SUCCESS);

    auto streamDesc = MakeRendererCreateStreamDesc();
    uint32_t audioFlag = AUDIO_OUTPUT_FLAG_NORMAL;
    uint32_t sessionId = 0;
    std::string networkId = LOCAL_NETWORK_ID;
    int32_t ret = testCoreService_->CreateRendererClient(streamDesc, audioFlag, sessionId, networkId);

    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    EXPECT_NE(sessionId, 0u);
    EXPECT_EQ(testCoreService_->sessionIdMap_.count(sessionId), 0);
    EXPECT_EQ(routeSelector->callCount_, 1);
    EXPECT_EQ(testCoreService_->UnsetAudioRouteSelectorCallback(), SUCCESS);
    testCoreService_->DeleteSessionId(sessionId);
}

/**
 * @tc.name  : AudioCoreServicePrivateRouteSelectorUnitTest_CreateCapturerClient_RejectCreate_001
 * @tc.number: CreateCapturerClient_RejectCreate_001
 * @tc.desc  : Test AudioCoreService::CreateCapturerClient does not add session id when route selector rejects create.
 */
HWTEST_F(AudioCoreServicePrivateRouteSelectorUnitTest, CreateCapturerClient_RejectCreate_001, TestSize.Level1)
{
    ASSERT_NE(testCoreService_, nullptr);

    auto routeSelector = std::make_shared<TestAudioRouteSelector>(
        SUCCESS, AudioRouteSelector::ROUTE_SELECT_RESULT_REJECT_CREATE);
    auto callback = MakeRouteSelectorCallback(routeSelector);
    ASSERT_NE(callback, nullptr);
    EXPECT_EQ(testCoreService_->SetAudioRouteSelectorCallback(callback), SUCCESS);

    auto streamDesc = MakeCapturerCreateStreamDesc();
    uint32_t audioFlag = AUDIO_INPUT_FLAG_NORMAL;
    uint32_t sessionId = 0;
    int32_t ret = testCoreService_->CreateCapturerClient(streamDesc, audioFlag, sessionId);

    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    EXPECT_NE(sessionId, 0u);
    EXPECT_EQ(testCoreService_->sessionIdMap_.count(sessionId), 0);
    EXPECT_EQ(routeSelector->callCount_, 1);
    EXPECT_EQ(testCoreService_->UnsetAudioRouteSelectorCallback(), SUCCESS);
    testCoreService_->DeleteSessionId(sessionId);
}
} // namespace AudioStandard
} // namespace OHOS
