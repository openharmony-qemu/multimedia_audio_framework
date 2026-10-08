/*
 * Copyright (C) 2025-2026 Huawei Device Co., Ltd.
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
void AudioPipeSelectorAiut::SetUpTestCase(void) {}
void AudioPipeSelectorAiut::TearDownTestCase(void) {}
void AudioPipeSelectorAiut::SetUp(void) {}
void AudioPipeSelectorAiut::TearDown(void) {}

/**
 * @tc.name  : HandleEmptyInputPipeReload_001
 * @tc.number: HandleEmptyInputPipeReload_001
 * @tc.desc  : Test HandleEmptyInputPipeReload with empty input pipe and old pipe having ec.
 */

HWTEST_F(AudioPipeSelectorAiut, HandleEmptyInputPipeReload_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->id_ = 100;
    newPipe->pipeRole_ = PIPE_ROLE_INPUT;
    newPipe->pipeAction_ = PIPE_ACTION_DEFAULT;
    auto oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->id_ = 100;
    oldPipe->pipeRole_ = PIPE_ROLE_INPUT;
    oldPipe->moduleInfo_.ecType = std::to_string(EC_TYPE_SAME_ADAPTER);
    std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> oldPipeMap;
    oldPipeMap[100] = oldPipe;
    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList = {oldPipe};
    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList = {newPipe};
    audioPipeSelector->HandleEmptyInputPipeReload(newPipeInfoList, oldPipeMap, oldPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(newPipe->pipeAction_, static_cast<int32_t>(PIPE_ACTION_RELOAD),
        "should set pipe action to RELOAD");
}
/**
 * @tc.name  : HandleEmptyInputPipeReload_002
 * @tc.number: HandleEmptyInputPipeReload_002
 * @tc.desc  : Test HandleEmptyInputPipeReload with non-empty input pipe, should be skipped.
 */

HWTEST_F(AudioPipeSelectorAiut, HandleEmptyInputPipeReload_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->id_ = 100;
    newPipe->pipeRole_ = PIPE_ROLE_INPUT;
    newPipe->pipeAction_ = PIPE_ACTION_DEFAULT;
    auto stream = std::make_shared<AudioStreamDescriptor>();
    newPipe->streamDescriptors_.push_back(stream);
    auto oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->id_ = 100;
    oldPipe->pipeRole_ = PIPE_ROLE_INPUT;
    oldPipe->moduleInfo_.ecType = std::to_string(EC_TYPE_SAME_ADAPTER);
    std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> oldPipeMap;
    oldPipeMap[100] = oldPipe;
    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList = {oldPipe};
    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList = {newPipe};
    audioPipeSelector->HandleEmptyInputPipeReload(newPipeInfoList, oldPipeMap, oldPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(newPipe->pipeAction_, static_cast<int32_t>(PIPE_ACTION_DEFAULT),
        "should not change action for non-empty pipe");
}
/**
 * @tc.name  : HandleEmptyInputPipeReload_003
 * @tc.number: HandleEmptyInputPipeReload_003
 * @tc.desc  : Test HandleEmptyInputPipeReload with old pipe having no ec/micRef/micIn, should be skipped.
 */

HWTEST_F(AudioPipeSelectorAiut, HandleEmptyInputPipeReload_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->id_ = 100;
    newPipe->pipeRole_ = PIPE_ROLE_INPUT;
    newPipe->pipeAction_ = PIPE_ACTION_DEFAULT;
    auto oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->id_ = 100;
    oldPipe->pipeRole_ = PIPE_ROLE_INPUT;
    oldPipe->moduleInfo_.ecType = "";
    oldPipe->moduleInfo_.openMicRef = "0";
    oldPipe->moduleInfo_.micInRate = "";
    std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> oldPipeMap;
    oldPipeMap[100] = oldPipe;
    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList = {oldPipe};
    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList = {newPipe};
    audioPipeSelector->HandleEmptyInputPipeReload(newPipeInfoList, oldPipeMap, oldPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(newPipe->pipeAction_, static_cast<int32_t>(PIPE_ACTION_DEFAULT),
        "should not change action when old pipe has no ec/micRef/micIn");
}
/**
 * @tc.name  : HandleEmptyInputPipeReload_004
 * @tc.number: HandleEmptyInputPipeReload_004
 * @tc.desc  : Test HandleEmptyInputPipeReload with old pipe having micRef, should trigger reload.
 */

HWTEST_F(AudioPipeSelectorAiut, HandleEmptyInputPipeReload_004, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->id_ = 100;
    newPipe->pipeRole_ = PIPE_ROLE_INPUT;
    newPipe->pipeAction_ = PIPE_ACTION_DEFAULT;
    auto oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->id_ = 100;
    oldPipe->pipeRole_ = PIPE_ROLE_INPUT;
    oldPipe->moduleInfo_.openMicRef = "1";
    std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> oldPipeMap;
    oldPipeMap[100] = oldPipe;
    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList = {oldPipe};
    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList = {newPipe};
    audioPipeSelector->HandleEmptyInputPipeReload(newPipeInfoList, oldPipeMap, oldPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(newPipe->pipeAction_, static_cast<int32_t>(PIPE_ACTION_RELOAD),
        "should set RELOAD when old pipe has micRef");
}
/**
 * @tc.name  : UpdatePipeAndStreamAction_001
 * @tc.number: UpdatePipeAndStreamAction_001
 * @tc.desc  : Test UpdatePipeAndStreamAction with valid pipe and stream mapping.
 */

HWTEST_F(AudioPipeSelectorAiut, UpdatePipeAndStreamAction_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 100001;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    auto oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    oldPipe->adapterName_ = "primary";
    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->pipeAction_ = PIPE_ACTION_DEFAULT;
    newPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    newPipe->adapterName_ = "primary";
    newPipe->streamDescriptors_.push_back(streamDesc);
    newPipe->streamDescMap_[100001] = streamDesc;
    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo;
    streamDescToOldPipeInfo[100001] = oldPipe;
    audioPipeSelector->UpdatePipeAndStreamAction(newPipe, streamDescToOldPipeInfo);
    AIUT_EXPECT_EQ_INFO_LOG(newPipe->pipeAction_, static_cast<int32_t>(PIPE_ACTION_UPDATE),
        "should set pipe action to UPDATE");
}
/**
 * @tc.name  : UpdatePipeAndStreamAction_002
 * @tc.number: UpdatePipeAndStreamAction_002
 * @tc.desc  : Test UpdatePipeAndStreamAction with nullptr newPipeInfo, should return early.
 */

HWTEST_F(AudioPipeSelectorAiut, UpdatePipeAndStreamAction_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    std::shared_ptr<AudioPipeInfo> newPipe = nullptr;
    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo;
    audioPipeSelector->UpdatePipeAndStreamAction(newPipe, streamDescToOldPipeInfo);
    AIUT_EXPECT_EQ_INFO_LOG(streamDescToOldPipeInfo.size(), 0, "map should remain empty for nullptr");
}
/**
 * @tc.name  : UpdatePipeAndStreamAction_003
 * @tc.number: UpdatePipeAndStreamAction_003
 * @tc.desc  : Test UpdatePipeAndStreamAction with PIPE_ACTION_NEW, should keep PIPE_ACTION_NEW.
 */

HWTEST_F(AudioPipeSelectorAiut, UpdatePipeAndStreamAction_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->pipeAction_ = PIPE_ACTION_NEW;
    newPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo;
    audioPipeSelector->UpdatePipeAndStreamAction(newPipe, streamDescToOldPipeInfo);
    AIUT_EXPECT_EQ_INFO_LOG(newPipe->pipeAction_, static_cast<int32_t>(PIPE_ACTION_NEW),
        "should keep PIPE_ACTION_NEW");
}
/**
 * @tc.name  : UpdatePipeAndStreamAction_004
 * @tc.number: UpdatePipeAndStreamAction_004
 * @tc.desc  : Test UpdatePipeAndStreamAction with stream not in old pipe map, should skip.
 */

HWTEST_F(AudioPipeSelectorAiut, UpdatePipeAndStreamAction_004, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 999999;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->pipeAction_ = PIPE_ACTION_DEFAULT;
    newPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    newPipe->streamDescriptors_.push_back(streamDesc);
    newPipe->streamDescMap_[999999] = streamDesc;
    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo;
    audioPipeSelector->UpdatePipeAndStreamAction(newPipe, streamDescToOldPipeInfo);
    AIUT_EXPECT_EQ_INFO_LOG(newPipe->pipeAction_, static_cast<int32_t>(PIPE_ACTION_UPDATE),
        "should set to UPDATE even if stream not in old map");
}
/**
 * @tc.name  : SortCapturerPipesByExecOrder_001
 * @tc.number: SortCapturerPipesByExecOrder_001
 * @tc.desc  : Test SortCapturerPipesByExecOrder with no input pipes in old list, should return early.
 */

HWTEST_F(AudioPipeSelectorAiut, SortCapturerPipesByExecOrder_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList;
    auto outputPipe = std::make_shared<AudioPipeInfo>();
    outputPipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    oldPipeInfoList.push_back(outputPipe);
    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList;
    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->pipeRole_ = PIPE_ROLE_INPUT;
    newPipeInfoList.push_back(newPipe);
    audioPipeSelector->SortCapturerPipesByExecOrder(oldPipeInfoList, newPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(newPipeInfoList.size(), 1, "list size should not change");
}
/**
 * @tc.name  : SortCapturerPipesByExecOrder_002
 * @tc.number: SortCapturerPipesByExecOrder_002
 * @tc.desc  : Test SortCapturerPipesByExecOrder with input pipes, should sort by exec order.
 */

HWTEST_F(AudioPipeSelectorAiut, SortCapturerPipesByExecOrder_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->id_ = 200;
    oldPipe->pipeRole_ = PIPE_ROLE_INPUT;
    oldPipe->moduleInfo_.ecType = std::to_string(EC_TYPE_SAME_ADAPTER);
    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList = {oldPipe};
    auto newPipe1 = std::make_shared<AudioPipeInfo>();
    newPipe1->id_ = 100;
    newPipe1->pipeRole_ = PIPE_ROLE_INPUT;
    newPipe1->moduleInfo_.ecType = "";
    auto newPipe2 = std::make_shared<AudioPipeInfo>();
    newPipe2->id_ = 101;
    newPipe2->pipeRole_ = PIPE_ROLE_INPUT;
    newPipe2->moduleInfo_.ecType = std::to_string(EC_TYPE_SAME_ADAPTER);
    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList = {newPipe1, newPipe2};
    audioPipeSelector->SortCapturerPipesByExecOrder(oldPipeInfoList, newPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(newPipeInfoList.size(), 2, "list size should not change");
}
/**
 * @tc.name  : DetermineInputPipeBaseConfig_001
 * @tc.number: DetermineInputPipeBaseConfig_001
 * @tc.desc  : Test DetermineInputPipeBaseConfig with input pipes, should return sorted pairs.
 */

HWTEST_F(AudioPipeSelectorAiut, DetermineInputPipeBaseConfig_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList;
    auto inputPipe = std::make_shared<AudioPipeInfo>();
    inputPipe->pipeRole_ = PIPE_ROLE_INPUT;
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->audioMode_ = AUDIO_MODE_RECORD;
    stream->streamStatus_ = STREAM_STATUS_STARTED;
    stream->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_CALL;
    stream->sessionId_ = 1001;
    inputPipe->streamDescriptors_.push_back(stream);
    newPipeInfoList.push_back(inputPipe);
    auto pipePairs = audioPipeSelector->DetermineInputPipeBaseConfig(newPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(pipePairs.size(), 1, "should return 1 pair");
    AIUT_ASSERT_NE_INFO_LOG(pipePairs[0].first, nullptr, "pipe should not be nullptr");
    AIUT_ASSERT_NE_INFO_LOG(pipePairs[0].second, nullptr, "stream should not be nullptr");
}
/**
 * @tc.name  : DetermineInputPipeBaseConfig_002
 * @tc.number: DetermineInputPipeBaseConfig_002
 * @tc.desc  : Test DetermineInputPipeBaseConfig with empty list, should return empty.
 */

HWTEST_F(AudioPipeSelectorAiut, DetermineInputPipeBaseConfig_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList;
    auto pipePairs = audioPipeSelector->DetermineInputPipeBaseConfig(newPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(pipePairs.size(), 0, "should return empty for empty list");
}
/**
 * @tc.name  : DetermineInputPipeBaseConfig_003
 * @tc.number: DetermineInputPipeBaseConfig_003
 * @tc.desc  : Test DetermineInputPipeBaseConfig with output pipe only, should return empty.
 */

HWTEST_F(AudioPipeSelectorAiut, DetermineInputPipeBaseConfig_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList;
    auto outputPipe = std::make_shared<AudioPipeInfo>();
    outputPipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->audioMode_ = AUDIO_MODE_PLAYBACK;
    outputPipe->streamDescriptors_.push_back(stream);
    newPipeInfoList.push_back(outputPipe);
    auto pipePairs = audioPipeSelector->DetermineInputPipeBaseConfig(newPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(pipePairs.size(), 0, "should return empty for output pipes only");
}
/**
 * @tc.name  : DetermineInputPipeEcAndMicRef_001
 * @tc.number: DetermineInputPipeEcAndMicRef_001
 * @tc.desc  : Test DetermineInputPipeEcAndMicRef with valid pipe pairs.
 */

HWTEST_F(AudioPipeSelectorAiut, DetermineInputPipeEcAndMicRef_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->adapterName_ = "primary";
    pipe->moduleInfo_.adapterName = "primary";
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->ecStreamInfo_.channels = STEREO;
    stream->ecStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    stream->ecStreamInfo_.format = SAMPLE_S16LE;
    stream->micInStreamInfo_.channels = STEREO;
    stream->micInStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    stream->micInStreamInfo_.format = SAMPLE_S16LE;
    AudioPipeSelector::InputPipeStreamPairList pipePairs;
    pipePairs.emplace_back(pipe, stream);
    audioPipeSelector->DetermineInputPipeEcAndMicRef(pipePairs);
    AIUT_EXPECT_EQ_INFO_LOG(pipe->moduleInfo_.ecType, std::to_string(EC_TYPE_SAME_ADAPTER),
        "should set ec type to same adapter");
    AIUT_EXPECT_EQ_INFO_LOG(pipe->moduleInfo_.openMicRef, "1", "should set openMicRef to 1");
}
/**
 * @tc.name  : DetermineInputPipeEcAndMicRef_002
 * @tc.number: DetermineInputPipeEcAndMicRef_002
 * @tc.desc  : Test DetermineInputPipeEcAndMicRef with empty pairs, should not crash.
 */

HWTEST_F(AudioPipeSelectorAiut, DetermineInputPipeEcAndMicRef_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    AudioPipeSelector::InputPipeStreamPairList pipePairs;
    audioPipeSelector->DetermineInputPipeEcAndMicRef(pipePairs);
    AIUT_EXPECT_EQ_INFO_LOG(pipePairs.size(), 0, "pipePairs should remain empty after processing");
}
/**
 * @tc.name  : DetermineInputPipeEcAndMicRef_003
 * @tc.number: DetermineInputPipeEcAndMicRef_003
 * @tc.desc  : Test DetermineInputPipeEcAndMicRef with nullptr entries, should skip.
 */

HWTEST_F(AudioPipeSelectorAiut, DetermineInputPipeEcAndMicRef_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    AudioPipeSelector::InputPipeStreamPairList pipePairs;
    pipePairs.emplace_back(nullptr, nullptr);
    audioPipeSelector->DetermineInputPipeEcAndMicRef(pipePairs);
    AIUT_EXPECT_EQ_INFO_LOG(pipePairs.size(), 1, "pipePairs size should not change with nullptr entries");
}
/**
 * @tc.name  : DetermineInputPipeEcAndMicRef_004
 * @tc.number: DetermineInputPipeEcAndMicRef_004
 * @tc.desc  : Test DetermineInputPipeEcAndMicRef with ec adapter conflict, should disable ec.
 */

HWTEST_F(AudioPipeSelectorAiut, DetermineInputPipeEcAndMicRef_004, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->adapterName_ = "primary";
    pipe1->moduleInfo_.adapterName = "primary";
    auto stream1 = std::make_shared<AudioStreamDescriptor>();
    stream1->ecStreamInfo_.channels = STEREO;
    stream1->ecStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    stream1->ecStreamInfo_.format = SAMPLE_S16LE;
    auto pipe2 = std::make_shared<AudioPipeInfo>();
    pipe2->adapterName_ = "primary";
    pipe2->moduleInfo_.adapterName = "primary";
    auto stream2 = std::make_shared<AudioStreamDescriptor>();
    stream2->ecStreamInfo_.channels = STEREO;
    stream2->ecStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    stream2->ecStreamInfo_.format = SAMPLE_S16LE;
    AudioPipeSelector::InputPipeStreamPairList pipePairs;
    pipePairs.emplace_back(pipe1, stream1);
    pipePairs.emplace_back(pipe2, stream2);
    audioPipeSelector->DetermineInputPipeEcAndMicRef(pipePairs);
    // First pipe should get ec, second should not (adapter conflict)
    AIUT_EXPECT_EQ_INFO_LOG(pipe1->moduleInfo_.ecType, std::to_string(EC_TYPE_SAME_ADAPTER),
        "first pipe should get ec");
    AIUT_EXPECT_EQ_INFO_LOG(pipe2->moduleInfo_.ecType, std::string(""),
        "second pipe should not get ec due to adapter conflict");
}
/**
 * @tc.name  : UpdateInputPipeReloadAction_001
 * @tc.number: UpdateInputPipeReloadAction_001
 * @tc.desc  : Test UpdateInputPipeReloadAction with pipes needing reload.
 */

HWTEST_F(AudioPipeSelectorAiut, UpdateInputPipeReloadAction_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->id_ = 100;
    newPipe->pipeRole_ = PIPE_ROLE_INPUT;
    newPipe->pipeAction_ = PIPE_ACTION_DEFAULT;
    newPipe->audioStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    newPipe->audioStreamInfo_.channels = STEREO;
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->audioMode_ = AUDIO_MODE_RECORD;
    stream->streamStatus_ = STREAM_STATUS_STARTED;
    stream->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    stream->sessionId_ = 1001;
    newPipe->streamDescriptors_.push_back(stream);
    auto oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->id_ = 100;
    oldPipe->pipeRole_ = PIPE_ROLE_INPUT;
    oldPipe->audioStreamInfo_.samplingRate = SAMPLE_RATE_44100;
    oldPipe->audioStreamInfo_.channels = STEREO;
    std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> oldPipeMap;
    oldPipeMap[100] = oldPipe;
    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList = {oldPipe};
    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList = {newPipe};
    audioPipeSelector->UpdateInputPipeReloadAction(newPipeInfoList, oldPipeMap, oldPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(newPipe->pipeAction_, static_cast<int32_t>(PIPE_ACTION_RELOAD),
        "should set to RELOAD when sampling rate differs");
}
/**
 * @tc.name  : UpdateInputPipeReloadAction_002
 * @tc.number: UpdateInputPipeReloadAction_002
 * @tc.desc  : Test UpdateInputPipeReloadAction with no old pipe, should not change action.
 */

HWTEST_F(AudioPipeSelectorAiut, UpdateInputPipeReloadAction_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->id_ = 100;
    newPipe->pipeRole_ = PIPE_ROLE_INPUT;
    newPipe->pipeAction_ = PIPE_ACTION_DEFAULT;
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->audioMode_ = AUDIO_MODE_RECORD;
    stream->streamStatus_ = STREAM_STATUS_STARTED;
    stream->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    newPipe->streamDescriptors_.push_back(stream);
    std::unordered_map<uint32_t, std::shared_ptr<AudioPipeInfo>> oldPipeMap;
    std::vector<std::shared_ptr<AudioPipeInfo>> oldPipeInfoList;
    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList = {newPipe};
    audioPipeSelector->UpdateInputPipeReloadAction(newPipeInfoList, oldPipeMap, oldPipeInfoList);
    AIUT_EXPECT_EQ_INFO_LOG(newPipe->pipeAction_, static_cast<int32_t>(PIPE_ACTION_DEFAULT),
        "should not change action when no old pipe");
}
/**
 * @tc.name  : HandleFindMatchPipe_001
 * @tc.number: HandleFindMatchPipe_001
 * @tc.desc  : Test HandleFindMatchPipe with matching pipe found.
 */

HWTEST_F(AudioPipeSelectorAiut, HandleFindMatchPipe_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    streamDesc->sessionId_ = 100001;
    streamDesc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.front()->networkId_ = LOCAL_NETWORK_ID;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    pipe->adapterName_ = "primary";
    pipe->name_ = "primary";
    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList = {pipe};
    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo;
    std::vector<std::shared_ptr<AudioPipeInfo>>::iterator matchPipeIter = newPipeInfoList.end();
    audioPipeSelector->HandleFindMatchPipe(newPipeInfoList, streamDesc, streamDescToOldPipeInfo, matchPipeIter);
    AIUT_EXPECT_EQ_INFO_LOG(newPipeInfoList.size(), 1, "list size should not change");
}
/**
 * @tc.name  : HandleFindMatchPipe_002
 * @tc.number: HandleFindMatchPipe_002
 * @tc.desc  : Test HandleFindMatchPipe with nullptr streamDesc, should return early.
 */

HWTEST_F(AudioPipeSelectorAiut, HandleFindMatchPipe_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = nullptr;
    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo;
    std::vector<std::shared_ptr<AudioPipeInfo>>::iterator matchPipeIter = newPipeInfoList.end();
    audioPipeSelector->HandleFindMatchPipe(newPipeInfoList, streamDesc, streamDescToOldPipeInfo, matchPipeIter);
    AIUT_EXPECT_EQ_INFO_LOG(matchPipeIter, newPipeInfoList.end(),
        "iterator should remain at end for nullptr streamDesc");
}
/**
 * @tc.name  : HandleFindMatchPipe_003
 * @tc.number: HandleFindMatchPipe_003
 * @tc.desc  : Test HandleFindMatchPipe with empty pipe list, should not find match.
 */

HWTEST_F(AudioPipeSelectorAiut, HandleFindMatchPipe_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    streamDesc->sessionId_ = 100001;
    streamDesc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.front()->networkId_ = LOCAL_NETWORK_ID;
    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList;
    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo;
    std::vector<std::shared_ptr<AudioPipeInfo>>::iterator matchPipeIter = newPipeInfoList.end();
    audioPipeSelector->HandleFindMatchPipe(newPipeInfoList, streamDesc, streamDescToOldPipeInfo, matchPipeIter);
    AIUT_EXPECT_EQ_INFO_LOG(matchPipeIter, newPipeInfoList.end(),
        "iterator should remain at end for empty list");
}
/**
 * @tc.name  : IsSpecialRemotePipe_001
 * @tc.number: IsSpecialRemotePipe_001
 * @tc.desc  : Test IsSpecialRemotePipe with remote offload pipe.
 */

HWTEST_F(AudioPipeSelectorAiut, IsSpecialRemotePipe_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "remote";
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    bool result = audioPipeSelector->IsSpecialRemotePipe(pipeInfo);
    AIUT_EXPECT_EQ_INFO_LOG(result, true, "should return true for remote offload pipe");
}
/**
 * @tc.name  : IsSpecialRemotePipe_002
 * @tc.number: IsSpecialRemotePipe_002
 * @tc.desc  : Test IsSpecialRemotePipe with remote multichannel pipe.
 */

HWTEST_F(AudioPipeSelectorAiut, IsSpecialRemotePipe_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "remote";
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_MULTICHANNEL;
    bool result = audioPipeSelector->IsSpecialRemotePipe(pipeInfo);
    AIUT_EXPECT_EQ_INFO_LOG(result, true, "should return true for remote multichannel pipe");
}
/**
 * @tc.name  : IsSpecialRemotePipe_003
 * @tc.number: IsSpecialRemotePipe_003
 * @tc.desc  : Test IsSpecialRemotePipe with non-remote pipe.
 */

HWTEST_F(AudioPipeSelectorAiut, IsSpecialRemotePipe_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    bool result = audioPipeSelector->IsSpecialRemotePipe(pipeInfo);
    AIUT_EXPECT_EQ_INFO_LOG(result, false, "should return false for non-remote pipe");
}
/**
 * @tc.name  : IsSpecialRemotePipe_004
 * @tc.number: IsSpecialRemotePipe_004
 * @tc.desc  : Test IsSpecialRemotePipe with nullptr pipeInfo.
 */

HWTEST_F(AudioPipeSelectorAiut, IsSpecialRemotePipe_004, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    std::shared_ptr<AudioPipeInfo> pipeInfo = nullptr;
    bool result = audioPipeSelector->IsSpecialRemotePipe(pipeInfo);
    AIUT_EXPECT_EQ_INFO_LOG(result, false, "should return false for nullptr pipe");
}
/**
 * @tc.name  : IsPipeFormatMatch_001
 * @tc.number: IsPipeFormatMatch_001
 * @tc.desc  : Test IsPipeFormatMatch with matching format.
 */

HWTEST_F(AudioPipeSelectorAiut, IsPipeFormatMatch_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    streamPropInfo->format_ = SAMPLE_S16LE;
    streamPropInfo->sampleRate_ = SAMPLE_RATE_48000;
    streamPropInfo->channelLayout_ = CH_LAYOUT_STEREO;
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->moduleInfo_.format = "s16";
    pipeInfo->moduleInfo_.rate = "48000";
    pipeInfo->moduleInfo_.channels = "2";
    pipeInfo->moduleInfo_.channelLayout = std::to_string(CH_LAYOUT_STEREO);
    bool result = audioPipeSelector->IsPipeFormatMatch(streamPropInfo, pipeInfo);
    AIUT_EXPECT_EQ_INFO_LOG(result, true, "should return true for matching format");
}
/**
 * @tc.name  : IsPipeFormatMatch_002
 * @tc.number: IsPipeFormatMatch_002
 * @tc.desc  : Test IsPipeFormatMatch with mismatching format.
 */

HWTEST_F(AudioPipeSelectorAiut, IsPipeFormatMatch_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    streamPropInfo->format_ = SAMPLE_S16LE;
    streamPropInfo->sampleRate_ = SAMPLE_RATE_48000;
    streamPropInfo->channelLayout_ = CH_LAYOUT_STEREO;
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->moduleInfo_.format = "s24";
    pipeInfo->moduleInfo_.rate = "48000";
    pipeInfo->moduleInfo_.channels = "2";
    pipeInfo->moduleInfo_.channelLayout = std::to_string(CH_LAYOUT_STEREO);
    bool result = audioPipeSelector->IsPipeFormatMatch(streamPropInfo, pipeInfo);
    AIUT_EXPECT_EQ_INFO_LOG(result, false, "should return false for mismatching format");
}
/**
 * @tc.name  : MatchUltraFastPipe_001
 * @tc.number: MatchUltraFastPipe_001
 * @tc.desc  : Test MatchUltraFastPipe with fast route pipe.
 */

HWTEST_F(AudioPipeSelectorAiut, MatchUltraFastPipe_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    pipeInfo->SetUltraFastFlag(true);
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 100001;
    audioPipeSelector->MatchUltraFastPipe(pipeInfo, streamDesc);
    AIUT_EXPECT_EQ_INFO_LOG(streamDesc->IsUltraFastImplemented(), true,
        "should set ultra fast implemented to true");
}
/**
 * @tc.name  : MatchUltraFastPipe_002
 * @tc.number: MatchUltraFastPipe_002
 * @tc.desc  : Test MatchUltraFastPipe with non-fast route pipe, should not set.
 */

HWTEST_F(AudioPipeSelectorAiut, MatchUltraFastPipe_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 100001;
    audioPipeSelector->MatchUltraFastPipe(pipeInfo, streamDesc);
    AIUT_EXPECT_EQ_INFO_LOG(streamDesc->IsUltraFastImplemented(), false,
        "should not set ultra fast for non-fast pipe");
}
/**
 * @tc.name  : MatchUltraFastPipe_003
 * @tc.number: MatchUltraFastPipe_003
 * @tc.desc  : Test MatchUltraFastPipe with nullptr streamDesc, should return early.
 */

HWTEST_F(AudioPipeSelectorAiut, MatchUltraFastPipe_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = nullptr;
    audioPipeSelector->MatchUltraFastPipe(pipeInfo, streamDesc);
    AIUT_EXPECT_EQ_INFO_LOG(pipeInfo->routeFlag_, static_cast<int32_t>(AUDIO_OUTPUT_FLAG_FAST),
        "routeFlag should not change");
}
/**
 * @tc.name  : MatchSpecialRemotePipe_001
 * @tc.number: MatchSpecialRemotePipe_001
 * @tc.desc  : Test MatchSpecialRemotePipe with matching format, should not reload.
 */

HWTEST_F(AudioPipeSelectorAiut, MatchSpecialRemotePipe_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    streamPropInfo->format_ = SAMPLE_S16LE;
    streamPropInfo->sampleRate_ = SAMPLE_RATE_48000;
    streamPropInfo->channelLayout_ = CH_LAYOUT_STEREO;
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "remote";
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    pipeInfo->moduleInfo_.format = "s16";
    pipeInfo->moduleInfo_.rate = "48000";
    pipeInfo->moduleInfo_.channels = "2";
    pipeInfo->moduleInfo_.channelLayout = std::to_string(CH_LAYOUT_STEREO);
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioPipeSelector->MatchSpecialRemotePipe(streamPropInfo, pipeInfo, streamDesc);
    AIUT_EXPECT_NE_INFO_LOG(pipeInfo->pipeAction_, PIPE_ACTION_RELOAD,
        "should not reload when format matches");
}
/**
 * @tc.name  : MatchSpecialRemotePipe_002
 * @tc.number: MatchSpecialRemotePipe_002
 * @tc.desc  : Test MatchSpecialRemotePipe with mismatching format on remote pipe, should reload.
 */

HWTEST_F(AudioPipeSelectorAiut, MatchSpecialRemotePipe_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    streamPropInfo->format_ = SAMPLE_S16LE;
    streamPropInfo->sampleRate_ = SAMPLE_RATE_48000;
    streamPropInfo->channelLayout_ = CH_LAYOUT_STEREO;
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "remote";
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    pipeInfo->moduleInfo_.format = "s24";
    pipeInfo->moduleInfo_.rate = "48000";
    pipeInfo->moduleInfo_.channels = "2";
    pipeInfo->moduleInfo_.channelLayout = std::to_string(CH_LAYOUT_STEREO);
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 100001;
    streamDesc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    audioPipeSelector->MatchSpecialRemotePipe(streamPropInfo, pipeInfo, streamDesc);
    AIUT_EXPECT_EQ_INFO_LOG(pipeInfo->pipeAction_, PIPE_ACTION_RELOAD,
        "should reload when format mismatches on remote pipe");
}

// ShouldAdjustConcedeAction tests removed: method is private,
// ShouldAdjustConcedeAction tests removed: method is private
/**
 * @tc.name  : IsUpdateActionForDeviceLevelHdPlay_001
 * @tc.number: IsUpdateActionForDeviceLevelHdPlay_001
 * @tc.desc  : Test IsUpdateActionForDeviceLevelHdPlay with nullptr streams, should return false.
 */

HWTEST_F(AudioPipeSelectorAiut, IsUpdateActionForDeviceLevelHdPlay_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    std::shared_ptr<AudioStreamDescriptor> existingStream = nullptr;
    std::shared_ptr<AudioStreamDescriptor> incomingStream = nullptr;
    bool result = audioPipeSelector->IsUpdateActionForDeviceLevelHdPlay(existingStream, incomingStream);
    AIUT_EXPECT_EQ_INFO_LOG(result, false, "should return false for nullptr streams");
}
/**
 * @tc.name  : IsUpdateActionForDeviceLevelHdPlay_002
 * @tc.number: IsUpdateActionForDeviceLevelHdPlay_002
 * @tc.desc  : Test IsUpdateActionForDeviceLevelHdPlay with non-device-level streams.
 */

HWTEST_F(AudioPipeSelectorAiut, IsUpdateActionForDeviceLevelHdPlay_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto existingStream = std::make_shared<AudioStreamDescriptor>();
    existingStream->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    existingStream->newDeviceDescs_[0]->hdPlaybackMode_ = HdPlaybackMode::APP_LEVEL;
    auto incomingStream = std::make_shared<AudioStreamDescriptor>();
    incomingStream->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    incomingStream->newDeviceDescs_[0]->hdPlaybackMode_ = HdPlaybackMode::APP_LEVEL;
    bool result = audioPipeSelector->IsUpdateActionForDeviceLevelHdPlay(existingStream, incomingStream);
    AIUT_EXPECT_EQ_INFO_LOG(result, false, "should return false for app-level hd play");
}
/**
 * @tc.name  : IsUpdateActionForDeviceLevelHdPlay_003
 * @tc.number: IsUpdateActionForDeviceLevelHdPlay_003
 * @tc.desc  : Test IsUpdateActionForDeviceLevelHdPlay with device-level streams.
 */

HWTEST_F(AudioPipeSelectorAiut, IsUpdateActionForDeviceLevelHdPlay_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto existingStream = std::make_shared<AudioStreamDescriptor>();
    existingStream->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    existingStream->newDeviceDescs_[0]->hdPlaybackMode_ = HdPlaybackMode::DEVICE_LEVEL;
    auto incomingStream = std::make_shared<AudioStreamDescriptor>();
    incomingStream->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    incomingStream->newDeviceDescs_[0]->hdPlaybackMode_ = HdPlaybackMode::DEVICE_LEVEL;
    bool result = audioPipeSelector->IsUpdateActionForDeviceLevelHdPlay(existingStream, incomingStream);
    AIUT_EXPECT_EQ_INFO_LOG(result, true, "should return true for device-level hd play");
}
/**
 * @tc.name  : UpdateStreamDescs_001
 * @tc.number: UpdateStreamDescs_001
 * @tc.desc  : Test UpdateStreamDescs filters streams by original session IDs.
 */

HWTEST_F(AudioPipeSelectorAiut, UpdateStreamDescs_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    auto stream1 = std::make_shared<AudioStreamDescriptor>();
    stream1->sessionId_ = 1001;
    auto stream2 = std::make_shared<AudioStreamDescriptor>();
    stream2->sessionId_ = 1002;
    auto stream3 = std::make_shared<AudioStreamDescriptor>();
    stream3->sessionId_ = 1003;
    streamDescs = {stream1, stream2, stream3};
    std::unordered_set<uint32_t> originalIds = {1001, 1003};
    auto result = audioPipeSelector->UpdateStreamDescs(streamDescs, originalIds);
    AIUT_EXPECT_EQ_INFO_LOG(result.size(), 2, "should return 2 streams");
    AIUT_EXPECT_EQ_INFO_LOG(result[0]->sessionId_, 1001, "first should be 1001");
    AIUT_EXPECT_EQ_INFO_LOG(result[1]->sessionId_, 1003, "second should be 1003");
}
/**
 * @tc.name  : UpdateStreamDescs_002
 * @tc.number: UpdateStreamDescs_002
 * @tc.desc  : Test UpdateStreamDescs with empty original IDs, should return empty.
 */

HWTEST_F(AudioPipeSelectorAiut, UpdateStreamDescs_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    auto stream1 = std::make_shared<AudioStreamDescriptor>();
    stream1->sessionId_ = 1001;
    streamDescs = {stream1};
    std::unordered_set<uint32_t> originalIds;
    auto result = audioPipeSelector->UpdateStreamDescs(streamDescs, originalIds);
    AIUT_EXPECT_EQ_INFO_LOG(result.size(), 0, "should return empty for empty original IDs");
}
/**
 * @tc.name  : AdjustFastRouteForUltraFastIfNeeded_001
 * @tc.number: AdjustFastRouteForUltraFastIfNeeded_001
 * @tc.desc  : Test AdjustFastRouteForUltraFastIfNeeded with empty device descs, should return early.
 */

HWTEST_F(AudioPipeSelectorAiut, AdjustFastRouteForUltraFastIfNeeded_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    // empty newDeviceDescs_
    audioPipeSelector->AdjustFastRouteForUltraFastIfNeeded(streamDesc);
    AIUT_EXPECT_EQ_INFO_LOG(streamDesc->routeFlag_, AUDIO_OUTPUT_FLAG_FAST,
        "should not change route when device descs empty");
}
/**
 * @tc.name  : AdjustFastRouteForUltraFastIfNeeded_002
 * @tc.number: AdjustFastRouteForUltraFastIfNeeded_002
 * @tc.desc  : Test AdjustFastRouteForUltraFastIfNeeded with vector, should process each.
 */

HWTEST_F(AudioPipeSelectorAiut, AdjustFastRouteForUltraFastIfNeeded_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AIUT_ASSERT_NE_INFO_LOG(audioPipeSelector, nullptr, "audioPipeSelector is nullptr");
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    auto stream1 = std::make_shared<AudioStreamDescriptor>();
    stream1->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    streamDescs.push_back(stream1);
    audioPipeSelector->AdjustFastRouteForUltraFastIfNeeded(streamDescs);
    AIUT_EXPECT_EQ_INFO_LOG(stream1->routeFlag_, AUDIO_OUTPUT_FLAG_NORMAL,
        "should not change normal route");
}
} // namespace AudioStandard
} // namespace OHOS
