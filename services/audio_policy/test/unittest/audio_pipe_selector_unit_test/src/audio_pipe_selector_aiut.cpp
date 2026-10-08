/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "aiut_log.h"

#include "audio_pipe_selector_aiut.h"
#include "audio_policy_utils.h"
#include "audio_stream_descriptor.h"
#include "audio_stream_enum.h"
#include "audio_device_descriptor.h"
#include "audio_info.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

/**
 * @tc.name  : BuildRouteSelectInfo_Playback_001
 * @tc.number: BuildRouteSelectInfo_Playback_001
 * @tc.desc  : Test BuildRouteSelectInfo with playback stream, verify streamUsage is set correctly.
 */
HWTEST_F(AudioPipeSelectorAiut, BuildRouteSelectInfo_Playback_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    streamDesc->bundleName_ = "com.test.playback";
    streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    streamDesc->appInfo_.appUid = 1001;

    auto routeSelectInfo = audioPipeSelector->BuildRouteSelectInfo(streamDesc);
    AIUT_ASSERT_NE_INFO_LOG(routeSelectInfo, nullptr, "routeSelectInfo is nullptr");
    AIUT_EXPECT_EQ_INFO_LOG(routeSelectInfo->audioMode, static_cast<int32_t>(AUDIO_MODE_PLAYBACK),
        "audioMode mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(routeSelectInfo->streamUsage, static_cast<int32_t>(STREAM_USAGE_MUSIC),
        "streamUsage mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(routeSelectInfo->sourceType, static_cast<int32_t>(SOURCE_TYPE_INVALID),
        "sourceType should be invalid");
    AIUT_EXPECT_EQ_INFO_LOG(routeSelectInfo->bundleName, "com.test.playback", "bundleName mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(routeSelectInfo->routeFlag, static_cast<AudioFlag>(streamDesc->routeFlag_),
        "routeFlag mismatch");
}

/**
 * @tc.name  : BuildRouteSelectInfo_Record_002
 * @tc.number: BuildRouteSelectInfo_Record_002
 * @tc.desc  : Test BuildRouteSelectInfo with record stream, verify sourceType is set correctly.
 */
HWTEST_F(AudioPipeSelectorAiut, BuildRouteSelectInfo_Record_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_CALL;
    streamDesc->bundleName_ = "com.test.record";
    streamDesc->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;

    auto routeSelectInfo = audioPipeSelector->BuildRouteSelectInfo(streamDesc);
    AIUT_ASSERT_NE_INFO_LOG(routeSelectInfo, nullptr, "routeSelectInfo is nullptr");
    AIUT_EXPECT_EQ_INFO_LOG(routeSelectInfo->audioMode, static_cast<int32_t>(AUDIO_MODE_RECORD),
        "audioMode mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(routeSelectInfo->streamUsage, static_cast<int32_t>(STREAM_USAGE_UNKNOWN),
        "streamUsage should be unknown");
    AIUT_EXPECT_EQ_INFO_LOG(routeSelectInfo->sourceType, static_cast<int32_t>(SOURCE_TYPE_VOICE_CALL),
        "sourceType mismatch");
}

/**
 * @tc.name  : BuildRouteSelectInfo_Nullptr_003
 * @tc.number: BuildRouteSelectInfo_Nullptr_003
 * @tc.desc  : Test BuildRouteSelectInfo with nullptr streamDesc, should return nullptr.
 */
HWTEST_F(AudioPipeSelectorAiut, BuildRouteSelectInfo_Nullptr_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    std::shared_ptr<AudioStreamDescriptor> streamDesc = nullptr;
    auto routeSelectInfo = audioPipeSelector->BuildRouteSelectInfo(streamDesc);
    AIUT_EXPECT_EQ_INFO_LOG(routeSelectInfo, nullptr, "should return nullptr for null streamDesc");
}

/**
 * @tc.name  : GetHighestPriorityStream_001
 * @tc.number: GetHighestPriorityStream_001
 * @tc.desc  : Test GetHighestPriorityStream with started streams, should return highest priority started stream.
 */
HWTEST_F(AudioPipeSelectorAiut, GetHighestPriorityStream_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    std::vector<std::shared_ptr<AudioStreamDescriptor>> streams;
    auto stream1 = std::make_shared<AudioStreamDescriptor>();
    stream1->audioMode_ = AUDIO_MODE_RECORD;
    stream1->streamStatus_ = STREAM_STATUS_STARTED;
    stream1->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    stream1->sessionId_ = 1001;
    streams.push_back(stream1);

    auto stream2 = std::make_shared<AudioStreamDescriptor>();
    stream2->audioMode_ = AUDIO_MODE_RECORD;
    stream2->streamStatus_ = STREAM_STATUS_STARTED;
    stream2->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_CALL;
    stream2->sessionId_ = 1002;
    streams.push_back(stream2);

    auto highest = audioPipeSelector->GetHighestPriorityStream(streams);
    AIUT_ASSERT_NE_INFO_LOG(highest, nullptr, "highest should not be nullptr");
    AIUT_EXPECT_EQ_INFO_LOG(highest->capturerInfo_.sourceType, static_cast<int32_t>(SOURCE_TYPE_VOICE_CALL),
        "should return voice call as highest priority");
}

/**
 * @tc.name  : GetHighestPriorityStream_002
 * @tc.number: GetHighestPriorityStream_002
 * @tc.desc  : Test GetHighestPriorityStream with no started streams, should return highest priority non-started.
 */
HWTEST_F(AudioPipeSelectorAiut, GetHighestPriorityStream_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    std::vector<std::shared_ptr<AudioStreamDescriptor>> streams;
    auto stream1 = std::make_shared<AudioStreamDescriptor>();
    stream1->audioMode_ = AUDIO_MODE_RECORD;
    stream1->streamStatus_ = STREAM_STATUS_PAUSED;
    stream1->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    stream1->sessionId_ = 1001;
    streams.push_back(stream1);

    auto stream2 = std::make_shared<AudioStreamDescriptor>();
    stream2->audioMode_ = AUDIO_MODE_RECORD;
    stream2->streamStatus_ = STREAM_STATUS_PAUSED;
    stream2->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    stream2->sessionId_ = 1002;
    streams.push_back(stream2);

    auto highest = audioPipeSelector->GetHighestPriorityStream(streams);
    AIUT_ASSERT_NE_INFO_LOG(highest, nullptr, "highest should not be nullptr");
    AIUT_EXPECT_EQ_INFO_LOG(highest->capturerInfo_.sourceType, static_cast<int32_t>(SOURCE_TYPE_VOICE_COMMUNICATION),
        "should return voice communication as highest priority");
}

/**
 * @tc.name  : GetHighestPriorityStream_003
 * @tc.number: GetHighestPriorityStream_003
 * @tc.desc  : Test GetHighestPriorityStream with empty vector, should return nullptr.
 */
HWTEST_F(AudioPipeSelectorAiut, GetHighestPriorityStream_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    std::vector<std::shared_ptr<AudioStreamDescriptor>> streams;
    auto highest = audioPipeSelector->GetHighestPriorityStream(streams);
    AIUT_EXPECT_EQ_INFO_LOG(highest, nullptr, "should return nullptr for empty streams");
}

/**
 * @tc.name  : GetHighestPriorityStream_004
 * @tc.number: GetHighestPriorityStream_004
 * @tc.desc  : Test GetHighestPriorityStream with playback streams only, should return nullptr.
 */
HWTEST_F(AudioPipeSelectorAiut, GetHighestPriorityStream_004, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    std::vector<std::shared_ptr<AudioStreamDescriptor>> streams;
    auto stream1 = std::make_shared<AudioStreamDescriptor>();
    stream1->audioMode_ = AUDIO_MODE_PLAYBACK;
    stream1->streamStatus_ = STREAM_STATUS_STARTED;
    stream1->sessionId_ = 1001;
    streams.push_back(stream1);

    auto highest = audioPipeSelector->GetHighestPriorityStream(streams);
    AIUT_EXPECT_EQ_INFO_LOG(highest, nullptr, "should return nullptr for playback only streams");
}

/**
 * @tc.name  : GetHighestPriorityStream_005
 * @tc.number: GetHighestPriorityStream_005
 * @tc.desc  : Test GetHighestPriorityStream with nullptr entries in vector, should skip nullptrs.
 */
HWTEST_F(AudioPipeSelectorAiut, GetHighestPriorityStream_005, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    std::vector<std::shared_ptr<AudioStreamDescriptor>> streams;
    streams.push_back(nullptr);
    auto stream1 = std::make_shared<AudioStreamDescriptor>();
    stream1->audioMode_ = AUDIO_MODE_RECORD;
    stream1->streamStatus_ = STREAM_STATUS_STARTED;
    stream1->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    stream1->sessionId_ = 1001;
    streams.push_back(stream1);

    auto highest = audioPipeSelector->GetHighestPriorityStream(streams);
    AIUT_ASSERT_NE_INFO_LOG(highest, nullptr, "highest should not be nullptr");
    AIUT_EXPECT_EQ_INFO_LOG(highest->sessionId_, 1001, "should return the valid stream");
}

/**
 * @tc.name  : GetPipePriority_001
 * @tc.number: GetPipePriority_001
 * @tc.desc  : Test GetPipePriority with valid pipe containing record streams.
 */
HWTEST_F(AudioPipeSelectorAiut, GetPipePriority_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->audioMode_ = AUDIO_MODE_RECORD;
    stream->streamStatus_ = STREAM_STATUS_STARTED;
    stream->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_CALL;
    stream->sessionId_ = 1001;
    pipeInfo->streamDescriptors_.push_back(stream);

    int32_t priority = audioPipeSelector->GetPipePriority(pipeInfo);
    AIUT_EXPECT_EQ_INFO_LOG(priority, 8, "voice call priority should be 8");
}

/**
 * @tc.name  : GetPipePriority_002
 * @tc.number: GetPipePriority_002
 * @tc.desc  : Test GetPipePriority with nullptr pipeInfo, should return 0.
 */
HWTEST_F(AudioPipeSelectorAiut, GetPipePriority_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    std::shared_ptr<AudioPipeInfo> pipeInfo = nullptr;
    int32_t priority = audioPipeSelector->GetPipePriority(pipeInfo);
    AIUT_EXPECT_EQ_INFO_LOG(priority, 0, "should return 0 for nullptr pipe");
}

/**
 * @tc.name  : GetPipePriority_003
 * @tc.number: GetPipePriority_003
 * @tc.desc  : Test GetPipePriority with pipe containing no record streams, should return 0.
 */
HWTEST_F(AudioPipeSelectorAiut, GetPipePriority_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->audioMode_ = AUDIO_MODE_PLAYBACK;
    stream->streamStatus_ = STREAM_STATUS_STARTED;
    stream->sessionId_ = 1001;
    pipeInfo->streamDescriptors_.push_back(stream);

    int32_t priority = audioPipeSelector->GetPipePriority(pipeInfo);
    AIUT_EXPECT_EQ_INFO_LOG(priority, 0, "should return 0 for no record streams");
}

/**
 * @tc.name  : DetermineTargetEcType_001
 * @tc.number: DetermineTargetEcType_001
 * @tc.desc  : Test DetermineTargetEcType with valid ec stream info and empty current ec type.
 */
HWTEST_F(AudioPipeSelectorAiut, DetermineTargetEcType_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->ecStreamInfo_.channels = STEREO;
    streamDesc->ecStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    streamDesc->ecStreamInfo_.format = SAMPLE_S16LE;

    std::string result = audioPipeSelector->DetermineTargetEcType(streamDesc, "");
    AIUT_EXPECT_EQ_INFO_LOG(result, std::to_string(EC_TYPE_SAME_ADAPTER),
        "should return same adapter ec type");
}

/**
 * @tc.name  : DetermineTargetEcType_002
 * @tc.number: DetermineTargetEcType_002
 * @tc.desc  : Test DetermineTargetEcType with valid ec stream info and existing ec type.
 */
HWTEST_F(AudioPipeSelectorAiut, DetermineTargetEcType_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->ecStreamInfo_.channels = STEREO;

    std::string existingEcType = std::to_string(EC_TYPE_DIFF_ADAPTER);
    std::string result = audioPipeSelector->DetermineTargetEcType(streamDesc, existingEcType);
    AIUT_EXPECT_EQ_INFO_LOG(result, existingEcType, "should return existing ec type");
}

/**
 * @tc.name  : DetermineTargetEcType_003
 * @tc.number: DetermineTargetEcType_003
 * @tc.desc  : Test DetermineTargetEcType with nullptr streamDesc, should return empty string.
 */
HWTEST_F(AudioPipeSelectorAiut, DetermineTargetEcType_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    std::shared_ptr<AudioStreamDescriptor> streamDesc = nullptr;
    std::string result = audioPipeSelector->DetermineTargetEcType(streamDesc, "");
    AIUT_EXPECT_EQ_INFO_LOG(result, std::string(""), "should return empty for nullptr streamDesc");
}

/**
 * @tc.name  : DetermineTargetEcType_004
 * @tc.number: DetermineTargetEcType_004
 * @tc.desc  : Test DetermineTargetEcType with invalid ec channels, should return empty string.
 */
HWTEST_F(AudioPipeSelectorAiut, DetermineTargetEcType_004, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->ecStreamInfo_.channels = CHANNEL_UNKNOW;

    std::string result = audioPipeSelector->DetermineTargetEcType(streamDesc, "");
    AIUT_EXPECT_EQ_INFO_LOG(result, std::string(""), "should return empty for unknown channels");
}

/**
 * @tc.name  : GetSortedInputPipes_001
 * @tc.number: GetSortedInputPipes_001
 * @tc.desc  : Test GetSortedInputPipes with mixed input and output pipes, should return only input pipes sorted.
 */
HWTEST_F(AudioPipeSelectorAiut, GetSortedInputPipes_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList;

    auto outputPipe = std::make_shared<AudioPipeInfo>();
    outputPipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    outputPipe->streamDescriptors_.push_back(std::make_shared<AudioStreamDescriptor>());
    newPipeInfoList.push_back(outputPipe);

    auto inputPipe1 = std::make_shared<AudioPipeInfo>();
    inputPipe1->pipeRole_ = PIPE_ROLE_INPUT;
    auto stream1 = std::make_shared<AudioStreamDescriptor>();
    stream1->audioMode_ = AUDIO_MODE_RECORD;
    stream1->streamStatus_ = STREAM_STATUS_STARTED;
    stream1->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    inputPipe1->streamDescriptors_.push_back(stream1);
    newPipeInfoList.push_back(inputPipe1);

    auto inputPipe2 = std::make_shared<AudioPipeInfo>();
    inputPipe2->pipeRole_ = PIPE_ROLE_INPUT;
    auto stream2 = std::make_shared<AudioStreamDescriptor>();
    stream2->audioMode_ = AUDIO_MODE_RECORD;
    stream2->streamStatus_ = STREAM_STATUS_STARTED;
    stream2->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_CALL;
    inputPipe2->streamDescriptors_.push_back(stream2);
    newPipeInfoList.push_back(inputPipe2);

    auto sortedPipes = audioPipeSelector->GetSortedInputPipes(newPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(sortedPipes.size(), 2, "should return 2 input pipes");
    AIUT_EXPECT_EQ_INFO_LOG(sortedPipes[0]->streamDescriptors_[0]->capturerInfo_.sourceType,
        SOURCE_TYPE_VOICE_CALL, "first should be voice call (higher priority)");
}

/**
 * @tc.name  : GetSortedInputPipes_002
 * @tc.number: GetSortedInputPipes_002
 * @tc.desc  : Test GetSortedInputPipes with empty list, should return empty.
 */
HWTEST_F(AudioPipeSelectorAiut, GetSortedInputPipes_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList;
    auto sortedPipes = audioPipeSelector->GetSortedInputPipes(newPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(sortedPipes.size(), 0, "should return empty for empty list");
}

/**
 * @tc.name  : GetSortedInputPipes_003
 * @tc.number: GetSortedInputPipes_003
 * @tc.desc  : Test GetSortedInputPipes with input pipe having no streams, should be skipped.
 */
HWTEST_F(AudioPipeSelectorAiut, GetSortedInputPipes_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList;
    auto emptyInputPipe = std::make_shared<AudioPipeInfo>();
    emptyInputPipe->pipeRole_ = PIPE_ROLE_INPUT;
    newPipeInfoList.push_back(emptyInputPipe);

    auto sortedPipes = audioPipeSelector->GetSortedInputPipes(newPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(sortedPipes.size(), 0, "should skip empty input pipe");
}

/**
 * @tc.name  : FindOldInputPipe_001
 * @tc.number: FindOldInputPipe_001
 * @tc.desc  : Test FindOldInputPipe when old pipe is found in oldPipeMap by id.
 */
HWTEST_F(AudioPipeSelectorAiut, FindOldInputPipe_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->id_ = 100;
    newPipe->pipeRole_ = PIPE_ROLE_INPUT;
    newPipe->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    newPipe->adapterName_ = "primary";

    auto oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->id_ = 100;
    oldPipe->pipeRole_ = PIPE_ROLE_INPUT;
    oldPipe->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    oldPipe->adapterName_ = "primary";

    std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> oldPipeMap;
    oldPipeMap[100] = oldPipe;

    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList;

    auto result = audioPipeSelector->FindOldInputPipe(newPipe, oldPipeMap, oldPipeInfoList);
    AIUT_ASSERT_NE_INFO_LOG(result, nullptr, "should find old pipe by id");
    AIUT_EXPECT_EQ_INFO_LOG(result->id_, 100, "should return pipe with id 100");
}

/**
 * @tc.name  : FindOldInputPipe_002
 * @tc.number: FindOldInputPipe_002
 * @tc.desc  : Test FindOldInputPipe when old pipe is found by matching routeFlag and adapterName.
 */
HWTEST_F(AudioPipeSelectorAiut, FindOldInputPipe_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->id_ = 200;
    newPipe->pipeRole_ = PIPE_ROLE_INPUT;
    newPipe->routeFlag_ = AUDIO_INPUT_FLAG_FAST;
    newPipe->adapterName_ = "primary";

    auto oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->id_ = 999;
    oldPipe->pipeRole_ = PIPE_ROLE_INPUT;
    oldPipe->routeFlag_ = AUDIO_INPUT_FLAG_FAST;
    oldPipe->adapterName_ = "primary";

    std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> oldPipeMap;
    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList = {oldPipe};

    auto result = audioPipeSelector->FindOldInputPipe(newPipe, oldPipeMap, oldPipeInfoList);
    AIUT_ASSERT_NE_INFO_LOG(result, nullptr, "should find old pipe by matching attributes");
    AIUT_EXPECT_EQ_INFO_LOG(result->id_, 999, "should return pipe with id 999");
}

/**
 * @tc.name  : FindOldInputPipe_003
 * @tc.number: FindOldInputPipe_003
 * @tc.desc  : Test FindOldInputPipe with nullptr newPipe, should return nullptr.
 */
HWTEST_F(AudioPipeSelectorAiut, FindOldInputPipe_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    std::shared_ptr<AudioPipeInfo> newPipe = nullptr;
    std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> oldPipeMap;
    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList;

    auto result = audioPipeSelector->FindOldInputPipe(newPipe, oldPipeMap, oldPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(result, nullptr, "should return nullptr for null newPipe");
}

/**
 * @tc.name  : FindOldInputPipe_004
 * @tc.number: FindOldInputPipe_004
 * @tc.desc  : Test FindOldInputPipe when no matching old pipe exists, should return nullptr.
 */
HWTEST_F(AudioPipeSelectorAiut, FindOldInputPipe_004, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->id_ = 300;
    newPipe->pipeRole_ = PIPE_ROLE_INPUT;
    newPipe->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    newPipe->adapterName_ = "primary";

    auto oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->id_ = 888;
    oldPipe->pipeRole_ = PIPE_ROLE_INPUT;
    oldPipe->routeFlag_ = AUDIO_INPUT_FLAG_FAST;
    oldPipe->adapterName_ = "usb";

    std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> oldPipeMap;
    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList = {oldPipe};

    auto result = audioPipeSelector->FindOldInputPipe(newPipe, oldPipeMap, oldPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(result, nullptr, "should return nullptr when no match");
}

/**
 * @tc.name  : BuildOldPipeMap_001
 * @tc.number: BuildOldPipeMap_001
 * @tc.desc  : Test BuildOldPipeMap with valid pipe list, should build correct map.
 */
HWTEST_F(AudioPipeSelectorAiut, BuildOldPipeMap_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList;
    auto pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->id_ = 101;
    oldPipeInfoList.push_back(pipe1);
    auto pipe2 = std::make_shared<AudioPipeInfo>();
    pipe2->id_ = 102;
    oldPipeInfoList.push_back(pipe2);

    auto oldPipeMap = audioPipeSelector->BuildOldPipeMap(oldPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(oldPipeMap.size(), 2, "map should have 2 entries");
    AIUT_EXPECT_EQ_INFO_LOG(oldPipeMap[101]->id_, 101, "should contain pipe with id 101");
    AIUT_EXPECT_EQ_INFO_LOG(oldPipeMap[102]->id_, 102, "should contain pipe with id 102");
}

/**
 * @tc.name  : BuildOldPipeMap_002
 * @tc.number: BuildOldPipeMap_002
 * @tc.desc  : Test BuildOldPipeMap with nullptr entries, should skip nullptrs.
 */
HWTEST_F(AudioPipeSelectorAiut, BuildOldPipeMap_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList;
    oldPipeInfoList.push_back(nullptr);
    auto pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->id_ = 201;
    oldPipeInfoList.push_back(pipe1);

    auto oldPipeMap = audioPipeSelector->BuildOldPipeMap(oldPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(oldPipeMap.size(), 1, "should skip nullptr and have 1 entry");
}

/**
 * @tc.name  : BuildOldPipeMap_003
 * @tc.number: BuildOldPipeMap_003
 * @tc.desc  : Test BuildOldPipeMap with empty list, should return empty map.
 */
HWTEST_F(AudioPipeSelectorAiut, BuildOldPipeMap_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList;
    auto oldPipeMap = audioPipeSelector->BuildOldPipeMap(oldPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(oldPipeMap.size(), 0, "should return empty map");
}

/**
 * @tc.name  : LogStreamsInfo_001
 * @tc.number: LogStreamsInfo_001
 * @tc.desc  : Test LogStreamsInfo with non-empty session list, should not crash.
 */
HWTEST_F(AudioPipeSelectorAiut, LogStreamsInfo_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    std::vector<std::pair<uint32_t, uint32_t>> sessionList = {{1001, 1}, {1002, 2}, {1003, 3}};
    audioPipeSelector->LogStreamsInfo(sessionList);
    AIUT_EXPECT_EQ_INFO_LOG(sessionList.size(), 3, "sessionList should not be modified by LogStreamsInfo");
}

/**
 * @tc.name  : LogStreamsInfo_002
 * @tc.number: LogStreamsInfo_002
 * @tc.desc  : Test LogStreamsInfo with empty session list, should return early.
 */
HWTEST_F(AudioPipeSelectorAiut, LogStreamsInfo_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    std::vector<std::pair<uint32_t, uint32_t>> sessionList;
    audioPipeSelector->LogStreamsInfo(sessionList);
    AIUT_EXPECT_EQ_INFO_LOG(sessionList.size(), 0, "sessionList should remain empty after LogStreamsInfo");
}

/**
 * @tc.name  : IsNeedReloadForInputPipe_001
 * @tc.number: IsNeedReloadForInputPipe_001
 * @tc.desc  : Test IsNeedReloadForInputPipe when sampling rates differ, should return true.
 */
HWTEST_F(AudioPipeSelectorAiut, IsNeedReloadForInputPipe_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->audioStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    auto oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->audioStreamInfo_.samplingRate = SAMPLE_RATE_44100;

    auto highestStream = std::make_shared<AudioStreamDescriptor>();
    auto oldHighestStream = std::make_shared<AudioStreamDescriptor>();

    bool result = audioPipeSelector->IsNeedReloadForInputPipe(newPipe, oldPipe, highestStream, oldHighestStream);
    AIUT_EXPECT_EQ_INFO_LOG(result, true, "should need reload when sampling rates differ");
}

/**
 * @tc.name  : IsNeedReloadForInputPipe_002
 * @tc.number: IsNeedReloadForInputPipe_002
 * @tc.desc  : Test IsNeedReloadForInputPipe when channels differ, should return true.
 */
HWTEST_F(AudioPipeSelectorAiut, IsNeedReloadForInputPipe_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->audioStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    newPipe->audioStreamInfo_.channels = STEREO;
    auto oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->audioStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    oldPipe->audioStreamInfo_.channels = CHANNEL_6;

    auto highestStream = std::make_shared<AudioStreamDescriptor>();
    auto oldHighestStream = std::make_shared<AudioStreamDescriptor>();

    bool result = audioPipeSelector->IsNeedReloadForInputPipe(newPipe, oldPipe, highestStream, oldHighestStream);
    AIUT_EXPECT_EQ_INFO_LOG(result, true, "should need reload when channels differ");
}

/**
 * @tc.name  : IsNeedReloadForInputPipe_003
 * @tc.number: IsNeedReloadForInputPipe_003
 * @tc.desc  : Test IsNeedReloadForInputPipe when ec type changes, should return true.
 */
HWTEST_F(AudioPipeSelectorAiut, IsNeedReloadForInputPipe_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->audioStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    newPipe->audioStreamInfo_.channels = STEREO;
    newPipe->moduleInfo_.ecType = std::to_string(EC_TYPE_SAME_ADAPTER);
    auto oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->audioStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    oldPipe->audioStreamInfo_.channels = STEREO;
    oldPipe->moduleInfo_.ecType = "";

    auto highestStream = std::make_shared<AudioStreamDescriptor>();
    auto oldHighestStream = std::make_shared<AudioStreamDescriptor>();

    bool result = audioPipeSelector->IsNeedReloadForInputPipe(newPipe, oldPipe, highestStream, oldHighestStream);
    AIUT_EXPECT_EQ_INFO_LOG(result, true, "should need reload when ec type changes");
}

/**
 * @tc.name  : IsNeedReloadForInputPipe_004
 * @tc.number: IsNeedReloadForInputPipe_004
 * @tc.desc  : Test IsNeedReloadForInputPipe when micRef state changes, should return true.
 */
HWTEST_F(AudioPipeSelectorAiut, IsNeedReloadForInputPipe_004, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->audioStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    newPipe->audioStreamInfo_.channels = STEREO;
    newPipe->moduleInfo_.openMicRef = "1";
    auto oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->audioStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    oldPipe->audioStreamInfo_.channels = STEREO;
    oldPipe->moduleInfo_.openMicRef = "0";

    auto highestStream = std::make_shared<AudioStreamDescriptor>();
    auto oldHighestStream = std::make_shared<AudioStreamDescriptor>();

    bool result = audioPipeSelector->IsNeedReloadForInputPipe(newPipe, oldPipe, highestStream, oldHighestStream);
    AIUT_EXPECT_EQ_INFO_LOG(result, true, "should need reload when micRef state changes");
}

/**
 * @tc.name  : IsNeedReloadForInputPipe_005
 * @tc.number: IsNeedReloadForInputPipe_005
 * @tc.desc  : Test IsNeedReloadForInputPipe when highest stream changes, should return true.
 */
HWTEST_F(AudioPipeSelectorAiut, IsNeedReloadForInputPipe_005, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->audioStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    newPipe->audioStreamInfo_.channels = STEREO;
    auto oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->audioStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    oldPipe->audioStreamInfo_.channels = STEREO;

    auto highestStream = std::make_shared<AudioStreamDescriptor>();
    highestStream->sessionId_ = 1001;
    auto oldHighestStream = std::make_shared<AudioStreamDescriptor>();
    oldHighestStream->sessionId_ = 1002;

    bool result = audioPipeSelector->IsNeedReloadForInputPipe(newPipe, oldPipe, highestStream, oldHighestStream);
    AIUT_EXPECT_EQ_INFO_LOG(result, true, "should need reload when highest stream changes");
}

/**
 * @tc.name  : IsNeedReloadForInputPipe_006
 * @tc.number: IsNeedReloadForInputPipe_006
 * @tc.desc  : Test IsNeedReloadForInputPipe when nothing changes, should return false.
 */
HWTEST_F(AudioPipeSelectorAiut, IsNeedReloadForInputPipe_006, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->audioStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    newPipe->audioStreamInfo_.channels = STEREO;
    auto oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->audioStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    oldPipe->audioStreamInfo_.channels = STEREO;

    auto highestStream = std::make_shared<AudioStreamDescriptor>();
    highestStream->sessionId_ = 1001;
    auto oldHighestStream = std::make_shared<AudioStreamDescriptor>();
    oldHighestStream->sessionId_ = 1001;

    bool result = audioPipeSelector->IsNeedReloadForInputPipe(newPipe, oldPipe, highestStream, oldHighestStream);
    AIUT_EXPECT_EQ_INFO_LOG(result, false, "should not need reload when nothing changes");
}

/**
 * @tc.name  : GetCapturerPipeExecOrder_001
 * @tc.number: GetCapturerPipeExecOrder_001
 * @tc.desc  : Test GetCapturerPipeExecOrder when old has ec and new doesn't, should return CLOSE_BEFORE_OPEN.
 */
HWTEST_F(AudioPipeSelectorAiut, GetCapturerPipeExecOrder_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->id_ = 100;
    newPipe->pipeRole_ = PIPE_ROLE_INPUT;
    newPipe->moduleInfo_.ecType = "";
    auto oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->id_ = 200;
    oldPipe->pipeRole_ = PIPE_ROLE_INPUT;
    oldPipe->moduleInfo_.ecType = std::to_string(EC_TYPE_SAME_ADAPTER);

    std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> oldPipeMap;
    oldPipeMap[200] = oldPipe;
    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList = {oldPipe};

    auto result = audioPipeSelector->GetCapturerPipeExecOrder(newPipe, oldPipeMap, oldPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(static_cast<int32_t>(result),
        static_cast<int32_t>(CapturerPipeExecOrder::CLOSE_BEFORE_OPEN),
        "should return CLOSE_BEFORE_OPEN");
}

/**
 * @tc.name  : GetCapturerPipeExecOrder_002
 * @tc.number: GetCapturerPipeExecOrder_002
 * @tc.desc  : Test GetCapturerPipeExecOrder when new has ec and old doesn't, should return OPEN_AFTER_CLOSE.
 */
HWTEST_F(AudioPipeSelectorAiut, GetCapturerPipeExecOrder_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->id_ = 100;
    newPipe->pipeRole_ = PIPE_ROLE_INPUT;
    newPipe->moduleInfo_.ecType = std::to_string(EC_TYPE_SAME_ADAPTER);
    auto oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->id_ = 200;
    oldPipe->pipeRole_ = PIPE_ROLE_INPUT;
    oldPipe->moduleInfo_.ecType = "";

    std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> oldPipeMap;
    oldPipeMap[200] = oldPipe;
    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList = {oldPipe};

    auto result = audioPipeSelector->GetCapturerPipeExecOrder(newPipe, oldPipeMap, oldPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(static_cast<int32_t>(result),
        static_cast<int32_t>(CapturerPipeExecOrder::OPEN_AFTER_CLOSE),
        "should return OPEN_AFTER_CLOSE");
}

/**
 * @tc.name  : GetCapturerPipeExecOrder_003
 * @tc.number: GetCapturerPipeExecOrder_003
 * @tc.desc  : Test GetCapturerPipeExecOrder when no old pipe exists, should return NORMAL.
 */
HWTEST_F(AudioPipeSelectorAiut, GetCapturerPipeExecOrder_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->id_ = 100;
    newPipe->pipeRole_ = PIPE_ROLE_INPUT;
    newPipe->moduleInfo_.ecType = std::to_string(EC_TYPE_SAME_ADAPTER);

    std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> oldPipeMap;
    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList;

    auto result = audioPipeSelector->GetCapturerPipeExecOrder(newPipe, oldPipeMap, oldPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(static_cast<int32_t>(result),
        static_cast<int32_t>(CapturerPipeExecOrder::NORMAL), "should return NORMAL");
}

/**
 * @tc.name  : GetCapturerPipeExecOrder_004
 * @tc.number: GetCapturerPipeExecOrder_004
 * @tc.desc  : Test GetCapturerPipeExecOrder with nullptr newPipe, should return NORMAL.
 */
HWTEST_F(AudioPipeSelectorAiut, GetCapturerPipeExecOrder_004, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    std::shared_ptr<AudioPipeInfo> newPipe = nullptr;
    std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> oldPipeMap;
    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList;

    auto result = audioPipeSelector->GetCapturerPipeExecOrder(newPipe, oldPipeMap, oldPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(static_cast<int32_t>(result),
        static_cast<int32_t>(CapturerPipeExecOrder::NORMAL), "should return NORMAL for nullptr");
}

/**
 * @tc.name  : GetCapturerPipeExecOrder_005
 * @tc.number: GetCapturerPipeExecOrder_005
 * @tc.desc  : Test GetCapturerPipeExecOrder with output pipe, should return NORMAL.
 */
HWTEST_F(AudioPipeSelectorAiut, GetCapturerPipeExecOrder_005, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->id_ = 100;
    newPipe->pipeRole_ = PIPE_ROLE_OUTPUT;

    std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> oldPipeMap;
    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList;

    auto result = audioPipeSelector->GetCapturerPipeExecOrder(newPipe, oldPipeMap, oldPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(static_cast<int32_t>(result),
        static_cast<int32_t>(CapturerPipeExecOrder::NORMAL), "should return NORMAL for output pipe");
}

/**
 * @tc.name  : GetCapturerPipeExecOrder_006
 * @tc.number: GetCapturerPipeExecOrder_006
 * @tc.desc  : Test GetCapturerPipeExecOrder when micRef state changes from open to closed.
 */
HWTEST_F(AudioPipeSelectorAiut, GetCapturerPipeExecOrder_006, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->id_ = 100;
    newPipe->pipeRole_ = PIPE_ROLE_INPUT;
    newPipe->moduleInfo_.openMicRef = "0";
    auto oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->id_ = 200;
    oldPipe->pipeRole_ = PIPE_ROLE_INPUT;
    oldPipe->moduleInfo_.openMicRef = "1";

    std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> oldPipeMap;
    oldPipeMap[200] = oldPipe;
    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList = {oldPipe};

    auto result = audioPipeSelector->GetCapturerPipeExecOrder(newPipe, oldPipeMap, oldPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(static_cast<int32_t>(result),
        static_cast<int32_t>(CapturerPipeExecOrder::CLOSE_BEFORE_OPEN),
        "should return CLOSE_BEFORE_OPEN for micRef close");
}

/**
 * @tc.name  : HandlePipeNotExist_001
 * @tc.number: HandlePipeNotExist_001
 * @tc.desc  : Test HandlePipeNotExist creates new pipe for stream.
 */
HWTEST_F(AudioPipeSelectorAiut, HandlePipeNotExist_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");

    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList;
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->sessionId_ = 100001;
    streamDesc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.front()->networkId_ = "0";

    audioPipeSelector->HandlePipeNotExist(newPipeInfoList, streamDesc);
    AIUT_EXPECT_EQ_INFO_LOG(newPipeInfoList.size(), 1, "should create 1 new pipe");
    AIUT_EXPECT_EQ_INFO_LOG(newPipeInfoList[0]->pipeAction_, PIPE_ACTION_NEW,
        "new pipe should have PIPE_ACTION_NEW");
}

} // namespace AudioStandard
} // namespace OHOS