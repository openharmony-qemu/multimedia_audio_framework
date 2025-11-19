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
#include "audio_pipe_selector_unit_test.h"

#include "audio_policy_utils.h"
#include "audio_stream_descriptor.h"
#include "audio_stream_enum.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static const uint32_t TEST_SESSION_ID_BASE = 100000;
static const uint32_t TEST_STREAM_1_SESSION_ID = 100001;

void AudioPipeSelectorUnitTest::SetUpTestCase(void) {}
void AudioPipeSelectorUnitTest::TearDownTestCase(void) {}
void AudioPipeSelectorUnitTest::SetUp(void) {}
void AudioPipeSelectorUnitTest::TearDown(void) {}

static std::shared_ptr<AudioPipeInfo> MakeTestPipe(AudioPipeRole role, std::string adapterName, uint32_t route)
{
    auto newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->pipeRole_ = role;
    newPipe->adapterName_ = adapterName;
    newPipe->routeFlag_ = route;
    return newPipe;
}

static void MakeDeviceInfoMap(AudioPolicyConfigData &configData, std::shared_ptr<AdapterPipeInfo> &adapterPipeInfo,
    std::shared_ptr<PolicyAdapterInfo> &policyAdapterInfo)
{
    uint32_t routerFlag = AUDIO_OUTPUT_FLAG_LOWPOWER;
    AudioPolicyConfigManager &manager = AudioPolicyConfigManager::GetInstance();
    EXPECT_EQ(manager.Init(true), true);
    configData.Reorganize();

    policyAdapterInfo->adapterName = "remote";
    adapterPipeInfo->adapterInfo_ = policyAdapterInfo;
    std::shared_ptr<PipeStreamPropInfo> propInfo = std::make_shared<PipeStreamPropInfo>();
    propInfo->format_ = AudioSampleFormat::SAMPLE_S16LE;
    propInfo->sampleRate_ = AudioSamplingRate::SAMPLE_RATE_48000;
    propInfo->channels_ = AudioChannel::STEREO;
    propInfo->channelLayout_ = AudioChannelLayout::CH_LAYOUT_STEREO;
    propInfo->pipeInfo_ = adapterPipeInfo;

    std::shared_ptr<PipeStreamPropInfo> propInfo_2 = std::make_shared<PipeStreamPropInfo>();
    propInfo_2->format_ = AudioSampleFormat::SAMPLE_S16LE;
    propInfo_2->sampleRate_ = AudioSamplingRate::SAMPLE_RATE_48000;
    propInfo_2->channels_ = AudioChannel::CHANNEL_6;
    propInfo_2->channelLayout_ = AudioChannelLayout::CH_LAYOUT_5POINT1;
    propInfo_2->pipeInfo_ = adapterPipeInfo;

    std::shared_ptr<AdapterPipeInfo> info = std::make_shared<AdapterPipeInfo>();
    info->dynamicStreamPropInfos_ = {propInfo, propInfo_2};
    info->role_ = PIPE_ROLE_OUTPUT;

    std::shared_ptr<AdapterDeviceInfo> deviceInfo = std::make_shared<AdapterDeviceInfo>();
    deviceInfo->supportPipeMap_.insert({routerFlag, info});
    std::set<std::shared_ptr<AdapterDeviceInfo>> deviceInfoSet = {deviceInfo};
    auto deviceKey = std::make_pair<DeviceType, DeviceRole>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    configData.deviceInfoMap[deviceKey] = deviceInfoSet;
}

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
    EXPECT_EQ(result, PIPE_TYPE_DIRECT_MUSIC);
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
 * @tc.name: UpdateDeviceStreamInfo_001
 * @tc.desc: Test UpdateDeviceStreamInfo
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, UpdateDeviceStreamInfo_001, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();

    streamDesc->newDeviceDescs_ = {};
    std::shared_ptr<PipeStreamPropInfo> streamPropInfo = nullptr;

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    // test empty
    audioPipeSelector->UpdateDeviceStreamInfo(streamDesc, streamPropInfo);

    std::shared_ptr<AudioDeviceDescriptor> temp = nullptr;
    streamDesc->newDeviceDescs_.push_back(temp);
    audioPipeSelector->UpdateDeviceStreamInfo(streamDesc, streamPropInfo);

    streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    audioPipeSelector->UpdateDeviceStreamInfo(streamDesc, streamPropInfo);

    streamDesc->newDeviceDescs_.front() = std::make_shared<AudioDeviceDescriptor>();
    audioPipeSelector->UpdateDeviceStreamInfo(streamDesc, streamPropInfo);

    // test nullptr
    streamPropInfo->format_ = AudioSampleFormat::SAMPLE_S16LE;
    streamPropInfo->sampleRate_ = static_cast<uint32_t>(AudioSamplingRate::SAMPLE_RATE_48000);
    streamPropInfo->channels_ = AudioChannel::STEREO;

    audioPipeSelector->UpdateDeviceStreamInfo(streamDesc, streamPropInfo);

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
 * @tc.name: FetchPipeAndExecute_003
 * @tc.desc: Test FetchPipeAndExecute selectedPipeInfoList->pipeAction_ == PIPE_ACTION_UPDATE.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, FetchPipeAndExecute_003, TestSize.Level1)
{
    AudioPolicyConfigData &configData = AudioPolicyConfigData::GetInstance();
    std::shared_ptr<AdapterPipeInfo> adapterPipeInfo = std::make_shared<AdapterPipeInfo>();
    std::shared_ptr<PolicyAdapterInfo> policyAdapterInfo = std::make_shared<PolicyAdapterInfo>();
    MakeDeviceInfoMap(configData, adapterPipeInfo, policyAdapterInfo);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.front()->deviceRole_ = OUTPUT_DEVICE;
    streamDesc->newDeviceDescs_.front()->networkId_ = "remote";
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S16LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;
    streamDesc->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfoList;
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipeInfo->adapterName_ = "remote";
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    pipeInfo->name_ = "offload_distributed_output";
    pipeInfo->moduleInfo_.format = AudioDefinitionPolicyUtils::enumToFormatStr[AudioSampleFormat::SAMPLE_S16LE];
    pipeInfo->moduleInfo_.rate = std::to_string(AudioSamplingRate::SAMPLE_RATE_48000);
    pipeInfo->moduleInfo_.channels = std::to_string(AudioDefinitionPolicyUtils::ConvertLayoutToAudioChannel(
        AudioChannelLayout::CH_LAYOUT_STEREO));
    pipeInfo->moduleInfo_.channelLayout = std::to_string(AudioChannelLayout::CH_LAYOUT_STEREO);
    pipeInfoList.push_back(pipeInfo);
    AudioPipeManager::GetPipeManager()->curPipeList_ = pipeInfoList;

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioPipeInfo>> selectedPipeInfoList
        = audioPipeSelector->FetchPipeAndExecute(streamDesc);
    EXPECT_EQ(selectedPipeInfoList[0]->pipeAction_, PIPE_ACTION_UPDATE);

    streamDesc->streamInfo_.channels = AudioChannel::CHANNEL_6;
    streamDesc->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_5POINT1;
    selectedPipeInfoList = audioPipeSelector->FetchPipeAndExecute(streamDesc);
    EXPECT_EQ(selectedPipeInfoList[0]->pipeAction_, PIPE_ACTION_RELOAD);

    auto deviceKey = std::make_pair<DeviceType, DeviceRole>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    configData.deviceInfoMap.erase(deviceKey);
}

/**
 * @tc.name: FetchPipeAndExecute_004
 * @tc.desc: Test FetchPipeAndExecute selectedPipeInfoList->pipeAction_ == PIPE_ACTION_UPDATE.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, FetchPipeAndExecute_004, TestSize.Level1)
{
    AudioPolicyConfigData &configData = AudioPolicyConfigData::GetInstance();
    std::shared_ptr<AdapterPipeInfo> adapterPipeInfo = std::make_shared<AdapterPipeInfo>();
    std::shared_ptr<PolicyAdapterInfo> policyAdapterInfo = std::make_shared<PolicyAdapterInfo>();
    MakeDeviceInfoMap(configData, adapterPipeInfo, policyAdapterInfo);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.front()->deviceRole_ = OUTPUT_DEVICE;
    streamDesc->newDeviceDescs_.front()->networkId_ = "remote";
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S16LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;
    streamDesc->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfoList;
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipeInfo->adapterName_ = "remote";
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    pipeInfo->name_ = "remote";
    pipeInfo->moduleInfo_.format = AudioDefinitionPolicyUtils::enumToFormatStr[AudioSampleFormat::SAMPLE_S16LE];
    pipeInfo->moduleInfo_.rate = std::to_string(AudioSamplingRate::SAMPLE_RATE_48000);
    pipeInfo->moduleInfo_.channels = std::to_string(AudioDefinitionPolicyUtils::ConvertLayoutToAudioChannel(
        AudioChannelLayout::CH_LAYOUT_STEREO));
    pipeInfo->moduleInfo_.channelLayout = std::to_string(AudioChannelLayout::CH_LAYOUT_STEREO);
    pipeInfoList.push_back(pipeInfo);
    AudioPipeManager::GetPipeManager()->curPipeList_ = pipeInfoList;

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioPipeInfo>> selectedPipeInfoList
        = audioPipeSelector->FetchPipeAndExecute(streamDesc);
    EXPECT_EQ(selectedPipeInfoList[0]->pipeAction_, PIPE_ACTION_UPDATE);

    streamDesc->streamInfo_.channels = AudioChannel::CHANNEL_6;
    streamDesc->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_5POINT1;
    selectedPipeInfoList = audioPipeSelector->FetchPipeAndExecute(streamDesc);
    EXPECT_EQ(selectedPipeInfoList[0]->pipeAction_, PIPE_ACTION_RELOAD);

    pipeInfo->adapterName_ = "primary";
    selectedPipeInfoList = audioPipeSelector->FetchPipeAndExecute(streamDesc);
    EXPECT_TRUE(selectedPipeInfoList.size() == 2);

    auto deviceKey = std::make_pair<DeviceType, DeviceRole>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    configData.deviceInfoMap.erase(deviceKey);
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
 * @tc.name: CheckAndHandleIncomingConcurrency_001
 * @tc.desc: Test CheckAndHandleIncomingConcurrency cmpStream->audioMode_ == AUDIO_MODE_RECORD
 *           && stream->audioMode_ == AUDIO_MODE_RECORD.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, CheckAndHandleIncomingConcurrency_001, TestSize.Level4)
{
    std::shared_ptr<AudioStreamDescriptor> stream = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioStreamDescriptor> cmpStream = std::make_shared<AudioStreamDescriptor>();
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    audioPipeSelector->CheckAndHandleIncomingConcurrency(stream, cmpStream);

    cmpStream->audioMode_ = AUDIO_MODE_RECORD;
    stream->audioMode_ = AUDIO_MODE_PLAYBACK;
    audioPipeSelector->CheckAndHandleIncomingConcurrency(stream, cmpStream);
    cmpStream->audioMode_ = AUDIO_MODE_PLAYBACK;
    stream->audioMode_ = AUDIO_MODE_RECORD;
    audioPipeSelector->CheckAndHandleIncomingConcurrency(stream, cmpStream);

    cmpStream->audioMode_ = AUDIO_MODE_RECORD;
    stream->audioMode_ = AUDIO_MODE_RECORD;
    audioPipeSelector->CheckAndHandleIncomingConcurrency(stream, cmpStream);
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
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamsToMove;
    bool ret = audioPipeSelector->ProcessConcurrency(stream, cmpStream, streamsToMove);
    EXPECT_EQ(stream->streamAction_, AUDIO_STREAM_ACTION_DEFAULT);
    EXPECT_FALSE(ret);

    cmpStream->audioMode_ = AUDIO_MODE_RECORD;
    ret = audioPipeSelector->ProcessConcurrency(stream, cmpStream, streamsToMove);
    EXPECT_EQ(cmpStream->routeFlag_, AUDIO_INPUT_FLAG_NORMAL);

    stream->audioMode_ = AUDIO_MODE_RECORD;
    ret = audioPipeSelector->ProcessConcurrency(stream, cmpStream, streamsToMove);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: UpdateProcessConcurrency_001
 * @tc.desc: Test UpdateProcessConcurrency with CALL_IN pipes and PLAY_BOTH action modification.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, UpdateProcessConcurrency_001, TestSize.Level4)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AudioPipeType existingPipe = PIPE_TYPE_CALL_IN;
    AudioPipeType commingPipe = PIPE_TYPE_CALL_IN;
    ConcurrencyAction action = CONCEDE_INCOMING; // Initial action

    SetInjectEnable(true);
    audioPipeSelector->UpdateProcessConcurrency(existingPipe, commingPipe, action);

    // Verify that action is updated to PLAY_BOTH when both pipes are CALL_IN and injection is enabled
    EXPECT_EQ(action, PLAY_BOTH);
}

/**
 * @tc.name: UpdateProcessConcurrency_002
 * @tc.desc: Test UpdateProcessConcurrency with different pipe types and no action modification.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, UpdateProcessConcurrency_002, TestSize.Level4)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AudioPipeType existingPipe = PIPE_TYPE_LOWLATENCY_OUT;
    AudioPipeType commingPipe = PIPE_TYPE_CALL_IN;
    ConcurrencyAction originalAction = CONCEDE_EXISTING;
    ConcurrencyAction action = originalAction;

    SetInjectEnable(true);
    audioPipeSelector->UpdateProcessConcurrency(existingPipe, commingPipe, action);

    // Verify that action remains unchanged when pipe types are different
    EXPECT_EQ(action, originalAction);
}

/**
 * @tc.name: UpdateProcessConcurrency_003
 * @tc.desc: Test UpdateProcessConcurrency with action already set to PLAY_BOTH and no modification.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, UpdateProcessConcurrency_003, TestSize.Level4)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AudioPipeType existingPipe = PIPE_TYPE_CALL_IN;
    AudioPipeType commingPipe = PIPE_TYPE_CALL_IN;
    ConcurrencyAction originalAction = PLAY_BOTH;
    ConcurrencyAction action = originalAction;

    SetInjectEnable(true);
    audioPipeSelector->UpdateProcessConcurrency(existingPipe, commingPipe, action);

    // Verify that action remains PLAY_BOTH when it's already set
    EXPECT_EQ(action, originalAction);
}

/**
 * @tc.name: UpdateProcessConcurrency_004
 * @tc.desc: Test UpdateProcessConcurrency with various CALL_IN scenarios and action modification.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeSelectorUnitTest, UpdateProcessConcurrency_004, TestSize.Level4)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    SetInjectEnable(true);

    // Test case 1: Both pipes are CALL_IN, injection enabled, action should be updated to PLAY_BOTH
    ConcurrencyAction action = CONCEDE_INCOMING;
    audioPipeSelector->UpdateProcessConcurrency(PIPE_TYPE_CALL_IN, PIPE_TYPE_CALL_IN, action);
    EXPECT_EQ(action, PLAY_BOTH);

    // Test case 2: Different pipe types, action should remain unchanged
    action = CONCEDE_EXISTING;
    audioPipeSelector->UpdateProcessConcurrency(PIPE_TYPE_CALL_IN, PIPE_TYPE_LOWLATENCY_OUT, action);
    EXPECT_EQ(action, CONCEDE_EXISTING);

    // Test case 3: Same pipe types but not CALL_IN, action should remain unchanged
    action = CONCEDE_INCOMING;
    audioPipeSelector->UpdateProcessConcurrency(PIPE_TYPE_LOWLATENCY_OUT, PIPE_TYPE_LOWLATENCY_OUT, action);
    EXPECT_EQ(action, CONCEDE_INCOMING);
}

/**
 * @tc.name: AudioPipeSelectorUnitTest_MoveStreamsToNormalPipes_001
 * @tc.number: MoveStreamsToNormalPipes_001
 * @tc.desc: Test MoveStreamsToNormalPipes different cases
 */
HWTEST_F(AudioPipeSelectorUnitTest, MoveStreamsToNormalPipes_001, TestSize.Level4)
{
    auto testSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> testStreamsToMove;
    std::vector<std::shared_ptr<AudioPipeInfo>> testPipeInfoList;

    // Test pipe IsSameRole() false, pipe IsRouteNormal() false, pipe->IsSameAdapter() false
    auto usbFastInputPipe = MakeTestPipe(PIPE_ROLE_INPUT, "usb", AUDIO_INPUT_FLAG_FAST);
    testPipeInfoList.push_back(usbFastInputPipe);

    // Test pipe IsSameRole() false, pipe IsRouteNormal() true, pipe->IsSameAdapter() false
    auto usbNormalInputPipe = MakeTestPipe(PIPE_ROLE_INPUT, "usb", AUDIO_INPUT_FLAG_NORMAL);
    testPipeInfoList.push_back(usbNormalInputPipe);

    // Test pipe IsSameRole() false, pipe IsRouteNormal() true, pipe->IsSameAdapter() true
    auto primaryNormalInputPipe = MakeTestPipe(PIPE_ROLE_INPUT, "primary", AUDIO_INPUT_FLAG_NORMAL);
    testPipeInfoList.push_back(primaryNormalInputPipe);

    // Test pipe IsSameRole() true, pipe IsRouteNormal() false, pipe->IsSameAdapter() false
    auto usbFastOutputPipe = MakeTestPipe(PIPE_ROLE_OUTPUT, "usb", AUDIO_OUTPUT_FLAG_FAST);
    testPipeInfoList.push_back(usbFastOutputPipe);

    // Test pipe IsSameRole() true, pipe IsRouteNormal() true && pipe->IsSameAdapter() false
    auto usbNormalOutputPipe = MakeTestPipe(PIPE_ROLE_OUTPUT, "usb", AUDIO_OUTPUT_FLAG_NORMAL);
    testPipeInfoList.push_back(usbNormalOutputPipe);

    // Test pipe IsSameRole() true, pipe IsRouteNormal() true && pipe->IsSameAdapter() true
    auto primaryNormalOutputPipe = MakeTestPipe(PIPE_ROLE_OUTPUT, "primary", AUDIO_OUTPUT_FLAG_NORMAL);
    testPipeInfoList.push_back(primaryNormalOutputPipe);

    // Test stream will be moved from primary adapter offload output pipe
    auto primaryOffloadOutputPipe = MakeTestPipe(PIPE_ROLE_OUTPUT, "primary", AUDIO_OUTPUT_FLAG_LOWPOWER);
    testPipeInfoList.push_back(primaryNormalOutputPipe);

    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->sessionId_ = TEST_STREAM_1_SESSION_ID;
    primaryOffloadOutputPipe->AddStream(stream);
    testStreamsToMove.push_back(stream);
    testPipeInfoList.push_back(primaryOffloadOutputPipe);
 
    testSelector->MoveStreamsToNormalPipes(testStreamsToMove, testPipeInfoList);
    EXPECT_EQ(false, primaryOffloadOutputPipe->ContainStream(TEST_STREAM_1_SESSION_ID));
    EXPECT_EQ(PIPE_ACTION_UPDATE, primaryNormalOutputPipe->GetAction());
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

    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo{};
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->adapterName_ = "primary";
    pipe1->routeFlag_ = 1;
    newPipeInfoList.push_back(pipe1);
    streamDescToOldPipeInfo[100001] = pipe1;

    audioPipeSelector->ProcessNewPipeList(newPipeInfoList, streamDescToOldPipeInfo, streamDescs);
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
    streamDesc1->routeFlag_ = AUDIO_OUTPUT_FLAG_DIRECT;
    streamDesc1->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc1->streamAction_ = AUDIO_STREAM_ACTION_NEW;
    streamDesc1->sessionId_ = 100001;
    streamDesc1->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc1->createTimeStamp_ = 1;
    streamDescs.push_back(streamDesc1);

    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList{};
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->adapterName_ = "direct";
    pipe1->routeFlag_ = AUDIO_OUTPUT_FLAG_DIRECT|AUDIO_OUTPUT_FLAG_HD;
    newPipeInfoList.push_back(pipe1);

    std::shared_ptr<AudioPipeInfo> pipe2 = std::make_shared<AudioPipeInfo>();
    pipe1->adapterName_ = "primary";
    pipe2->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    newPipeInfoList.push_back(pipe2);

    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo{};
    std::shared_ptr<AudioPipeInfo> pipeinfo1 = std::make_shared<AudioPipeInfo>();
    pipeinfo1->adapterName_ = "offload";
    pipeinfo1->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    newPipeInfoList.push_back(pipeinfo1);
    streamDescToOldPipeInfo[100001] = pipeinfo1;

    audioPipeSelector->ProcessNewPipeList(newPipeInfoList, streamDescToOldPipeInfo, streamDescs);
    EXPECT_FALSE(newPipeInfoList.size() == 2);
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

/**
 * @tc.name: SetOriginalFlagForcedNormalIfNeed_001
 * @tc.desc: Test SetOriginalFlagForcedNormalIfNeed - StreamDesc select flag is offload, route flag is normal.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorUnitTest, SetOriginalFlagForcedNormalIfNeed_001, TestSize.Level1)
{
    std::shared_ptr<AudioPipeSelector> audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_NORMAL;
    streamDesc->SetAudioFlag(AUDIO_OUTPUT_FLAG_LOWPOWER);
    streamDesc->SetRoute(AUDIO_OUTPUT_FLAG_NORMAL);
    audioPipeSelector->SetOriginalFlagForcedNormalIfNeed(streamDesc);
    EXPECT_EQ(streamDesc->rendererInfo_.originalFlag, AUDIO_FLAG_FORCED_NORMAL);
}

/**
 * @tc.name: SetOriginalFlagForcedNormalIfNeed_002
 * @tc.desc: Test SetOriginalFlagForcedNormalIfNeed - StreamDesc select flag is not offload, route flag is normal.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorUnitTest, SetOriginalFlagForcedNormalIfNeed_002, TestSize.Level1)
{
    std::shared_ptr<AudioPipeSelector> audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_NORMAL;
    streamDesc->SetAudioFlag(AUDIO_OUTPUT_FLAG_NORMAL);
    streamDesc->SetRoute(AUDIO_OUTPUT_FLAG_NORMAL);
    audioPipeSelector->SetOriginalFlagForcedNormalIfNeed(streamDesc);
    EXPECT_NE(streamDesc->rendererInfo_.originalFlag, AUDIO_FLAG_FORCED_NORMAL);
}

/**
 * @tc.name: SetOriginalFlagForcedNormalIfNeed_003
 * @tc.desc: Test SetOriginalFlagForcedNormalIfNeed - StreamDesc select flag is offload, route flag is not normal.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorUnitTest, SetOriginalFlagForcedNormalIfNeed_003, TestSize.Level1)
{
    std::shared_ptr<AudioPipeSelector> audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_NORMAL;
    streamDesc->SetAudioFlag(AUDIO_OUTPUT_FLAG_LOWPOWER);
    streamDesc->SetRoute(AUDIO_OUTPUT_FLAG_LOWPOWER);
    audioPipeSelector->SetOriginalFlagForcedNormalIfNeed(streamDesc);
    EXPECT_NE(streamDesc->rendererInfo_.originalFlag, AUDIO_FLAG_FORCED_NORMAL);
}

/**
 * @tc.name: SetOriginalFlagForcedNormalIfNeed_004
 * @tc.desc: Test SetOriginalFlagForcedNormalIfNeed - StreamDesc select flag is not offload, route flag is not normal.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorUnitTest, SetOriginalFlagForcedNormalIfNeed_004, TestSize.Level1)
{
    std::shared_ptr<AudioPipeSelector> audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_NORMAL;
    streamDesc->SetAudioFlag(AUDIO_OUTPUT_FLAG_NORMAL);
    streamDesc->SetRoute(AUDIO_OUTPUT_FLAG_LOWPOWER);
    audioPipeSelector->SetOriginalFlagForcedNormalIfNeed(streamDesc);
    EXPECT_NE(streamDesc->rendererInfo_.originalFlag, AUDIO_FLAG_FORCED_NORMAL);
}

/**
 * @tc.name: SetOriginalFlagForcedNormalIfNeed_005
 * @tc.desc: Test SetOriginalFlagForcedNormalIfNeed - StreamDesc select flag is direct, route flag is normal.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorUnitTest, SetOriginalFlagForcedNormalIfNeed_005, TestSize.Level1)
{
    std::shared_ptr<AudioPipeSelector> audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_NORMAL;
    streamDesc->SetAudioFlag(AUDIO_OUTPUT_FLAG_HD);
    streamDesc->SetRoute(AUDIO_OUTPUT_FLAG_NORMAL);
    audioPipeSelector->SetOriginalFlagForcedNormalIfNeed(streamDesc);
    EXPECT_EQ(streamDesc->rendererInfo_.originalFlag, AUDIO_FLAG_FORCED_NORMAL);
}

/**
 * @tc.name: ProcessNewPipeList_003
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorUnitTest, ProcessNewPipeList_003, TestSize.Level1)
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
    streamDesc1->rendererTarget_ = 1;
    streamDescs.push_back(streamDesc1);

    std::shared_ptr<AudioStreamDescriptor> streamDesc2 = std::make_shared<AudioStreamDescriptor>();
    streamDesc2->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    streamDesc2->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc2->streamAction_ = AUDIO_STREAM_ACTION_NEW;
    streamDesc2->sessionId_ = 100001;
    streamDesc2->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc2->createTimeStamp_ = 1;
    streamDesc2->rendererTarget_ = 0;
    streamDescs.push_back(streamDesc2);

    std::vector<std::shared_ptr<AudioPipeInfo>> newPipeInfoList{};
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->adapterName_ = "primary";
    pipe1->routeFlag_ = 1;
    newPipeInfoList.push_back(pipe1);

    std::shared_ptr<AudioPipeInfo> pipe2 = std::make_shared<AudioPipeInfo>();
    pipe2->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    newPipeInfoList.push_back(pipe2);

    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo{};
    std::shared_ptr<AudioPipeInfo> pipeinfo1 = std::make_shared<AudioPipeInfo>();
    pipeinfo1->adapterName_ = "primary";
    pipeinfo1->routeFlag_ = 1;
    newPipeInfoList.push_back(pipeinfo1);
    streamDescToOldPipeInfo[100001] = pipeinfo1;

    audioPipeSelector->ProcessNewPipeList(newPipeInfoList, streamDescToOldPipeInfo, streamDescs);
    EXPECT_TRUE(newPipeInfoList.size() == 2);
}

/**
 * @tc.name: IsNeedTempMoveToNormal_001
 * @tc.desc: Test IsNeedTempMoveToNormal_001
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorUnitTest, IsNeedTempMoveToNormal_001, TestSize.Level1)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    streamDesc1->routeFlag_ = AUDIO_OUTPUT_FLAG_DIRECT;
    streamDesc1->sessionId_ = 100001;

    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo{};
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->routeFlag_ = AUDIO_OUTPUT_FLAG_MULTICHANNEL;
    streamDescToOldPipeInfo[100001] = pipe1;

    EXPECT_TRUE(audioPipeSelector->IsNeedTempMoveToNormal(streamDesc1, streamDescToOldPipeInfo));
}

/**
 * @tc.name: IsNeedTempMoveToNormal_002
 * @tc.desc: Test IsNeedTempMoveToNormal_002
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorUnitTest, IsNeedTempMoveToNormal_002, TestSize.Level1)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    streamDesc1->routeFlag_ = AUDIO_OUTPUT_FLAG_DIRECT;
    streamDesc1->sessionId_ = 100001;

    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo{};
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    streamDescToOldPipeInfo[100001] = pipe1;

    EXPECT_TRUE(audioPipeSelector->IsNeedTempMoveToNormal(streamDesc1, streamDescToOldPipeInfo));
}

/**
 * @tc.name: IsNeedTempMoveToNormal_003
 * @tc.desc: Test IsNeedTempMoveToNormal_003
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorUnitTest, IsNeedTempMoveToNormal_003, TestSize.Level1)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    streamDesc1->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    streamDesc1->sessionId_ = 100001;

    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo{};
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->routeFlag_ = AUDIO_OUTPUT_FLAG_MULTICHANNEL;
    streamDescToOldPipeInfo[100001] = pipe1;

    EXPECT_TRUE(audioPipeSelector->IsNeedTempMoveToNormal(streamDesc1, streamDescToOldPipeInfo));
}

/**
 * @tc.name: IsNeedTempMoveToNormal_004
 * @tc.desc: Test IsNeedTempMoveToNormal_004
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorUnitTest, IsNeedTempMoveToNormal_004, TestSize.Level1)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    streamDesc1->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    streamDesc1->sessionId_ = 100001;

    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo{};
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    streamDescToOldPipeInfo[100001] = pipe1;

    EXPECT_TRUE(audioPipeSelector->IsNeedTempMoveToNormal(streamDesc1, streamDescToOldPipeInfo));
}

/**
 * @tc.name: IsNeedTempMoveToNormal_005
 * @tc.desc: Test IsNeedTempMoveToNormal_005
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorUnitTest, IsNeedTempMoveToNormal_005, TestSize.Level1)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    streamDesc1->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    streamDesc1->sessionId_ = 100001;

    std::map<uint32_t, std::shared_ptr<AudioPipeInfo>> streamDescToOldPipeInfo{};
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    streamDescToOldPipeInfo[100001] = pipe1;

    EXPECT_FALSE(audioPipeSelector->IsNeedTempMoveToNormal(streamDesc1, streamDescToOldPipeInfo));
}

} // namespace AudioStandard
} // namespace OHOS
