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
#include "audio_pipe_manager_unit_test.h"
#include "audio_stream_descriptor.h"
#include "audio_pipe_info.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
constexpr uint32_t DEFAULT_PIPE_ID = 0;

void AudioPipeManagerUnitTest::SetUpTestCase(void) {}
void AudioPipeManagerUnitTest::TearDownTestCase(void) {}
void AudioPipeManagerUnitTest::SetUp(void) {}
void AudioPipeManagerUnitTest::TearDown(void) {}

static std::shared_ptr<AudioStreamDescriptor> MakeCallStreamDesc(StreamUsage usage, AudioStreamStatus status)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.streamUsage = usage;
    streamDesc->streamStatus_ = status;
    return streamDesc;
}

/**
 * @tc.name: Init_001
 * @tc.desc: Test Init when adapter info has been loaded.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, Init_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->audioConfigManager_.isAdapterInfoMap_ = true;

    bool result = audioPipeManager->Init();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: GetHasEarpiece_001
 * @tc.desc: Test GetHasEarpiece returns value from config manager.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetHasEarpiece_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->audioConfigManager_.hasEarpiece_ = true;
    EXPECT_TRUE(audioPipeManager->GetHasEarpiece());

    audioPipeManager->audioConfigManager_.hasEarpiece_ = false;
    EXPECT_FALSE(audioPipeManager->GetHasEarpiece());
}


/**
 * @tc.name: RemoveAudioPipeInfo_001

 * @tc.desc: Test RemoveAudioPipeInfo when entering the if branch in the for loop.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, RemoveAudioPipeInfo_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    std::shared_ptr<AudioPipeInfo> targetPipe = std::make_shared<AudioPipeInfo>();
    targetPipe->adapterName_ = "test_adapter";
    targetPipe->routeFlag_ = 1;

    audioPipeManager->AddAudioPipeInfo(targetPipe);
    audioPipeManager->RemoveAudioPipeInfo(targetPipe);
    auto pipeList = audioPipeManager->GetPipeList();
    EXPECT_EQ(pipeList.size(), 0);
}

/**
 * @tc.name: RemoveAudioPipeInfo_002
 * @tc.desc: Test RemoveAudioPipeInfo when not entering the if branch in the for loop.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, RemoveAudioPipeInfo_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    std::shared_ptr<AudioPipeInfo> existingPipe = std::make_shared<AudioPipeInfo>();
    existingPipe->adapterName_ = "existing_adapter";
    existingPipe->routeFlag_ = 1;

    std::shared_ptr<AudioPipeInfo> nonExistingPipe = std::make_shared<AudioPipeInfo>();
    nonExistingPipe->adapterName_ = "non_existing_adapter";
    nonExistingPipe->routeFlag_ = 2;
    EXPECT_NE(nonExistingPipe, nullptr);

    auto pipeList01 = audioPipeManager->GetPipeList();
    EXPECT_EQ(pipeList01.size(), 0);
    audioPipeManager->RemoveAudioPipeInfo(nonExistingPipe);
    auto pipeList02 = audioPipeManager->GetPipeList();
    EXPECT_EQ(pipeList02.size(), 0);
}

/**
 * @tc.name: RemoveAudioPipeInfoById_001
 * @tc.desc: Test RemoveAudioPipeInfo when entering the if branch in the for loop.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, RemoveAudioPipeInfoById_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    std::shared_ptr<AudioPipeInfo> targetPipe = std::make_shared<AudioPipeInfo>();
    targetPipe->id_ = 123;
    targetPipe->adapterName_ = "test_adapter";

    auto pipeList01 = audioPipeManager->GetPipeList();
    EXPECT_EQ(pipeList01.size(), 0);
    audioPipeManager->AddAudioPipeInfo(targetPipe);
    audioPipeManager->RemoveAudioPipeInfo(targetPipe->id_);
    auto pipeList = audioPipeManager->GetPipeList();
    EXPECT_EQ(pipeList.size(), 0);
}

/**
 * @tc.name: RemoveAudioPipeInfoById_002
 * @tc.desc: Test RemoveAudioPipeInfo when not entering the if branch in the for loop.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, RemoveAudioPipeInfoById_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    std::shared_ptr<AudioPipeInfo> existingPipe = std::make_shared<AudioPipeInfo>();
    existingPipe->id_ = 123;
    existingPipe->adapterName_ = "existing_adapter";
    audioPipeManager->AddAudioPipeInfo(existingPipe);

    AudioIOHandle nonExistingId = 456;
    audioPipeManager->RemoveAudioPipeInfo(nonExistingId);
    auto pipeList = audioPipeManager->GetPipeList();
    EXPECT_EQ(pipeList.size(), 1);
    EXPECT_EQ(pipeList[0]->adapterName_, "existing_adapter");
}

/**
 * @tc.name: UpdateAudioPipeInfo_001
 * @tc.desc: Test UpdateAudioPipeInfo when entering the if branch in the for loop.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, UpdateAudioPipeInfo_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    auto pipeList01 = audioPipeManager->GetPipeList();
    EXPECT_EQ(pipeList01.size(), 0);

    std::shared_ptr<AudioPipeInfo> existingPipe = std::make_shared<AudioPipeInfo>();
    existingPipe->adapterName_ = "existing_adapter";
    existingPipe->routeFlag_ = 1;
    existingPipe->id_ = 123;
    audioPipeManager->AddAudioPipeInfo(existingPipe);

    std::shared_ptr<AudioPipeInfo> newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->adapterName_ = "existing_adapter";
    newPipe->routeFlag_ = 1;
    newPipe->id_ = 999;

    audioPipeManager->UpdateAudioPipeInfo(newPipe);
    auto pipeList = audioPipeManager->GetPipeList();
    EXPECT_EQ(pipeList.size(), 1);
}

/**
 * @tc.name: UpdateAudioPipeInfo_002
 * @tc.desc: Test UpdateAudioPipeInfo when not entering the if branch in the for loop.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, UpdateAudioPipeInfo_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> existingPipe = std::make_shared<AudioPipeInfo>();
    existingPipe->adapterName_ = "existing_adapter";
    existingPipe->routeFlag_ = 1;
    existingPipe->id_ = 123;
    audioPipeManager->AddAudioPipeInfo(existingPipe);

    std::shared_ptr<AudioPipeInfo> newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->adapterName_ = "new_adapter";
    newPipe->routeFlag_ = 2;
    newPipe->id_ = 999;

    audioPipeManager->UpdateAudioPipeInfo(newPipe);

    auto pipeList = audioPipeManager->GetPipeList();
    EXPECT_EQ(pipeList.size(), 1);
    EXPECT_EQ(pipeList[0]->adapterName_, "existing_adapter");
}

/**
 * @tc.name: IsSamePipe_001
 * @tc.desc: Test IsSamePipe when adapterName and routeFlag are the same.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsSamePipe_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    std::shared_ptr<AudioPipeInfo> info = std::make_shared<AudioPipeInfo>();
    info->adapterName_ = "test_adapter";
    info->routeFlag_ = 1;
    info->id_ = 123;

    std::shared_ptr<AudioPipeInfo> cmpInfo = std::make_shared<AudioPipeInfo>();
    cmpInfo->adapterName_ = "test_adapter";
    cmpInfo->routeFlag_ = 1;
    cmpInfo->id_ = 456;

    bool result = audioPipeManager->IsSamePipe(info, cmpInfo);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsSamePipe_002
 * @tc.desc: Test IsSamePipe when id is the same but adapterName is different.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsSamePipe_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    std::shared_ptr<AudioPipeInfo> info = std::make_shared<AudioPipeInfo>();
    info->adapterName_ = "test_adapter";
    info->routeFlag_ = 1;
    info->id_ = 123;

    std::shared_ptr<AudioPipeInfo> cmpInfo = std::make_shared<AudioPipeInfo>();
    cmpInfo->adapterName_ = "different_adapter";
    cmpInfo->routeFlag_ = 2;
    cmpInfo->id_ = 123;

    bool result = audioPipeManager->IsSamePipe(info, cmpInfo);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsSamePipe_003
 * @tc.desc: Test IsSamePipe when adapterName and id are the same but routeFlag is different.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsSamePipe_003, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    std::shared_ptr<AudioPipeInfo> info = std::make_shared<AudioPipeInfo>();
    info->adapterName_ = "test_adapter";
    info->routeFlag_ = 1;
    info->id_ = 123;

    std::shared_ptr<AudioPipeInfo> cmpInfo = std::make_shared<AudioPipeInfo>();
    cmpInfo->adapterName_ = "test_adapter";
    cmpInfo->routeFlag_ = 2;
    cmpInfo->id_ = 123;

    bool result = audioPipeManager->IsSamePipe(info, cmpInfo);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsSamePipe_004
 * @tc.desc: Test IsSamePipe when none of the conditions are met.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsSamePipe_004, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    std::shared_ptr<AudioPipeInfo> info = std::make_shared<AudioPipeInfo>();
    info->adapterName_ = "test_adapter";
    info->routeFlag_ = 1;
    info->id_ = 123;

    std::shared_ptr<AudioPipeInfo> cmpInfo = std::make_shared<AudioPipeInfo>();
    cmpInfo->adapterName_ = "different_adapter";
    cmpInfo->routeFlag_ = 2;
    cmpInfo->id_ = 456;

    bool result = audioPipeManager->IsSamePipe(info, cmpInfo);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsSamePipe_005
 * @tc.desc: Test IsSamePipe when adapterName, routeFlag and name_ are all the same.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsSamePipe_005, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    std::shared_ptr<AudioPipeInfo> info = std::make_shared<AudioPipeInfo>();
    info->adapterName_ = "test_adapter";
    info->routeFlag_ = 1;
    info->name_ = "pipe_name";
    info->id_ = 123;

    std::shared_ptr<AudioPipeInfo> cmpInfo = std::make_shared<AudioPipeInfo>();
    cmpInfo->adapterName_ = "test_adapter";
    cmpInfo->routeFlag_ = 1;
    cmpInfo->name_ = "pipe_name";
    cmpInfo->id_ = 456;

    bool result = audioPipeManager->IsSamePipe(info, cmpInfo);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsSamePipe_006
 * @tc.desc: Test IsSamePipe when adapterName and routeFlag are the same but name_ is different.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsSamePipe_006, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    std::shared_ptr<AudioPipeInfo> info = std::make_shared<AudioPipeInfo>();
    info->adapterName_ = "test_adapter";
    info->routeFlag_ = 1;
    info->name_ = "pipe_name_1";
    info->id_ = 123;

    std::shared_ptr<AudioPipeInfo> cmpInfo = std::make_shared<AudioPipeInfo>();
    cmpInfo->adapterName_ = "test_adapter";
    cmpInfo->routeFlag_ = 1;
    cmpInfo->name_ = "pipe_name_2";
    cmpInfo->id_ = 456;

    bool result = audioPipeManager->IsSamePipe(info, cmpInfo);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: GetUnusedPipe_001
 * @tc.desc: Test GetUnusedPipe when pipe meets the conditions (streamDescriptors is empty and routeFlag is special).
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetUnusedPipe_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    pipe1->streamDescriptors_.clear();
    audioPipeManager->AddAudioPipeInfo(pipe1);

    auto unusedPipes = audioPipeManager->GetUnusedPipe();
    EXPECT_EQ(unusedPipes.size(), 1);
    EXPECT_EQ(unusedPipes[0]->routeFlag_, AUDIO_OUTPUT_FLAG_FAST);
}

/**
 * @tc.name: GetUnusedPipe_002
 * @tc.desc: Test GetUnusedPipe when pipe does not meet the condition (streamDescriptors is not empty).
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetUnusedPipe_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    pipe1->streamDescriptors_.push_back(std::make_shared<AudioStreamDescriptor>());
    audioPipeManager->AddAudioPipeInfo(pipe1);
    auto unusedPipes = audioPipeManager->GetUnusedPipe();
    EXPECT_EQ(unusedPipes.size(), 0);
}

/**
 * @tc.name: GetUnusedPipe_003
 * @tc.desc: Test GetUnusedPipe when pipe meets streamDescriptors empty but routeFlag is not special.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetUnusedPipe_003, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->routeFlag_ = 0;
    pipe1->streamDescriptors_.clear();
    audioPipeManager->AddAudioPipeInfo(pipe1);
    auto unusedPipes = audioPipeManager->GetUnusedPipe();
    EXPECT_EQ(unusedPipes.size(), 0);
}

/**
 * @tc.name: GetUnusedPipe_004
 * @tc.desc: Test GetUnusedPipe when pipe meets streamDescriptors empty but routeFlag is not special.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetUnusedPipe_004, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
 
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->adapterName_ = ADAPTER_TYPE_VA;
    pipe1->streamDescriptors_.clear();
    audioPipeManager->AddAudioPipeInfo(pipe1);
    auto unusedPipes = audioPipeManager->GetUnusedPipe();
    EXPECT_EQ(unusedPipes.size(), 0);
}
 
/**
 * @tc.name: GetUnusedPipe_005
 * @tc.desc: Test GetUnusedPipe when pipe meets streamDescriptors empty but routeFlag is not special.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetUnusedPipe_005, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
 
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->adapterName_ = ADAPTER_TYPE_VA;
    pipe1->routeFlag_ == AUDIO_INPUT_FLAG_NORMAL;
    pipe1->streamDescriptors_.clear();
    audioPipeManager->AddAudioPipeInfo(pipe1);
    auto unusedPipes = audioPipeManager->GetUnusedPipe();
    EXPECT_EQ(unusedPipes.size(), 0);
}
 
/**
 * @tc.name: GetUnusedPipe_006
 * @tc.desc: Test GetUnusedPipe when pipe meets streamDescriptors empty but routeFlag is not special.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetUnusedPipe_006, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
 
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->routeFlag_ == AUDIO_INPUT_FLAG_NORMAL;
    pipe1->streamDescriptors_.clear();
    audioPipeManager->AddAudioPipeInfo(pipe1);
    auto unusedPipes = audioPipeManager->GetUnusedPipe();
    EXPECT_EQ(unusedPipes.size(), 0);
}

/**
 * @tc.name: IsSpecialPipe_001
 * @tc.desc: Test IsSpecialPipe when routeFlag includes AUDIO_OUTPUT_FLAG_FAST.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsSpecialPipe_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t routeFlag = AUDIO_OUTPUT_FLAG_FAST;
    bool result = audioPipeManager->IsSpecialPipe(routeFlag);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsSpecialPipe_002
 * @tc.desc: Test IsSpecialPipe when routeFlag includes AUDIO_INPUT_FLAG_FAST.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsSpecialPipe_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t routeFlag = AUDIO_INPUT_FLAG_FAST;
    bool result = audioPipeManager->IsSpecialPipe(routeFlag);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsSpecialPipe_003
 * @tc.desc: Test IsSpecialPipe when routeFlag includes AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsSpecialPipe_003, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t routeFlag = AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD;
    bool result = audioPipeManager->IsSpecialPipe(routeFlag);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsSpecialPipe_004
 * @tc.desc: Test IsSpecialPipe when none of the conditions are met.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsSpecialPipe_004, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t routeFlag = AUDIO_OUTPUT_FLAG_NORMAL;
    bool result = audioPipeManager->IsSpecialPipe(routeFlag);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsSpecialPipe_005
 * @tc.desc: Test IsSpecialPipe when none of the conditions are met.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsSpecialPipe_005, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t routeFlag = AUDIO_INPUT_FLAG_AI;
    bool result = audioPipeManager->IsSpecialPipe(routeFlag);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsSpecialPipe_006
 * @tc.desc: Test IsSpecialPipe when none of the conditions are met.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsSpecialPipe_006, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t routeFlag = AUDIO_INPUT_FLAG_UNPROCESS;
    bool result = audioPipeManager->IsSpecialPipe(routeFlag);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsSpecialPipe_007
 * @tc.desc: Test IsSpecialPipe when none of the conditions are met.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsSpecialPipe_007, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t routeFlag = AUDIO_INPUT_FLAG_ULTRASONIC;
    bool result = audioPipeManager->IsSpecialPipe(routeFlag);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsSpecialPipe_008
 * @tc.desc: Test IsSpecialPipe when none of the conditions are met.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsSpecialPipe_008, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t routeFlag = AUDIO_INPUT_FLAG_VOICE_RECOGNITION;
    bool result = audioPipeManager->IsSpecialPipe(routeFlag);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsSpecialPipe_009
 * @tc.desc: Test IsSpecialPipe when none of the conditions are met.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsSpecialPipe_009, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t routeFlag = AUDIO_INPUT_FLAG_RAW_AI;
    bool result = audioPipeManager->IsSpecialPipe(routeFlag);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsSpecialPipe_010
 * @tc.desc: Test IsSpecialPipe when none of the conditions are met.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsSpecialPipe_010, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t routeFlag = AUDIO_OUTPUT_FLAG_HWDECODING;
    bool result = audioPipeManager->IsSpecialPipe(routeFlag);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsSpecialPipe_011
 * @tc.desc: Test IsSpecialPipe when route flag is live.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsSpecialPipe_011, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t routeFlag = AUDIO_INPUT_FLAG_LIVE;
    bool result = audioPipeManager->IsSpecialPipe(routeFlag);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsSpecialPipe_012
 * @tc.desc: Test IsSpecialPipe when routeFlag includes AUDIO_INPUT_FLAG_CAMCORDER.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsSpecialPipe_012, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t routeFlag = AUDIO_INPUT_FLAG_CAMCORDER;
    bool result = audioPipeManager->IsSpecialPipe(routeFlag);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: GetPipeinfoByNameAndFlag_001
 * @tc.desc: Test GetPipeinfoByNameAndFlag when adapterName does not match.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPipeinfoByNameAndFlag_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->adapterName_ = "existing_adapter";
    pipe1->routeFlag_ = 1;
    audioPipeManager->AddAudioPipeInfo(pipe1);

    std::string targetAdapterName = "non_existing_adapter";
    uint32_t targetRouteFlag = 1;
    auto result = audioPipeManager->GetPipeinfoByNameAndFlag(targetAdapterName, targetRouteFlag);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetPipeinfoByNameAndFlag_002
 * @tc.desc: Test GetPipeinfoByNameAndFlag when adapterName matches but routeFlag does not.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPipeinfoByNameAndFlag_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->adapterName_ = "existing_adapter";
    pipe1->routeFlag_ = 1;
    audioPipeManager->AddAudioPipeInfo(pipe1);
    std::string targetAdapterName = "existing_adapter";
    uint32_t targetRouteFlag = 2;
    auto result = audioPipeManager->GetPipeinfoByNameAndFlag(targetAdapterName, targetRouteFlag);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetPipeinfoByNameAndFlag_003
 * @tc.desc: Test GetPipeinfoByNameAndFlag when both adapterName and routeFlag match.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPipeinfoByNameAndFlag_003, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->adapterName_ = "existing_adapter";
    pipe1->routeFlag_ = 1;
    audioPipeManager->AddAudioPipeInfo(pipe1);
    std::string targetAdapterName = "existing_adapter";
    uint32_t targetRouteFlag = 1;

    auto result = audioPipeManager->GetPipeinfoByNameAndFlag(targetAdapterName, targetRouteFlag);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->adapterName_, "existing_adapter");
}

/**
 * @tc.name: GetModuleNameBySessionId_001
 * @tc.desc: Test GetModuleNameBySessionId when finding a matching sessionId.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetModuleNameBySessionId_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->moduleInfo_.name = "TestAdapter";
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 123;
    desc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    desc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);
    uint32_t targetSessionId = 123;

    std::string result = audioPipeManager->GetModuleNameBySessionId(targetSessionId);
    EXPECT_EQ(result, "TestAdapter");
}

/**
 * @tc.name: GetModuleNameBySessionId_002
 * @tc.desc: Test GetModuleNameBySessionId when not finding a matching sessionId.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetModuleNameBySessionId_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->moduleInfo_.name = "TestAdapter";
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 123;
    desc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    desc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);
    uint32_t targetSessionId = 456;

    std::string result = audioPipeManager->GetModuleNameBySessionId(targetSessionId);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: GetProcessDeviceInfoBySessionId_001
 * @tc.desc: Test GetProcessDeviceInfoBySessionId when finding a matching sessionId.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetProcessDeviceInfoBySessionId_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 123;
    desc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    desc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    AudioStreamInfo info;
    uint32_t targetSessionId = 123;
    auto result = audioPipeManager->GetProcessDeviceInfoBySessionId(targetSessionId, info);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_SPEAKER);
}

/**
 * @tc.name: GetProcessDeviceInfoBySessionId_002
 * @tc.desc: Test GetProcessDeviceInfoBySessionId when not finding a matching sessionId.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetProcessDeviceInfoBySessionId_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 123;
    desc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    desc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    uint32_t targetSessionId = 456;
    AudioStreamInfo info;
    auto result = audioPipeManager->GetProcessDeviceInfoBySessionId(targetSessionId, info);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetAllOutputStreamDescs_001
 * @tc.desc: Test GetAllOutputStreamDescs when finding output stream descriptors.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetAllOutputStreamDescs_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    std::shared_ptr<AudioStreamDescriptor> desc1 = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioStreamDescriptor> desc2 = std::make_shared<AudioStreamDescriptor>();
    pipeInfo->streamDescriptors_.push_back(desc1);
    pipeInfo->streamDescriptors_.push_back(desc2);

    audioPipeManager->AddAudioPipeInfo(pipeInfo);
    auto result = audioPipeManager->GetAllOutputStreamDescs();
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], desc1);
    EXPECT_EQ(result[1], desc2);
}

/**
 * @tc.name: GetAllOutputStreamDescs_002
 * @tc.desc: Test GetAllOutputStreamDescs when no output stream descriptors are found.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetAllOutputStreamDescs_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    std::shared_ptr<AudioStreamDescriptor> desc1 = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioStreamDescriptor> desc2 = std::make_shared<AudioStreamDescriptor>();
    pipeInfo->streamDescriptors_.push_back(desc1);
    pipeInfo->streamDescriptors_.push_back(desc2);

    audioPipeManager->AddAudioPipeInfo(pipeInfo);
    auto result = audioPipeManager->GetAllOutputStreamDescs();
    EXPECT_EQ(result.size(), 0);
}

/**
 * @tc.name: GetAllInputStreamDescs_001
 * @tc.desc: Test GetAllInputStreamDescs when finding input stream descriptors.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetAllInputStreamDescs_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    std::shared_ptr<AudioStreamDescriptor> desc1 = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioStreamDescriptor> desc2 = std::make_shared<AudioStreamDescriptor>();
    pipeInfo->streamDescriptors_.push_back(desc1);
    pipeInfo->streamDescriptors_.push_back(desc2);

    audioPipeManager->AddAudioPipeInfo(pipeInfo);
    auto result = audioPipeManager->GetAllInputStreamDescs();
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], desc1);
    EXPECT_EQ(result[1], desc2);
}

/**
 * @tc.name: GetAllInputStreamDescs_002
 * @tc.desc: Test GetAllInputStreamDescs when no input stream descriptors are found.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetAllInputStreamDescs_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    std::shared_ptr<AudioStreamDescriptor> desc1 = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioStreamDescriptor> desc2 = std::make_shared<AudioStreamDescriptor>();
    pipeInfo->streamDescriptors_.push_back(desc1);
    pipeInfo->streamDescriptors_.push_back(desc2);

    audioPipeManager->AddAudioPipeInfo(pipeInfo);
    auto result = audioPipeManager->GetAllInputStreamDescs();
    EXPECT_EQ(result.size(), 0);
}

/**
 * @tc.name: GetStreamDescByIdInner_001
 * @tc.desc: Test GetStreamDescByIdInner when finding a matching sessionId.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetStreamDescByIdInner_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 123;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    uint32_t targetSessionId = 123;
    auto result = audioPipeManager->GetStreamDescByIdInner(targetSessionId);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->sessionId_, 123);
}

/**
 * @tc.name: GetStreamDescByIdInner_002
 * @tc.desc: Test GetStreamDescByIdInner when not finding a matching sessionId.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetStreamDescByIdInner_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 123;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    uint32_t targetSessionId = 456;
    auto result = audioPipeManager->GetStreamDescByIdInner(targetSessionId);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetStreamCount_001
 * @tc.desc: Test GetStreamCount when adapterName does not match.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetStreamCount_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "existing_adapter";
    pipeInfo->routeFlag_ = 1;
    pipeInfo->streamDescriptors_.resize(2);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    std::string targetAdapterName = "non_existing_adapter";
    uint32_t targetRouteFlag = 1;
    int32_t result = audioPipeManager->GetStreamCount(targetAdapterName, targetRouteFlag);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: GetStreamCount_002
 * @tc.desc: Test GetStreamCount when adapterName matches but routeFlag does not.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetStreamCount_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "existing_adapter";
    pipeInfo->routeFlag_ = 1;
    pipeInfo->streamDescriptors_.resize(2);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    std::string targetAdapterName = "existing_adapter";
    uint32_t targetRouteFlag = 2;
    int32_t result = audioPipeManager->GetStreamCount(targetAdapterName, targetRouteFlag);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: GetStreamCount_003
 * @tc.desc: Test GetStreamCount when both adapterName and routeFlag match.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetStreamCount_003, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "existing_adapter";
    pipeInfo->routeFlag_ = 1;
    pipeInfo->streamDescriptors_.resize(2);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    std::string targetAdapterName = "existing_adapter";
    uint32_t targetRouteFlag = 1;
    int32_t result = audioPipeManager->GetStreamCount(targetAdapterName, targetRouteFlag);
    EXPECT_EQ(result, 2);
}

/**
 * @tc.name: GetPaIndexByIoHandle_001
 * @tc.desc: Test GetPaIndexByIoHandle when finding a matching id.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPaIndexByIoHandle_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = 123;
    pipeInfo->paIndex_ = 456;
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    AudioIOHandle targetId = 123;
    uint32_t result = audioPipeManager->GetPaIndexByIoHandle(targetId);
    EXPECT_EQ(result, 456);
}

/**
 * @tc.name: GetPaIndexByIoHandle_002
 * @tc.desc: Test GetPaIndexByIoHandle when not finding a matching id.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPaIndexByIoHandle_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = 123;
    pipeInfo->paIndex_ = 456;
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    AudioIOHandle targetId = 789;
    uint32_t result = audioPipeManager->GetPaIndexByIoHandle(targetId);
    EXPECT_EQ(result, HDI_INVALID_ID);
}

/**
 * @tc.name: UpdateRendererPipeInfos_001
 * @tc.desc: Test UpdateRendererPipeInfos when entering the if branch in the for loop.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, UpdateRendererPipeInfos_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> inputPipe = std::make_shared<AudioPipeInfo>();
    inputPipe->pipeRole_ = PIPE_ROLE_INPUT;
    audioPipeManager->AddAudioPipeInfo(inputPipe);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos;
    std::shared_ptr<AudioPipeInfo> newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipeInfos.push_back(newPipe);

    audioPipeManager->UpdateRendererPipeInfos(pipeInfos);
    auto pipeList = audioPipeManager->GetPipeList();
    EXPECT_EQ(pipeList.size(), 2);
    EXPECT_EQ(pipeList[0]->pipeRole_, PIPE_ROLE_INPUT);
    EXPECT_EQ(pipeList[1]->pipeRole_, PIPE_ROLE_OUTPUT);
}

/**
 * @tc.name: UpdateRendererPipeInfos_002
 * @tc.desc: Test UpdateRendererPipeInfos when not entering the if branch in the for loop.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, UpdateRendererPipeInfos_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> outputPipe = std::make_shared<AudioPipeInfo>();
    outputPipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    audioPipeManager->AddAudioPipeInfo(outputPipe);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos;
    std::shared_ptr<AudioPipeInfo> newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipeInfos.push_back(newPipe);

    audioPipeManager->UpdateRendererPipeInfos(pipeInfos);
    auto pipeList = audioPipeManager->GetPipeList();
    EXPECT_EQ(pipeList.size(), 1);
    EXPECT_EQ(pipeList[0]->pipeRole_, PIPE_ROLE_OUTPUT);
}

/**
 * @tc.name: UpdateCapturerPipeInfos_001
 * @tc.desc: Test UpdateCapturerPipeInfos when entering the if branch in the for loop.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, UpdateCapturerPipeInfos_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> outputPipe = std::make_shared<AudioPipeInfo>();
    outputPipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    audioPipeManager->AddAudioPipeInfo(outputPipe);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos;
    std::shared_ptr<AudioPipeInfo> newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipeInfos.push_back(newPipe);
    audioPipeManager->UpdateCapturerPipeInfos(pipeInfos);

    auto pipeList = audioPipeManager->GetPipeList();
    EXPECT_EQ(pipeList.size(), 2);
    EXPECT_EQ(pipeList[0]->pipeRole_, PIPE_ROLE_OUTPUT);
    EXPECT_EQ(pipeList[1]->pipeRole_, PIPE_ROLE_INPUT);
}

/**
 * @tc.name: UpdateCapturerPipeInfos_002
 * @tc.desc: Test UpdateCapturerPipeInfos when not entering the if branch in the for loop.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, UpdateCapturerPipeInfos_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> inputPipe = std::make_shared<AudioPipeInfo>();
    inputPipe->pipeRole_ = PIPE_ROLE_INPUT;
    audioPipeManager->AddAudioPipeInfo(inputPipe);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos;
    std::shared_ptr<AudioPipeInfo> newPipe = std::make_shared<AudioPipeInfo>();
    newPipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipeInfos.push_back(newPipe);

    audioPipeManager->UpdateCapturerPipeInfos(pipeInfos);
    auto pipeList = audioPipeManager->GetPipeList();
    EXPECT_EQ(pipeList.size(), 1);
    EXPECT_EQ(pipeList[0]->pipeRole_, PIPE_ROLE_INPUT);
}

/**
 * @tc.name: PcmOffloadSessionCount_001
 * @tc.desc: Test PcmOffloadSessionCount when entering the if branch in the for loop.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, PcmOffloadSessionCount_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    pipeInfo->streamDescriptors_.resize(2);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    uint32_t result = audioPipeManager->PcmOffloadSessionCount();
    EXPECT_EQ(result, 2);
}

/**
 * @tc.name: PcmOffloadSessionCount_002
 * @tc.desc: Test PcmOffloadSessionCount when not entering the if branch in the for loop.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, PcmOffloadSessionCount_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->routeFlag_ = 0;
    pipeInfo->streamDescriptors_.resize(2);

    audioPipeManager->AddAudioPipeInfo(pipeInfo);
    uint32_t result = audioPipeManager->PcmOffloadSessionCount();
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: AddModemCommunicationId_001
 * @tc.desc: Test AddModemCommunicationId when sessionId is less than FIRST_SESSIONID.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, AddModemCommunicationId_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t sessionId = 99999;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioPipeManager->modemCommunicationIdMap_.clear();
    audioPipeManager->AddModemCommunicationId(sessionId, streamDesc);

    auto modemMap = audioPipeManager->GetModemCommunicationMap();
    EXPECT_EQ(modemMap.size(), 1);
    EXPECT_EQ(modemMap.find(sessionId)->second, streamDesc);
}

/**
 * @tc.name: AddModemCommunicationId_002
 * @tc.desc: Test AddModemCommunicationId when sessionId is greater than MAX_VALID_SESSIONID.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, AddModemCommunicationId_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t sessionId = 4294867296;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioPipeManager->modemCommunicationIdMap_.clear();

    audioPipeManager->AddModemCommunicationId(sessionId, streamDesc);
    auto modemMap = audioPipeManager->GetModemCommunicationMap();
    EXPECT_EQ(modemMap.size(), 1);
    EXPECT_EQ(modemMap.find(sessionId)->second, streamDesc);
}

/**
 * @tc.name: AddModemCommunicationId_003
 * @tc.desc: Test AddModemCommunicationId when sessionId is within the valid range.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, AddModemCommunicationId_003, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t sessionId = 100000;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioPipeManager->modemCommunicationIdMap_.clear();

    audioPipeManager->AddModemCommunicationId(sessionId, streamDesc);
    auto modemMap = audioPipeManager->GetModemCommunicationMap();
    EXPECT_EQ(modemMap.size(), 1);
    EXPECT_EQ(modemMap.find(sessionId)->second, streamDesc);
}

/**
 * @tc.name: RemoveModemCommunicationId_001
 * @tc.desc: Test RemoveModemCommunicationId when sessionId exists.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, RemoveModemCommunicationId_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t sessionId = 12345;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();

    audioPipeManager->modemCommunicationIdMap_[sessionId] = streamDesc;
    audioPipeManager->RemoveModemCommunicationId(sessionId);
    auto modemMap = audioPipeManager->GetModemCommunicationMap();
    EXPECT_EQ(modemMap.find(sessionId), modemMap.end());
}

/**
 * @tc.name: RemoveModemCommunicationId_002
 * @tc.desc: Test RemoveModemCommunicationId when sessionId does not exist.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, RemoveModemCommunicationId_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t sessionId = 12345;
    audioPipeManager->modemCommunicationIdMap_.clear();

    audioPipeManager->RemoveModemCommunicationId(sessionId);
    auto modemMap = audioPipeManager->GetModemCommunicationMap();
    EXPECT_EQ(modemMap.find(sessionId), modemMap.end());
}

/**
 * @tc.name: GetModemCommunicationStreamDescById_001
 * @tc.desc: Test GetModemCommunicationStreamDescById when sessionId is within the valid range.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetModemCommunicationStreamDescById_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t sessionId = 100000;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioPipeManager->modemCommunicationIdMap_.clear();

    audioPipeManager->AddModemCommunicationId(sessionId, streamDesc);
    auto streamDescRet = audioPipeManager->GetModemCommunicationStreamDescById(sessionId);
    EXPECT_EQ(streamDescRet != nullptr, true);
}

/**
 * @tc.name: GetModemCommunicationStreamDescById_002
 * @tc.desc: Test GetModemCommunicationStreamDescById when sessionId is without the valid range.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetModemCommunicationStreamDescById_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    uint32_t sessionId = 100000;
    audioPipeManager->modemCommunicationIdMap_.clear();
    auto streamDescRet = audioPipeManager->GetModemCommunicationStreamDescById(sessionId);
    EXPECT_EQ(streamDescRet == nullptr, true);
}

/**
 * @tc.name: GetLastestRunningCallStreamUsage_001
 * @tc.desc: Test GetLastestRunningCallStreamUsage returns running modem stream usage first.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetLastestRunningCallStreamUsage_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    audioPipeManager->modemCommunicationIdMap_.clear();

    audioPipeManager->modemCommunicationIdMap_[100000] =
        MakeCallStreamDesc(STREAM_USAGE_VOICE_MODEM_COMMUNICATION, STREAM_STATUS_STARTED);

    StreamUsage result = audioPipeManager->GetLastestRunningCallStreamUsage();
    EXPECT_EQ(result, STREAM_USAGE_VOICE_MODEM_COMMUNICATION);
}

/**
 * @tc.name: GetLastestRunningCallStreamUsage_002
 * @tc.desc: Test GetLastestRunningCallStreamUsage skips invalid modem and pipe entries, then returns voice usage.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetLastestRunningCallStreamUsage_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    audioPipeManager->modemCommunicationIdMap_.clear();

    audioPipeManager->modemCommunicationIdMap_[100000] = nullptr;
    audioPipeManager->modemCommunicationIdMap_[100001] =
        MakeCallStreamDesc(STREAM_USAGE_VOICE_MODEM_COMMUNICATION, STREAM_STATUS_STOPPED);

    audioPipeManager->curPipeList_.push_back(nullptr);
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    ASSERT_NE(nullptr, pipeInfo);
    pipeInfo->streamDescriptors_.push_back(nullptr);
    pipeInfo->streamDescriptors_.push_back(
        MakeCallStreamDesc(STREAM_USAGE_VOICE_COMMUNICATION, STREAM_STATUS_STARTED));
    audioPipeManager->curPipeList_.push_back(pipeInfo);

    StreamUsage result = audioPipeManager->GetLastestRunningCallStreamUsage();
    EXPECT_EQ(result, STREAM_USAGE_VOICE_COMMUNICATION);
}

/**
 * @tc.name: GetLastestRunningCallStreamUsage_003
 * @tc.desc: Test GetLastestRunningCallStreamUsage returns running video communication usage from pipe list.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetLastestRunningCallStreamUsage_003, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    audioPipeManager->modemCommunicationIdMap_.clear();

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    ASSERT_NE(nullptr, pipeInfo);
    pipeInfo->streamDescriptors_.push_back(
        MakeCallStreamDesc(STREAM_USAGE_VIDEO_COMMUNICATION, STREAM_STATUS_STARTED));
    audioPipeManager->curPipeList_.push_back(pipeInfo);

    StreamUsage result = audioPipeManager->GetLastestRunningCallStreamUsage();
    EXPECT_EQ(result, STREAM_USAGE_VIDEO_COMMUNICATION);
}

/**
 * @tc.name: GetLastestRunningCallStreamUsage_004
 * @tc.desc: Test GetLastestRunningCallStreamUsage returns unknown when no running call stream exists.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetLastestRunningCallStreamUsage_004, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    audioPipeManager->modemCommunicationIdMap_.clear();

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    ASSERT_NE(nullptr, pipeInfo);
    pipeInfo->streamDescriptors_.push_back(
        MakeCallStreamDesc(STREAM_USAGE_VOICE_COMMUNICATION, STREAM_STATUS_STOPPED));
    pipeInfo->streamDescriptors_.push_back(
        MakeCallStreamDesc(STREAM_USAGE_MUSIC, STREAM_STATUS_STARTED));
    audioPipeManager->curPipeList_.push_back(pipeInfo);

    StreamUsage result = audioPipeManager->GetLastestRunningCallStreamUsage();
    EXPECT_EQ(result, STREAM_USAGE_UNKNOWN);
}

/**
 * @tc.name: GetNormalSourceInfo_001
 * @tc.desc: Test GetNormalSourceInfo when isEcFeatureEnable is true.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetNormalSourceInfo_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> bluetoothPipe = std::make_shared<AudioPipeInfo>();
    bluetoothPipe->moduleInfo_.name = PRIMARY_MIC;
    bluetoothPipe->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    audioPipeManager->AddAudioPipeInfo(bluetoothPipe);

    bool isEcFeatureEnable = true;
    auto result = audioPipeManager->GetNormalSourceInfo(isEcFeatureEnable);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->moduleInfo_.name, PRIMARY_MIC);
}

/**
 * @tc.name: GetNormalSourceInfo_002
 * @tc.desc: Test GetNormalSourceInfo when isEcFeatureEnable is false.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetNormalSourceInfo_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> primaryPipe = std::make_shared<AudioPipeInfo>();
    primaryPipe->moduleInfo_.name = PRIMARY_MIC;
    primaryPipe->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    audioPipeManager->AddAudioPipeInfo(primaryPipe);

    bool isEcFeatureEnable = false;
    auto result = audioPipeManager->GetNormalSourceInfo(isEcFeatureEnable);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->moduleInfo_.name, PRIMARY_MIC);
}

/**
 * @tc.name: GetPipeByModuleAndFlag_001
 * @tc.desc: Test GetPipeByModuleAndFlag when module name does not match.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPipeByModuleAndFlag_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->moduleInfo_.name = "EXISTING_MODULE";
    pipeInfo->routeFlag_ = 1;
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    std::string targetModuleName = "NON_EXISTING_MODULE";
    uint32_t targetRouteFlag = 1;
    auto result = audioPipeManager->GetPipeByModuleAndFlag(targetModuleName, targetRouteFlag);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetPipeByModuleAndFlag_002
 * @tc.desc: Test GetPipeByModuleAndFlag when module name matches but route flag does not.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPipeByModuleAndFlag_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->moduleInfo_.name = "EXISTING_MODULE";
    pipeInfo->routeFlag_ = 1;
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    std::string targetModuleName = "EXISTING_MODULE";
    uint32_t targetRouteFlag = 2;
    auto result = audioPipeManager->GetPipeByModuleAndFlag(targetModuleName, targetRouteFlag);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetPipeByModuleAndFlag_003
 * @tc.desc: Test GetPipeByModuleAndFlag when both module name and route flag match.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPipeByModuleAndFlag_003, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->moduleInfo_.name = "EXISTING_MODULE";
    pipeInfo->routeFlag_ = 1;
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    std::string targetModuleName = "EXISTING_MODULE";
    uint32_t targetRouteFlag = 1;
    auto result = audioPipeManager->GetPipeByModuleAndFlag(targetModuleName, targetRouteFlag);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->moduleInfo_.name, "EXISTING_MODULE");
}

/**
 * @tc.name: AudioPipeInfo_ToString_001
 * @tc.desc: Test AudioPipeInfo ToString.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, AudioPipeInfo_ToString_001, TestSize.Level2)
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::string out = pipeInfo->ToString();
    EXPECT_NE(out, "");
}

/**
 * @tc.name: AudioPipeInfo_Dump_001
 * @tc.desc: Test AudioPipeInfo ToString.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, AudioPipeInfo_Dump_001, TestSize.Level2)
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::string dumpString = "";
    pipeInfo->Dump(dumpString);
    EXPECT_NE(dumpString, "");
}

/**
 * @tc.name: AudioStreamDescriptor_GetNewDevicesTypeString_001
 * @tc.desc: Test AudioStreamDescriptor GetNewDevicesTypeString.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, AudioStreamDescriptor_GetNewDevicesTypeString_001, TestSize.Level2)
{
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 1;
    desc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    desc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;

    std::string out = desc->GetNewDevicesTypeString();
    EXPECT_NE(out, "");
}

/**
 * @tc.name: AudioStreamDescriptor_GetNewDupDevicesTypeString_001
 * @tc.desc: Test AudioStreamDescriptor GetNewDevicesTypeString.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, AudioStreamDescriptor_GetNewDupDevicesTypeString_001, TestSize.Level2)
{
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 1;
    desc->newDupDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    desc->newDupDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;

    std::string out = desc->GetNewDupDevicesTypeString();
    EXPECT_EQ(out, "SPEAKER:");

    desc->newDupDeviceDescs_.clear();

    out = desc->GetNewDupDevicesTypeString();
    EXPECT_EQ(out, "");
}

/**
 * @tc.name: GetAllCapturerStreamDescs_001
 * @tc.desc: Test GetAllCapturerStreamDescs when finding capture stream descriptors.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetAllCapturerStreamDescs_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    std::shared_ptr<AudioStreamDescriptor> desc1 = std::make_shared<AudioStreamDescriptor>();
    desc1->audioMode_ = AUDIO_MODE_RECORD;
    std::shared_ptr<AudioStreamDescriptor> desc2 = std::make_shared<AudioStreamDescriptor>();
    desc2->audioMode_ = AUDIO_MODE_RECORD;
    pipeInfo->streamDescriptors_.push_back(desc1);
    pipeInfo->streamDescriptors_.push_back(desc2);

    audioPipeManager->AddAudioPipeInfo(pipeInfo);
    auto result = audioPipeManager->GetAllCapturerStreamDescs();
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], desc1);
    EXPECT_EQ(result[1], desc2);
}

/**
 * @tc.name: GetAllCapturerStreamDescs_002
 * @tc.desc: Test GetAllCapturerStreamDescs when no capture stream descriptors are found.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetAllCapturerStreamDescs_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    std::shared_ptr<AudioStreamDescriptor> desc1 = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioStreamDescriptor> desc2 = std::make_shared<AudioStreamDescriptor>();
    pipeInfo->streamDescriptors_.push_back(desc1);
    pipeInfo->streamDescriptors_.push_back(desc2);

    audioPipeManager->AddAudioPipeInfo(pipeInfo);
    auto result = audioPipeManager->GetAllCapturerStreamDescs();
    EXPECT_EQ(result.size(), 0);
}

/**
 * @tc.name: IsStreamUsageActive_001
 * @tc.desc: Test IsStreamUsageActive.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, IsStreamUsageActive_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->rendererInfo_.streamUsage = STREAM_USAGE_ALARM;
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    pipeInfo->streamDescriptors_.push_back(desc);

    audioPipeManager->AddAudioPipeInfo(pipeInfo);
    EXPECT_TRUE(audioPipeManager->IsStreamUsageActive(STREAM_USAGE_ALARM));
    EXPECT_FALSE(audioPipeManager->IsStreamUsageActive(STREAM_USAGE_VOICE_RINGTONE));

    desc->streamStatus_ = STREAM_STATUS_STOPPED;
    pipeInfo->streamDescriptors_.push_back(desc);

    audioPipeManager->AddAudioPipeInfo(pipeInfo);
    EXPECT_FALSE(audioPipeManager->IsStreamUsageActive(STREAM_USAGE_ALARM));
}

/**
 * @tc.name: IsCaptureVoipCall_001
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsCaptureVoipCall_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    auto pipeList01 = audioPipeManager->GetPipeList();
    EXPECT_EQ(pipeList01.size(), 0);

    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->adapterName_ = "aaa";
    pipe1->routeFlag_ = 1;
    pipe1->id_ = 123;

    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->streamStatus_ = STREAM_STATUS_STOPPED;
    pipe1->streamDescriptors_.push_back(desc);

    audioPipeManager->AddAudioPipeInfo(pipe1);

    int ret = audioPipeManager->IsCaptureVoipCall();

    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: IsCaptureVoipCall_002
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsCaptureVoipCall_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    auto pipeList01 = audioPipeManager->GetPipeList();
    EXPECT_EQ(pipeList01.size(), 0);

    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->adapterName_ = "aaa";
    pipe1->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    pipe1->id_ = 123;

    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    pipe1->streamDescriptors_.push_back(desc);

    audioPipeManager->AddAudioPipeInfo(pipe1);

    int ret = audioPipeManager->IsCaptureVoipCall();

    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name: IsCaptureVoipCall_004
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsCaptureVoipCall_004, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    auto pipeList01 = audioPipeManager->GetPipeList();
    EXPECT_EQ(pipeList01.size(), 0);

    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->adapterName_ = "aaa";
    pipe1->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    pipe1->id_ = 123;

    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_INVALID;
    pipe1->streamDescriptors_.push_back(desc);

    audioPipeManager->AddAudioPipeInfo(pipe1);

    int ret = audioPipeManager->IsCaptureVoipCall();
    
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: IsCaptureVoipCall_003
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, IsCaptureVoipCall_003, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    auto pipeList01 = audioPipeManager->GetPipeList();
    EXPECT_EQ(pipeList01.size(), 0);

    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->adapterName_ = "aaa";
    pipe1->routeFlag_ = AUDIO_INPUT_FLAG_FAST;
    pipe1->id_ = 123;

    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_FAST;
    pipe1->streamDescriptors_.push_back(desc);

    audioPipeManager->AddAudioPipeInfo(pipe1);

    int ret = audioPipeManager->IsCaptureVoipCall();

    EXPECT_EQ(ret, 2);
}

/**
 * @tc.name: GetPaIndexByName_001
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPaIndexByName_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    auto pipeList01 = audioPipeManager->GetPipeList();
    EXPECT_EQ(pipeList01.size(), 0);

    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->name_ = "aaa";
    pipe1->routeFlag_ = 1;
    pipe1->paIndex_ = 123;
    audioPipeManager->AddAudioPipeInfo(pipe1);

    uint32_t ret = audioPipeManager->GetPaIndexByName("abc");
    EXPECT_EQ(0xFFFFFFFF, ret);

    ret = audioPipeManager->GetPaIndexByName("aaa");
    EXPECT_EQ(123, ret);
}

/**
 * @tc.name: DecideStreamInfo_001
 * @tc.desc: test DecideStreamInfo()
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, DecideStreamInfo_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_USB_ARM_HEADSET, OUTPUT_DEVICE);

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->audioStreamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    pipeInfo->moduleInfo_.rate = "192000";
    auto streamInfo = audioPipeManager->DecideStreamInfo(pipeInfo, deviceDesc);
    EXPECT_EQ(streamInfo.samplingRate, AudioSamplingRate::SAMPLE_RATE_48000);

    pipeInfo->audioStreamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    pipeInfo->moduleInfo_.rate = "96000";
    streamInfo = audioPipeManager->DecideStreamInfo(pipeInfo, deviceDesc);
    EXPECT_EQ(streamInfo.samplingRate, AudioSamplingRate::SAMPLE_RATE_96000);

    deviceDesc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    pipeInfo->audioStreamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    pipeInfo->moduleInfo_.rate = "96000";
    streamInfo = audioPipeManager->DecideStreamInfo(pipeInfo, deviceDesc);
    EXPECT_EQ(streamInfo.samplingRate, AudioSamplingRate::SAMPLE_RATE_48000);
}

/**
 * @tc.name  : Test IsOnPrimaryAdapter.
 * @tc.number: IsOnPrimaryAdapter_001.
 * @tc.desc  : Primary adapter exists and the target stream session is on the primary adapter
 */
HWTEST_F(AudioPipeManagerUnitTest, IsOnPrimaryAdapter_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 10000;

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    pipeInfo->AddStream(streamDesc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    uint32_t targetSessionId = 10000;
    bool ret = audioPipeManager->IsOnPrimaryAdapter(targetSessionId);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test IsOnPrimaryAdapter.
 * @tc.number: IsOnPrimaryAdapter_002.
 * @tc.desc  : Primary adapter exists and the target stream session is not on the primary adapter
 */
HWTEST_F(AudioPipeManagerUnitTest, IsOnPrimaryAdapter_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 10000;

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    pipeInfo->AddStream(streamDesc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    uint32_t targetSessionId = 10001;
    bool ret = audioPipeManager->IsOnPrimaryAdapter(targetSessionId);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test IsOnPrimaryAdapter.
 * @tc.number: IsOnPrimaryAdapter_003.
 * @tc.desc  : Primary adapter does not exist
 */
HWTEST_F(AudioPipeManagerUnitTest, IsOnPrimaryAdapter_003, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 10000;

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "remote";
    pipeInfo->AddStream(streamDesc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    uint32_t targetSessionId = 10000;
    bool ret = audioPipeManager->IsOnPrimaryAdapter(targetSessionId);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetUnusedPipe_007
 * @tc.desc: Test GetUnusedPipe when pipe className is primary_extra.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetUnusedPipe_007, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    pipe1->moduleInfo_.className = "primary_extra";
    pipe1->streamDescriptors_.clear();
    audioPipeManager->AddAudioPipeInfo(pipe1);

    auto unusedPipes = audioPipeManager->GetUnusedPipe();
    EXPECT_EQ(unusedPipes.size(), 1);
    EXPECT_EQ(unusedPipes[0]->moduleInfo_.className, "primary_extra");
}

/**
 * @tc.name: GetUnusedPipe_008
 * @tc.desc: Test GetUnusedPipe when pipe className is primary_extra but streamDescriptors is not empty.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetUnusedPipe_008, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    pipe1->moduleInfo_.className = "primary_extra";
    pipe1->streamDescriptors_.push_back(std::make_shared<AudioStreamDescriptor>());
    audioPipeManager->AddAudioPipeInfo(pipe1);

    auto unusedPipes = audioPipeManager->GetUnusedPipe();
    EXPECT_EQ(unusedPipes.size(), 0);
}

/**
 * @tc.name: AudioPipeInfo_IsSameName_001
 * @tc.desc: Test AudioPipeInfo IsSameName when name matches.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, AudioPipeInfo_IsSameName_001, TestSize.Level1)
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->name_ = "primary_output";

    bool result = pipeInfo->IsSameName("primary_output");
    EXPECT_TRUE(result);
}

/**
 * @tc.name: AudioPipeInfo_IsSameName_002
 * @tc.desc: Test AudioPipeInfo IsSameName when name does not match.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, AudioPipeInfo_IsSameName_002, TestSize.Level1)
{
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->name_ = "primary_output";

    bool result = pipeInfo->IsSameName("primary_typec_output");
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test ClearStandbyFlag.
 * @tc.number: ClearStandbyFlag_001.
 * @tc.desc  : Valid sessionId, streamDesc exists, sets isStandby_ to false
 */
HWTEST_F(AudioPipeManagerUnitTest, ClearStandbyFlag_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 10000;
    streamDesc->isStandby_ = true;

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    pipeInfo->AddStream(streamDesc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    uint32_t targetSessionId = 10000;
    audioPipeManager->ClearStandbyFlag(targetSessionId);
    EXPECT_FALSE(streamDesc->isStandby_);
}

/**
 * @tc.name  : Test ClearStandbyFlag.
 * @tc.number: ClearStandbyFlag_002.
 * @tc.desc  : Invalid sessionId, streamDesc doesn't exist, returns early without modification
 */
HWTEST_F(AudioPipeManagerUnitTest, ClearStandbyFlag_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 10000;
    streamDesc->isStandby_ = true;

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    pipeInfo->AddStream(streamDesc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    uint32_t invalidSessionId = 99999;
    audioPipeManager->ClearStandbyFlag(invalidSessionId);
    EXPECT_TRUE(streamDesc->isStandby_);
}

/**
 * @tc.name  : CheckNoRunningCondition_001
 * @tc.number: CheckNoRunningCondition_001
 * @tc.desc  : Test CheckNoRunningCondition with REMOTE+NORMAL pipe calls CheckRemoteNoRunningCondition.
 */
HWTEST_F(AudioPipeManagerUnitTest, CheckNoRunningCondition_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(audioPipeManager, nullptr);
    audioPipeManager->curPipeList_.clear();

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = ADAPTER_TYPE_REMOTE;
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    pipeInfo->isRunning_ = false;
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    auto remoteOffloadPipe = std::make_shared<AudioPipeInfo>();
    remoteOffloadPipe->adapterName_ = ADAPTER_TYPE_REMOTE;
    remoteOffloadPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    audioPipeManager->AddAudioPipeInfo(remoteOffloadPipe);

    bool result = audioPipeManager->CheckNoRunningCondition(pipeInfo);
    EXPECT_FALSE(result);

    audioPipeManager->RemoveAudioPipeInfo(pipeInfo);
    audioPipeManager->RemoveAudioPipeInfo(remoteOffloadPipe);
}

/**
 * @tc.name  : CheckNoRunningCondition_002
 * @tc.number: CheckNoRunningCondition_002
 * @tc.desc  : Test CheckNoRunningCondition with non-REMOTE pipe returns false.
 */
HWTEST_F(AudioPipeManagerUnitTest, CheckNoRunningCondition_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(audioPipeManager, nullptr);
    audioPipeManager->curPipeList_.clear();

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = ADAPTER_TYPE_PRIMARY;
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    bool result = audioPipeManager->CheckNoRunningCondition(pipeInfo);
    EXPECT_FALSE(result);

    audioPipeManager->RemoveAudioPipeInfo(pipeInfo);
}

/**
 * @tc.name  : CheckRemoteNoRunningCondition_001
 * @tc.number: CheckRemoteNoRunningCondition_001
 * @tc.desc  : Test CheckRemoteNoRunningCondition returns true when conditions met and isRunning is true.
 */
HWTEST_F(AudioPipeManagerUnitTest, CheckRemoteNoRunningCondition_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(audioPipeManager, nullptr);
    audioPipeManager->curPipeList_.clear();

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = ADAPTER_TYPE_REMOTE;
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    pipeInfo->isRunning_ = true;
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->hdPlaybackMode_ = HdPlaybackMode::APP_LEVEL;
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    auto remoteOffloadPipe = std::make_shared<AudioPipeInfo>();
    remoteOffloadPipe->adapterName_ = ADAPTER_TYPE_REMOTE;
    remoteOffloadPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_LOWPOWER;
    remoteOffloadPipe->AddStream(streamDesc);
    audioPipeManager->AddAudioPipeInfo(remoteOffloadPipe);

    bool result = audioPipeManager->CheckRemoteNoRunningCondition(pipeInfo);
    EXPECT_TRUE(result);

    audioPipeManager->RemoveAudioPipeInfo(pipeInfo);
    audioPipeManager->RemoveAudioPipeInfo(remoteOffloadPipe);
}

/**
 * @tc.name  : CheckRemoteNoRunningCondition_002
 * @tc.number: CheckRemoteNoRunningCondition_002
 * @tc.desc  : Test CheckRemoteNoRunningCondition returns false when remoteOffloadPipe is nullptr.
 */
HWTEST_F(AudioPipeManagerUnitTest, CheckRemoteNoRunningCondition_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(audioPipeManager, nullptr);
    audioPipeManager->curPipeList_.clear();

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = ADAPTER_TYPE_REMOTE;
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    pipeInfo->isRunning_ = true;
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    bool result = audioPipeManager->CheckRemoteNoRunningCondition(pipeInfo);
    EXPECT_FALSE(result);

    audioPipeManager->RemoveAudioPipeInfo(pipeInfo);
}

/**
 * @tc.name  : IsPipeNeedStop_001
 * @tc.number: IsPipeNeedStop_001
 * @tc.desc  : Test IsPipeNeedStop returns true when isRunning is true and sets isRunning to false.
 */
HWTEST_F(AudioPipeManagerUnitTest, IsPipeNeedStop_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(audioPipeManager, nullptr);
    audioPipeManager->curPipeList_.clear();

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->isRunning_ = true;
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    bool result = audioPipeManager->IsPipeNeedStop(pipeInfo);
    EXPECT_TRUE(result);
    EXPECT_FALSE(pipeInfo->isRunning_);

    audioPipeManager->RemoveAudioPipeInfo(pipeInfo);
}

/**
 * @tc.name  : IsPipeNeedStop_002
 * @tc.number: IsPipeNeedStop_002
 * @tc.desc  : Test IsPipeNeedStop returns false when isRunning is false.
 */
HWTEST_F(AudioPipeManagerUnitTest, IsPipeNeedStop_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(audioPipeManager, nullptr);
    audioPipeManager->curPipeList_.clear();

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->isRunning_ = false;
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    bool result = audioPipeManager->IsPipeNeedStop(pipeInfo);
    EXPECT_FALSE(result);

    audioPipeManager->RemoveAudioPipeInfo(pipeInfo);
}

/**
* @tc.name  : Test AudioPipeManager.
* @tc.number: HasRunningNormalCapturerStream_001
* @tc.desc  : Test HasRunningNormalCapturerStream with no streams.
*/
HWTEST_F(AudioPipeManagerUnitTest, HasRunningNormalCapturerStream_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(audioPipeManager, nullptr);
    audioPipeManager->curPipeList_.clear();

    EXPECT_EQ(audioPipeManager->HasRunningNormalCapturerStream(DEVICE_TYPE_NONE), false);
}

/**
* @tc.name  : Test AudioPipeManager.
* @tc.number: HasRunningNormalCapturerStream_002
* @tc.desc  : Test HasRunningNormalCapturerStream with running normal capturer stream.
*/
HWTEST_F(AudioPipeManagerUnitTest, HasRunningNormalCapturerStream_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(audioPipeManager, nullptr);
    audioPipeManager->curPipeList_.clear();

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    auto desc = std::make_shared<AudioStreamDescriptor>();
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->audioMode_ = AUDIO_MODE_RECORD;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    desc->sessionId_ = 123;
    desc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    desc->newDeviceDescs_[0]->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    EXPECT_EQ(audioPipeManager->HasRunningNormalCapturerStream(DEVICE_TYPE_NONE), true);
}

/**
* @tc.name  : Test AudioPipeManager.
* @tc.number: HasRunningNormalCapturerStream_003
* @tc.desc  : Test HasRunningNormalCapturerStream with specific device type matching.
*/
HWTEST_F(AudioPipeManagerUnitTest, HasRunningNormalCapturerStream_003, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(audioPipeManager, nullptr);
    audioPipeManager->curPipeList_.clear();

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    auto desc = std::make_shared<AudioStreamDescriptor>();
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->audioMode_ = AUDIO_MODE_RECORD;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    desc->sessionId_ = 123;
    desc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    desc->newDeviceDescs_[0]->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    EXPECT_EQ(audioPipeManager->HasRunningNormalCapturerStream(DEVICE_TYPE_WIRED_HEADSET), false);
}

/**
* @tc.name  : Test AudioPipeManager.
* @tc.number: HasRunningNormalCapturerStream_004
* @tc.desc  : Test HasRunningNormalCapturerStream with device type matching.
*/
HWTEST_F(AudioPipeManagerUnitTest, HasRunningNormalCapturerStream_004, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(audioPipeManager, nullptr);
    audioPipeManager->curPipeList_.clear();

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    auto desc = std::make_shared<AudioStreamDescriptor>();
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->audioMode_ = AUDIO_MODE_RECORD;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    desc->sessionId_ = 123;
    desc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    desc->newDeviceDescs_[0]->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    EXPECT_EQ(audioPipeManager->HasRunningNormalCapturerStream(DEVICE_TYPE_BLUETOOTH_SCO), true);
}

/**
* @tc.name  : Test AudioPipeManager.
* @tc.number: HasRunningRecognitionCapturerStream_001
* @tc.desc  : Test HasRunningRecognitionCapturerStream with no streams.
*/
HWTEST_F(AudioPipeManagerUnitTest, HasRunningRecognitionCapturerStream_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(audioPipeManager, nullptr);
    audioPipeManager->curPipeList_.clear();

    EXPECT_EQ(audioPipeManager->HasRunningRecognitionCapturerStream(), false);
}

/**
* @tc.name  : Test AudioPipeManager.
* @tc.number: HasRunningRecognitionCapturerStream_002
* @tc.desc  : Test HasRunningRecognitionCapturerStream with running recognition capturer stream.
*/
HWTEST_F(AudioPipeManagerUnitTest, HasRunningRecognitionCapturerStream_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(audioPipeManager, nullptr);
    audioPipeManager->curPipeList_.clear();

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    auto desc = std::make_shared<AudioStreamDescriptor>();
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->audioMode_ = AUDIO_MODE_RECORD;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    desc->sessionId_ = 123;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    EXPECT_EQ(audioPipeManager->HasRunningRecognitionCapturerStream(), true);
}

/**
 * @tc.name: GetVolumeTypeFromStreamDesc_001
 * @tc.desc: Test GetVolumeTypeFromStreamDesc when VOICE_ASSISTANT with non-system app returns STREAM_MUSIC.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetVolumeTypeFromStreamDesc_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_ASSISTANT;
    streamDesc->callerUid_ = 1;  // uid=1 CheckoutSystemApp returns false
    
    AudioVolumeType result = audioPipeManager->GetVolumeTypeFromStreamDesc(streamDesc);
    EXPECT_EQ(result, STREAM_VOICE_ASSISTANT);

    streamDesc->callerUid_ = 1003;
    result = audioPipeManager->GetVolumeTypeFromStreamDesc(streamDesc);
    EXPECT_EQ(result, STREAM_MUSIC);
}

/**
 * @tc.name: GetVolumeTypeFromStreamDesc_002
 * @tc.desc: Test GetVolumeTypeFromStreamDesc with normal streamUsage returns VolumeUtils result.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetVolumeTypeFromStreamDesc_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    
    AudioVolumeType result = audioPipeManager->GetVolumeTypeFromStreamDesc(streamDesc);
    EXPECT_EQ(result, STREAM_MUSIC);
}

/**
 * @tc.name: AddOrUpdateVolumeInfo_001
 * @tc.desc: Test AddOrUpdateVolumeInfo when existing item matches, insert volumeType.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, AddOrUpdateVolumeInfo_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    
    std::vector<PipeDeviceVolumeInfo> result;
    AudioIOHandle ioHandle = 100;
    std::shared_ptr<AudioDeviceDescriptor> device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->macAddress_ = "00:11:22:33:44:55";
    
    audioPipeManager->AddOrUpdateVolumeInfo(result, ioHandle, device, STREAM_MUSIC);
    EXPECT_EQ(result.size(), 1);
    
    audioPipeManager->AddOrUpdateVolumeInfo(result, ioHandle, device, STREAM_VOICE_CALL);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].volumeTypes_.size(), 2);
}

/**
 * @tc.name: AddOrUpdateVolumeInfo_002
 * @tc.desc: Test AddOrUpdateVolumeInfo when no match, creates new PipeDeviceVolumeInfo.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, AddOrUpdateVolumeInfo_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    
    std::vector<PipeDeviceVolumeInfo> result;
    AudioIOHandle ioHandle1 = 100;
    AudioIOHandle ioHandle2 = 200;
    std::shared_ptr<AudioDeviceDescriptor> device1 = std::make_shared<AudioDeviceDescriptor>();
    device1->deviceType_ = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<AudioDeviceDescriptor> device2 = std::make_shared<AudioDeviceDescriptor>();
    device2->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    
    audioPipeManager->AddOrUpdateVolumeInfo(result, ioHandle1, device1, STREAM_MUSIC);
    EXPECT_EQ(result.size(), 1);

    audioPipeManager->AddOrUpdateVolumeInfo(result, ioHandle2, device1, STREAM_MUSIC);
    EXPECT_EQ(result.size(), 2);
    
    audioPipeManager->AddOrUpdateVolumeInfo(result, ioHandle2, device2, STREAM_VOICE_CALL);
    EXPECT_EQ(result.size(), 3);
}

/**
 * @tc.name: ProcessPipeForVolumeInfo_001
 * @tc.desc: Test ProcessPipeForVolumeInfo when pipeInfo is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, ProcessPipeForVolumeInfo_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    
    std::shared_ptr<AudioPipeInfo> pipeInfo = nullptr;
    auto result = audioPipeManager->ProcessPipeForVolumeInfo(pipeInfo);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: ProcessPipeForVolumeInfo_002
 * @tc.desc: Test ProcessPipeForVolumeInfo when streamDescriptors is empty.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, ProcessPipeForVolumeInfo_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->streamDescriptors_.clear();
    
    auto result = audioPipeManager->ProcessPipeForVolumeInfo(pipeInfo);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: ProcessPipeForVolumeInfo_003
 * @tc.desc: Test ProcessPipeForVolumeInfo when streamDesc is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, ProcessPipeForVolumeInfo_003, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->streamDescriptors_.push_back(nullptr);
    
    auto result = audioPipeManager->ProcessPipeForVolumeInfo(pipeInfo);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: ProcessPipeForVolumeInfo_004
 * @tc.desc: Test ProcessPipeForVolumeInfo when newDeviceDescs is empty.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, ProcessPipeForVolumeInfo_004, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.clear();
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    
    auto result = audioPipeManager->ProcessPipeForVolumeInfo(pipeInfo);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: ProcessPipeForVolumeInfo_005
 * @tc.desc: Test ProcessPipeForVolumeInfo when firstDevice is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, ProcessPipeForVolumeInfo_005, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.push_back(nullptr);
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    
    auto result = audioPipeManager->ProcessPipeForVolumeInfo(pipeInfo);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: ProcessPipeForVolumeInfo_006
 * @tc.desc: Test ProcessPipeForVolumeInfo with valid single device.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, ProcessPipeForVolumeInfo_006, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = 100;
    
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 1;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    
    std::shared_ptr<AudioDeviceDescriptor> device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.push_back(device);
    
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    
    auto result = audioPipeManager->ProcessPipeForVolumeInfo(pipeInfo);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].ioHandle_, 100);
    EXPECT_TRUE(result[0].volumeTypes_.count(STREAM_MUSIC) > 0);
}

/**
 * @tc.name: ProcessPipeForVolumeInfo_007
 * @tc.desc: Test ProcessPipeForVolumeInfo with multiple devices, one is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, ProcessPipeForVolumeInfo_007, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = 100;
    
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    
    std::shared_ptr<AudioDeviceDescriptor> device1 = std::make_shared<AudioDeviceDescriptor>();
    device1->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.push_back(device1);
    streamDesc->newDeviceDescs_.push_back(nullptr);
    
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    
    auto result = audioPipeManager->ProcessPipeForVolumeInfo(pipeInfo);
    EXPECT_EQ(result.size(), 1);
}

/**
 * @tc.name: ProcessPipeForVolumeInfo_008
 * @tc.desc: Test ProcessPipeForVolumeInfo when multiple streams same pipeId+device merges volumeTypes.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, ProcessPipeForVolumeInfo_008, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = 100;
    
    std::shared_ptr<AudioDeviceDescriptor> device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    streamDesc1->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    streamDesc1->newDeviceDescs_.push_back(device);
    
    std::shared_ptr<AudioStreamDescriptor> streamDesc2 = std::make_shared<AudioStreamDescriptor>();
    streamDesc2->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    streamDesc2->newDeviceDescs_.push_back(device);
    
    pipeInfo->streamDescriptors_.push_back(streamDesc1);
    pipeInfo->streamDescriptors_.push_back(streamDesc2);
    
    auto result = audioPipeManager->ProcessPipeForVolumeInfo(pipeInfo);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].volumeTypes_.size(), 2);
}

/**
 * @tc.name: GetAllPipeDeviceVolumeInfo_001
 * @tc.desc: Test GetAllPipeDeviceVolumeInfo when pipeInfo is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetAllPipeDeviceVolumeInfo_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    
    audioPipeManager->curPipeList_.push_back(nullptr);
    
    auto result = audioPipeManager->GetAllPipeDeviceVolumeInfo();
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetAllPipeDeviceVolumeInfo_002
 * @tc.desc: Test GetAllPipeDeviceVolumeInfo when pipeRole is not OUTPUT.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetAllPipeDeviceVolumeInfo_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    audioPipeManager->curPipeList_.push_back(pipeInfo);
    
    auto result = audioPipeManager->GetAllPipeDeviceVolumeInfo();
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetAllPipeDeviceVolumeInfo_003
 * @tc.desc: Test GetAllPipeDeviceVolumeInfo with valid output pipes.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetAllPipeDeviceVolumeInfo_003, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = 100;
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    
    std::shared_ptr<AudioDeviceDescriptor> device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.push_back(device);
    
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    audioPipeManager->curPipeList_.push_back(pipeInfo);
    
    auto result = audioPipeManager->GetAllPipeDeviceVolumeInfo();
    EXPECT_EQ(result.size(), 1);
}

/**
 * @tc.name: GetPipeDeviceVolumeInfoForPipe_001
 * @tc.desc: Test GetPipeDeviceVolumeInfoForPipe when pipeInfo is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPipeDeviceVolumeInfoForPipe_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    
    audioPipeManager->curPipeList_.push_back(nullptr);
    
    AudioIOHandle targetId = 100;
    auto result = audioPipeManager->GetPipeDeviceVolumeInfoForPipe(targetId);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetPipeDeviceVolumeInfoForPipe_002
 * @tc.desc: Test GetPipeDeviceVolumeInfoForPipe when id does not match.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPipeDeviceVolumeInfoForPipe_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = 200;
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    audioPipeManager->curPipeList_.push_back(pipeInfo);
    
    AudioIOHandle targetId = 100;
    auto result = audioPipeManager->GetPipeDeviceVolumeInfoForPipe(targetId);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetPipeDeviceVolumeInfoForPipe_003
 * @tc.desc: Test GetPipeDeviceVolumeInfoForPipe when pipeRole is not OUTPUT.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPipeDeviceVolumeInfoForPipe_003, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = 100;
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    audioPipeManager->curPipeList_.push_back(pipeInfo);
    
    AudioIOHandle targetId = 100;
    auto result = audioPipeManager->GetPipeDeviceVolumeInfoForPipe(targetId);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetPipeDeviceVolumeInfoForPipe_004
 * @tc.desc: Test GetPipeDeviceVolumeInfoForPipe when matching pipe found.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPipeDeviceVolumeInfoForPipe_004, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = 100;
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    
    std::shared_ptr<AudioDeviceDescriptor> device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.push_back(device);
    
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    audioPipeManager->curPipeList_.push_back(pipeInfo);
    
    AudioIOHandle targetId = 100;
    auto result = audioPipeManager->GetPipeDeviceVolumeInfoForPipe(targetId);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].ioHandle_, 100);
}

/**
 * @tc.name: GetPipeDeviceVolumeInfoForPipe_005
 * @tc.desc: Test GetPipeDeviceVolumeInfoForPipe when no matching pipe.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPipeDeviceVolumeInfoForPipe_005, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    
    AudioIOHandle targetId = 999;
    auto result = audioPipeManager->GetPipeDeviceVolumeInfoForPipe(targetId);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetPipeDeviceVolumeInfoForDevice_001
 * @tc.desc: Test GetPipeDeviceVolumeInfoForDevice when deviceDesc is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPipeDeviceVolumeInfoForDevice_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = nullptr;
    auto result = audioPipeManager->GetPipeDeviceVolumeInfoForDevice(deviceDesc);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetPipeDeviceVolumeInfoForDevice_002
 * @tc.desc: Test GetPipeDeviceVolumeInfoForDevice when pipeInfo is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPipeDeviceVolumeInfoForDevice_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    
    audioPipeManager->curPipeList_.push_back(nullptr);
    
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    auto result = audioPipeManager->GetPipeDeviceVolumeInfoForDevice(deviceDesc);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetPipeDeviceVolumeInfoForDevice_003
 * @tc.desc: Test GetPipeDeviceVolumeInfoForDevice when pipeRole is not OUTPUT.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPipeDeviceVolumeInfoForDevice_003, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    audioPipeManager->curPipeList_.push_back(pipeInfo);
    
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    auto result = audioPipeManager->GetPipeDeviceVolumeInfoForDevice(deviceDesc);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetPipeDeviceVolumeInfoForDevice_004
 * @tc.desc: Test GetPipeDeviceVolumeInfoForDevice when streamDescriptors is empty.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPipeDeviceVolumeInfoForDevice_004, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipeInfo->streamDescriptors_.clear();
    audioPipeManager->curPipeList_.push_back(pipeInfo);
    
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    auto result = audioPipeManager->GetPipeDeviceVolumeInfoForDevice(deviceDesc);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetPipeDeviceVolumeInfoForDevice_005
 * @tc.desc: Test GetPipeDeviceVolumeInfoForDevice when streamDesc is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPipeDeviceVolumeInfoForDevice_005, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipeInfo->streamDescriptors_.push_back(nullptr);
    audioPipeManager->curPipeList_.push_back(pipeInfo);
    
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    auto result = audioPipeManager->GetPipeDeviceVolumeInfoForDevice(deviceDesc);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetPipeDeviceVolumeInfoForDevice_006
 * @tc.desc: Test GetPipeDeviceVolumeInfoForDevice when newDeviceDescs is empty.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPipeDeviceVolumeInfoForDevice_006, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.clear();
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    
    audioPipeManager->curPipeList_.push_back(pipeInfo);
    
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    auto result = audioPipeManager->GetPipeDeviceVolumeInfoForDevice(deviceDesc);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetPipeDeviceVolumeInfoForDevice_007
 * @tc.desc: Test GetPipeDeviceVolumeInfoForDevice when firstDevice is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPipeDeviceVolumeInfoForDevice_007, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.push_back(nullptr);
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    
    audioPipeManager->curPipeList_.push_back(pipeInfo);
    
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    auto result = audioPipeManager->GetPipeDeviceVolumeInfoForDevice(deviceDesc);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetPipeDeviceVolumeInfoForDevice_008
 * @tc.desc: Test GetPipeDeviceVolumeInfoForDevice when device matches.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPipeDeviceVolumeInfoForDevice_008, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = 100;
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    
    std::shared_ptr<AudioDeviceDescriptor> device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    device->macAddress_ = "00:11:22:33:44:55";
    streamDesc->newDeviceDescs_.push_back(device);
    
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    audioPipeManager->curPipeList_.push_back(pipeInfo);
    
    std::shared_ptr<AudioDeviceDescriptor> queryDevice = std::make_shared<AudioDeviceDescriptor>();
    queryDevice->deviceType_ = DEVICE_TYPE_SPEAKER;
    queryDevice->macAddress_ = "00:11:22:33:44:55";
    
    auto result = audioPipeManager->GetPipeDeviceVolumeInfoForDevice(queryDevice);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].ioHandle_, 100);
}

/**
 * @tc.name: GetPipeDeviceVolumeInfoForDevice_009
 * @tc.desc: Test GetPipeDeviceVolumeInfoForDevice when device does not match.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, GetPipeDeviceVolumeInfoForDevice_009, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = 100;
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    
    std::shared_ptr<AudioDeviceDescriptor> device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.push_back(device);
    
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    audioPipeManager->curPipeList_.push_back(pipeInfo);
    
    std::shared_ptr<AudioDeviceDescriptor> queryDevice = std::make_shared<AudioDeviceDescriptor>();
    queryDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    
    auto result = audioPipeManager->GetPipeDeviceVolumeInfoForDevice(queryDevice);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: QueryPipeIdBySessionId_001
 * @tc.desc: Test QueryPipeIdBySessionId when sessionId not found and pipeList is empty.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, QueryPipeIdBySessionId_001, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();

    uint32_t targetSessionId = 123456;
    uint32_t pipeId = audioPipeManager->QueryPipeIdBySessionId(targetSessionId);
    // IS_DEVICE_ENHANCED_SUPPORTED is compile-time constant (default false)
    // When disabled: expect DEFAULT_PIPE_ID (0)
    EXPECT_EQ(pipeId, DEFAULT_PIPE_ID);
}

/**
 * @tc.name: QueryPipeIdBySessionId_002
 * @tc.desc: Test QueryPipeIdBySessionId when sessionId exists in pipeList.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, QueryPipeIdBySessionId_002, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();

    uint32_t targetSessionId = 123456;
    uint32_t expectedPipeId = 100;

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = expectedPipeId;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = targetSessionId;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    uint32_t pipeId = audioPipeManager->QueryPipeIdBySessionId(targetSessionId);
    EXPECT_EQ(pipeId, expectedPipeId);
}

/**
 * @tc.name: QueryPipeIdBySessionId_003
 * @tc.desc: Test QueryPipeIdBySessionId when sessionId not found but pipeList has other sessions.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, QueryPipeIdBySessionId_003, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();

    uint32_t existingSessionId = 111;
    uint32_t targetSessionId = 222;

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = 50;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = existingSessionId;
    pipeInfo->streamDescriptors_.push_back(desc);
    audioPipeManager->AddAudioPipeInfo(pipeInfo);

    uint32_t pipeId = audioPipeManager->QueryPipeIdBySessionId(targetSessionId);
    // sessionId not found, return DEFAULT_PIPE_ID when multi-device disabled
    EXPECT_EQ(pipeId, DEFAULT_PIPE_ID);
}

/**
 * @tc.name: QueryPipeIdBySessionId_004
 * @tc.desc: Test QueryPipeIdBySessionId with multiple pipeInfos containing different sessions.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, QueryPipeIdBySessionId_004, TestSize.Level1)
{
    auto audioPipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();

    uint32_t sessionId1 = 100;
    uint32_t sessionId2 = 200;
    uint32_t pipeId1 = 10;
    uint32_t pipeId2 = 20;

    std::shared_ptr<AudioPipeInfo> pipeInfo1 = std::make_shared<AudioPipeInfo>();
    pipeInfo1->id_ = pipeId1;
    std::shared_ptr<AudioStreamDescriptor> desc1 = std::make_shared<AudioStreamDescriptor>();
    desc1->sessionId_ = sessionId1;
    pipeInfo1->streamDescriptors_.push_back(desc1);
    audioPipeManager->AddAudioPipeInfo(pipeInfo1);

    std::shared_ptr<AudioPipeInfo> pipeInfo2 = std::make_shared<AudioPipeInfo>();
    pipeInfo2->id_ = pipeId2;
    std::shared_ptr<AudioStreamDescriptor> desc2 = std::make_shared<AudioStreamDescriptor>();
    desc2->sessionId_ = sessionId2;
    pipeInfo2->streamDescriptors_.push_back(desc2);
    audioPipeManager->AddAudioPipeInfo(pipeInfo2);

    EXPECT_EQ(audioPipeManager->QueryPipeIdBySessionId(sessionId1), pipeId1);
    EXPECT_EQ(audioPipeManager->QueryPipeIdBySessionId(sessionId2), pipeId2);
}
} // namespace AudioStandard
} // namespace OHOS
