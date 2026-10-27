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
#include "audio_injector_policy_unit_test.h"
#include "audio_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioInjectorPolicyUnitTest::SetUpTestCase(void) {}
void AudioInjectorPolicyUnitTest::TearDownTestCase(void) {}
void AudioInjectorPolicyUnitTest::SetUp(void) {}
void AudioInjectorPolicyUnitTest::TearDown(void) {}

/**
 * @tc.name: Init_001
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, Init_001, TestSize.Level1)
{
    AudioInjectorPolicy::GetInstance().isOpened_ = false;
    AudioInjectorPolicy::GetInstance().Init();
    AudioInjectorPolicy::GetInstance().isOpened_ = true;
    int32_t ret = AudioInjectorPolicy::GetInstance().Init();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: DeInit_001
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, DeInit_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isOpened_ = false;
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.DeInit();
    audioInjectorPolicy.isOpened_ = false;
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.rendererStreamMap_[1111] = streamDesc1;
    int32_t ret = audioInjectorPolicy.DeInit();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: DeInit_002
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, DeInit_002, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isOpened_ = true;
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.DeInit();
    audioInjectorPolicy.isOpened_ = true;
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.rendererStreamMap_[1111] = streamDesc1;
    int32_t ret = audioInjectorPolicy.DeInit();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: HasRunningVoipStream_001
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, HasRunningVoipStream_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamVec = {};
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    EXPECT_NE(nullptr, desc);
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    streamVec.push_back(desc);
    bool ret = audioInjectorPolicy.HasRunningVoipStream(streamVec);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: HasRunningVoipStream_002
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, HasRunningVoipStream_002, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamVec = {};
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    EXPECT_NE(nullptr, desc);
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_CALL;
    streamVec.push_back(desc);
    bool ret = audioInjectorPolicy.HasRunningVoipStream(streamVec);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: HasRunningVoipStream_003
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, HasRunningVoipStream_003, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamVec = {};
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    EXPECT_NE(nullptr, desc);
    desc->streamStatus_ = STREAM_STATUS_STOPPED;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    streamVec.push_back(desc);
    bool ret = audioInjectorPolicy.HasRunningVoipStream(streamVec);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: HasRunningVoipStream_004
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, HasRunningVoipStream_004, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamVec = {};
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    EXPECT_NE(nullptr, desc);
    desc->streamStatus_ = STREAM_STATUS_STOPPED;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_CALL;
    streamVec.push_back(desc);
    bool ret = audioInjectorPolicy.HasRunningVoipStream(streamVec);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: IsContainStream_001
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, IsContainStream_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.rendererStreamMap_.clear();
    bool ret = audioInjectorPolicy.IsContainStream(1111);
    EXPECT_EQ(false, ret);
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    audioInjectorPolicy.rendererStreamMap_[1111] = streamDesc1;
    ret = audioInjectorPolicy.IsContainStream(1111);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: AddCaptureInjector_001
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, AddCaptureInjector_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.isConnected_ = false;
    audioInjectorPolicy.voipType_ = NO_VOIP;
    audioInjectorPolicy.AddCaptureInjector();

    audioInjectorPolicy.isConnected_ = true;
    int32_t ret = audioInjectorPolicy.AddCaptureInjector();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: AddCaptureInjectorInner_001
 * @tc.desc: Test AddCaptureInjectorInner when enable_new_route is enabled
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, AddCaptureInjectorInner_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    SetSysPara("persist.multimedia.audio.enable_new_route", 1);

    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.isConnected_ = false;
    audioInjectorPolicy.voipType_ = NO_VOIP;
    audioInjectorPolicy.renderPortIdx_ = 1111;
    audioInjectorPolicy.capturePortIdx_ = 2222;

    int32_t ret = audioInjectorPolicy.AddCaptureInjectorInner();
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(true, audioInjectorPolicy.isConnected_);

    audioInjectorPolicy.isConnected_ = false;
    audioInjectorPolicy.capturePortIdx_ = HDI_INVALID_ID;
    audioInjectorPolicy.voipType_ = NO_VOIP;
    SetSysPara("persist.multimedia.audio.enable_new_route", 0);
}

/**
 * @tc.name: RemoveCaptureInjector_001
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RemoveCaptureInjector_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isConnected_ = false;
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.RemoveCaptureInjector(false);
    audioInjectorPolicy.isConnected_ = false;
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.rendererStreamMap_[1111] = streamDesc1;
    int32_t ret = audioInjectorPolicy.RemoveCaptureInjector(false);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: RemoveCaptureInjector_002
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RemoveCaptureInjector_002, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isConnected_ = true;
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.RemoveCaptureInjector(false);
    audioInjectorPolicy.isConnected_ = true;
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.rendererStreamMap_[1111] = streamDesc1;
    int32_t ret = audioInjectorPolicy.RemoveCaptureInjector(false);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: RemoveCaptureInjector_003
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RemoveCaptureInjector_003, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isConnected_ = false;
    audioInjectorPolicy.isOpened_ = false;
    int32_t ret = audioInjectorPolicy.RemoveCaptureInjector(true);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: RemoveCaptureInjector_004
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RemoveCaptureInjector_004, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isConnected_ = false;
    audioInjectorPolicy.isOpened_ = false;
    int32_t ret = audioInjectorPolicy.RemoveCaptureInjector(false);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: RemoveCaptureInjectorInner_001
 * @tc.desc: Test RemoveCaptureInjectorInner when enable_new_route is enabled
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RemoveCaptureInjectorInner_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    SetSysPara("persist.multimedia.audio.enable_new_route", 1);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.rendererStreamMap_[1111] = streamDesc;
    audioInjectorPolicy.isConnected_ = true;
    audioInjectorPolicy.voipType_ = NO_VOIP;
    audioInjectorPolicy.renderPortIdx_ = 2222;
    audioInjectorPolicy.capturePortIdx_ = 3333;

    int32_t ret = audioInjectorPolicy.RemoveCaptureInjectorInner(true);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(false, audioInjectorPolicy.isConnected_);
    EXPECT_EQ(HDI_INVALID_ID, audioInjectorPolicy.capturePortIdx_);
    EXPECT_EQ(NO_VOIP, audioInjectorPolicy.voipType_);

    audioInjectorPolicy.rendererStreamMap_.clear();
    SetSysPara("persist.multimedia.audio.enable_new_route", 0);
}

/**
 * @tc.name: RemoveCaptureInjectorInner_002
 * @tc.desc: Test RemoveCaptureInjectorInner when enable_new_route is disabled
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RemoveCaptureInjectorInner_002, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    SetSysPara("persist.multimedia.audio.enable_new_route", 0);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.rendererStreamMap_[1111] = streamDesc;
    audioInjectorPolicy.isConnected_ = true;
    audioInjectorPolicy.voipType_ = NO_VOIP;
    audioInjectorPolicy.renderPortIdx_ = 2222;
    audioInjectorPolicy.capturePortIdx_ = 3333;

    int32_t ret = audioInjectorPolicy.RemoveCaptureInjectorInner(true);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(true, audioInjectorPolicy.isConnected_);
    EXPECT_EQ(3333, audioInjectorPolicy.capturePortIdx_);
    EXPECT_EQ(NO_VOIP, audioInjectorPolicy.voipType_);

    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.isConnected_ = false;
    audioInjectorPolicy.capturePortIdx_ = HDI_INVALID_ID;
}

/**
 * @tc.name: RebuildCaptureInjector_002
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RebuildCaptureInjector_002, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isOpened_ = true;
    audioInjectorPolicy.RebuildCaptureInjector(1111);
    audioInjectorPolicy.isOpened_ = false;
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 4321;
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    pipe1->streamDescriptors_.push_back(desc);
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    audioInjectorPolicy.pipeManager_->AddAudioPipeInfo(pipe1);
    audioInjectorPolicy.capturePortIdx_ = 1234;
    audioInjectorPolicy.RebuildCaptureInjector(4321);
}

/**
 * @tc.name: RebuildCaptureInjector_003
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RebuildCaptureInjector_003, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isOpened_ = true;
    audioInjectorPolicy.RebuildCaptureInjector(1111);
    audioInjectorPolicy.isOpened_ = false;
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 4321;
    desc->streamStatus_ = STREAM_STATUS_STOPPED;
    pipe1->streamDescriptors_.push_back(desc);
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    audioInjectorPolicy.pipeManager_->AddAudioPipeInfo(pipe1);
    audioInjectorPolicy.capturePortIdx_ = 1234;
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.RebuildCaptureInjector(4321);
}

/**
 * @tc.name: RebuildCaptureInjector_004
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RebuildCaptureInjector_004, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isOpened_ = true;
    audioInjectorPolicy.RebuildCaptureInjector(1111);
    audioInjectorPolicy.isOpened_ = false;
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 4321;
    desc->streamStatus_ = STREAM_STATUS_STOPPED;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    pipe1->streamDescriptors_.push_back(desc);
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    audioInjectorPolicy.pipeManager_->AddAudioPipeInfo(pipe1);
    audioInjectorPolicy.capturePortIdx_ = 1234;
    std::shared_ptr<AudioStreamDescriptor> stream = std::make_shared<AudioStreamDescriptor>();
    audioInjectorPolicy.rendererStreamMap_[6789] = stream;
    audioInjectorPolicy.RebuildCaptureInjector(4321);
}

/**
 * @tc.name: RebuildCaptureInjector_006
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RebuildCaptureInjector_006, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isOpened_ = true;
    audioInjectorPolicy.RebuildCaptureInjector(1111);
    audioInjectorPolicy.isOpened_ = false;
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 4321;
    desc->streamStatus_ = STREAM_STATUS_STOPPED;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_VOIP;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    pipe1->streamDescriptors_.push_back(desc);
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    audioInjectorPolicy.pipeManager_->AddAudioPipeInfo(pipe1);
    audioInjectorPolicy.capturePortIdx_ = 1234;
    std::shared_ptr<AudioStreamDescriptor> stream = std::make_shared<AudioStreamDescriptor>();
    audioInjectorPolicy.rendererStreamMap_[6789] = stream;
    audioInjectorPolicy.RebuildCaptureInjector(4321);
}

/**
 * @tc.name: RebuildCaptureInjector_005
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RebuildCaptureInjector_005, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isOpened_ = true;
    audioInjectorPolicy.RebuildCaptureInjector(1111);
    audioInjectorPolicy.isOpened_ = false;
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 4321;
    desc->streamStatus_ = STREAM_STATUS_STOPPED;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_CALL;
    pipe1->streamDescriptors_.push_back(desc);
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    audioInjectorPolicy.pipeManager_->AddAudioPipeInfo(pipe1);
    audioInjectorPolicy.capturePortIdx_ = 1234;
    std::shared_ptr<AudioStreamDescriptor> stream = std::make_shared<AudioStreamDescriptor>();
    audioInjectorPolicy.rendererStreamMap_[6789] = stream;
    audioInjectorPolicy.RebuildCaptureInjector(4321);
}

/**
 * @tc.name: FindCaptureVoipPipe_001
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, FindCaptureVoipPipe_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 4321;
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->streamAction_ = AUDIO_STREAM_ACTION_DEFAULT;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    pipe1->streamDescriptors_.push_back(desc);
    pipelist.push_back(pipe1);
    VoipType type = VoipType::NO_VOIP;
    uint32_t wid = 1234;
    audioInjectorPolicy.FindCaptureVoipPipe(pipelist, wid);
}

/**
 * @tc.name: FindCaptureVoipPipe_002
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, FindCaptureVoipPipe_002, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 4321;
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->streamAction_ = AUDIO_STREAM_ACTION_DEFAULT;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_CALL;
    pipe1->streamDescriptors_.push_back(desc);
    pipelist.push_back(pipe1);
    VoipType type = VoipType::NO_VOIP;
    uint32_t wid = 1234;
    audioInjectorPolicy.FindCaptureVoipPipe(pipelist, wid);
}

/**
 * @tc.name: FindCaptureVoipPipe_003
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, FindCaptureVoipPipe_003, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 4321;
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->streamAction_ = AUDIO_STREAM_ACTION_DEFAULT;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_VOIP;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_CALL;
    pipe1->streamDescriptors_.push_back(desc);
    pipelist.push_back(pipe1);
    VoipType type = VoipType::NO_VOIP;
    uint32_t wid = 1234;
    audioInjectorPolicy.FindCaptureVoipPipe(pipelist, wid);
}

/**
 * @tc.name: FindCaptureVoipPipe_004
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, FindCaptureVoipPipe_004, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 4321;
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->streamAction_ = AUDIO_STREAM_ACTION_DEFAULT;
    desc->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_CALL;
    pipe1->streamDescriptors_.push_back(desc);
    pipelist.push_back(pipe1);
    VoipType type = VoipType::NO_VOIP;
    uint32_t wid = 1234;
    audioInjectorPolicy.FindCaptureVoipPipe(pipelist, wid);
}

/**
 * @tc.name: FetchCapDeviceInjectPreProc_001
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, FetchCapDeviceInjectPreProc_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isOpened_ = false;
    bool flag = true;
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    uint32_t wid = 1234;
    audioInjectorPolicy.FetchCapDeviceInjectPreProc(pipelist, flag, wid);

    audioInjectorPolicy.isOpened_ = true;
    pipelist.clear();
    audioInjectorPolicy.FetchCapDeviceInjectPreProc(pipelist, flag, wid);
}

/**
 * @tc.name: FetchCapDeviceInjectPreProc_002
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, FetchCapDeviceInjectPreProc_002, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isOpened_ = false;
    bool flag = true;
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    uint32_t wid = 1234;
    audioInjectorPolicy.FetchCapDeviceInjectPreProc(pipelist, flag, wid);

    audioInjectorPolicy.isOpened_ = true;
    pipelist.clear();
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->streamAction_ = AUDIO_STREAM_ACTION_DEFAULT;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    pipe1->streamDescriptors_.push_back(desc);
    pipelist.push_back(pipe1);
    audioInjectorPolicy.capturePortIdx_ = 1234;
    audioInjectorPolicy.FetchCapDeviceInjectPreProc(pipelist, flag, wid);
}

/**
 * @tc.name: FetchCapDeviceInjectPreProc_003
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, FetchCapDeviceInjectPreProc_003, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    bool flag = true;
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    audioInjectorPolicy.isOpened_ = true;
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->streamAction_ = AUDIO_STREAM_ACTION_DEFAULT;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    pipe1->streamDescriptors_.push_back(desc);
    pipelist.push_back(pipe1);
    audioInjectorPolicy.capturePortIdx_ = 4321;
    uint32_t wid = 1234;
    audioInjectorPolicy.FetchCapDeviceInjectPreProc(pipelist, flag, wid);
}

/**
 * @tc.name: FetchCapDeviceInjectPostProc_001
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, FetchCapDeviceInjectPostProc_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isOpened_ = false;
    bool flag = true;
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    uint32_t wid = 1234;
    audioInjectorPolicy.FetchCapDeviceInjectPostProc(pipelist, flag, wid);

    audioInjectorPolicy.isOpened_ = true;
    pipelist.clear();
    audioInjectorPolicy.FetchCapDeviceInjectPostProc(pipelist, flag, wid);
}

/**
 * @tc.name: FetchCapDeviceInjectPostProc_002
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, FetchCapDeviceInjectPostProc_002, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    bool flag = true;
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    audioInjectorPolicy.isOpened_ = true;
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->streamAction_ = AUDIO_STREAM_ACTION_DEFAULT;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    pipe1->streamDescriptors_.push_back(desc);
    pipelist.push_back(pipe1);
    uint32_t wid = 1234;
    audioInjectorPolicy.FetchCapDeviceInjectPostProc(pipelist, flag, wid);
}

/**
 * @tc.name: FetchCapDeviceInjectPostProc_003
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, FetchCapDeviceInjectPostProc_003, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    bool flag = false;
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    audioInjectorPolicy.isOpened_ = true;
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->streamAction_ = AUDIO_STREAM_ACTION_DEFAULT;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    pipe1->streamDescriptors_.push_back(desc);
    pipelist.push_back(pipe1);
    uint32_t wid = 1234;
    audioInjectorPolicy.FetchCapDeviceInjectPostProc(pipelist, flag, wid);
}

/**
 * @tc.name: SetInjectorStreamsMute_001
 * @tc.desc: Test SetInjectorStreamsMute
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, SetInjectorStreamsMute_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    audioInjectorPolicy.rendererStreamMap_[1111] = streamDesc1;
    audioInjectorPolicy.SetInjectorStreamsMute(true);
}

/**
 * @tc.name: SetInjectorStreamsMute_002
 * @tc.desc: Test SetInjectorStreamsMute
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, SetInjectorStreamsMute_002, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.SetInjectorStreamsMute(true);
}

/**
 * @tc.name: SetInjectorStreamsMute_003
 * @tc.desc: Test SetInjectorStreamsMute
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, SetInjectorStreamsMute_003, TestSize.Level1)
{
    uint32_t streamId = 100024;
    bool newMicrophoneMute = false;
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.injectorStreamIds_.insert(streamId);
    audioInjectorPolicy.rendererMuteStreamMap_.insert(std::make_pair(streamId, newMicrophoneMute));
    audioInjectorPolicy.SetInjectorStreamsMute(newMicrophoneMute);
    EXPECT_EQ(audioInjectorPolicy.rendererMuteStreamMap_.size(), 1);
}
 
/**
 * @tc.name: SetInjectorStreamsMute_004
 * @tc.desc: Test SetInjectorStreamsMute
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, SetInjectorStreamsMute_004, TestSize.Level1)
{
    uint32_t streamId = 100024;
    bool newMicrophoneMute = false;
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.injectorStreamIds_.insert(streamId);
    audioInjectorPolicy.rendererMuteStreamMap_.clear();
    audioInjectorPolicy.SetInjectorStreamsMute(newMicrophoneMute);
    EXPECT_EQ(audioInjectorPolicy.rendererMuteStreamMap_.size(), 1);
}

/**
 * @tc.name: SetInjectStreamsMuteForInjection_001
 * @tc.desc: Test SetInjectStreamsMuteForInjection
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, SetInjectStreamsMuteForInjection_001, TestSize.Level1)
{
    uint32_t streamId = 100024;
    bool newMicrophoneMute = true;
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isNeedMuteRenderer_ = newMicrophoneMute;
    audioInjectorPolicy.rendererMuteStreamMap_.clear();
    audioInjectorPolicy.SetInjectStreamsMuteForInjection(streamId);
    EXPECT_EQ(audioInjectorPolicy.rendererMuteStreamMap_.size(), 1);
}
 
/**
 * @tc.name: SetInjectStreamsMuteForInjection_002
 * @tc.desc: Test SetInjectStreamsMuteForInjection
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, SetInjectStreamsMuteForInjection_002, TestSize.Level1)
{
    uint32_t streamId = 100024;
    bool newMicrophoneMute = false;
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isNeedMuteRenderer_ = newMicrophoneMute;
    audioInjectorPolicy.rendererMuteStreamMap_.clear();
    audioInjectorPolicy.SetInjectStreamsMuteForInjection(streamId);
    EXPECT_EQ(audioInjectorPolicy.rendererMuteStreamMap_.size(), 0);
}

/**
 * @tc.name: SetInjectStreamsMuteForInjection_003
 * @tc.desc: Test SetInjectStreamsMuteForInjection
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, SetInjectStreamsMuteForInjection_003, TestSize.Level1)
{
    uint32_t streamId = 100024;
    bool newMicrophoneMute = true;
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isNeedMuteRenderer_ = newMicrophoneMute;
    audioInjectorPolicy.rendererMuteStreamMap_.clear();
    audioInjectorPolicy.rendererMuteStreamMap_.insert(std::make_pair(streamId, newMicrophoneMute));
    audioInjectorPolicy.SetInjectStreamsMuteForInjection(streamId);
    EXPECT_EQ(audioInjectorPolicy.rendererMuteStreamMap_.size(), 1);
}
 
/**
 * @tc.name: SetInjectStreamsMuteForInjection_004
 * @tc.desc: Test SetInjectStreamsMuteForInjection
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, SetInjectStreamsMuteForInjection_004, TestSize.Level1)
{
    uint32_t streamId = 100024;
    bool newMicrophoneMute = false;
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isNeedMuteRenderer_ = newMicrophoneMute;
    audioInjectorPolicy.rendererMuteStreamMap_.clear();
    audioInjectorPolicy.rendererMuteStreamMap_.insert(std::make_pair(streamId, newMicrophoneMute));
    audioInjectorPolicy.SetInjectStreamsMuteForInjection(streamId);
    EXPECT_EQ(audioInjectorPolicy.rendererMuteStreamMap_.size(), 1);
}
 
/**
 * @tc.name: SetInjectStreamsMuteForPlayback_001
 * @tc.desc: Test SetInjectStreamsMuteForPlayback
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, SetInjectStreamsMuteForPlayback_001, TestSize.Level1)
{
    uint32_t streamId = 100024;
    bool newMicrophoneMute = true;
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.rendererMuteStreamMap_.clear();
    audioInjectorPolicy.rendererMuteStreamMap_.insert(std::make_pair(streamId, newMicrophoneMute));
    audioInjectorPolicy.isNeedMuteRenderer_ = newMicrophoneMute;
    audioInjectorPolicy.SetInjectStreamsMuteForPlayback(streamId);
    EXPECT_EQ(audioInjectorPolicy.rendererMuteStreamMap_.size(), 1);
}
 
/**
 * @tc.name: SetInjectStreamsMuteForPlayback_002
 * @tc.desc: Test SetInjectStreamsMuteForPlayback
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, SetInjectStreamsMuteForPlayback_002, TestSize.Level1)
{
    uint32_t streamId = 100024;
    bool newMicrophoneMute = false;
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.rendererMuteStreamMap_.clear();
    audioInjectorPolicy.rendererMuteStreamMap_.insert(std::make_pair(streamId, newMicrophoneMute));
    audioInjectorPolicy.isNeedMuteRenderer_ = newMicrophoneMute;
    audioInjectorPolicy.SetInjectStreamsMuteForPlayback(streamId);
    EXPECT_EQ(audioInjectorPolicy.rendererMuteStreamMap_.size(), 1);
}
 
/**
 * @tc.name: SetInjectStreamsMuteForPlayback_003
 * @tc.desc: Test SetInjectStreamsMuteForPlayback
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, SetInjectStreamsMuteForPlayback_003, TestSize.Level1)
{
    uint32_t streamId = 100024;
    uint32_t streamId1 = 100025;
    bool newMicrophoneMute = true;
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.rendererMuteStreamMap_.clear();
    audioInjectorPolicy.rendererMuteStreamMap_.insert(std::make_pair(streamId1, newMicrophoneMute));
    audioInjectorPolicy.isNeedMuteRenderer_ = newMicrophoneMute;
    audioInjectorPolicy.SetInjectStreamsMuteForPlayback(streamId);
    EXPECT_EQ(audioInjectorPolicy.rendererMuteStreamMap_.size(), 1);
}
 
/**
 * @tc.name: SetInjectStreamsMuteForPlayback_00
 * @tc.desc: Test SetInjectStreamsMuteForPlayback
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ  false false
 */
HWTEST_F(AudioInjectorPolicyUnitTest, SetInjectStreamsMuteForPlayback_004, TestSize.Level1)
{
    uint32_t streamId = 100024;
    uint32_t streamId1 = 100025;
    bool newMicrophoneMute = false;
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.rendererMuteStreamMap_.clear();
    audioInjectorPolicy.rendererMuteStreamMap_.insert(std::make_pair(streamId1, newMicrophoneMute));
    audioInjectorPolicy.isNeedMuteRenderer_ = newMicrophoneMute;
    audioInjectorPolicy.SetInjectStreamsMuteForPlayback(streamId);
    EXPECT_EQ(audioInjectorPolicy.rendererMuteStreamMap_.size(), 1);
}

/**
 * @tc.name: PreparePreciseRenderer_InvalidTargetUid
 * @tc.desc: Test PreparePreciseRenderer with invalid targetUid (negative value)
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, PreparePreciseRenderer_InvalidTargetUid, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.injectorWorkMode_ = INJECTOR_MODE_NONE;
    int32_t invalidTargetUid = -1;
    uint32_t renderId = 1001;
    uint32_t targetStreamId = 2001;
    int32_t ret = audioInjectorPolicy.PreparePreciseRenderer(renderId, invalidTargetUid, targetStreamId);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name: PreparePreciseRenderer_LegacyTargetMode
 * @tc.desc: Test PreparePreciseRenderer when legacy target mode is active
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, PreparePreciseRenderer_LegacyTargetMode, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.injectorWorkMode_ = INJECTOR_MODE_LEGACY_TARGET;
    int32_t targetUid = 1234;
    uint32_t renderId = 1001;
    uint32_t targetStreamId = 2001;
    int32_t ret = audioInjectorPolicy.PreparePreciseRenderer(renderId, targetUid, targetStreamId);
    EXPECT_EQ(ERR_NOT_SUPPORTED, ret);
}

/**
 * @tc.name: PreparePreciseRenderer_StreamIdTargetNotMatch
 * @tc.desc: Test PreparePreciseRenderer when stream ID target mode is active but target does not match
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, PreparePreciseRenderer_StreamIdTargetNotMatch, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
    audioInjectorPolicy.activeTargetUid_ = 9999;
    audioInjectorPolicy.activeTargetStreamId_ = 8888;
    int32_t targetUid = 1234;
    uint32_t renderId = 1001;
    uint32_t targetStreamId = 2001;
    int32_t ret = audioInjectorPolicy.PreparePreciseRenderer(renderId, targetUid, targetStreamId);
    EXPECT_EQ(ERR_NOT_SUPPORTED, ret);
}

/**
 * @tc.name: FindCaptureVoipPipeByTarget_SourceTypeError
 * @tc.desc: Test FindCaptureVoipPipeByTarget when source type is not VOICE_COMMUNICATION
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, FindCaptureVoipPipeByTarget_SourceTypeError, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipelist = {};
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    pipe1->pipeRole_ = PIPE_ROLE_INPUT;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 4321;
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->audioMode_ = AUDIO_MODE_RECORD; // Required for IsRecording() to return true
    desc->callerUid_ = 1234; // Required for GetRealUid() to return 1234
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_CALL; // Not VOICE_COMMUNICATION
    desc->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    pipe1->streamDescriptors_.push_back(desc);
    pipelist.push_back(pipe1);
    VoipType type = VoipType::NO_VOIP;
    int32_t ret = ERR_INVALID_PARAM;
    auto result = audioInjectorPolicy.FindCaptureVoipPipeByTarget(pipelist, 1234, 4321, type, ret);
    EXPECT_EQ(nullptr, result);
    EXPECT_EQ(ERR_NOT_SUPPORTED, ret);
}

/**
 * @tc.name: FindCaptureVoipPipeByTarget_NotRunning
 * @tc.desc: Test FindCaptureVoipPipeByTarget when stream is not running
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, FindCaptureVoipPipeByTarget_NotRunning, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipelist = {};
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    pipe1->pipeRole_ = PIPE_ROLE_INPUT;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 4321;
    desc->streamStatus_ = STREAM_STATUS_STOPPED; // Not running
    desc->audioMode_ = AUDIO_MODE_RECORD; // Required for IsRecording() to return true
    desc->callerUid_ = 1234; // Required for GetRealUid() to return 1234
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    pipe1->streamDescriptors_.push_back(desc);
    pipelist.push_back(pipe1);
    VoipType type = VoipType::NO_VOIP;
    int32_t ret = ERR_INVALID_PARAM;
    auto result = audioInjectorPolicy.FindCaptureVoipPipeByTarget(pipelist, 1234, 4321, type, ret);
    EXPECT_EQ(nullptr, result);
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
 * @tc.name: FindCaptureVoipPipeByTarget_UnsupportedFlag
 * @tc.desc: Test FindCaptureVoipPipeByTarget when route flag is not supported
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, FindCaptureVoipPipeByTarget_UnsupportedFlag, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipelist = {};
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    pipe1->pipeRole_ = PIPE_ROLE_INPUT;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 4321;
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->audioMode_ = AUDIO_MODE_RECORD; // Required for IsRecording() to return true
    desc->callerUid_ = 1234; // Required for GetRealUid() to return 1234
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_WAKEUP; // Unsupported flag
    pipe1->streamDescriptors_.push_back(desc);
    pipelist.push_back(pipe1);
    VoipType type = VoipType::NO_VOIP;
    int32_t ret = ERR_INVALID_PARAM;
    auto result = audioInjectorPolicy.FindCaptureVoipPipeByTarget(pipelist, 1234, 4321, type, ret);
    EXPECT_EQ(nullptr, result);
    EXPECT_EQ(ERR_NOT_SUPPORTED, ret);
}

/**
 * @tc.name: FindCaptureVoipPipeByTarget_NormalVoip
 * @tc.desc: Test FindCaptureVoipPipeByTarget returns NORMAL_VOIP type
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, FindCaptureVoipPipeByTarget_NormalVoip, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipelist = {};
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    pipe1->pipeRole_ = PIPE_ROLE_INPUT;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 4321;
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->audioMode_ = AUDIO_MODE_RECORD; // Required for IsRecording() to return true
    desc->callerUid_ = 1234; // Required for GetRealUid() to return 1234
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    pipe1->streamDescriptors_.push_back(desc);
    pipelist.push_back(pipe1);
    VoipType type = VoipType::NO_VOIP;
    int32_t ret = ERR_INVALID_PARAM;
    auto result = audioInjectorPolicy.FindCaptureVoipPipeByTarget(pipelist, 1234, 4321, type, ret);
    EXPECT_NE(nullptr, result);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(NORMAL_VOIP, type);
}

/**
 * @tc.name: FindCaptureVoipPipeByTarget_FastVoip
 * @tc.desc: Test FindCaptureVoipPipeByTarget returns FAST_VOIP type
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, FindCaptureVoipPipeByTarget_FastVoip, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipelist = {};
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    pipe1->pipeRole_ = PIPE_ROLE_INPUT;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 4321;
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->audioMode_ = AUDIO_MODE_RECORD; // Required for IsRecording() to return true
    desc->callerUid_ = 1234; // Required for GetRealUid() to return 1234
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_FAST;
    pipe1->streamDescriptors_.push_back(desc);
    pipelist.push_back(pipe1);
    VoipType type = VoipType::NO_VOIP;
    int32_t ret = ERR_INVALID_PARAM;
    auto result = audioInjectorPolicy.FindCaptureVoipPipeByTarget(pipelist, 1234, 4321, type, ret);
    EXPECT_NE(nullptr, result);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(FAST_VOIP, type);
}

/**
 * @tc.name: AddCaptureInjectorInner_StreamIdTargetMode
 * @tc.desc: Test AddCaptureInjectorInner when work mode is STREAM_ID_TARGET
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, AddCaptureInjectorInner_StreamIdTargetMode, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
    audioInjectorPolicy.activeTargetStreamId_ = 4321;
    audioInjectorPolicy.isConnected_ = false;
    audioInjectorPolicy.voipType_ = NO_VOIP;
    int32_t ret = audioInjectorPolicy.AddCaptureInjectorInner();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: RemoveCaptureInjectorInner_StreamIdTargetMode
 * @tc.desc: Test RemoveCaptureInjectorInner when work mode is STREAM_ID_TARGET
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RemoveCaptureInjectorInner_StreamIdTargetMode, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
    audioInjectorPolicy.activeTargetStreamId_ = 4321;
    audioInjectorPolicy.isConnected_ = false;
    int32_t ret = audioInjectorPolicy.RemoveCaptureInjectorInner(true);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: FetchCapDeviceInjectPreProc_StreamIdTargetMode
 * @tc.desc: Test FetchCapDeviceInjectPreProc when work mode is STREAM_ID_TARGET
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, FetchCapDeviceInjectPreProc_StreamIdTargetMode, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isOpened_ = true;
    audioInjectorPolicy.injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
    audioInjectorPolicy.activeTargetUid_ = 1234;
    audioInjectorPolicy.activeTargetStreamId_ = 4321;
    audioInjectorPolicy.capturePortIdx_ = 1111;
    bool removeFlag = false;
    uint32_t streamId = 0;
    std::vector<std::shared_ptr<AudioPipeInfo>> pipelist = {};
    audioInjectorPolicy.FetchCapDeviceInjectPreProc(pipelist, removeFlag, streamId);
}

/**
 * @tc.name: FetchCapDeviceInjectPostProc_StreamIdTargetMode
 * @tc.desc: Test FetchCapDeviceInjectPostProc when work mode is STREAM_ID_TARGET with removeFlag true
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, FetchCapDeviceInjectPostProc_StreamIdTargetMode, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isOpened_ = true;
    audioInjectorPolicy.injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
    audioInjectorPolicy.activeTargetUid_ = 1234;
    audioInjectorPolicy.activeTargetStreamId_ = 4321;
    audioInjectorPolicy.capturePortIdx_ = 1111;
    bool removeFlag = true;
    uint32_t streamId = 0;
    std::vector<std::shared_ptr<AudioPipeInfo>> pipelist = {};
    audioInjectorPolicy.FetchCapDeviceInjectPostProc(pipelist, removeFlag, streamId);
}

/**
 * @tc.name: RebuildCaptureInjector_StreamIdTargetMode
 * @tc.desc: Test RebuildCaptureInjector when work mode is STREAM_ID_TARGET
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RebuildCaptureInjector_StreamIdTargetMode, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isOpened_ = true;
    audioInjectorPolicy.injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
    audioInjectorPolicy.activeTargetUid_ = 1234;
    audioInjectorPolicy.activeTargetStreamId_ = 4321;
    uint32_t streamId = 4321;
    audioInjectorPolicy.RebuildCaptureInjector(streamId);
}

/**
 * @tc.name: RebuildCaptureInjector_StreamMatched
 * @tc.desc: Test RebuildCaptureInjector when stream->sessionId_ matches streamId
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RebuildCaptureInjector_StreamMatched, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isOpened_ = true;
    audioInjectorPolicy.injectorWorkMode_ = INJECTOR_MODE_LEGACY_TARGET;
    audioInjectorPolicy.capturePortIdx_ = 1234;
    audioInjectorPolicy.voipType_ = NO_VOIP;
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    pipe1->pipeRole_ = PIPE_ROLE_INPUT;
    pipe1->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 4321;
    desc->streamStatus_ = STREAM_STATUS_STOPPED;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    pipe1->streamDescriptors_.push_back(desc);
    audioInjectorPolicy.pipeManager_->AddAudioPipeInfo(pipe1);
    audioInjectorPolicy.RebuildCaptureInjector(4321);
}

/**
 * @tc.name: RebuildCaptureInjector_HasRunningVoip
 * @tc.desc: Test RebuildCaptureInjector when HasRunningVoipStream returns true
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RebuildCaptureInjector_HasRunningVoip, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isOpened_ = true;
    audioInjectorPolicy.injectorWorkMode_ = INJECTOR_MODE_LEGACY_TARGET;
    audioInjectorPolicy.capturePortIdx_ = 1234;
    audioInjectorPolicy.voipType_ = NO_VOIP;
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    pipe1->pipeRole_ = PIPE_ROLE_INPUT;
    pipe1->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 4321;
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    pipe1->streamDescriptors_.push_back(desc);
    audioInjectorPolicy.pipeManager_->AddAudioPipeInfo(pipe1);
    audioInjectorPolicy.RebuildCaptureInjector(4321);
}

/**
 * @tc.name: RebuildCaptureInjector_EmptyRendererMap
 * @tc.desc: Test RebuildCaptureInjector when rendererStreamMap_ is empty
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RebuildCaptureInjector_EmptyRendererMap, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isOpened_ = true;
    audioInjectorPolicy.injectorWorkMode_ = INJECTOR_MODE_LEGACY_TARGET;
    audioInjectorPolicy.capturePortIdx_ = 1234;
    audioInjectorPolicy.voipType_ = NO_VOIP;
    audioInjectorPolicy.rendererStreamMap_.clear();
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    pipe1->pipeRole_ = PIPE_ROLE_INPUT;
    pipe1->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 4321;
    desc->streamStatus_ = STREAM_STATUS_STOPPED;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    pipe1->streamDescriptors_.push_back(desc);
    audioInjectorPolicy.pipeManager_->AddAudioPipeInfo(pipe1);
    audioInjectorPolicy.RebuildCaptureInjector(4321);
}

/**
 * @tc.name: ReleaseCaptureInjector_StreamIdTargetMode
 * @tc.desc: Test ReleaseCaptureInjector when work mode is STREAM_ID_TARGET
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, ReleaseCaptureInjector_StreamIdTargetMode, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
    audioInjectorPolicy.activeTargetUid_ = 1234;
    audioInjectorPolicy.activeTargetStreamId_ = 4321;
    audioInjectorPolicy.capturePortIdx_ = 1111;
    audioInjectorPolicy.ReleaseCaptureInjector();
}

/**
 * @tc.name: ClearRendererTarget_001
 * @tc.desc: Test ClearRendererTarget clears injector stream IDs
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, ClearRendererTarget_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    uint32_t renderId = 1001;
    audioInjectorPolicy.injectCaptureTargetUidMap_[renderId] = 1234;
    audioInjectorPolicy.injectCaptureTargetStreamIdMap_[renderId] = 4321;
    audioInjectorPolicy.injectorStreamIds_.insert(renderId);
    audioInjectorPolicy.injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
    audioInjectorPolicy.ClearRendererTarget(renderId);
    EXPECT_EQ(0, audioInjectorPolicy.injectCaptureTargetUidMap_.count(renderId));
    EXPECT_EQ(0, audioInjectorPolicy.injectCaptureTargetStreamIdMap_.count(renderId));
    EXPECT_EQ(0, audioInjectorPolicy.injectorStreamIds_.count(renderId));
}

/**
 * @tc.name: IsPreciseRenderer_001
 * @tc.desc: Test IsPreciseRenderer returns true when renderId is in injectCaptureTargetStreamIdMap_
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, IsPreciseRenderer_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    uint32_t renderId = 1001;
    audioInjectorPolicy.injectCaptureTargetStreamIdMap_.clear();
    audioInjectorPolicy.injectCaptureTargetStreamIdMap_[renderId] = 4321;
    bool ret = audioInjectorPolicy.IsPreciseRenderer(renderId);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: IsPreciseRenderer_002
 * @tc.desc: Test IsPreciseRenderer returns false when renderId is not in injectCaptureTargetStreamIdMap_
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, IsPreciseRenderer_002, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    uint32_t renderId = 1001;
    audioInjectorPolicy.injectCaptureTargetStreamIdMap_.clear();
    bool ret = audioInjectorPolicy.IsPreciseRenderer(renderId);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: AddCaptureInjectorByTargetStream_001
 * @tc.desc: Test AddCaptureInjectorByTargetStream with valid renderId
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, AddCaptureInjectorByTargetStream_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    uint32_t renderId = 1001;
    audioInjectorPolicy.injectCaptureTargetUidMap_.clear();
    audioInjectorPolicy.injectCaptureTargetStreamIdMap_.clear();
    audioInjectorPolicy.injectCaptureTargetUidMap_[renderId] = 1234;
    audioInjectorPolicy.injectCaptureTargetStreamIdMap_[renderId] = 4321;
    audioInjectorPolicy.isConnected_ = false;
    audioInjectorPolicy.voipType_ = NO_VOIP;
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1234;
    pipe1->pipeRole_ = PIPE_ROLE_INPUT;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 4321;
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->audioMode_ = AUDIO_MODE_RECORD; // Required for IsRecording() to return true
    desc->callerUid_ = 1234; // Required for GetRealUid() to return 1234
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    pipe1->streamDescriptors_.push_back(desc);
    pipelist.push_back(pipe1);
    int32_t ret = audioInjectorPolicy.AddCaptureInjectorByTargetStream(renderId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: AddCaptureInjectorByTargetStream_002
 * @tc.desc: Test AddCaptureInjectorByTargetStream without target info (should fail)
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, AddCaptureInjectorByTargetStream_002, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    uint32_t renderId = 1001;
    audioInjectorPolicy.injectCaptureTargetUidMap_.clear();
    audioInjectorPolicy.injectCaptureTargetStreamIdMap_.clear();
    int32_t ret = audioInjectorPolicy.AddCaptureInjectorByTargetStream(renderId);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name: DeleteInjectorStreamId_001
 * @tc.desc: Test DeleteInjectorStreamId removes streamId and resets injector mode
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, DeleteInjectorStreamId_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    uint32_t streamId = 1001;
    audioInjectorPolicy.injectorStreamIds_.insert(streamId);
    audioInjectorPolicy.injectorWorkMode_ = INJECTOR_MODE_STREAM_ID_TARGET;
    audioInjectorPolicy.injectorStreamIds_.clear();
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.injectCaptureTargetStreamIdMap_.clear();
    audioInjectorPolicy.DeleteInjectorStreamId(streamId);
    EXPECT_EQ(0, audioInjectorPolicy.injectorStreamIds_.count(streamId));
}

/**
 * @tc.name: AddInjectorStreamId_001
 * @tc.desc: Test AddInjectorStreamId when not in STREAM_ID_TARGET mode
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, AddInjectorStreamId_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    uint32_t streamId = 1001;
    audioInjectorPolicy.injectorWorkMode_ = INJECTOR_MODE_LEGACY_TARGET;
    audioInjectorPolicy.injectorStreamIds_.clear();
    audioInjectorPolicy.AddInjectorStreamId(streamId);
    EXPECT_EQ(1, audioInjectorPolicy.injectorStreamIds_.count(streamId));
}

/**
 * @tc.name: IsActivateInterruptStreamId_001
 * @tc.desc: Test IsActivateInterruptStreamId returns true when streamId is in injectorStreamIds_
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, IsActivateInterruptStreamId_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    uint32_t streamId = 1001;
    audioInjectorPolicy.injectorStreamIds_.clear();
    audioInjectorPolicy.injectorStreamIds_.insert(streamId);
    bool ret = audioInjectorPolicy.IsActivateInterruptStreamId(streamId);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: IsActivateInterruptStreamId_002
 * @tc.desc: Test IsActivateInterruptStreamId returns false when streamId is not in injectorStreamIds_
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, IsActivateInterruptStreamId_002, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    uint32_t streamId = 1001;
    audioInjectorPolicy.injectorStreamIds_.clear();
    bool ret = audioInjectorPolicy.IsActivateInterruptStreamId(streamId);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: RebuildCaptureInjectorByTargetStream_001
 * @tc.desc: Test RebuildCaptureInjectorByTargetStream when streamId matches activeTargetStreamId_
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RebuildCaptureInjectorByTargetStream_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    uint32_t streamId = 4321;
    audioInjectorPolicy.activeTargetStreamId_ = 4321;
    audioInjectorPolicy.activeTargetUid_ = 1234;
    audioInjectorPolicy.isConnected_ = true;
    audioInjectorPolicy.capturePortIdx_ = 1111;
    audioInjectorPolicy.voipType_ = NORMAL_VOIP;
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 1111;
    pipe1->pipeRole_ = PIPE_ROLE_INPUT;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 4321;
    desc->streamStatus_ = STREAM_STATUS_STARTED;
    desc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    desc->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    pipe1->streamDescriptors_.push_back(desc);
    pipelist.push_back(pipe1);
    audioInjectorPolicy.RebuildCaptureInjectorByTargetStream(streamId);
}

/**
 * @tc.name: RebuildCaptureInjectorByTargetStream_002
 * @tc.desc: Test RebuildCaptureInjectorByTargetStream when streamId does not match activeTargetStreamId_
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RebuildCaptureInjectorByTargetStream_002, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    uint32_t streamId = 9999;
    audioInjectorPolicy.activeTargetStreamId_ = 4321;
    audioInjectorPolicy.activeTargetUid_ = 1234;
    audioInjectorPolicy.RebuildCaptureInjectorByTargetStream(streamId);
}

/**
 * @tc.name: RebuildCaptureInjectorByTargetStream_003
 * @tc.desc: Test RebuildCaptureInjectorByTargetStream when pipe is not found
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RebuildCaptureInjectorByTargetStream_003, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    uint32_t streamId = 4321;
    audioInjectorPolicy.activeTargetStreamId_ = 4321;
    audioInjectorPolicy.activeTargetUid_ = 1234;
    audioInjectorPolicy.isConnected_ = true;
    audioInjectorPolicy.capturePortIdx_ = 1111;
    audioInjectorPolicy.voipType_ = NORMAL_VOIP;
    auto &pipelist = audioInjectorPolicy.pipeManager_->curPipeList_;
    pipelist.clear();
    audioInjectorPolicy.RebuildCaptureInjectorByTargetStream(streamId);
}

/**
 * @tc.name: AddCaptureInjectorByTargetStreamInner_001
 * @tc.desc: Test AddCaptureInjectorByTargetStreamInner when not connected
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, AddCaptureInjectorByTargetStreamInner_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    uint32_t targetStreamId = 4321;
    audioInjectorPolicy.isConnected_ = false;
    audioInjectorPolicy.voipType_ = NO_VOIP;
    int32_t ret = audioInjectorPolicy.AddCaptureInjectorByTargetStreamInner(targetStreamId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: AddCaptureInjectorByTargetStreamInner_002
 * @tc.desc: Test AddCaptureInjectorByTargetStreamInner when connected
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, AddCaptureInjectorByTargetStreamInner_002, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    uint32_t targetStreamId = 4321;
    audioInjectorPolicy.isConnected_ = true;
    int32_t ret = audioInjectorPolicy.AddCaptureInjectorByTargetStreamInner(targetStreamId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: RemoveCaptureInjectorByTargetStreamInner_001
 * @tc.desc: Test RemoveCaptureInjectorByTargetStreamInner when connected and no capturer
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RemoveCaptureInjectorByTargetStreamInner_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isConnected_ = true;
    audioInjectorPolicy.capturePortIdx_ = 1111;
    audioInjectorPolicy.voipType_ = NORMAL_VOIP;
    audioInjectorPolicy.activeTargetStreamId_ = 4321;
    audioInjectorPolicy.rendererStreamMap_.clear();
    int32_t ret = audioInjectorPolicy.RemoveCaptureInjectorByTargetStreamInner(true);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: RemoveCaptureInjectorByTargetStreamInner_002
 * @tc.desc: Test RemoveCaptureInjectorByTargetStreamInner when connected with capturer
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RemoveCaptureInjectorByTargetStreamInner_002, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isConnected_ = true;
    audioInjectorPolicy.capturePortIdx_ = 1111;
    audioInjectorPolicy.voipType_ = NORMAL_VOIP;
    audioInjectorPolicy.activeTargetStreamId_ = 4321;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    audioInjectorPolicy.rendererStreamMap_[1001] = streamDesc;
    int32_t ret = audioInjectorPolicy.RemoveCaptureInjectorByTargetStreamInner(false);
    EXPECT_EQ(SUCCESS, ret);
}
} // namespace AudioStandard
} // namespace OHOS
