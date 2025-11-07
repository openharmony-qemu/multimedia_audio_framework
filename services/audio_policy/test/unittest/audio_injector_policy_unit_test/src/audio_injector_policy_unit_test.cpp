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
 * @tc.name: RebuildCaptureInjector_001
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RebuildCaptureInjector_001, TestSize.Level1)
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
    pipe1->streamDescriptors_.push_back(desc);
    EXPECT_NE(nullptr, audioInjectorPolicy.pipeManager_);
    audioInjectorPolicy.pipeManager_->AddAudioPipeInfo(pipe1);
    audioInjectorPolicy.capturePortIdx_ = 1111;
    audioInjectorPolicy.RebuildCaptureInjector(1111);
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
    EXPECT_EQ(audioInjectorPolicy.rendererMuteStreamMap_.size(), 0);
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
    EXPECT_EQ(audioInjectorPolicy.rendererMuteStreamMap_.size(), 2);
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
    EXPECT_EQ(audioInjectorPolicy.rendererMuteStreamMap_.size(), 2);
}
} // namespace AudioStandard
} // namespace OHOS