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
#include "audio_unit_test.h"
#include "audio_system_manager.h"
#include "audio_stream_collector.h"
#include "audio_pipe_manager.h"
#include "audio_pipe_selector.h"
#include "audio_core_service.h"
#include "audio_renderer.h"
#include "audio_capturer.h"
#include "audio_interphone_branch_test.h"
#include "id_handler.h"
#include "audio_definition_policy_utils.h"
#include "audio_concurrency_manager.h"

using namespace std;
using namespace OHOS::AudioStandard;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace V1_0 {

void AudioInterphoneBranchTest::SetUpTestCase(void)
{
    ASSERT_NE(nullptr, AudioSystemManager::GetInstance());
}

void AudioInterphoneBranchTest::TearDownTestCase(void) {}

void AudioInterphoneBranchTest::SetUp(void) {}

void AudioInterphoneBranchTest::TearDown(void) {}

/*
 * Test PIPE_TYPE_OUT_INTERPHONE Branch
 */
HWTEST_F(AudioInterphoneBranchTest, TestPipeTypeOutInterphoneBranch, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    uint32_t flag = AUDIO_OUTPUT_FLAG_INTERPHONE;
    AudioPipeType pipeType = AudioPipeSelector::GetInstance().GetPipeType(flag, AUDIO_MODE_PLAYBACK);
    EXPECT_EQ(PIPE_TYPE_OUT_INTERPHONE, pipeType);

    MockNative::Resume();
}

/*
 * Test PIPE_TYPE_IN_INTERPHONE Branch
 */
HWTEST_F(AudioInterphoneBranchTest, TestPipeTypeInInterphoneBranch, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    uint32_t flag = AUDIO_INPUT_FLAG_INTERPHONE;
    AudioPipeType pipeType = AudioPipeSelector::GetInstance().GetInputNormalPipeType(flag);
    EXPECT_EQ(PIPE_TYPE_IN_INTERPHONE, pipeType);

    MockNative::Resume();
}

/*
 * Test STREAM_USAGE_INTERPHONE in Stream Map
 */
HWTEST_F(AudioInterphoneBranchTest, TestStreamUsageInterphoneMapping, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    AudioStreamType streamType = IAudioStream::GetStreamType(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_INTERPHONE);
    EXPECT_EQ(STREAM_VOICE_COMMUNICATION, streamType);

    streamType = IAudioStream::GetStreamType(CONTENT_TYPE_SPEECH, STREAM_USAGE_INTERPHONE);
    EXPECT_EQ(STREAM_VOICE_COMMUNICATION, streamType);

    MockNative::Resume();
}

/*
 * Test IsSpecialPipe with INTERPHONE Flag
 */
HWTEST_F(AudioInterphoneBranchTest, TestIsSpecialPipeWithInterphoneFlag, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_TRUE(AudioPipeManager::GetInstance().IsSpecialPipe(AUDIO_OUTPUT_FLAG_INTERPHONE));
    EXPECT_TRUE(AudioPipeManager::GetInstance().IsSpecialPipe(AUDIO_INPUT_FLAG_INTERPHONE));
    EXPECT_FALSE(AudioPipeManager::GetInstance().IsSpecialPipe(0));

    MockNative::Resume();
}

/*
 * Test IsCallStreamUsage with INTERPHONE
 */
HWTEST_F(AudioInterphoneBranchTest, TestIsCallStreamUsageWithInterphone, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_TRUE(AudioStreamCollector::GetInstance().IsCallStreamUsage(STREAM_USAGE_INTERPHONE));
    EXPECT_FALSE(AudioStreamCollector::GetInstance().IsCallStreamUsage(STREAM_USAGE_MUSIC));

    MockNative::Resume();
}

/*
 * Test IsVoiceStreamType with INTERPHONE
 */
HWTEST_F(AudioInterphoneBranchTest, TestIsVoiceStreamTypeWithInterphone, TestSize.Level1)
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
HWTEST_F(AudioInterphoneBranchTest, TestGetPreferredTypeWithInterphone, TestSize.Level1)
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
 * Test Interphone Stream Conflict - VoIP Active
 */
HWTEST_F(AudioInterphoneBranchTest, TestInterphoneVoipConflictVoipActive, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneStreamActive());
    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsVoipOrCellularStreamActive());

    MockNative::Resume();
}

/*
 * Test Interphone Stream Conflict - Interphone Active
 */
HWTEST_F(AudioInterphoneBranchTest, TestInterphoneVoipConflictInterphoneActive, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneStreamActive());
    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneCapturerActive());

    MockNative::Resume();
}

/*
 * Test ForceStopInterphoneRenderer When No Stream Active
 */
HWTEST_F(AudioInterphoneBranchTest, TestForceStopInterphoneRendererNoStream, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneStreamActive());
    AudioCoreService::GetInstance().ForceStopInterphoneRenderer();

    MockNative::Resume();
}

/*
 * Test GetStreamDescsByStreamUsage Empty Result
 */
HWTEST_F(AudioInterphoneBranchTest, TestGetStreamDescsByStreamUsageEmpty, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    auto result = AudioPipeManager::GetInstance().GetStreamDescsByStreamUsage(STREAM_USAGE_INTERPHONE);
    EXPECT_TRUE(result.empty());

    MockNative::Resume();
}

/*
 * Test GetStreamDescsBySourceType Empty Result
 */
HWTEST_F(AudioInterphoneBranchTest, TestGetStreamDescsBySourceTypeEmpty, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    auto result = AudioPipeManager::GetInstance().GetStreamDescsBySourceType(SOURCE_TYPE_INTERPHONE);
    EXPECT_TRUE(result.empty());

    MockNative::Resume();
}

/*
 * Test IsNoDataStream with INTERPHONE Flag - Playback
 */
HWTEST_F(AudioInterphoneBranchTest, TestIsNoDataStreamPlayback, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_TRUE(AudioServer::GetInstance().IsNoDataStream(AUDIO_MODE_PLAYBACK, AUDIO_OUTPUT_FLAG_INTERPHONE));
    EXPECT_FALSE(AudioServer::GetInstance().IsNoDataStream(AUDIO_MODE_PLAYBACK, AUDIO_OUTPUT_FLAG_NORMAL));

    MockNative::Resume();
}

/*
 * Test IsNoDataStream with INTERPHONE Flag - Record
 */
HWTEST_F(AudioInterphoneBranchTest, TestIsNoDataStreamRecord, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_TRUE(AudioServer::GetInstance().IsNoDataStream(AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_INTERPHONE));
    EXPECT_FALSE(AudioServer::GetInstance().IsNoDataStream(AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_NORMAL));

    MockNative::Resume();
}

/*
 * Test Interphone Route Flag in AudioRenderer
 */
HWTEST_F(AudioInterphoneBranchTest, TestInterphoneRouteFlagInRenderer, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    AudioRendererOptions rendererOptions;
    rendererOptions.rendererInfo.streamUsage = STREAM_USAGE_INTERPHONE;
    rendererOptions.rendererInfo.rendererFlags = AUDIO_OUTPUT_FLAG_INTERPHONE;

    AudioRenderer *renderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, renderer);

    renderer->Release();
    delete renderer;

    MockNative::Resume();
}

/*
 * Test Interphone Source Type in AudioCapturer
 */
HWTEST_F(AudioInterphoneBranchTest, TestInterphoneSourceTypeInCapturer, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    AudioCapturerOptions capturerOptions;
    capturerOptions.capturerInfo.sourceType = SOURCE_TYPE_INTERPHONE;
    capturerOptions.capturerInfo.capturerFlags = AUDIO_INPUT_FLAG_INTERPHONE;

    AudioCapturer *capturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, capturer);

    capturer->Release();
    delete capturer;

    MockNative::Resume();
}

/*
 * Test Volume Range for INTERPHONE
 */
HWTEST_F(AudioInterphoneBranchTest, TestVolumeRangeForInterphone, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    int32_t maxVolume = AudioVolumeClientManager::GetInstance().GetMaxVolumeByUsage(STREAM_USAGE_INTERPHONE);
    EXPECT_EQ(15, maxVolume);

    int32_t minVolume = AudioVolumeClientManager::GetInstance().GetMinVolumeByUsage(STREAM_USAGE_INTERPHONE);
    EXPECT_EQ(0, minVolume);

    MockNative::Resume();
}

/*
 * Test Multiple Interphone Flag Combinations
 */
HWTEST_F(AudioInterphoneBranchTest, TestMultipleInterphoneFlagCombinations, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    uint32_t combinedFlag = AUDIO_OUTPUT_FLAG_INTERPHONE | AUDIO_OUTPUT_FLAG_FAST;
    AudioPipeType pipeType = AudioPipeSelector::GetInstance().GetPipeType(combinedFlag, AUDIO_MODE_PLAYBACK);
    EXPECT_EQ(PIPE_TYPE_OUT_INTERPHONE, pipeType);

    MockNative::Resume();
}

HWTEST_F(AudioInterphoneBranchTest, TestForceStopInterphoneStreamsWhenVoipStarts, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    AudioCoreService::GetInstance().ForceStopInterphoneRenderer();
    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneStreamActive());
    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneCapturerActive());
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneBranchTest, TestInterphoneRendererAndCapturerActiveCheck, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneStreamActive());
    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneCapturerActive());
    AudioCoreService::GetInstance().ForceStopInterphoneRenderer();
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneBranchTest, TestVoipOrCellularStreamActiveCheck, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsVoipOrCellularStreamActive());
    AudioCoreService::GetInstance().ForceStopInterphoneRenderer();
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneBranchTest, TestCheckStreamConflictsNormal, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    uint32_t audioFlag = 0;
    
    EXPECT_EQ(SUCCESS, AudioCoreService::GetInstance().CheckStreamConflicts(streamDesc, audioFlag));
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneBranchTest, TestCheckCapturerStreamConflictsNormal, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    
    EXPECT_EQ(SUCCESS, AudioCoreService::GetInstance().CheckCapturerStreamConflicts(streamDesc));
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneBranchTest, TestCheckStreamConflictsNullStream, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = nullptr;
    uint32_t audioFlag = 0;
    
    EXPECT_EQ(ERR_INVALID_PARAM, AudioCoreService::GetInstance().CheckStreamConflicts(streamDesc, audioFlag));
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneBranchTest, TestCheckCapturerStreamConflictsNullStream, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = nullptr;
    
    EXPECT_EQ(ERR_INVALID_PARAM, AudioCoreService::GetInstance().CheckCapturerStreamConflicts(streamDesc));
    MockNative::Resume();
}

/*
 * Test IsStreamSupportOutputInterPhone with valid streamDesc
 */
HWTEST_F(AudioInterphoneBranchTest, TestIsStreamSupportOutputInterPhoneValid, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_INTERPHONE;
    
    EXPECT_TRUE(AudioCoreService::GetInstance().IsStreamSupportOutputInterPhone(streamDesc));
    
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    EXPECT_FALSE(AudioCoreService::GetInstance().IsStreamSupportOutputInterPhone(streamDesc));
    MockNative::Resume();
}

/*
 * Test SetFlagForSpecialStream with INTERPHONE stream
 */
HWTEST_F(AudioInterphoneBranchTest, TestSetFlagForSpecialStreamInterphone, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_INTERPHONE;
    
    AudioFlag audioFlag = AudioCoreService::GetInstance().SetFlagForSpecialStream(streamDesc);
    EXPECT_EQ(AUDIO_OUTPUT_FLAG_INTERPHONE, audioFlag);
    
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    audioFlag = AudioCoreService::GetInstance().SetFlagForSpecialStream(streamDesc);
    EXPECT_EQ(AUDIO_OUTPUT_FLAG_NORMAL, audioFlag);
    MockNative::Resume();
}

/*
 * Test IdHandler GetRenderIdByDeviceClass with INTERPHONE routeFlag
 */
HWTEST_F(AudioInterphoneBranchTest, TestGetRenderIdByDeviceClassInterphone, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    IdHandler idHandler;
    uint32_t id = idHandler.GetRenderIdByDeviceClass("primary", "", AUDIO_OUTPUT_FLAG_INTERPHONE);
    EXPECT_NE(id, HDI_INVALID_ID);
    
    id = idHandler.GetRenderIdByDeviceClass("primary", "", 0);
    EXPECT_NE(id, HDI_INVALID_ID);
    MockNative::Resume();
}

/*
 * Test IdHandler GetIdForSolePipeSource with INTERPHONE routeFlag
 */
HWTEST_F(AudioInterphoneBranchTest, TestGetIdForSolePipeSourceInterphone, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    IdHandler idHandler;
    uint32_t id = idHandler.GetIdForSolePipeSource(SOURCE_TYPE_MIC, AUDIO_INPUT_FLAG_INTERPHONE);
    EXPECT_NE(id, HDI_INVALID_ID);
    
    id = idHandler.GetIdForSolePipeSource(SOURCE_TYPE_MIC, AUDIO_INPUT_FLAG_NORMAL);
    EXPECT_NE(id, HDI_INVALID_ID);
    MockNative::Resume();
}

/*
 * Test AudioDefinitionPolicyUtils flagStrToEnum for INTERPHONE
 */
HWTEST_F(AudioInterphoneBranchTest, TestFlagStrToEnumInterphone, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    AudioFlag inputFlag = AudioDefinitionPolicyUtils::GetInstance().flagStrToEnum("AUDIO_INPUT_FLAG_INTERPHONE");
    EXPECT_EQ(inputFlag, AUDIO_INPUT_FLAG_INTERPHONE);
    
    AudioFlag outputFlag = AudioDefinitionPolicyUtils::GetInstance().flagStrToEnum("AUDIO_OUTPUT_FLAG_INTERPHONE");
    EXPECT_EQ(outputFlag, AUDIO_OUTPUT_FLAG_INTERPHONE);
    MockNative::Resume();
}

/*
 * Test AudioConcurrencyManager pipeTypeMap for INTERPHONE
 */
HWTEST_F(AudioInterphoneBranchTest, TestConcurrencyManagerInterphonePipeType, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    AudioConcurrencyManager &manager = AudioConcurrencyManager::GetInstance();
    AudioPipeType playbackPipeType = manager.GetPipeType(AUDIO_MODE_PLAYBACK, AUDIO_OUTPUT_FLAG_INTERPHONE);
    EXPECT_EQ(playbackPipeType, PIPE_TYPE_OUT_INTERPHONE);
    
    AudioPipeType recordPipeType = manager.GetPipeType(AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_INTERPHONE);
    EXPECT_EQ(recordPipeType, PIPE_TYPE_IN_INTERPHONE);
    MockNative::Resume();
}

/*
 * Test AudioConcurrencyManager priorityOrder for INTERPHONE
 */
HWTEST_F(AudioInterphoneBranchTest, TestConcurrencyManagerInterphonePriority, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    AudioConcurrencyManager &manager = AudioConcurrencyManager::GetInstance();
    uint32_t playbackPriority = manager.GetPriority(AUDIO_MODE_PLAYBACK, AUDIO_OUTPUT_FLAG_INTERPHONE);
    EXPECT_GT(playbackPriority, 0);
    
    uint32_t recordPriority = manager.GetPriority(AUDIO_MODE_RECORD, AUDIO_INPUT_FLAG_INTERPHONE);
    EXPECT_GT(recordPriority, 0);
    MockNative::Resume();
}

} // namespace V1_0
} // namespace AudioStandard
} // namespace OHOS