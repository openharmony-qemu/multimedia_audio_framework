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

#include "audio_errors.h"
#include "audio_concurrency_service_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioConcurrencyServiceUnitTest::SetUpTestCase(void) {}
void AudioConcurrencyServiceUnitTest::TearDownTestCase(void) {}
void AudioConcurrencyServiceUnitTest::SetUp(void) {}
void AudioConcurrencyServiceUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_001.
* @tc.desc  : Test OnRemoteDied.
*/
HWTEST_F(AudioConcurrencyServiceUnitTest, AudioConcurrencyService_001, TestSize.Level1)
{
    std::shared_ptr<AudioConcurrencyService> service = std::make_shared<AudioConcurrencyService>();
    uint32_t sessionID = 0;
    auto deathRecipient = std::make_shared<AudioConcurrencyService::AudioConcurrencyDeathRecipient>(service, sessionID);
    deathRecipient->OnRemoteDied(nullptr);
    EXPECT_NE(deathRecipient, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_002.
* @tc.desc  : Test UnsetAudioConcurrencyCallback.
*/
HWTEST_F(AudioConcurrencyServiceUnitTest, AudioConcurrencyService_002, TestSize.Level1)
{
    uint32_t sessionID = 0;
    std::shared_ptr<AudioConcurrencyService> audioConcurrencyService = std::make_shared<AudioConcurrencyService>();
    auto result = audioConcurrencyService->UnsetAudioConcurrencyCallback(sessionID);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_003.
* @tc.desc  : Test ActivateAudioConcurrency.
*/
HWTEST_F(AudioConcurrencyServiceUnitTest, AudioConcurrencyService_003, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    auto audioConcurrencyService = std::make_shared<AudioConcurrencyService>();
    auto result = audioConcurrencyService->ActivateAudioConcurrency(AudioPipeType::PIPE_TYPE_CALL_IN,
        audioRendererChangeInfos, audioCapturerChangeInfos);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_004.
* @tc.desc  : Test ActivateAudioConcurrency.
*/
HWTEST_F(AudioConcurrencyServiceUnitTest, AudioConcurrencyService_004, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    auto audioConcurrencyService = std::make_shared<AudioConcurrencyService>();

    auto audioRendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    audioRendererChangeInfo->rendererInfo.pipeType = PIPE_TYPE_OFFLOAD;
    audioRendererChangeInfos.push_back(audioRendererChangeInfo);

    std::pair<AudioPipeType, AudioPipeType> pair;
    audioConcurrencyService->concurrencyCfgMap_.insert({pair, ConcurrencyAction::PLAY_BOTH});

    auto result = audioConcurrencyService->ActivateAudioConcurrency(AudioPipeType::PIPE_TYPE_OFFLOAD,
        audioRendererChangeInfos, audioCapturerChangeInfos);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_005.
* @tc.desc  : Test ActivateAudioConcurrency.
*/
HWTEST_F(AudioConcurrencyServiceUnitTest, AudioConcurrencyService_005, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    auto audioConcurrencyService = std::make_shared<AudioConcurrencyService>();

    auto audioRendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    audioRendererChangeInfo->rendererInfo.pipeType = PIPE_TYPE_CALL_OUT;
    audioRendererChangeInfos.push_back(audioRendererChangeInfo);

    std::pair<AudioPipeType, AudioPipeType> pair;
    audioConcurrencyService->concurrencyCfgMap_.insert({pair, ConcurrencyAction::PLAY_BOTH});

    auto result = audioConcurrencyService->ActivateAudioConcurrency(AudioPipeType::PIPE_TYPE_CALL_OUT,
        audioRendererChangeInfos, audioCapturerChangeInfos);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_006.
* @tc.desc  : Test ActivateAudioConcurrency.
*/
HWTEST_F(AudioConcurrencyServiceUnitTest, AudioConcurrencyService_006, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    auto audioConcurrencyService = std::make_shared<AudioConcurrencyService>();

    auto audioRendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    audioRendererChangeInfo->rendererInfo.pipeType = PIPE_TYPE_NORMAL_OUT;
    audioRendererChangeInfos.push_back(audioRendererChangeInfo);

    std::pair<AudioPipeType, AudioPipeType> pair;
    audioConcurrencyService->concurrencyCfgMap_.insert({pair, ConcurrencyAction::PLAY_BOTH});

    auto result = audioConcurrencyService->ActivateAudioConcurrency(AudioPipeType::PIPE_TYPE_CALL_OUT,
        audioRendererChangeInfos, audioCapturerChangeInfos);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_007.
* @tc.desc  : Test ActivateAudioConcurrency.
*/
HWTEST_F(AudioConcurrencyServiceUnitTest, AudioConcurrencyService_007, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    auto audioConcurrencyService = std::make_shared<AudioConcurrencyService>();

    auto audioRendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    audioRendererChangeInfo->rendererInfo.pipeType = PIPE_TYPE_NORMAL_OUT;
    audioRendererChangeInfos.push_back(audioRendererChangeInfo);

    std::pair<AudioPipeType, AudioPipeType> pair = std::make_pair(audioRendererChangeInfo->rendererInfo.pipeType,
        AudioPipeType::PIPE_TYPE_MULTICHANNEL);
    audioConcurrencyService->concurrencyCfgMap_.insert({pair, ConcurrencyAction::CONCEDE_EXISTING});

    audioConcurrencyService->handler_ = std::make_shared<AudioPolicyServerHandler>();
    auto result = audioConcurrencyService->ActivateAudioConcurrency(AudioPipeType::PIPE_TYPE_MULTICHANNEL,
        audioRendererChangeInfos, audioCapturerChangeInfos);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_008.
* @tc.desc  : Test ActivateAudioConcurrency.
*/
HWTEST_F(AudioConcurrencyServiceUnitTest, AudioConcurrencyService_008, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    auto audioConcurrencyService = std::make_shared<AudioConcurrencyService>();

    auto audioRendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    audioRendererChangeInfo->rendererInfo.pipeType = PIPE_TYPE_NORMAL_OUT;
    audioRendererChangeInfos.push_back(audioRendererChangeInfo);

    std::pair<AudioPipeType, AudioPipeType> pair = std::make_pair(audioRendererChangeInfo->rendererInfo.pipeType,
        AudioPipeType::PIPE_TYPE_MULTICHANNEL);
    audioConcurrencyService->concurrencyCfgMap_.insert({pair, ConcurrencyAction::CONCEDE_EXISTING});

    audioConcurrencyService->handler_ = nullptr;
    auto result = audioConcurrencyService->ActivateAudioConcurrency(AudioPipeType::PIPE_TYPE_MULTICHANNEL,
        audioRendererChangeInfos, audioCapturerChangeInfos);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_009.
* @tc.desc  : Test ActivateAudioConcurrency.
*/
HWTEST_F(AudioConcurrencyServiceUnitTest, AudioConcurrencyService_009, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    auto audioConcurrencyService = std::make_shared<AudioConcurrencyService>();

    auto audioRendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    audioRendererChangeInfo->rendererInfo.pipeType = PIPE_TYPE_NORMAL_OUT;
    audioRendererChangeInfos.push_back(audioRendererChangeInfo);

    std::pair<AudioPipeType, AudioPipeType> pair = std::make_pair(audioRendererChangeInfo->rendererInfo.pipeType,
        AudioPipeType::PIPE_TYPE_MULTICHANNEL);
    audioConcurrencyService->concurrencyCfgMap_.insert({pair, ConcurrencyAction::PLAY_BOTH});

    audioConcurrencyService->handler_ = std::make_shared<AudioPolicyServerHandler>();
    auto result = audioConcurrencyService->ActivateAudioConcurrency(AudioPipeType::PIPE_TYPE_MULTICHANNEL,
        audioRendererChangeInfos, audioCapturerChangeInfos);
    EXPECT_EQ(result, SUCCESS);

    audioConcurrencyService->handler_ = nullptr;
    result = audioConcurrencyService->ActivateAudioConcurrency(AudioPipeType::PIPE_TYPE_MULTICHANNEL,
        audioRendererChangeInfos, audioCapturerChangeInfos);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_010.
* @tc.desc  : Test ActivateAudioConcurrency.
*/
HWTEST_F(AudioConcurrencyServiceUnitTest, AudioConcurrencyService_010, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    auto audioConcurrencyService = std::make_shared<AudioConcurrencyService>();

    auto audioCapturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    audioCapturerChangeInfo->capturerInfo.pipeType = PIPE_TYPE_CALL_IN;
    audioCapturerChangeInfos.push_back(audioCapturerChangeInfo);

    std::pair<AudioPipeType, AudioPipeType> pair = std::make_pair(audioCapturerChangeInfo->capturerInfo.pipeType,
        AudioPipeType::PIPE_TYPE_CALL_IN);
    audioConcurrencyService->concurrencyCfgMap_.insert({pair, ConcurrencyAction::PLAY_BOTH});

    audioConcurrencyService->handler_ = std::make_shared<AudioPolicyServerHandler>();
    auto result = audioConcurrencyService->ActivateAudioConcurrency(AudioPipeType::PIPE_TYPE_CALL_IN,
        audioRendererChangeInfos, audioCapturerChangeInfos);
    EXPECT_EQ(result, SUCCESS);

    audioConcurrencyService->handler_ = nullptr;
    result = audioConcurrencyService->ActivateAudioConcurrency(AudioPipeType::PIPE_TYPE_CALL_IN,
        audioRendererChangeInfos, audioCapturerChangeInfos);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_011.
* @tc.desc  : Test ActivateAudioConcurrency.
*/
HWTEST_F(AudioConcurrencyServiceUnitTest, AudioConcurrencyService_011, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    auto audioConcurrencyService = std::make_shared<AudioConcurrencyService>();

    auto audioCapturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    audioCapturerChangeInfo->capturerInfo.pipeType = PIPE_TYPE_CALL_IN;
    audioCapturerChangeInfos.push_back(audioCapturerChangeInfo);

    std::pair<AudioPipeType, AudioPipeType> pair = std::make_pair(audioCapturerChangeInfo->capturerInfo.pipeType,
        AudioPipeType::PIPE_TYPE_CALL_IN);
    audioConcurrencyService->concurrencyCfgMap_.insert({pair, ConcurrencyAction::CONCEDE_EXISTING});

    audioConcurrencyService->handler_ = nullptr;
    auto result = audioConcurrencyService->ActivateAudioConcurrency(AudioPipeType::PIPE_TYPE_CALL_IN,
        audioRendererChangeInfos, audioCapturerChangeInfos);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_012.
* @tc.desc  : Test ActivateAudioConcurrency.
*/
HWTEST_F(AudioConcurrencyServiceUnitTest, AudioConcurrencyService_012, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    auto audioConcurrencyService = std::make_shared<AudioConcurrencyService>();

    auto audioCapturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    audioCapturerChangeInfo->capturerInfo.pipeType = PIPE_TYPE_CALL_IN;
    audioCapturerChangeInfos.push_back(audioCapturerChangeInfo);

    std::pair<AudioPipeType, AudioPipeType> pair = std::make_pair(audioCapturerChangeInfo->capturerInfo.pipeType,
        AudioPipeType::PIPE_TYPE_CALL_IN);
    audioConcurrencyService->concurrencyCfgMap_.insert({pair, ConcurrencyAction::CONCEDE_EXISTING});

    audioConcurrencyService->handler_ = std::make_shared<AudioPolicyServerHandler>();
    auto result = audioConcurrencyService->ActivateAudioConcurrency(AudioPipeType::PIPE_TYPE_CALL_IN,
        audioRendererChangeInfos, audioCapturerChangeInfos);
    EXPECT_EQ(result, ERR_CONCEDE_INCOMING_STREAM);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_013.
* @tc.desc  : Test ActivateAudioConcurrency.
*/
HWTEST_F(AudioConcurrencyServiceUnitTest, AudioConcurrencyService_013, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    auto audioConcurrencyService = std::make_shared<AudioConcurrencyService>();

    auto audioCapturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    audioCapturerChangeInfo->capturerInfo.pipeType = PIPE_TYPE_LOWLATENCY_OUT;
    audioCapturerChangeInfos.push_back(audioCapturerChangeInfo);

    std::pair<AudioPipeType, AudioPipeType> pair = std::make_pair(audioCapturerChangeInfo->capturerInfo.pipeType,
        AudioPipeType::PIPE_TYPE_CALL_IN);
    audioConcurrencyService->concurrencyCfgMap_.insert({pair, ConcurrencyAction::CONCEDE_EXISTING});

    audioConcurrencyService->handler_ = std::make_shared<AudioPolicyServerHandler>();
    auto result = audioConcurrencyService->ActivateAudioConcurrency(AudioPipeType::PIPE_TYPE_CALL_IN,
        audioRendererChangeInfos, audioCapturerChangeInfos);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_014.
* @tc.desc  : Test ActivateAudioConcurrency.
*/
HWTEST_F(AudioConcurrencyServiceUnitTest, AudioConcurrencyService_014, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    auto audioConcurrencyService = std::make_shared<AudioConcurrencyService>();

    auto audioCapturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    audioCapturerChangeInfo->capturerInfo.pipeType = PIPE_TYPE_CALL_IN;
    audioCapturerChangeInfos.push_back(audioCapturerChangeInfo);

    std::pair<AudioPipeType, AudioPipeType> pair = std::make_pair(audioCapturerChangeInfo->capturerInfo.pipeType,
        AudioPipeType::PIPE_TYPE_LOWLATENCY_OUT);
    audioConcurrencyService->concurrencyCfgMap_.insert({pair, ConcurrencyAction::CONCEDE_EXISTING});

    audioConcurrencyService->handler_ = std::make_shared<AudioPolicyServerHandler>();
    auto result = audioConcurrencyService->ActivateAudioConcurrency(AudioPipeType::PIPE_TYPE_LOWLATENCY_OUT,
        audioRendererChangeInfos, audioCapturerChangeInfos);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_015.
* @tc.desc  : Test ActivateAudioConcurrency.
*/
HWTEST_F(AudioConcurrencyServiceUnitTest, AudioConcurrencyService_015, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    auto audioConcurrencyService = std::make_shared<AudioConcurrencyService>();

    auto audioCapturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    audioCapturerChangeInfo->capturerInfo.pipeType = PIPE_TYPE_LOWLATENCY_OUT;
    audioCapturerChangeInfos.push_back(audioCapturerChangeInfo);

    std::pair<AudioPipeType, AudioPipeType> pair = std::make_pair(audioCapturerChangeInfo->capturerInfo.pipeType,
        AudioPipeType::PIPE_TYPE_LOWLATENCY_OUT);
    audioConcurrencyService->concurrencyCfgMap_.insert({pair, ConcurrencyAction::CONCEDE_EXISTING});

    audioConcurrencyService->handler_ = std::make_shared<AudioPolicyServerHandler>();
    auto result = audioConcurrencyService->ActivateAudioConcurrency(AudioPipeType::PIPE_TYPE_LOWLATENCY_OUT,
        audioRendererChangeInfos, audioCapturerChangeInfos);
    EXPECT_EQ(result, SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS