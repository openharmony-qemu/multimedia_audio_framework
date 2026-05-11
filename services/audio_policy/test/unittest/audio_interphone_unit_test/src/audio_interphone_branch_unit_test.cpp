/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "audio_errors.h"
#include "audio_info.h"
#include "audio_unit_test.h"
#include "audio_system_manager.h"
#include "audio_stream_collector.h"
#include "audio_pipe_manager.h"
#include "audio_pipe_selector.h"
#include "audio_core_service.h"
#include "audio_renderer.h"
#include "audio_capturer.h"
#include "audio_interphone_unit_test.h"
#include "id_handler.h"
#include "audio_definition_policy_utils.h"
#include "audio_concurrency_manager.h"
#include "audio_policy_utils.h"
#include "audio_volume_client_manager.h"
#include <memory>

using namespace std;
using namespace OHOS::AudioStandard;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioInterphoneBranchUnitTest::SetUpTestCase(void)
{
    ASSERT_NE(nullptr, AudioSystemManager::GetInstance());
}

void AudioInterphoneBranchUnitTest::TearDownTestCase(void) {}

void AudioInterphoneBranchUnitTest::SetUp(void) {}

void AudioInterphoneBranchUnitTest::TearDown(void) {}

/*
 * Test PIPE_TYPE_OUT_INTERPHONE Branch
 */
HWTEST_F(AudioInterphoneBranchUnitTest, TestPipeTypeOutInterphoneBranch, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    uint32_t flag = AUDIO_OUTPUT_FLAG_INTERPHONE;
    AudioPipeType pipeType = AudioPipeSelector::GetPipeSelector()->GetPipeType(flag, AUDIO_MODE_PLAYBACK);
    EXPECT_EQ(PIPE_TYPE_OUT_INTERPHONE, pipeType);

    MockNative::Resume();
}

/*
 * Test PIPE_TYPE_IN_INTERPHONE Branch
 */
HWTEST_F(AudioInterphoneBranchUnitTest, TestPipeTypeInInterphoneBranch, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    uint32_t flag = AUDIO_INPUT_FLAG_INTERPHONE;
    AudioPipeType pipeType = AudioPipeSelector::GetPipeSelector()->GetPipeType(flag, AUDIO_MODE_RECORD);
    EXPECT_EQ(PIPE_TYPE_IN_INTERPHONE, pipeType);

    MockNative::Resume();
}

/*
 * Test STREAM_USAGE_INTERPHONE in Stream Map
 */
HWTEST_F(AudioInterphoneBranchUnitTest, TestStreamUsageInterphoneMapping, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    AudioStreamType streamType = AudioSystemManager::GetStreamType(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_INTERPHONE);
    EXPECT_EQ(STREAM_VOICE_COMMUNICATION, streamType);

    streamType = AudioSystemManager::GetStreamType(CONTENT_TYPE_SPEECH, STREAM_USAGE_INTERPHONE);
    EXPECT_EQ(STREAM_VOICE_COMMUNICATION, streamType);

    MockNative::Resume();
}

/*
 * Test IsSpecialPipe with INTERPHONE Flag
 */
HWTEST_F(AudioInterphoneBranchUnitTest, TestIsSpecialPipeWithInterphoneFlag, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_TRUE(AudioPipeManager::GetPipeManager()->IsSpecialPipe(AUDIO_OUTPUT_FLAG_INTERPHONE));
    EXPECT_TRUE(AudioPipeManager::GetPipeManager()->IsSpecialPipe(AUDIO_INPUT_FLAG_INTERPHONE));
    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsSpecialPipe(0));

    MockNative::Resume();
}

/*
 * Test IsCallStreamUsage with INTERPHONE
 */
HWTEST_F(AudioInterphoneBranchUnitTest, TestIsCallStreamUsageWithInterphone, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_TRUE(AudioStreamCollector::GetAudioStreamCollector().IsCallStreamUsage(STREAM_USAGE_INTERPHONE));
    EXPECT_FALSE(AudioStreamCollector::GetAudioStreamCollector().IsCallStreamUsage(STREAM_USAGE_MUSIC));

    MockNative::Resume();
}

/*
 * Test IsVoiceStreamType with INTERPHONE
 */
HWTEST_F(AudioInterphoneBranchUnitTest, TestIsVoiceStreamTypeWithInterphone, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_TRUE(AudioPolicyUtils::GetInstance().IsVoiceStreamType(STREAM_USAGE_INTERPHONE));
    EXPECT_FALSE(AudioPolicyUtils::GetInstance().IsVoiceStreamType(STREAM_USAGE_MUSIC));

    MockNative::Resume();
}

/*
 * Test GetPreferredTypeByStreamUsage with INTERPHONE
 */
HWTEST_F(AudioInterphoneBranchUnitTest, TestGetPreferredTypeWithInterphone, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    PreferredType type = AudioPolicyUtils::GetInstance().GetPreferredTypeByStreamUsage(STREAM_USAGE_INTERPHONE);
    EXPECT_EQ(AUDIO_CALL_RENDER, type);

    type = AudioPolicyUtils::GetInstance().GetPreferredTypeByStreamUsage(STREAM_USAGE_MUSIC);
    EXPECT_EQ(AUDIO_MEDIA_RENDER, type);

    MockNative::Resume();
}

/*
 * Test Interphone Route Flag Configuration in AudioRendererOptions
 */
HWTEST_F(AudioInterphoneBranchUnitTest, TestInterphoneRouteFlagConfiguration, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    AudioRendererOptions rendererOptions;
    rendererOptions.rendererInfo.streamUsage = STREAM_USAGE_INTERPHONE;
    rendererOptions.rendererInfo.rendererFlags = AUDIO_OUTPUT_FLAG_INTERPHONE;

    EXPECT_EQ(STREAM_USAGE_INTERPHONE, rendererOptions.rendererInfo.streamUsage);
    EXPECT_EQ(AUDIO_OUTPUT_FLAG_INTERPHONE, rendererOptions.rendererInfo.rendererFlags);

    MockNative::Resume();
}

/*
 * Test Interphone Source Type Configuration in AudioCapturerOptions
 */
HWTEST_F(AudioInterphoneBranchUnitTest, TestInterphoneSourceTypeConfiguration, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    AudioCapturerOptions capturerOptions;
    capturerOptions.capturerInfo.sourceType = SOURCE_TYPE_INTERPHONE;
    capturerOptions.capturerInfo.capturerFlags = AUDIO_INPUT_FLAG_INTERPHONE;

    EXPECT_EQ(SOURCE_TYPE_INTERPHONE, capturerOptions.capturerInfo.sourceType);
    EXPECT_EQ(AUDIO_INPUT_FLAG_INTERPHONE, capturerOptions.capturerInfo.capturerFlags);

    MockNative::Resume();
}

/*
 * Test Volume Range for INTERPHONE
 */
HWTEST_F(AudioInterphoneBranchUnitTest, TestVolumeRangeForInterphone, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    int32_t maxVolume = AudioVolumeClientManager::GetInstance().GetMaxVolumeByUsage(STREAM_USAGE_INTERPHONE);
    EXPECT_GE(maxVolume, 1);

    int32_t minVolume = AudioVolumeClientManager::GetInstance().GetMinVolumeByUsage(STREAM_USAGE_INTERPHONE);
    EXPECT_GE(minVolume, 1);
    EXPECT_LE(minVolume, maxVolume);

    MockNative::Resume();
}

/*
 * Test Multiple Interphone Flag Combinations - FAST has higher priority
 */
HWTEST_F(AudioInterphoneBranchUnitTest, TestMultipleInterphoneFlagCombinations, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    uint32_t combinedFlag = AUDIO_OUTPUT_FLAG_INTERPHONE | AUDIO_OUTPUT_FLAG_FAST;
    AudioPipeType pipeType = AudioPipeSelector::GetPipeSelector()->GetPipeType(combinedFlag, AUDIO_MODE_PLAYBACK);
    EXPECT_EQ(PIPE_TYPE_OUT_LOWLATENCY, pipeType);

    MockNative::Resume();
}

/*
 * Test SetFlagForSpecialStream with INTERPHONE stream
 * Note: SetFlagForSpecialStream requires valid newDeviceDescs_, otherwise returns NORMAL
 */
HWTEST_F(AudioInterphoneBranchUnitTest, TestSetFlagForSpecialStreamInterphone, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_INTERPHONE;
    
    // Without valid newDeviceDescs_, returns NORMAL
    AudioFlag audioFlag = AudioCoreService::GetCoreService()->SetFlagForSpecialStream(streamDesc, true);
    EXPECT_EQ(AUDIO_OUTPUT_FLAG_NORMAL, audioFlag);
    
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    audioFlag = AudioCoreService::GetCoreService()->SetFlagForSpecialStream(streamDesc, true);
    EXPECT_EQ(AUDIO_OUTPUT_FLAG_NORMAL, audioFlag);
    MockNative::Resume();
}

/*
 * Test IdHandler GetRenderIdByDeviceClass with INTERPHONE routeFlag
 */
HWTEST_F(AudioInterphoneBranchUnitTest, TestGetRenderIdByDeviceClassInterphone, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    IdHandler &idHandler = IdHandler::GetInstance();
    uint32_t id = idHandler.GetRenderIdByDeviceClass("primary", "", AUDIO_OUTPUT_FLAG_INTERPHONE);
    EXPECT_NE(id, HDI_INVALID_ID);
    
    id = idHandler.GetRenderIdByDeviceClass("primary", "", 0);
    EXPECT_NE(id, HDI_INVALID_ID);
    MockNative::Resume();
}

/*
 * Test AudioDefinitionPolicyUtils flagStrToEnum for INTERPHONE
 */
HWTEST_F(AudioInterphoneBranchUnitTest, TestFlagStrToEnumInterphone, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    auto inputIt = AudioDefinitionPolicyUtils::flagStrToEnum.find("AUDIO_INPUT_FLAG_INTERPHONE");
    EXPECT_TRUE(inputIt != AudioDefinitionPolicyUtils::flagStrToEnum.end());
    EXPECT_EQ(inputIt->second, AUDIO_INPUT_FLAG_INTERPHONE);
    
    auto outputIt = AudioDefinitionPolicyUtils::flagStrToEnum.find("AUDIO_OUTPUT_FLAG_INTERPHONE");
    EXPECT_TRUE(outputIt != AudioDefinitionPolicyUtils::flagStrToEnum.end());
    EXPECT_EQ(outputIt->second, AUDIO_OUTPUT_FLAG_INTERPHONE);
    MockNative::Resume();
}

/*
 * Test AudioDefinitionPolicyUtils flagStrToEnum for CAMCORDER
 */
HWTEST_F(AudioInterphoneBranchUnitTest, TestFlagStrToEnumCamcorder, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    auto inputIt = AudioDefinitionPolicyUtils::flagStrToEnum.find("AUDIO_INPUT_FLAG_CAMCORDER");
    EXPECT_TRUE(inputIt != AudioDefinitionPolicyUtils::flagStrToEnum.end());
    EXPECT_EQ(inputIt->second, AUDIO_INPUT_FLAG_CAMCORDER);
    MockNative::Resume();
}

} // namespace AudioStandard
} // namespace OHOS
