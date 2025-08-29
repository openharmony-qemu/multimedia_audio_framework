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
#include "audio_policy_utils.h"
#include "audio_pipe_selector_unit_test.h"
#include "audio_stream_descriptor.h"
#include "audio_stream_descriptor.h"
#include "audio_stream_enum.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioPipeSelectorUnitTest::SetUpTestCase(void) {}
void AudioPipeSelectorUnitTest::TearDownTestCase(void) {}
void AudioPipeSelectorUnitTest::SetUp(void) {}
void AudioPipeSelectorUnitTest::TearDown(void) {}

/**
 * @tc.name: GetPipeType_001
 * @tc.desc: Test GetPipeType when audioMode is AUDIO_MODE_PLAYBACK and flag contains
 *  AUDIO_OUTPUT_FLAG_FAST and AUDIO_OUTPUT_FLAG_VOIP.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, GetPipeType_001, TestSize.Level1)
{
    uint32_t flag = AUDIO_OUTPUT_FLAG_FAST | AUDIO_OUTPUT_FLAG_VOIP;
    AudioMode audioMode = AUDIO_MODE_PLAYBACK;
    AudioPipeType result = AudioPipeSelector::GetPipeSelector()->GetPipeType(flag, audioMode);
    EXPECT_EQ(result, PIPE_TYPE_CALL_OUT);
}

/**
 * @tc.name: GetPipeType_002
 * @tc.desc: Test GetPipeType when audioMode is AUDIO_MODE_PLAYBACK and flag contains AUDIO_OUTPUT_FLAG_FAST.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, GetPipeType_002, TestSize.Level1)
{
    uint32_t flag = AUDIO_OUTPUT_FLAG_FAST;
    AudioMode audioMode = AUDIO_MODE_PLAYBACK;
    AudioPipeType result = AudioPipeSelector::GetPipeSelector()->GetPipeType(flag, audioMode);
    EXPECT_EQ(result, PIPE_TYPE_LOWLATENCY_OUT);
}

/**
 * @tc.name: GetPipeType_003
 * @tc.desc: Test GetPipeType when audioMode is AUDIO_MODE_PLAYBACK and flag contains
 *  AUDIO_OUTPUT_FLAG_DIRECT and AUDIO_OUTPUT_FLAG_VOIP.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, GetPipeType_003, TestSize.Level1)
{
    uint32_t flag = AUDIO_OUTPUT_FLAG_DIRECT | AUDIO_OUTPUT_FLAG_VOIP;
    AudioMode audioMode = AUDIO_MODE_PLAYBACK;
    AudioPipeType result = AudioPipeSelector::GetPipeSelector()->GetPipeType(flag, audioMode);
    EXPECT_EQ(result, PIPE_TYPE_CALL_OUT);
}

/**
 * @tc.name: GetPipeType_004
 * @tc.desc: Test GetPipeType when audioMode is AUDIO_MODE_PLAYBACK and flag contains AUDIO_OUTPUT_FLAG_DIRECT.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, GetPipeType_004, TestSize.Level1)
{
    uint32_t flag = AUDIO_OUTPUT_FLAG_DIRECT;
    AudioMode audioMode = AUDIO_MODE_PLAYBACK;
    AudioPipeType result = AudioPipeSelector::GetPipeSelector()->GetPipeType(flag, audioMode);
    EXPECT_EQ(result, PIPE_TYPE_DIRECT_OUT);
}

/**
 * @tc.name: GetPipeType_005
 * @tc.desc: Test GetPipeType when audioMode is AUDIO_MODE_PLAYBACK and flag contains
 *  AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, GetPipeType_005, TestSize.Level1)
{
    uint32_t flag = AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD;
    AudioMode audioMode = AUDIO_MODE_PLAYBACK;
    AudioPipeType result = AudioPipeSelector::GetPipeSelector()->GetPipeType(flag, audioMode);
    EXPECT_EQ(result, PIPE_TYPE_OFFLOAD);
}

/**
 * @tc.name: GetPipeType_006
 * @tc.desc: Test GetPipeType when audioMode is AUDIO_MODE_PLAYBACK and flag contains AUDIO_OUTPUT_FLAG_MULTICHANNEL.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, GetPipeType_006, TestSize.Level1)
{
    uint32_t flag = AUDIO_OUTPUT_FLAG_MULTICHANNEL;
    AudioMode audioMode = AUDIO_MODE_PLAYBACK;
    AudioPipeType result = AudioPipeSelector::GetPipeSelector()->GetPipeType(flag, audioMode);
    EXPECT_EQ(result, PIPE_TYPE_MULTICHANNEL);
}

/**
 * @tc.name: GetPipeType_007
 * @tc.desc: Test GetPipeType when audioMode is AUDIO_MODE_PLAYBACK and flag does not contain any specific flags.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, GetPipeType_007, TestSize.Level1)
{
    uint32_t flag = AUDIO_OUTPUT_FLAG_NORMAL;
    AudioMode audioMode = AUDIO_MODE_PLAYBACK;
    AudioPipeType result = AudioPipeSelector::GetPipeSelector()->GetPipeType(flag, audioMode);
    EXPECT_EQ(result, PIPE_TYPE_NORMAL_OUT);
}

/**
 * @tc.name: GetPipeType_008
 * @tc.desc: Test GetPipeType when audioMode is AUDIO_MODE_RECORD and flag contains
 *  AUDIO_INPUT_FLAG_FAST and AUDIO_INPUT_FLAG_VOIP.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, GetPipeType_008, TestSize.Level1)
{
    uint32_t flag = AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_VOIP;
    AudioMode audioMode = AUDIO_MODE_RECORD;
    AudioPipeType result = AudioPipeSelector::GetPipeSelector()->GetPipeType(flag, audioMode);
    EXPECT_EQ(result, PIPE_TYPE_CALL_IN);
}

/**
 * @tc.name: GetPipeType_009
 * @tc.desc: Test GetPipeType when audioMode is AUDIO_MODE_RECORD and flag contains AUDIO_INPUT_FLAG_FAST.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, GetPipeType_009, TestSize.Level1)
{
    uint32_t flag = AUDIO_INPUT_FLAG_FAST;
    AudioMode audioMode = AUDIO_MODE_RECORD;
    AudioPipeType result = AudioPipeSelector::GetPipeSelector()->GetPipeType(flag, audioMode);
    EXPECT_EQ(result, PIPE_TYPE_LOWLATENCY_IN);
}

/**
 * @tc.name: GetPipeType_010
 * @tc.desc: Test GetPipeType when audioMode is AUDIO_MODE_RECORD and flag does not contain any specific flags.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, GetPipeType_010, TestSize.Level1)
{
    uint32_t flag = AUDIO_INPUT_FLAG_NORMAL;
    AudioMode audioMode = AUDIO_MODE_RECORD;
    AudioPipeType result = AudioPipeSelector::GetPipeSelector()->GetPipeType(flag, audioMode);
    EXPECT_EQ(result, PIPE_TYPE_NORMAL_IN);
}

/**
 * @tc.name: GetAdapterNameByStreamDesc_001
 * @tc.desc: Test GetAdapterNameByStreamDesc when streamDesc is not nullptr and pipeInfoPtr
 *  and adapterInfoPtr are not nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, GetAdapterNameByStreamDesc_001, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.front()->networkId_ = "0";
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S16LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::string result = audioPipeSelector->GetAdapterNameByStreamDesc(streamDesc);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: ConvertStreamDescToPipeInfo_001
 * @tc.desc: Test ConvertStreamDescToPipeInfo when pipeInfoPtr and adapterInfoPtr are not nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, ConvertStreamDescToPipeInfo_001, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->routeFlag_ = 1;
    streamDesc->sessionId_ = 100;
    streamDesc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.front()->networkId_ = "0";
    streamDesc->capturerInfo_.sourceType = SourceType::SOURCE_TYPE_MIC;

    std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    streamPropInfo->format_ = AudioSampleFormat::SAMPLE_S16LE;
    streamPropInfo->sampleRate_ = 44100;
    streamPropInfo->channelLayout_ = AudioChannelLayout::CH_LAYOUT_STEREO;
    streamPropInfo->bufferSize_ = 1024;

    std::shared_ptr<AdapterPipeInfo> pipeInfoPtr = std::make_shared<AdapterPipeInfo>();
    pipeInfoPtr->paProp_.lib_ = "test_lib";
    pipeInfoPtr->paProp_.role_ = "test_role";
    pipeInfoPtr->paProp_.moduleName_ = "test_module";
    pipeInfoPtr->name_ = "test_name";
    pipeInfoPtr->role_ = PIPE_ROLE_OUTPUT;

    std::shared_ptr<PolicyAdapterInfo> adapterInfoPtr = std::make_shared<PolicyAdapterInfo>();
    adapterInfoPtr->adapterName = "test_adapter";

    pipeInfoPtr->adapterInfo_ = adapterInfoPtr;
    streamPropInfo->pipeInfo_ = pipeInfoPtr;

    AudioPipeInfo info;
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    audioPipeSelector->ConvertStreamDescToPipeInfo(streamDesc, streamPropInfo, info);
    EXPECT_EQ(info.pipeRole_, PIPE_ROLE_OUTPUT);

    pipeInfoPtr->name_ = "multichannel_output";
    audioPipeSelector->ConvertStreamDescToPipeInfo(streamDesc, streamPropInfo, info);
    EXPECT_EQ(info.pipeRole_, PIPE_ROLE_OUTPUT);
    pipeInfoPtr->name_ = "offload_output";
    audioPipeSelector->ConvertStreamDescToPipeInfo(streamDesc, streamPropInfo, info);
    EXPECT_EQ(info.pipeRole_, PIPE_ROLE_OUTPUT);
    pipeInfoPtr->name_ = "offload_distributed_output";
    audioPipeSelector->ConvertStreamDescToPipeInfo(streamDesc, streamPropInfo, info);
    EXPECT_EQ(info.pipeRole_, PIPE_ROLE_OUTPUT);
}

/**
 * @tc.name: JudgeStreamAction_001
 * @tc.desc: Test JudgeStreamAction when newPipe and oldPipe have the same adapterName and routeFlag.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, JudgeStreamAction_001, TestSize.Level1)
{
    std::shared_ptr<AudioPipeInfo> newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->adapterName_ = "test_adapter";
    newPipe->routeFlag_ = 1;

    std::shared_ptr<AudioPipeInfo> oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->adapterName_ = "test_adapter";
    oldPipe->routeFlag_ = 1;
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AudioStreamAction result = audioPipeSelector->JudgeStreamAction(newPipe, oldPipe);
    EXPECT_EQ(result, AUDIO_STREAM_ACTION_DEFAULT);
}

/**
 * @tc.name: JudgeStreamAction_002
 * @tc.desc: Test JudgeStreamAction when newPipe and oldPipe have different adapterName and
 *  routeFlag, and neither is FAST or DIRECT.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, JudgeStreamAction_002, TestSize.Level1)
{
    std::shared_ptr<AudioPipeInfo> newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->adapterName_ = "new_adapter";
    newPipe->routeFlag_ = 0x1234;

    std::shared_ptr<AudioPipeInfo> oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->adapterName_ = "old_adapter";
    oldPipe->routeFlag_ = 0x123456;

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AudioStreamAction result = audioPipeSelector->JudgeStreamAction(newPipe, oldPipe);
    EXPECT_EQ(result, AUDIO_STREAM_ACTION_RECREATE);
}

/**
 * @tc.name: JudgeStreamAction_003
 * @tc.desc: Test JudgeStreamAction when oldPipe's routeFlag is AUDIO_OUTPUT_FLAG_FAST.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, JudgeStreamAction_003, TestSize.Level1)
{
    std::shared_ptr<AudioPipeInfo> newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->adapterName_ = "new_adapter";
    newPipe->routeFlag_ = 0x1234;

    std::shared_ptr<AudioPipeInfo> oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->adapterName_ = "old_adapter";
    oldPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AudioStreamAction result = audioPipeSelector->JudgeStreamAction(newPipe, oldPipe);
    EXPECT_EQ(result, AUDIO_STREAM_ACTION_RECREATE);
}

/**
 * @tc.name: JudgeStreamAction_004
 * @tc.desc: Test JudgeStreamAction when newPipe's routeFlag is AUDIO_OUTPUT_FLAG_FAST.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, JudgeStreamAction_004, TestSize.Level1)
{
    std::shared_ptr<AudioPipeInfo> newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->adapterName_ = "new_adapter";
    newPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;

    std::shared_ptr<AudioPipeInfo> oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->adapterName_ = "old_adapter";
    oldPipe->routeFlag_ = 0x123456;

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AudioStreamAction result = audioPipeSelector->JudgeStreamAction(newPipe, oldPipe);
    EXPECT_EQ(result, AUDIO_STREAM_ACTION_RECREATE);
}

/**
 * @tc.name: JudgeStreamAction_005
 * @tc.desc: Test JudgeStreamAction when oldPipe's routeFlag is AUDIO_OUTPUT_FLAG_DIRECT.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, JudgeStreamAction_005, TestSize.Level1)
{
    std::shared_ptr<AudioPipeInfo> newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->adapterName_ = "new_adapter";
    newPipe->routeFlag_ = 0x1234;
    std::shared_ptr<AudioPipeInfo> oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->adapterName_ = "old_adapter";
    oldPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_DIRECT;

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AudioStreamAction result = audioPipeSelector->JudgeStreamAction(newPipe, oldPipe);
    EXPECT_EQ(result, AUDIO_STREAM_ACTION_RECREATE);
}

/**
 * @tc.name: JudgeStreamAction_006
 * @tc.desc: Test JudgeStreamAction when newPipe's routeFlag is AUDIO_OUTPUT_FLAG_DIRECT.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, JudgeStreamAction_006, TestSize.Level1)
{
    std::shared_ptr<AudioPipeInfo> newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->adapterName_ = "new_adapter";
    newPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_DIRECT;

    std::shared_ptr<AudioPipeInfo> oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->adapterName_ = "old_adapter";
    oldPipe->routeFlag_ = 0x123456;
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AudioStreamAction result = audioPipeSelector->JudgeStreamAction(newPipe, oldPipe);
    EXPECT_EQ(result, AUDIO_STREAM_ACTION_RECREATE);
}

/**
 * @tc.name: FetchPipeAndExecute_001
 * @tc.desc: Test FetchPipeAndExecute when streamDesc->routeFlag_ == AUDIO_FLAG_NONE and enter the first if branch,
 *           then enter the first for loop's if branch, do not enter the second if branch, finally enter the second
 *           for loop's second if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, FetchPipeAndExecute_001, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->routeFlag_ = AUDIO_FLAG_NONE;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.front()->networkId_ = "0";
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S16LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfoList;
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipeInfo->adapterName_ = "test_adapter";
    pipeInfo->routeFlag_ = 1;
    pipeInfoList.push_back(pipeInfo);
    AudioPipeManager::GetPipeManager()->curPipeList_ = pipeInfoList;

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioPipeInfo>> result = audioPipeSelector->FetchPipeAndExecute(streamDesc);
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result[0]->pipeAction_, PIPE_ACTION_DEFAULT);
}

/**
 * @tc.name: UpdataDeviceStreamInfo_001
 * @tc.desc: Test UpdataDeviceStreamInfo
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, UpdataDeviceStreamInfo_001, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();

    streamDesc->newDeviceDescs_ = {};
    std::shared_ptr<PipeStreamPropInfo> streamPropInfo = nullptr;

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    // test empty
    audioPipeSelector->UpdataDeviceStreamInfo(streamDesc, streamPropInfo);

    std::shared_ptr<AudioDeviceDescriptor> temp = nullptr;
    streamDesc->newDeviceDescs_.push_back(temp);
    audioPipeSelector->UpdataDeviceStreamInfo(streamDesc, streamPropInfo);

    streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    audioPipeSelector->UpdataDeviceStreamInfo(streamDesc, streamPropInfo);

    streamDesc->newDeviceDescs_.front() = std::make_shared<AudioDeviceDescriptor>();
    audioPipeSelector->UpdataDeviceStreamInfo(streamDesc, streamPropInfo);

    // test nullptr
    streamPropInfo->format_ = AudioSampleFormat::SAMPLE_S16LE;
    streamPropInfo->sampleRate_ = static_cast<uint32_t>(AudioSamplingRate::SAMPLE_RATE_48000);
    streamPropInfo->channels_ = AudioChannel::STEREO;

    audioPipeSelector->UpdataDeviceStreamInfo(streamDesc, streamPropInfo);

    EXPECT_EQ(streamDesc->newDeviceDescs_.front()->audioStreamInfo_.front().format, streamPropInfo->format_);
    EXPECT_EQ(*(streamDesc->newDeviceDescs_.front()->audioStreamInfo_.front().samplingRate.rbegin()),
        AudioSamplingRate::SAMPLE_RATE_48000);
}

/**
 * @tc.name: FetchPipesAndExecute_001
 * @tc.desc: Test FetchPipesAndExecute when streamDescs is empty.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, FetchPipesAndExecute_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioPipeInfo>> result = audioPipeSelector->FetchPipesAndExecute(streamDescs);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: FetchPipeAndExecute_002
 * @tc.desc: Test FetchPipeAndExecute streamDesc->routeFlag_ != AUDIO_FLAG_NONE.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, FetchPipeAndExecute_002, TestSize.Level4)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.front()->networkId_ = "0";
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S16LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;
    streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    AudioPipeManager::GetPipeManager()->curPipeList_.clear();
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioPipeInfo>> result = audioPipeSelector->FetchPipeAndExecute(streamDesc);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: FetchPipesAndExecute_002
 * @tc.desc: Test FetchPipesAndExecute streamDescs.size() != 0.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, FetchPipesAndExecute_002, TestSize.Level4)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    streamDesc1->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc1->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    std::shared_ptr<AudioStreamDescriptor> streamDesc2 = std::make_shared<AudioStreamDescriptor>();
    streamDesc2->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc2->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs = { streamDesc1, streamDesc2 };

    std::shared_ptr<AudioPipeInfo> pipeInfo1 = std::make_shared<AudioPipeInfo>();
    pipeInfo1->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipeInfo1->streamDescriptors_.push_back(std::make_shared<AudioStreamDescriptor>());
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipeInfo1);
    std::shared_ptr<AudioPipeInfo> pipeInfo2 = std::make_shared<AudioPipeInfo>();
    pipeInfo2->pipeRole_ = PIPE_ROLE_INPUT;
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipeInfo2);

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioPipeInfo>> result = audioPipeSelector->FetchPipesAndExecute(streamDescs);
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: IncomingConcurrency_001
 * @tc.desc: Test IncomingConcurrency cmpStream->audioMode_ == AUDIO_MODE_RECORD
 *           && stream->audioMode_ == AUDIO_MODE_RECORD.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, IncomingConcurrency_001, TestSize.Level4)
{
    std::shared_ptr<AudioStreamDescriptor> stream = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioStreamDescriptor> cmpStream = std::make_shared<AudioStreamDescriptor>();
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    audioPipeSelector->IncomingConcurrency(stream, cmpStream);

    cmpStream->audioMode_ = AUDIO_MODE_RECORD;
    stream->audioMode_ = AUDIO_MODE_PLAYBACK;
    audioPipeSelector->IncomingConcurrency(stream, cmpStream);
    cmpStream->audioMode_ = AUDIO_MODE_PLAYBACK;
    stream->audioMode_ = AUDIO_MODE_RECORD;
    audioPipeSelector->IncomingConcurrency(stream, cmpStream);

    cmpStream->audioMode_ = AUDIO_MODE_RECORD;
    stream->audioMode_ = AUDIO_MODE_RECORD;
    audioPipeSelector->IncomingConcurrency(stream, cmpStream);
    EXPECT_EQ(cmpStream->routeFlag_, AUDIO_INPUT_FLAG_NORMAL);
}

/**
 * @tc.name: ProcessConcurrency_001
 * @tc.desc: Test ProcessConcurrency switch (action).
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, ProcessConcurrency_001, TestSize.Level4)
{
    std::map<std::pair<AudioPipeType, AudioPipeType>, ConcurrencyAction> ruleMap = {
        {{PIPE_TYPE_LOWLATENCY_OUT, PIPE_TYPE_LOWLATENCY_OUT}, PLAY_BOTH},
        {{PIPE_TYPE_LOWLATENCY_OUT, PIPE_TYPE_NORMAL_IN}, CONCEDE_INCOMING},
        {{PIPE_TYPE_NORMAL_IN, PIPE_TYPE_NORMAL_IN}, CONCEDE_EXISTING}
    };
    AudioStreamCollector::GetAudioStreamCollector().audioConcurrencyService_->concurrencyCfgMap_ = ruleMap;
    AudioPipeManager::GetPipeManager()->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "test_adapterName";
    auto descriptor = std::make_shared<AudioStreamDescriptor>();
    descriptor->sessionId_ = 1;
    pipeInfo->streamDescriptors_.push_back(descriptor);
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipeInfo);

    std::shared_ptr<AudioStreamDescriptor> stream = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioStreamDescriptor> cmpStream = std::make_shared<AudioStreamDescriptor>();
    stream->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    stream->audioMode_ = AUDIO_MODE_PLAYBACK;
    stream->streamAction_ = AUDIO_STREAM_ACTION_NEW;
    stream->sessionId_ = 1;
    cmpStream->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    cmpStream->audioMode_ = AUDIO_MODE_PLAYBACK;
    cmpStream->sessionId_ = 1;

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    bool ret = audioPipeSelector->ProcessConcurrency(stream, cmpStream);
    EXPECT_EQ(stream->streamAction_, AUDIO_STREAM_ACTION_DEFAULT);
    EXPECT_FALSE(ret);

    cmpStream->audioMode_ = AUDIO_MODE_RECORD;
    ret = audioPipeSelector->ProcessConcurrency(stream, cmpStream);
    EXPECT_EQ(cmpStream->routeFlag_, AUDIO_INPUT_FLAG_NORMAL);

    stream->audioMode_ = AUDIO_MODE_RECORD;
    ret = audioPipeSelector->ProcessConcurrency(stream, cmpStream);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: ConvertStreamDescToPipeInfo_002
 * @tc.desc: Test ConvertStreamDescToPipeInfo pipeInfoPtr->name_ == "dp_multichannel_output".
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, ConvertStreamDescToPipeInfo_002, TestSize.Level4)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->routeFlag_ = 1;
    streamDesc->sessionId_ = 100;
    streamDesc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.front()->networkId_ = "0";
    streamDesc->capturerInfo_.sourceType = SourceType::SOURCE_TYPE_MIC;
    std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    streamPropInfo->format_ = AudioSampleFormat::SAMPLE_S16LE;
    streamPropInfo->sampleRate_ = 44100;
    streamPropInfo->channelLayout_ = AudioChannelLayout::CH_LAYOUT_STEREO;
    streamPropInfo->bufferSize_ = 1024;
    AudioPipeInfo info;

    std::shared_ptr<AdapterPipeInfo> pipeInfo = std::make_shared<AdapterPipeInfo>();
    pipeInfo->paProp_.lib_ = "test_lib";
    pipeInfo->paProp_.role_ = "test_role";
    pipeInfo->paProp_.moduleName_ = "test_module";
    pipeInfo->role_ = PIPE_ROLE_OUTPUT;
    std::shared_ptr<PolicyAdapterInfo> adapterInfo = std::make_shared<PolicyAdapterInfo>();
    adapterInfo->adapterName = "test_adapter";
    pipeInfo->adapterInfo_ = adapterInfo;
    pipeInfo->name_ = "dp_multichannel_output";
    streamPropInfo->pipeInfo_ = pipeInfo;
    audioPipeSelector->ConvertStreamDescToPipeInfo(streamDesc, streamPropInfo, info);
    EXPECT_EQ(info.moduleInfo_.className, "dp_multichannel");
}

/**
 * @tc.name: ConvertStreamDescToPipeInfo_003
 * @tc.desc: Test ConvertStreamDescToPipeInfo pipeInfoPtr == nullptr and adapterInfoPtr == nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, ConvertStreamDescToPipeInfo_003, TestSize.Level4)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    streamPropInfo->sampleRate_ = 44100;
    streamPropInfo->pipeInfo_.reset();
    AudioPipeInfo info;
    audioPipeSelector->ConvertStreamDescToPipeInfo(streamDesc, streamPropInfo, info);
    EXPECT_EQ(info.moduleInfo_.rate, "");

    std::shared_ptr<AdapterPipeInfo> pipeInfo = std::make_shared<AdapterPipeInfo>();
    pipeInfo->adapterInfo_.reset();
    streamPropInfo->pipeInfo_ = pipeInfo;
    audioPipeSelector->ConvertStreamDescToPipeInfo(streamDesc, streamPropInfo, info);
    EXPECT_EQ(info.moduleInfo_.rate, "");
}

/**
 * @tc.name: JudgeStreamAction_007
 * @tc.desc: Test JudgeStreamAction when return AUDIO_STREAM_ACTION_MOVE.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, JudgeStreamAction_007, TestSize.Level4)
{
    std::shared_ptr<AudioPipeInfo> newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->adapterName_ = "new_adapter";
    newPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    std::shared_ptr<AudioPipeInfo> oldPipe = std::make_shared<AudioPipeInfo>();
    oldPipe->adapterName_ = "old_adapter";
    oldPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_DIRECT;

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AudioStreamAction result = audioPipeSelector->JudgeStreamAction(newPipe, oldPipe);
    EXPECT_EQ(result, AUDIO_STREAM_ACTION_RECREATE);

    newPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_DIRECT;
    oldPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    result = audioPipeSelector->JudgeStreamAction(newPipe, oldPipe);
    EXPECT_EQ(result, AUDIO_STREAM_ACTION_RECREATE);

    newPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    oldPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    result = audioPipeSelector->JudgeStreamAction(newPipe, oldPipe);
    EXPECT_EQ(result, AUDIO_STREAM_ACTION_MOVE);
}

/**
 * @tc.name: DecideFinalRouteFlag_001
 * @tc.desc: Test DecideFinalRouteFlag when streamDescs.size() == 0 || streamDescs.size() == 1.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorUnitTest, DecideFinalRouteFlag_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    audioPipeSelector->DecideFinalRouteFlag(streamDescs);

    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    streamDesc1->routeFlag_ = AUDIO_FLAG_MAX;
    streamDesc1->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc1->streamAction_ = AUDIO_STREAM_ACTION_NEW;
    streamDesc1->sessionId_ = 100001;
    streamDesc1->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDescs.push_back(streamDesc1);
    audioPipeSelector->DecideFinalRouteFlag(streamDescs);
    EXPECT_EQ(streamDescs[0]->routeFlag_, AUDIO_FLAG_NONE);
}

/**
 * @tc.name: DecideFinalRouteFlag_002
 * @tc.desc: Test DecideFinalRouteFlag when streamDescs.size() > 1.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorUnitTest, DecideFinalRouteFlag_002, TestSize.Level1)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    streamDesc1->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    streamDesc1->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc1->streamAction_ = AUDIO_STREAM_ACTION_NEW;
    streamDesc1->sessionId_ = 100001;
    streamDesc1->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc1->createTimeStamp_ = 1;
    streamDescs.push_back(streamDesc1);

    std::shared_ptr<AudioStreamDescriptor> streamDesc2 = std::make_shared<AudioStreamDescriptor>();
    streamDesc2->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    streamDesc2->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc2->streamAction_ = AUDIO_STREAM_ACTION_NEW;
    streamDesc2->sessionId_ = 100002;
    streamDesc2->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc2->createTimeStamp_ = 2;
    streamDescs.push_back(streamDesc2);
    audioPipeSelector->DecideFinalRouteFlag(streamDescs);
    EXPECT_NE(streamDescs[1]->routeFlag_, AUDIO_OUTPUT_FLAG_FAST);
}

/**
 * @tc.name: ProcessNewPipeList_001
 * @tc.desc: Test ProcessNewPipeList when cannot find exist pipe.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorUnitTest, ProcessNewPipeList_001, TestSize.Level1)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    streamDesc1->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    streamDesc1->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc1->streamAction_ = AUDIO_STREAM_ACTION_NEW;
    streamDesc1->sessionId_ = 100001;
    streamDesc1->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc1->createTimeStamp_ = 1;
    streamDescs.push_back(streamDesc1);

    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList{};

    audioPipeSelector->ProcessNewPipeList(newPipeInfoList, streamDescs);
    EXPECT_TRUE(newPipeInfoList.size() != 0);
}

/**
 * @tc.name: ProcessNewPipeList_002
 * @tc.desc: Test ProcessNewPipeList when pipe already exist.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorUnitTest, ProcessNewPipeList_002, TestSize.Level1)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    streamDesc1->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    streamDesc1->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc1->streamAction_ = AUDIO_STREAM_ACTION_NEW;
    streamDesc1->sessionId_ = 100001;
    streamDesc1->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc1->createTimeStamp_ = 1;
    streamDescs.push_back(streamDesc1);

    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList{};
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->adapterName_ = "primary";
    pipe1->routeFlag_ = 1;
    newPipeInfoList.push_back(pipe1);

    std::shared_ptr<AudioPipeInfo> pipe2 = std::make_shared<AudioPipeInfo>();
    pipe2->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    newPipeInfoList.push_back(pipe2);

    audioPipeSelector->ProcessNewPipeList(newPipeInfoList, streamDescs);
    EXPECT_TRUE(newPipeInfoList.size() == 2);
}

/**
 * @tc.name: DecidePipesAndStreamAction_001
 * @tc.desc: Test DecidePipesAndStreamAction.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorUnitTest, DecidePipesAndStreamAction_001, TestSize.Level1)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    streamDesc1->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    streamDesc1->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc1->streamAction_ = AUDIO_STREAM_ACTION_NEW;
    streamDesc1->sessionId_ = 100001;
    streamDesc1->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc1->createTimeStamp_ = 1;

    std::shared_ptr<AudioStreamDescriptor> streamDesc2 = std::make_shared<AudioStreamDescriptor>();
    streamDesc2->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    streamDesc2->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc2->streamAction_ = AUDIO_STREAM_ACTION_NEW;
    streamDesc2->sessionId_ = 100002;
    streamDesc2->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc2->createTimeStamp_ = 2;

    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList{};
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->adapterName_ = "primary";
    pipe1->routeFlag_ = 1;
    pipe1->streamDescMap_[100001] = streamDesc1;
    pipe1->streamDescriptors_.push_back(streamDesc1);
    pipe1->pipeAction_ = PIPE_ACTION_NEW;
    newPipeInfoList.push_back(pipe1);

    std::shared_ptr<AudioPipeInfo> pipe2 = std::make_shared<AudioPipeInfo>();
    pipe2->routeFlag_ = 1;
    newPipeInfoList.push_back(pipe2);

    std::shared_ptr<AudioPipeInfo> pipe3 = std::make_shared<AudioPipeInfo>();
    pipe3->adapterName_ = "test_pipe";
    pipe3->routeFlag_ = 1;
    pipe3->streamDescMap_[100002] = streamDesc2;
    pipe3->streamDescriptors_.push_back(streamDesc2);
    newPipeInfoList.push_back(pipe3);

    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo{};
    streamDescToOldPipeInfo[100001] = pipe1;
    streamDescToOldPipeInfo[100002] = pipe2;

    audioPipeSelector->DecidePipesAndStreamAction(newPipeInfoList, streamDescToOldPipeInfo);
    EXPECT_TRUE(newPipeInfoList[0]->streamDescriptors_[0]->streamAction_ == AUDIO_STREAM_ACTION_DEFAULT);
}

} // namespace AudioStandard
} // namespace OHOS