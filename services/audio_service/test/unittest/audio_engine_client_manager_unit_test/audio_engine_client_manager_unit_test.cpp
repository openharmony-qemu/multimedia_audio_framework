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
#include "gtest/gtest.h"

#include "audio_engine_client_manager.h"
#include "audio_errors.h"
#include "audio_common_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioEngineClientManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};
    virtual void SetUp();
    virtual void TearDown(){};

private:
    std::shared_ptr<AudioEngineClientManager> testClient_ = nullptr;
};

void AudioEngineClientManagerUnitTest::SetUp()
{
    testClient_ = DelayedSingleton<AudioEngineClientManager>::GetInstance();
}

class TestAudioOutputPipeCallback : public AudioOutputPipeCallback {
public:
    void OnOutputPipeChange(AudioPipeChangeType changeType,
        const std::shared_ptr<AudioOutputPipeInfo> &changedPipeInfo)
    {
        AUDIO_INFO_LOG("Receive output pipe change");
    }
};

class TestAudioInputPipeCallback : public AudioInputPipeCallback {
public:
    void OnInputPipeChange(AudioPipeChangeType changeType,
        const std::shared_ptr<AudioInputPipeInfo> &changedPipeInfo)
    {
        AUDIO_INFO_LOG("Receive input pipe change");
    }
};

/**
 * @tc.name   : AudioEngineClientManagerUnitTest_GetCurrentOutputPipeChangeInfos_001
 * @tc.number : GetCurrentOutputPipeChangeInfos_001
 * @tc.desc   : Test basic GetCurrentOutputPipeChangeInfos()
 */
HWTEST_F(AudioEngineClientManagerUnitTest, GetCurrentOutputPipeChangeInfos_001, TestSize.Level2)
{
    std::vector<std::shared_ptr<AudioOutputPipeInfo>> pipeChangeInfos;
    int32_t status = testClient_->GetCurrentOutputPipeChangeInfos(pipeChangeInfos);
    EXPECT_EQ(SUCCESS, status);
}

/**
 * @tc.name   : AudioEngineClientManagerUnitTest_GetCurrentInputPipeChangeInfos_001
 * @tc.number : GetCurrentInputPipeChangeInfos_001
 * @tc.desc   : Test basic GetCurrentInputPipeChangeInfos()
 */
HWTEST_F(AudioEngineClientManagerUnitTest, GetCurrentInputPipeChangeInfos_001, TestSize.Level2)
{
    std::vector<std::shared_ptr<AudioInputPipeInfo>> pipeChangeInfos;
    int32_t status = testClient_->GetCurrentInputPipeChangeInfos(pipeChangeInfos);
    EXPECT_EQ(SUCCESS, status);
}

/**
 * @tc.name   : AudioEngineClientManagerUnitTest_RegisterOutputPipeChangeCallback_001
 * @tc.number : RegisterOutputPipeChangeCallback_001
 * @tc.desc   : Test basic RegisterOutputPipeChangeCallback() and UnregisterOutputPipeChangeCallback()
 */
HWTEST_F(AudioEngineClientManagerUnitTest, RegisterOutputPipeChangeCallback_001, TestSize.Level2)
{
    std::shared_ptr<AudioOutputPipeCallback> pipeCb1 = std::make_shared<TestAudioOutputPipeCallback>();
    int32_t status = testClient_->RegisterOutputPipeChangeCallback(pipeCb1);
    EXPECT_EQ(SUCCESS, status);
    std::shared_ptr<AudioOutputPipeCallback> pipeCb2 = std::make_shared<TestAudioOutputPipeCallback>();
    status = testClient_->RegisterOutputPipeChangeCallback(pipeCb2);
    EXPECT_EQ(SUCCESS, status);
    status = testClient_->UnregisterOutputPipeChangeCallback(pipeCb1);
    EXPECT_EQ(SUCCESS, status);
    status = testClient_->UnregisterOutputPipeChangeCallback(pipeCb2);
    EXPECT_EQ(SUCCESS, status);
}

/**
 * @tc.name   : AudioEngineClientManagerUnitTest_RegisterOutputPipeChangeCallback_002
 * @tc.number : RegisterOutputPipeChangeCallback_002
 * @tc.desc   : Test abnormal RegisterOutputPipeChangeCallback()
 */
HWTEST_F(AudioEngineClientManagerUnitTest, RegisterOutputPipeChangeCallback_002, TestSize.Level4)
{
    std::shared_ptr<AudioOutputPipeCallback> nullCb = nullptr;
    int32_t status = testClient_->RegisterOutputPipeChangeCallback(nullCb);
    EXPECT_EQ(SUCCESS, status);

    std::shared_ptr<AudioOutputPipeCallback> pipeCb = std::make_shared<TestAudioOutputPipeCallback>();
    status = testClient_->RegisterOutputPipeChangeCallback(pipeCb);
    status = testClient_->RegisterOutputPipeChangeCallback(pipeCb);
    EXPECT_EQ(SUCCESS, status);
}

/**
 * @tc.name   : AudioEngineClientManagerUnitTest_UnregisterOutputPipeChangeCallback_001
 * @tc.number : UnregisterOutputPipeChangeCallback_001
 * @tc.desc   : Test abnormal UnregisterOutputPipeChangeCallback()
 */
HWTEST_F(AudioEngineClientManagerUnitTest, UnregisterOutputPipeChangeCallback_001, TestSize.Level4)
{
    std::shared_ptr<AudioOutputPipeCallback> nullCb = nullptr;
    int32_t status = testClient_->UnregisterOutputPipeChangeCallback(nullCb);
    EXPECT_EQ(SUCCESS, status);

    std::shared_ptr<AudioOutputPipeCallback> pipeCb = std::make_shared<TestAudioOutputPipeCallback>();
    status = testClient_->UnregisterOutputPipeChangeCallback(pipeCb);
    EXPECT_EQ(SUCCESS, status);
}

/**
 * @tc.name   : AudioEngineClientManagerUnitTest_RegisterInputPipeChangeCallback_001
 * @tc.number : RegisterInputPipeChangeCallback_001
 * @tc.desc   : Test basic RegisterInputPipeChangeCallback() and UnregisterInputPipeChangeCallback()
 */
HWTEST_F(AudioEngineClientManagerUnitTest, RegisterInputPipeChangeCallback_001, TestSize.Level2)
{
    std::shared_ptr<AudioInputPipeCallback> pipeCb1 = std::make_shared<TestAudioInputPipeCallback>();
    int32_t status = testClient_->RegisterInputPipeChangeCallback(pipeCb1);
    EXPECT_EQ(SUCCESS, status);
    std::shared_ptr<AudioInputPipeCallback> pipeCb2 = std::make_shared<TestAudioInputPipeCallback>();
    status = testClient_->RegisterInputPipeChangeCallback(pipeCb2);
    EXPECT_EQ(SUCCESS, status);
    status = testClient_->UnregisterInputPipeChangeCallback(pipeCb1);
    EXPECT_EQ(SUCCESS, status);
    status = testClient_->UnregisterInputPipeChangeCallback(pipeCb2);
    EXPECT_EQ(SUCCESS, status);
}

/**
 * @tc.name   : AudioEngineClientManagerUnitTest_RegisterInputPipeChangeCallback_002
 * @tc.number : RegisterInputPipeChangeCallback_002
 * @tc.desc   : Test abnormal RegisterInputPipeChangeCallback()
 */
HWTEST_F(AudioEngineClientManagerUnitTest, RegisterInputPipeChangeCallback_002, TestSize.Level4)
{
    std::shared_ptr<AudioInputPipeCallback> nullCb = nullptr;
    int32_t status = testClient_->RegisterInputPipeChangeCallback(nullCb);
    EXPECT_EQ(SUCCESS, status);

    std::shared_ptr<AudioInputPipeCallback> pipeCb = std::make_shared<TestAudioInputPipeCallback>();
    status = testClient_->RegisterInputPipeChangeCallback(pipeCb);
    status = testClient_->RegisterInputPipeChangeCallback(pipeCb);
    EXPECT_EQ(SUCCESS, status);
}

/**
 * @tc.name   : AudioEngineClientManagerUnitTest_UnregisterInputPipeChangeCallback_001
 * @tc.number : UnregisterInputPipeChangeCallback_001
 * @tc.desc   : Test abnormal UnregisterInputPipeChangeCallback()
 */
HWTEST_F(AudioEngineClientManagerUnitTest, UnregisterInputPipeChangeCallback_001, TestSize.Level4)
{
    std::shared_ptr<AudioInputPipeCallback> nullCb = nullptr;
    int32_t status = testClient_->UnregisterInputPipeChangeCallback(nullCb);
    EXPECT_EQ(SUCCESS, status);

    std::shared_ptr<AudioInputPipeCallback> pipeCb = std::make_shared<TestAudioInputPipeCallback>();
    status = testClient_->UnregisterInputPipeChangeCallback(pipeCb);
    EXPECT_EQ(SUCCESS, status);
}

}  // namespace AudioStandard
}  // namespace OHOS
