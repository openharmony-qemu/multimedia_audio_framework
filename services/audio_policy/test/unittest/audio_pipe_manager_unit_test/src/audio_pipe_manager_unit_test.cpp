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
#include "audio_stream_descriptor.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioPipeManagerUnitTest::SetUpTestCase(void) {}
void AudioPipeManagerUnitTest::TearDownTestCase(void) {}
void AudioPipeManagerUnitTest::SetUp(void) {}
void AudioPipeManagerUnitTest::TearDown(void) {}

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
 * @tc.name: GetAdapterNameBySessionId_001
 * @tc.desc: Test GetAdapterNameBySessionId when finding a matching sessionId.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetAdapterNameBySessionId_001, TestSize.Level1)
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

    std::string result = audioPipeManager->GetAdapterNameBySessionId(targetSessionId);
    EXPECT_EQ(result, "TestAdapter");
}

/**
 * @tc.name: GetAdapterNameBySessionId_002
 * @tc.desc: Test GetAdapterNameBySessionId when not finding a matching sessionId.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioPipeManagerUnitTest, GetAdapterNameBySessionId_002, TestSize.Level1)
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

    std::string result = audioPipeManager->GetAdapterNameBySessionId(targetSessionId);
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
    EXPECT_NE(nullptr, audioPipeManager);
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
    EXPECT_NE(nullptr, audioPipeManager);
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
    EXPECT_NE(nullptr, audioPipeManager);
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
    EXPECT_NE(nullptr, audioPipeManager);
    audioPipeManager->curPipeList_.clear();
    auto pipeList01 = audioPipeManager->GetPipeList();
    EXPECT_EQ(pipeList01.size(), 0);

    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->adapterName_ = "aaa";
    pipe1->routeFlag_ = AUDIO_INPUT_FLAG_VOIP_FAST;
    pipe1->id_ = 123;

    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_VOIP_FAST;
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
    EXPECT_NE(nullptr, audioPipeManager);
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
} // namespace AudioStandard
} // namespace OHOS