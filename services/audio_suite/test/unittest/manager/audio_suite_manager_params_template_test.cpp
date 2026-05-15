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

#include <string>
#include <thread>
#include <chrono>
#include <cstdio>
#include <unistd.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "audio_errors.h"
#include "audio_suite_manager_private.h"
#include "audio_suite_manager_callback.h"
#include "audio_suite_serializer.h"
#include "audio_suite_unittest_tools.h"
#include "i_audio_suite_engine.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

namespace {

class MockIAudioSuiteEngine : public IAudioSuiteEngine {
public:
    MockIAudioSuiteEngine() = default;
    ~MockIAudioSuiteEngine() = default;

    MOCK_METHOD(int32_t, Init, ());
    MOCK_METHOD(int32_t, DeInit, ());
    MOCK_METHOD(int32_t, CreatePipeline, (PipelineWorkMode workMode));
    MOCK_METHOD(int32_t, DestroyPipeline, (uint32_t pipelineId));
    MOCK_METHOD(int32_t, StartPipeline, (uint32_t pipelineId));
    MOCK_METHOD(int32_t, StopPipeline, (uint32_t pipelineId));
    MOCK_METHOD(int32_t, GetPipelineState, (uint32_t pipelineId));
    MOCK_METHOD(int32_t, CreateNode, (uint32_t pipelineId, AudioNodeBuilder& builder));
    MOCK_METHOD(int32_t, DestroyNode, (uint32_t nodeId));
    MOCK_METHOD(int32_t, BypassEffectNode, (uint32_t nodeId, bool bypass));
    MOCK_METHOD(int32_t, GetNodeBypassStatus, (uint32_t nodeId));
    MOCK_METHOD(int32_t, SetAudioFormat, (uint32_t nodeId, AudioFormat audioFormat));
    MOCK_METHOD(int32_t, SetRequestDataCallback, (uint32_t nodeId,
        std::shared_ptr<InputNodeRequestDataCallBack> callback));
    MOCK_METHOD(int32_t, ConnectNodes, (uint32_t srcNodeId, uint32_t destNodeId));
    MOCK_METHOD(int32_t, DisConnectNodes, (uint32_t srcNodeId, uint32_t destNodeId));
    MOCK_METHOD(int32_t, RenderFrame, (uint32_t pipelineId, uint8_t *audioData,
        int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag));
    MOCK_METHOD(int32_t, MultiRenderFrame, (uint32_t pipelineId, AudioDataArray *audioDataArray,
        int32_t *responseSize, bool *finishedFlag));
    MOCK_METHOD(int32_t, SetOptions, (uint32_t nodeId, std::string name, std::string value));
    MOCK_METHOD(int32_t, GetOptions, (uint32_t nodeId, std::string name, std::string& value));
    MOCK_METHOD(int32_t, PrintInfo, (uint32_t pipelineId, int32_t fd));
};

class AudioSuiteManagerParamsTemplateUnitTest : public testing::Test {
public:
    void SetUp() override
    {
        if (!AllNodeTypesSupported()) {
            GTEST_SKIP() << "not support all node types, skip this test";
        }
        manager_.Init();
    }
    
    void TearDown() override
    {
        manager_.DeInit();
    }

protected:
    void SetMockEngine(std::shared_ptr<MockIAudioSuiteEngine> mockEngine)
    {
        manager_.suiteEngine_ = mockEngine;
    }

    AudioSuiteManager manager_;
};

HWTEST_F(AudioSuiteManagerParamsTemplateUnitTest, SetParamsTemplate_EngineNull, TestSize.Level0)
{
    AudioSpaceRenderPositionParams params;
    params.x = 1.0f;
    params.y = 2.0f;
    params.z = 3.0f;

    manager_.suiteEngine_ = nullptr;
    int32_t ret = manager_.SetSpaceRenderPositionParams(1, params);
    EXPECT_EQ(ret, ERR_AUDIO_SUITE_NODE_NOT_EXIST);
}

HWTEST_F(AudioSuiteManagerParamsTemplateUnitTest, SetParamsTemplate_SetOptionsFailed, TestSize.Level0)
{
    auto mockEngine = std::make_shared<MockIAudioSuiteEngine>();
    SetMockEngine(mockEngine);

    AudioSpaceRenderPositionParams params;
    params.x = 1.0f;
    params.y = 2.0f;
    params.z = 3.0f;

    EXPECT_CALL(*mockEngine, SetOptions(_, _, _))
        .WillOnce(Return(ERR_ILLEGAL_STATE));

    int32_t ret = manager_.SetSpaceRenderPositionParams(1, params);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

HWTEST_F(AudioSuiteManagerParamsTemplateUnitTest, SetParamsTemplate_CallbackFailed, TestSize.Level0)
{
    auto mockEngine = std::make_shared<MockIAudioSuiteEngine>();
    SetMockEngine(mockEngine);

    AudioSpaceRenderPositionParams params;
    params.x = 1.0f;
    params.y = 2.0f;
    params.z = 3.0f;

    EXPECT_CALL(*mockEngine, SetOptions(_, _, _))
        .WillOnce(Return(SUCCESS));

    manager_.setOptionsResult_ = ERR_OPERATION_FAILED;

    std::thread notifyThread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        manager_.isFinishSetOptions_ = true;
        manager_.callbackCV_.notify_one();
    });

    int32_t ret = manager_.SetSpaceRenderPositionParams(1, params);
    notifyThread.join();
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(AudioSuiteManagerParamsTemplateUnitTest, GetParamsTemplate_EngineNull, TestSize.Level0)
{
    AudioSpaceRenderPositionParams params;

    manager_.suiteEngine_ = nullptr;
    int32_t ret = manager_.GetSpaceRenderPositionParams(1, params);
    EXPECT_EQ(ret, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST);
}

HWTEST_F(AudioSuiteManagerParamsTemplateUnitTest, GetParamsTemplate_GetOptionsFailed, TestSize.Level0)
{
    auto mockEngine = std::make_shared<MockIAudioSuiteEngine>();
    SetMockEngine(mockEngine);

    AudioSpaceRenderPositionParams params;

    EXPECT_CALL(*mockEngine, GetOptions(_, _, _))
        .WillOnce(Return(ERR_ILLEGAL_STATE));

    int32_t ret = manager_.GetSpaceRenderPositionParams(1, params);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

HWTEST_F(AudioSuiteManagerParamsTemplateUnitTest, GetParamsTemplate_CallbackFailed, TestSize.Level0)
{
    auto mockEngine = std::make_shared<MockIAudioSuiteEngine>();
    SetMockEngine(mockEngine);

    AudioSpaceRenderPositionParams params;

    EXPECT_CALL(*mockEngine, GetOptions(_, _, _))
        .WillOnce(Return(SUCCESS));

    manager_.getOptionsResult_ = ERR_OPERATION_FAILED;

    std::thread notifyThread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        manager_.isFinishGetOptions_ = true;
        manager_.callbackCV_.notify_one();
    });

    int32_t ret = manager_.GetSpaceRenderPositionParams(1, params);
    notifyThread.join();
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(AudioSuiteManagerParamsTemplateUnitTest, GetParamsTemplate_DeserializeFailed, TestSize.Level0)
{
    auto mockEngine = std::make_shared<MockIAudioSuiteEngine>();
    SetMockEngine(mockEngine);

    AudioSpaceRenderPositionParams params;

    EXPECT_CALL(*mockEngine, GetOptions(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(std::string("invalid")), Return(SUCCESS)));

    manager_.getOptionsResult_ = SUCCESS;

    std::thread notifyThread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        manager_.isFinishGetOptions_ = true;
        manager_.callbackCV_.notify_one();
    });

    int32_t ret = manager_.GetSpaceRenderPositionParams(1, params);
    notifyThread.join();
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(AudioSuiteManagerParamsTemplateUnitTest, SetParamsTemplate_Timeout, TestSize.Level0)
{
    auto mockEngine = std::make_shared<MockIAudioSuiteEngine>();
    SetMockEngine(mockEngine);

    AudioSpaceRenderPositionParams params;
    params.x = 1.0f;
    params.y = 2.0f;
    params.z = 3.0f;

    EXPECT_CALL(*mockEngine, SetOptions(_, _, _))
        .WillOnce(Return(SUCCESS));

    int32_t ret = manager_.SetSpaceRenderPositionParams(1, params);
    EXPECT_EQ(ret, ERR_AUDIO_SUITE_TIMEOUT);
}

HWTEST_F(AudioSuiteManagerParamsTemplateUnitTest, GetParamsTemplate_Timeout, TestSize.Level0)
{
    auto mockEngine = std::make_shared<MockIAudioSuiteEngine>();
    SetMockEngine(mockEngine);

    AudioSpaceRenderPositionParams params;

    EXPECT_CALL(*mockEngine, GetOptions(_, _, _))
        .WillOnce(Return(SUCCESS));

    int32_t ret = manager_.GetSpaceRenderPositionParams(1, params);
    EXPECT_EQ(ret, ERR_AUDIO_SUITE_TIMEOUT);
}

HWTEST_F(AudioSuiteManagerParamsTemplateUnitTest, SetRotationParamsTemplate_Success, TestSize.Level0)
{
    auto mockEngine = std::make_shared<MockIAudioSuiteEngine>();
    SetMockEngine(mockEngine);

    AudioSpaceRenderRotationParams params;
    params.x = 1.0f;
    params.y = 2.0f;
    params.z = 3.0f;
    params.surroundTime = 100;
    params.surroundDirection = AudioSurroundDirection::SPACE_RENDER_CW;

    EXPECT_CALL(*mockEngine, SetOptions(_, _, _))
        .WillOnce(Return(SUCCESS));

    manager_.setOptionsResult_ = SUCCESS;

    std::thread notifyThread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        manager_.isFinishSetOptions_ = true;
        manager_.callbackCV_.notify_one();
    });

    int32_t ret = manager_.SetSpaceRenderRotationParams(1, params);
    notifyThread.join();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteManagerParamsTemplateUnitTest, GetRotationParamsTemplate_Success, TestSize.Level0)
{
    auto mockEngine = std::make_shared<MockIAudioSuiteEngine>();
    SetMockEngine(mockEngine);

    AudioSpaceRenderRotationParams params;

    EXPECT_CALL(*mockEngine, GetOptions(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(std::string("1.0,2.0,3.0,100,1")), Return(SUCCESS)));

    manager_.getOptionsResult_ = SUCCESS;

    std::thread notifyThread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        manager_.isFinishGetOptions_ = true;
        manager_.callbackCV_.notify_one();
    });

    int32_t ret = manager_.GetSpaceRenderRotationParams(1, params);
    notifyThread.join();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FLOAT_EQ(params.x, 1.0f);
    EXPECT_FLOAT_EQ(params.y, 2.0f);
    EXPECT_FLOAT_EQ(params.z, 3.0f);
    EXPECT_EQ(params.surroundTime, 100);
    EXPECT_EQ(params.surroundDirection, AudioSurroundDirection::SPACE_RENDER_CW);
}

HWTEST_F(AudioSuiteManagerParamsTemplateUnitTest, SetExtensionParamsTemplate_Success, TestSize.Level0)
{
    auto mockEngine = std::make_shared<MockIAudioSuiteEngine>();
    SetMockEngine(mockEngine);

    AudioSpaceRenderExtensionParams params;
    params.extRadius = 1.5f;
    params.extAngle = 45;

    EXPECT_CALL(*mockEngine, SetOptions(_, _, _))
        .WillOnce(Return(SUCCESS));

    manager_.setOptionsResult_ = SUCCESS;

    std::thread notifyThread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        manager_.isFinishSetOptions_ = true;
        manager_.callbackCV_.notify_one();
    });

    int32_t ret = manager_.SetSpaceRenderExtensionParams(1, params);
    notifyThread.join();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteManagerParamsTemplateUnitTest, GetExtensionParamsTemplate_Success, TestSize.Level0)
{
    auto mockEngine = std::make_shared<MockIAudioSuiteEngine>();
    SetMockEngine(mockEngine);

    AudioSpaceRenderExtensionParams params;

    EXPECT_CALL(*mockEngine, GetOptions(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(std::string("1.5,45")), Return(SUCCESS)));

    manager_.getOptionsResult_ = SUCCESS;

    std::thread notifyThread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        manager_.isFinishGetOptions_ = true;
        manager_.callbackCV_.notify_one();
    });

    int32_t ret = manager_.GetSpaceRenderExtensionParams(1, params);
    notifyThread.join();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FLOAT_EQ(params.extRadius, 1.5f);
    EXPECT_EQ(params.extAngle, 45);
}

HWTEST_F(AudioSuiteManagerParamsTemplateUnitTest, SetTempoAndPitchTemplate_Success, TestSize.Level0)
{
    auto mockEngine = std::make_shared<MockIAudioSuiteEngine>();
    SetMockEngine(mockEngine);

    EXPECT_CALL(*mockEngine, SetOptions(_, _, _))
        .WillOnce(Return(SUCCESS));

    manager_.setOptionsResult_ = SUCCESS;

    std::thread notifyThread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        manager_.isFinishSetOptions_ = true;
        manager_.callbackCV_.notify_one();
    });

    float speed = 1.5f;
    float pitch = 2.0f;
    int32_t ret = manager_.SetTempoAndPitch(1, speed, pitch);
    notifyThread.join();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteManagerParamsTemplateUnitTest, GetTempoAndPitchTemplate_Success, TestSize.Level0)
{
    auto mockEngine = std::make_shared<MockIAudioSuiteEngine>();
    SetMockEngine(mockEngine);

    EXPECT_CALL(*mockEngine, GetOptions(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(std::string("1.5,2.0")), Return(SUCCESS)));

    manager_.getOptionsResult_ = SUCCESS;

    std::thread notifyThread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        manager_.isFinishGetOptions_ = true;
        manager_.callbackCV_.notify_one();
    });

    float speed = 0.0f;
    float pitch = 0.0f;
    int32_t ret = manager_.GetTempoAndPitch(1, speed, pitch);
    notifyThread.join();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FLOAT_EQ(speed, 1.5f);
    EXPECT_FLOAT_EQ(pitch, 2.0f);
}

HWTEST_F(AudioSuiteManagerParamsTemplateUnitTest, SetPureVoiceChangeTemplate_Success, TestSize.Level0)
{
    auto mockEngine = std::make_shared<MockIAudioSuiteEngine>();
    SetMockEngine(mockEngine);

    AudioPureVoiceChangeOption option;
    option.optionGender = AudioPureVoiceChangeGenderOption::PURE_VOICE_CHANGE_FEMALE;
    option.optionType = AudioPureVoiceChangeType::PURE_VOICE_CHANGE_TYPE_CUTE;
    option.pitch = 1.5f;

    EXPECT_CALL(*mockEngine, SetOptions(_, _, _))
        .WillOnce(Return(SUCCESS));

    manager_.setOptionsResult_ = SUCCESS;

    std::thread notifyThread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        manager_.isFinishSetOptions_ = true;
        manager_.callbackCV_.notify_one();
    });

    int32_t ret = manager_.SetPureVoiceChangeOption(1, option);
    notifyThread.join();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteManagerParamsTemplateUnitTest, GetPureVoiceChangeTemplate_Success, TestSize.Level0)
{
    auto mockEngine = std::make_shared<MockIAudioSuiteEngine>();
    SetMockEngine(mockEngine);

    AudioPureVoiceChangeOption option;

    EXPECT_CALL(*mockEngine, GetOptions(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(std::string("1,2,1.5")), Return(SUCCESS)));

    manager_.getOptionsResult_ = SUCCESS;

    std::thread notifyThread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        manager_.isFinishGetOptions_ = true;
        manager_.callbackCV_.notify_one();
    });

    int32_t ret = manager_.GetPureVoiceChangeOption(1, option);
    notifyThread.join();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(option.optionGender, AudioPureVoiceChangeGenderOption::PURE_VOICE_CHANGE_FEMALE);
    EXPECT_EQ(option.optionType, AudioPureVoiceChangeType::PURE_VOICE_CHANGE_TYPE_CUTE);
    EXPECT_FLOAT_EQ(option.pitch, 1.5f);
}

HWTEST_F(AudioSuiteManagerParamsTemplateUnitTest, SetGeneralVoiceChangeTemplate_Success, TestSize.Level0)
{
    auto mockEngine = std::make_shared<MockIAudioSuiteEngine>();
    SetMockEngine(mockEngine);

    AudioGeneralVoiceChangeType type = AudioGeneralVoiceChangeType::GENERAL_VOICE_CHANGE_TYPE_CUTE;

    EXPECT_CALL(*mockEngine, SetOptions(_, _, _))
        .WillOnce(Return(SUCCESS));

    manager_.setOptionsResult_ = SUCCESS;

    std::thread notifyThread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        manager_.isFinishSetOptions_ = true;
        manager_.callbackCV_.notify_one();
    });

    int32_t ret = manager_.SetGeneralVoiceChangeType(1, type);
    notifyThread.join();
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioSuiteManagerParamsTemplateUnitTest, GetGeneralVoiceChangeTemplate_Success, TestSize.Level0)
{
    auto mockEngine = std::make_shared<MockIAudioSuiteEngine>();
    SetMockEngine(mockEngine);

    AudioGeneralVoiceChangeType type;

    EXPECT_CALL(*mockEngine, GetOptions(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(std::string("1")), Return(SUCCESS)));

    manager_.getOptionsResult_ = SUCCESS;

    std::thread notifyThread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        manager_.isFinishGetOptions_ = true;
        manager_.callbackCV_.notify_one();
    });

    int32_t ret = manager_.GetGeneralVoiceChangeType(1, type);
    notifyThread.join();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(type, AudioGeneralVoiceChangeType::GENERAL_VOICE_CHANGE_TYPE_CUTE);
}

HWTEST_F(AudioSuiteManagerParamsTemplateUnitTest, SetParamsTemplate_ConcurrentAccess, TestSize.Level1)
{
    auto mockEngine = std::make_shared<MockIAudioSuiteEngine>();
    SetMockEngine(mockEngine);

    AudioSpaceRenderPositionParams params;
    params.x = 1.0f;
    params.y = 2.0f;
    params.z = 3.0f;

    EXPECT_CALL(*mockEngine, SetOptions(_, _, _))
        .Times(2)
        .WillRepeatedly(Return(SUCCESS));

    manager_.setOptionsResult_ = SUCCESS;

    std::thread thread1([this, params]() {
        manager_.SetSpaceRenderPositionParams(1, params);
    });

    std::thread thread2([this, params]() {
        manager_.SetSpaceRenderPositionParams(2, params);
    });

    std::thread notifyThread([this]() {
        for (int i = 0; i < 2; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            {
                std::lock_guard<std::mutex> lock(manager_.callbackMutex_);
                manager_.isFinishSetOptions_ = true;
            }
            manager_.callbackCV_.notify_all();
        }
    });

    thread1.join();
    thread2.join();
    notifyThread.join();
}

}