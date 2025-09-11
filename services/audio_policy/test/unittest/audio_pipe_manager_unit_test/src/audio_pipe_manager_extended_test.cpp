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

#include "audio_pipe_manager_unit_test.h"
#include "audio_policy_utils.h"
#include "audio_stream_descriptor.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static std::shared_ptr<AudioPipeManager> sPipeManager_ = nullptr;

void AudioPipeManagerExtendedUnitTest::SetUpTestCase(void) {}
void AudioPipeManagerExtendedUnitTest::TearDownTestCase(void) {}
void AudioPipeManagerExtendedUnitTest::SetUp(void)
{
    sPipeManager_ = std::make_shared<AudioPipeManager>();
}
void AudioPipeManagerExtendedUnitTest::TearDown(void)
{
    sPipeManager_ = nullptr;
}

/**
 * @tc.name: AudioPipeManager_001
 * @tc.desc: Test AudioPipeManager RemoveAudioPipeInfo.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerExtendedUnitTest, AudioPipeManager_001, TestSize.Level4)
{
    auto pipeInfo1 = std::make_shared<AudioPipeInfo>();
    pipeInfo1->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    pipeInfo1->id_ = 1;
    auto pipeInfo2 = std::make_shared<AudioPipeInfo>();
    pipeInfo2->routeFlag_ = AUDIO_INPUT_FLAG_FAST;
    pipeInfo2->id_ = 2;
    sPipeManager_->AddAudioPipeInfo(pipeInfo1);
    sPipeManager_->RemoveAudioPipeInfo(pipeInfo2);
    EXPECT_FALSE(sPipeManager_->GetPipeList().empty());
}

/**
 * @tc.name: AudioPipeManager_002
 * @tc.desc: Test AudioPipeManager GetUnusedRecordPipe.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerExtendedUnitTest, AudioPipeManager_002, TestSize.Level4)
{
    auto pipeInfo1 = std::make_shared<AudioPipeInfo>();
    pipeInfo1->pipeRole_ = PIPE_ROLE_INPUT;
    pipeInfo1->adapterName_ = PRIMARY_CLASS;
    pipeInfo1->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    sPipeManager_->AddAudioPipeInfo(pipeInfo1);
    auto pipeInfo2 = std::make_shared<AudioPipeInfo>(pipeInfo1);
    pipeInfo2->adapterName_ = "test_adapter";
    sPipeManager_->AddAudioPipeInfo(pipeInfo2);
    auto pipeInfo3 = std::make_shared<AudioPipeInfo>(pipeInfo2);
    pipeInfo3->streamDescriptors_.emplace_back(std::make_shared<AudioStreamDescriptor>());
    sPipeManager_->AddAudioPipeInfo(pipeInfo3);
    auto pipeInfo4 = std::make_shared<AudioPipeInfo>(pipeInfo1);
    pipeInfo4->streamDescriptors_.emplace_back(std::make_shared<AudioStreamDescriptor>());
    sPipeManager_->AddAudioPipeInfo(pipeInfo4);

    auto pipeInfo5 = std::make_shared<AudioPipeInfo>();
    pipeInfo5->pipeRole_ = PIPE_ROLE_NONE;
    pipeInfo5->streamDescriptors_.emplace_back(std::make_shared<AudioStreamDescriptor>());
    pipeInfo5->adapterName_ = "test_adapter";
    sPipeManager_->AddAudioPipeInfo(pipeInfo5);
    auto pipeInfo6 = std::make_shared<AudioPipeInfo>(pipeInfo5);
    pipeInfo6->streamDescriptors_.clear();
    sPipeManager_->AddAudioPipeInfo(pipeInfo6);
    auto pipeInfo7 = std::make_shared<AudioPipeInfo>(pipeInfo5);
    pipeInfo7->adapterName_ = PRIMARY_CLASS;
    pipeInfo7->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    sPipeManager_->AddAudioPipeInfo(pipeInfo7);
    auto pipeInfo8 = std::make_shared<AudioPipeInfo>(pipeInfo7);
    pipeInfo8->streamDescriptors_.clear();
    sPipeManager_->AddAudioPipeInfo(pipeInfo8);

    EXPECT_EQ(sPipeManager_->GetUnusedRecordPipe().size(), 1);
}

/**
 * @tc.name: AudioPipeManager_003
 * @tc.desc: Test AudioPipeManager IsNormalRecordPipe.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerExtendedUnitTest, AudioPipeManager_003, TestSize.Level4)
{
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = PRIMARY_CLASS;
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    EXPECT_TRUE(sPipeManager_->IsNormalRecordPipe(pipeInfo));

    pipeInfo->adapterName_ = USB_CLASS;
    EXPECT_TRUE(sPipeManager_->IsNormalRecordPipe(pipeInfo));

    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_FAST;
    EXPECT_FALSE(sPipeManager_->IsNormalRecordPipe(pipeInfo));
}

/**
 * @tc.name: AudioPipeManager_004
 * @tc.desc: Test AudioPipeManager GetStreamDescsByIoHandle.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerExtendedUnitTest, AudioPipeManager_004, TestSize.Level4)
{
    AudioIOHandle id = 1;
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = id;
    pipeInfo->streamDescriptors_.emplace_back(std::make_shared<AudioStreamDescriptor>());
    sPipeManager_->AddAudioPipeInfo(pipeInfo);
    EXPECT_FALSE(sPipeManager_->GetStreamDescsByIoHandle(id).empty());
}

/**
 * @tc.name: AudioPipeManager_005
 * @tc.desc: Test AudioPipeManager Dump.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerExtendedUnitTest, AudioPipeManager_005, TestSize.Level4)
{
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "test_adapterName";
    pipeInfo->moduleInfo_.rate = "test_rate";
    pipeInfo->moduleInfo_.channels = "test_channels";
    pipeInfo->moduleInfo_.format = "test_format";
    pipeInfo->moduleInfo_.bufferSize = "test_bufferSize";
    pipeInfo->moduleInfo_.renderInIdleState = "test_renderInIdleState";
    pipeInfo->moduleInfo_.sourceType = "test_sourceType";
    sPipeManager_->AddAudioPipeInfo(pipeInfo);
    sPipeManager_->curPipeList_.push_back(nullptr);
    EXPECT_EQ(sPipeManager_->curPipeList_.size(), 2);
    std::string dumpString = "test_dumpString";
    EXPECT_NO_THROW(sPipeManager_->Dump(dumpString));
}

/**
 * @tc.name: AudioPipeManager_006
 * @tc.desc: Test AudioPipeManager GetModemCommunicationStreamDescById.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerExtendedUnitTest, AudioPipeManager_006, TestSize.Level4)
{
    uint32_t sessionId = 1;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    sPipeManager_->AddModemCommunicationId(sessionId, streamDesc);
    EXPECT_TRUE(sPipeManager_->GetModemCommunicationStreamDescById(sessionId) != nullptr);

    sessionId = 2;
    EXPECT_TRUE(sPipeManager_->GetModemCommunicationStreamDescById(sessionId) == nullptr);
}

/**
 * @tc.name: AudioPipeManager_007
 * @tc.desc: Test AudioPipeManager GetNormalSourceInfo.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerExtendedUnitTest, AudioPipeManager_007, TestSize.Level4)
{
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->moduleInfo_.name = PRIMARY_CLASS;
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    bool isEcFeatureEnable = true;
    EXPECT_TRUE(sPipeManager_->GetNormalSourceInfo(isEcFeatureEnable) == nullptr);
    isEcFeatureEnable = false;
    EXPECT_TRUE(sPipeManager_->GetNormalSourceInfo(isEcFeatureEnable) == nullptr);
}

/**
 * @tc.name: AudioPipeManager_008
 * @tc.desc: Test AudioPipeManager GetStreamIdsByUidAndPid.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerExtendedUnitTest, AudioPipeManager_008, TestSize.Level4)
{
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->moduleInfo_.name = PRIMARY_CLASS;
    auto streamDescriptor1 = std::make_shared<AudioStreamDescriptor>();
    streamDescriptor1->callerUid_ = 1;
    streamDescriptor1->callerPid_ = 1;
    auto streamDescriptor2 = std::make_shared<AudioStreamDescriptor>();
    streamDescriptor2->callerUid_ = 2;
    streamDescriptor2->callerPid_ = 2;
    pipeInfo->streamDescriptors_.push_back(streamDescriptor1);
    pipeInfo->streamDescriptors_.push_back(streamDescriptor2);
    sPipeManager_->AddAudioPipeInfo(pipeInfo);

    int32_t uid = 1;
    int32_t pid = 1;
    EXPECT_EQ(sPipeManager_->GetStreamIdsByUidAndPid(uid, pid).size(), 1);
}

/**
 * @tc.name: AudioPipeManager_009
 * @tc.desc: Test AudioPipeManager UpdateOutputStreamDescsByIoHandle.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerExtendedUnitTest, AudioPipeManager_009, TestSize.Level4)
{
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = 1;
    sPipeManager_->AddAudioPipeInfo(pipeInfo);
    AudioIOHandle id = 2;
    std::vector<std::shared_ptr<AudioStreamDescriptor>> descs;
    descs.push_back(std::make_shared<AudioStreamDescriptor>());
    sPipeManager_->UpdateOutputStreamDescsByIoHandle(id, descs);
    EXPECT_TRUE(pipeInfo->streamDescriptors_.empty());
}
} // namespace AudioStandard
} // namespace OHOS