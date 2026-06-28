/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "audio_playback_engine_unit_test.h"
#include "audio_errors.h"
#include "audio_device_descriptor.h"
#include "mock/mock_i_renderer_stream.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

std::shared_ptr<AudioPlaybackEngine> AudioPlaybackEngineUnitTest::audioPlaybackEngine_ = nullptr;

void AudioPlaybackEngineUnitTest::SetUpTestCase(void)
{
    // Setup for all test cases
    audioPlaybackEngine_ = std::make_shared<AudioPlaybackEngine>();
}

void AudioPlaybackEngineUnitTest::TearDownTestCase(void)
{
    // Teardown for all test cases
    audioPlaybackEngine_ = nullptr;
}

void AudioPlaybackEngineUnitTest::SetUp(void)
{
    // Setup before each test case
}

void AudioPlaybackEngineUnitTest::TearDown(void)
{
    // Teardown after each test case
}

/**
 * @tc.name  : Test AudioPlaybackEngine constructor
 * @tc.type  : FUNC
 * @tc.number: AudioPlaybackEngineConstructor_001
 * @tc.desc  : Test AudioPlaybackEngine constructor.
 */
HWTEST_F(AudioPlaybackEngineUnitTest, AudioPlaybackEngineConstructor_001, TestSize.Level1)
{
    // Verify that audioPlaybackEngine_ is properly created in SetUpTestCase
    // AudioPlaybackEngine is already created in SetUpTestCase
    // We can verify it's not null and can call its methods
    int32_t ret = audioPlaybackEngine_->Init(AudioDeviceDescriptor(), false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Init
 * @tc.type  : FUNC
 * @tc.number: AudioPlaybackEngineInit_001
 * @tc.desc  : Test Init interface.
 */
HWTEST_F(AudioPlaybackEngineUnitTest, AudioPlaybackEngineInit_001, TestSize.Level1)
{
    // Test Init function
    AudioDeviceDescriptor type;
    bool isVoip = false;
    
    int32_t ret = audioPlaybackEngine_->Init(type, isVoip);
    
    // Init always returns SUCCESS
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AddRenderer
 * @tc.type  : FUNC
 * @tc.number: AudioPlaybackEngineAddRenderer_001
 * @tc.desc  : Test AddRenderer with valid stream.
 */
HWTEST_F(AudioPlaybackEngineUnitTest, AudioPlaybackEngineAddRenderer_001, TestSize.Level1)
{
    // Test AddRenderer function with valid stream
    auto stream = std::make_shared<MockIRendererStream>();
    
    int32_t ret = audioPlaybackEngine_->AddRenderer(stream);
    
    // AddRenderer always returns SUCCESS
    EXPECT_EQ(ret, SUCCESS);
    
    // Verify that the stream was added to streams_ vector
    // Note: We can't directly access streams_ vector (it's protected),
    // but we can verify that adding the same stream again doesn't crash
    int32_t ret2 = audioPlaybackEngine_->AddRenderer(stream);
    EXPECT_EQ(ret2, SUCCESS);  // Adding same stream again should not crash
}

/**
 * @tc.name  : Test RemoveRenderer
 * @tc.type  : FUNC
 * @tc.number: AudioPlaybackEngineRemoveRenderer_001
 * @tc.desc  : Test RemoveRenderer with valid stream.
 */
HWTEST_F(AudioPlaybackEngineUnitTest, AudioPlaybackEngineRemoveRenderer_001, TestSize.Level1)
{
    // Test RemoveRenderer function with valid stream
    auto stream = std::make_shared<MockIRendererStream>();
    
    // First, add the stream
    int32_t ret = audioPlaybackEngine_->AddRenderer(stream);
    EXPECT_EQ(ret, SUCCESS);
    
    // Now remove it
    // RemoveRenderer should not crash
    EXPECT_NO_FATAL_FAILURE(audioPlaybackEngine_->RemoveRenderer(stream));
}

/**
 * @tc.name  : Test Start
 * @tc.type  : FUNC
 * @tc.number: AudioPlaybackEngineStart_001
 * @tc.desc  : Test Start interface.
 */
HWTEST_F(AudioPlaybackEngineUnitTest, AudioPlaybackEngineStart_001, TestSize.Level1)
{
    // Test Start function
    int32_t ret = audioPlaybackEngine_->Start();
    
    // Start always returns SUCCESS
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Stop
 * @tc.type  : FUNC
 * @tc.number: AudioPlaybackEngineStop_001
 * @tc.desc  : Test Stop interface.
 */
HWTEST_F(AudioPlaybackEngineUnitTest, AudioPlaybackEngineStop_001, TestSize.Level1)
{
    // Test Stop function
    int32_t ret = audioPlaybackEngine_->Stop();
    
    // Stop always returns SUCCESS
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Pause
 * @tc.type  : FUNC
 * @tc.number: AudioPlaybackEnginePause_001
 * @tc.desc  : Test Pause interface.
 */
HWTEST_F(AudioPlaybackEngineUnitTest, AudioPlaybackEnginePause_001, TestSize.Level1)
{
    // Test Pause function
    bool isStandby = false;
    
    int32_t ret = audioPlaybackEngine_->Pause(isStandby);
    
    // Pause always returns SUCCESS
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Flush
 * @tc.type  : FUNC
 * @tc.number: AudioPlaybackEngineFlush_001
 * @tc.desc  : Test Flush interface.
 */
HWTEST_F(AudioPlaybackEngineUnitTest, AudioPlaybackEngineFlush_001, TestSize.Level1)
{
    // Test Flush function
    int32_t ret = audioPlaybackEngine_->Flush();
    
    // Flush always returns SUCCESS
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test IsPlaybackEngineRunning
 * @tc.type  : FUNC
 * @tc.number: AudioPlaybackEngineIsPlaybackEngineRunning_001
 * @tc.desc  : Test IsPlaybackEngineRunning interface.
 */
HWTEST_F(AudioPlaybackEngineUnitTest, AudioPlaybackEngineIsPlaybackEngineRunning_001, TestSize.Level1)
{
    // Test IsPlaybackEngineRunning function
    bool result = audioPlaybackEngine_->IsPlaybackEngineRunning();
    
    // IsPlaybackEngineRunning always returns false
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test GetLatency
 * @tc.type  : FUNC
 * @tc.number: AudioPlaybackEngineGetLatency_001
 * @tc.desc  : Test GetLatency interface.
 */
HWTEST_F(AudioPlaybackEngineUnitTest, AudioPlaybackEngineGetLatency_001, TestSize.Level1)
{
    // Test GetLatency function
    uint64_t latency = audioPlaybackEngine_->GetLatency();
    
    // GetLatency always returns 0
    EXPECT_EQ(latency, 0);
}

} // namespace AudioStandard
} // namespace OHOS
