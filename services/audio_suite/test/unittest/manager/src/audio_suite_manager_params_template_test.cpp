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
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "audio_errors.h"
#include "audio_suite_pipeline_impl.h"
#include "audio_suite_base.h"
#include "audio_suite_unittest_tools.h"
#include "audio_suite_manager_test_common.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;

namespace {

struct PipelineNodeHandle {
    std::shared_ptr<AudioSuitePipeline> pipeline_;
    uint32_t nodeId_ = INVALID_NODE_ID;
};

class AudioSuiteParamsTemplateUnitTest : public testing::Test {
public:
    void SetUp() override
    {
        if (!AllNodeTypesSupported()) {
            GTEST_SKIP() << "not support all node types, skip this test";
        }
    }

    void TearDown() override
    {
        pipelines_.clear();
    }

protected:
    // Helper to create a pipeline with input, effect, output nodes
    // Returns the effect nodeId via the handle
    PipelineNodeHandle CreateTestPipeline(AudioNodeType effectType)
    {
        PipelineNodeHandle handle;
        EXPECT_EQ(AudioSuitePipelineImpl::Create(PIPELINE_EDIT_MODE, handle.pipeline_), SUCCESS);
        EXPECT_NE(handle.pipeline_, nullptr);

        AudioNodeBuilder inputBuilder;
        inputBuilder.nodeType = NODE_TYPE_INPUT;
        uint32_t inputNodeId = INVALID_NODE_ID;
        EXPECT_EQ(handle.pipeline_->CreateNode(inputBuilder, inputNodeId), SUCCESS);

        AudioNodeBuilder effectBuilder;
        effectBuilder.nodeType = effectType;
        EXPECT_EQ(handle.pipeline_->CreateNode(effectBuilder, handle.nodeId_), SUCCESS);

        AudioNodeBuilder outputBuilder;
        outputBuilder.nodeType = NODE_TYPE_OUTPUT;
        uint32_t outputNodeId = INVALID_NODE_ID;
        EXPECT_EQ(handle.pipeline_->CreateNode(outputBuilder, outputNodeId), SUCCESS);

        EXPECT_EQ(handle.pipeline_->ConnectNodes(inputNodeId, handle.nodeId_), SUCCESS);
        EXPECT_EQ(handle.pipeline_->ConnectNodes(handle.nodeId_, outputNodeId), SUCCESS);

        pipelines_.push_back(handle.pipeline_);
        return handle;
    }

    std::vector<std::shared_ptr<AudioSuitePipeline>> pipelines_;
};

// Test SetSpaceRenderPositionParams with valid params
HWTEST_F(AudioSuiteParamsTemplateUnitTest, SetSpaceRenderPositionParamsSuccess, TestSize.Level0)
{
    auto handle = CreateTestPipeline(NODE_TYPE_SPACE_RENDER);

    AudioSpaceRenderPositionParams params;
    params.x = 1.0f;
    params.y = 2.0f;
    params.z = 3.0f;
    int32_t ret = handle.pipeline_->SetSpaceRenderPositionParams(handle.nodeId_, params);
    EXPECT_EQ(ret, SUCCESS);
}

// Test GetSpaceRenderPositionParams
HWTEST_F(AudioSuiteParamsTemplateUnitTest, GetSpaceRenderPositionParamsSuccess, TestSize.Level0)
{
    auto handle = CreateTestPipeline(NODE_TYPE_SPACE_RENDER);

    AudioSpaceRenderPositionParams params;
    params.x = 1.0f;
    params.y = 2.0f;
    params.z = 3.0f;
    EXPECT_EQ(handle.pipeline_->SetSpaceRenderPositionParams(handle.nodeId_, params), SUCCESS);

    AudioSpaceRenderPositionParams getResult;
    int32_t ret = handle.pipeline_->GetSpaceRenderPositionParams(handle.nodeId_, getResult);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FLOAT_EQ(getResult.x, 1.0f);
    EXPECT_FLOAT_EQ(getResult.y, 2.0f);
    EXPECT_FLOAT_EQ(getResult.z, 3.0f);
}

// Test SetSpaceRenderPositionParams with non-existent node
HWTEST_F(AudioSuiteParamsTemplateUnitTest, SetSpaceRenderPositionParamsNodeNotExist, TestSize.Level0)
{
    auto handle = CreateTestPipeline(NODE_TYPE_SPACE_RENDER);

    AudioSpaceRenderPositionParams params;
    params.x = 1.0f;
    params.y = 2.0f;
    params.z = 3.0f;
    int32_t ret = handle.pipeline_->SetSpaceRenderPositionParams(999, params);
    EXPECT_NE(ret, SUCCESS);
}

// Test SetSpaceRenderRotationParams
HWTEST_F(AudioSuiteParamsTemplateUnitTest, SetSpaceRenderRotationParamsSuccess, TestSize.Level0)
{
    auto handle = CreateTestPipeline(NODE_TYPE_SPACE_RENDER);

    AudioSpaceRenderRotationParams params;
    params.x = 1.0f;
    params.y = 2.0f;
    params.z = 3.0f;
    params.surroundTime = 5.0f;
    params.surroundDirection = AudioSurroundDirection::SPACE_RENDER_CW;
    int32_t ret = handle.pipeline_->SetSpaceRenderRotationParams(handle.nodeId_, params);
    EXPECT_EQ(ret, SUCCESS);
}

// Test GetSpaceRenderRotationParams
HWTEST_F(AudioSuiteParamsTemplateUnitTest, GetSpaceRenderRotationParamsSuccess, TestSize.Level0)
{
    auto handle = CreateTestPipeline(NODE_TYPE_SPACE_RENDER);

    AudioSpaceRenderRotationParams params;
    params.x = 1.0f;
    params.y = 2.0f;
    params.z = 3.0f;
    params.surroundTime = 5.0f;
    params.surroundDirection = AudioSurroundDirection::SPACE_RENDER_CW;
    EXPECT_EQ(handle.pipeline_->SetSpaceRenderRotationParams(handle.nodeId_, params), SUCCESS);

    AudioSpaceRenderRotationParams getResult;
    int32_t ret = handle.pipeline_->GetSpaceRenderRotationParams(handle.nodeId_, getResult);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FLOAT_EQ(getResult.x, 1.0f);
    EXPECT_FLOAT_EQ(getResult.y, 2.0f);
    EXPECT_FLOAT_EQ(getResult.z, 3.0f);
}

// Test SetSpaceRenderExtensionParams
HWTEST_F(AudioSuiteParamsTemplateUnitTest, SetSpaceRenderExtensionParamsSuccess, TestSize.Level0)
{
    auto handle = CreateTestPipeline(NODE_TYPE_SPACE_RENDER);

    AudioSpaceRenderExtensionParams params;
    params.extRadius = 1.5f;
    params.extAngle = 45;
    int32_t ret = handle.pipeline_->SetSpaceRenderExtensionParams(handle.nodeId_, params);
    EXPECT_EQ(ret, SUCCESS);
}

// Test GetSpaceRenderExtensionParams
HWTEST_F(AudioSuiteParamsTemplateUnitTest, GetSpaceRenderExtensionParamsSuccess, TestSize.Level0)
{
    auto handle = CreateTestPipeline(NODE_TYPE_SPACE_RENDER);

    AudioSpaceRenderExtensionParams params;
    params.extRadius = 1.5f;
    params.extAngle = 45;
    EXPECT_EQ(handle.pipeline_->SetSpaceRenderExtensionParams(handle.nodeId_, params), SUCCESS);

    AudioSpaceRenderExtensionParams getResult;
    int32_t ret = handle.pipeline_->GetSpaceRenderExtensionParams(handle.nodeId_, getResult);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FLOAT_EQ(getResult.extRadius, 1.5f);
    EXPECT_EQ(getResult.extAngle, 45);
}

// Test SetTempoAndPitch
HWTEST_F(AudioSuiteParamsTemplateUnitTest, SetTempoAndPitchSuccess, TestSize.Level0)
{
    auto handle = CreateTestPipeline(NODE_TYPE_TEMPO_PITCH);

    float speed = 1.5f;
    float pitch = 2.0f;
    int32_t ret = handle.pipeline_->SetTempoAndPitch(handle.nodeId_, speed, pitch);
    EXPECT_EQ(ret, SUCCESS);
}

// Test GetTempoAndPitch
HWTEST_F(AudioSuiteParamsTemplateUnitTest, GetTempoAndPitchSuccess, TestSize.Level0)
{
    auto handle = CreateTestPipeline(NODE_TYPE_TEMPO_PITCH);

    float speed = 1.5f;
    float pitch = 2.0f;
    EXPECT_EQ(handle.pipeline_->SetTempoAndPitch(handle.nodeId_, speed, pitch), SUCCESS);

    float getResultSpeed = 0.0f;
    float getResultPitch = 0.0f;
    int32_t ret = handle.pipeline_->GetTempoAndPitch(handle.nodeId_, getResultSpeed, getResultPitch);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FLOAT_EQ(getResultSpeed, 1.5f);
    EXPECT_FLOAT_EQ(getResultPitch, 2.0f);
}

// Test SetPureVoiceChangeOption
HWTEST_F(AudioSuiteParamsTemplateUnitTest, SetPureVoiceChangeOptionSuccess, TestSize.Level0)
{
    auto handle = CreateTestPipeline(NODE_TYPE_PURE_VOICE_CHANGE);

    AudioPureVoiceChangeOption option;
    option.optionGender = AudioPureVoiceChangeGenderOption::PURE_VOICE_CHANGE_FEMALE;
    option.optionType = AudioPureVoiceChangeType::PURE_VOICE_CHANGE_TYPE_CUTE;
    option.pitch = 1.5f;
    int32_t ret = handle.pipeline_->SetPureVoiceChangeOption(handle.nodeId_, option);
    EXPECT_EQ(ret, SUCCESS);
}

// Test GetPureVoiceChangeOption
HWTEST_F(AudioSuiteParamsTemplateUnitTest, GetPureVoiceChangeOptionSuccess, TestSize.Level0)
{
    auto handle = CreateTestPipeline(NODE_TYPE_PURE_VOICE_CHANGE);

    AudioPureVoiceChangeOption option;
    option.optionGender = AudioPureVoiceChangeGenderOption::PURE_VOICE_CHANGE_FEMALE;
    option.optionType = AudioPureVoiceChangeType::PURE_VOICE_CHANGE_TYPE_CUTE;
    option.pitch = 1.5f;
    EXPECT_EQ(handle.pipeline_->SetPureVoiceChangeOption(handle.nodeId_, option), SUCCESS);

    AudioPureVoiceChangeOption getResult;
    int32_t ret = handle.pipeline_->GetPureVoiceChangeOption(handle.nodeId_, getResult);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(getResult.optionGender, AudioPureVoiceChangeGenderOption::PURE_VOICE_CHANGE_FEMALE);
    EXPECT_EQ(getResult.optionType, AudioPureVoiceChangeType::PURE_VOICE_CHANGE_TYPE_CUTE);
    EXPECT_FLOAT_EQ(getResult.pitch, 1.5f);
}

// Test SetGeneralVoiceChangeType
HWTEST_F(AudioSuiteParamsTemplateUnitTest, SetGeneralVoiceChangeTypeSuccess, TestSize.Level0)
{
    auto handle = CreateTestPipeline(NODE_TYPE_GENERAL_VOICE_CHANGE);

    AudioGeneralVoiceChangeType type = AudioGeneralVoiceChangeType::GENERAL_VOICE_CHANGE_TYPE_CUTE;
    int32_t ret = handle.pipeline_->SetGeneralVoiceChangeType(handle.nodeId_, type);
    EXPECT_EQ(ret, SUCCESS);
}

// Test GetGeneralVoiceChangeType
HWTEST_F(AudioSuiteParamsTemplateUnitTest, GetGeneralVoiceChangeTypeSuccess, TestSize.Level0)
{
    auto handle = CreateTestPipeline(NODE_TYPE_GENERAL_VOICE_CHANGE);

    AudioGeneralVoiceChangeType type = AudioGeneralVoiceChangeType::GENERAL_VOICE_CHANGE_TYPE_CUTE;
    EXPECT_EQ(handle.pipeline_->SetGeneralVoiceChangeType(handle.nodeId_, type), SUCCESS);

    AudioGeneralVoiceChangeType getResult;
    int32_t ret = handle.pipeline_->GetGeneralVoiceChangeType(handle.nodeId_, getResult);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(getResult, AudioGeneralVoiceChangeType::GENERAL_VOICE_CHANGE_TYPE_CUTE);
}

// Test SetSpaceRenderPositionParams with invalid params (out of range)
HWTEST_F(AudioSuiteParamsTemplateUnitTest, SetSpaceRenderPositionParamsInvalid, TestSize.Level0)
{
    auto handle = CreateTestPipeline(NODE_TYPE_SPACE_RENDER);

    AudioSpaceRenderPositionParams params;
    params.x = 6.0f;  // out of range (-5.0 to 5.0)
    params.y = 2.0f;
    params.z = 3.0f;
    int32_t ret = handle.pipeline_->SetSpaceRenderPositionParams(handle.nodeId_, params);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

// Test SetTempoAndPitch with invalid params
HWTEST_F(AudioSuiteParamsTemplateUnitTest, SetTempoAndPitchInvalid, TestSize.Level0)
{
    auto handle = CreateTestPipeline(NODE_TYPE_TEMPO_PITCH);

    // speed out of range (0.5~10.0)
    int32_t ret = handle.pipeline_->SetTempoAndPitch(handle.nodeId_, 0.4f, 1.0f);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    // pitch out of range (0.1~5.0)
    ret = handle.pipeline_->SetTempoAndPitch(handle.nodeId_, 1.0f, 0.09f);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

// Test SetPureVoiceChangeOption with invalid pitch
HWTEST_F(AudioSuiteParamsTemplateUnitTest, SetPureVoiceChangeOptionInvalid, TestSize.Level0)
{
    auto handle = CreateTestPipeline(NODE_TYPE_PURE_VOICE_CHANGE);

    AudioPureVoiceChangeOption option;
    option.optionGender = AudioPureVoiceChangeGenderOption::PURE_VOICE_CHANGE_FEMALE;
    option.optionType = AudioPureVoiceChangeType::PURE_VOICE_CHANGE_TYPE_CUTE;
    option.pitch = 0.29f;  // out of range (0.3~3.0 or 0.0)
    int32_t ret = handle.pipeline_->SetPureVoiceChangeOption(handle.nodeId_, option);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

// Test concurrent access to SetParamsTemplate (thread safety)
HWTEST_F(AudioSuiteParamsTemplateUnitTest, SetParamsTemplateConcurrentAccess, TestSize.Level1)
{
    auto handle1 = CreateTestPipeline(NODE_TYPE_SPACE_RENDER);
    auto handle2 = CreateTestPipeline(NODE_TYPE_SPACE_RENDER);

    AudioSpaceRenderPositionParams params1;
    params1.x = 1.0f;
    params1.y = 2.0f;
    params1.z = 3.0f;

    AudioSpaceRenderPositionParams params2;
    params2.x = 4.0f;
    params2.y = 5.0f;
    params2.z = 6.0f;

    auto pipeline1 = handle1.pipeline_;
    auto pipeline2 = handle2.pipeline_;
    uint32_t nodeId1 = handle1.nodeId_;
    uint32_t nodeId2 = handle2.nodeId_;

    std::thread thread1([pipeline1, nodeId1, params1]() {
        pipeline1->SetSpaceRenderPositionParams(nodeId1, params1);
    });

    std::thread thread2([pipeline2, nodeId2, params2]() {
        pipeline2->SetSpaceRenderPositionParams(nodeId2, params2);
    });

    thread1.join();
    thread2.join();

    // Verify both were set correctly — concurrent Set should not interfere with each other
    AudioSpaceRenderPositionParams result1;
    int32_t ret1 = pipeline1->GetSpaceRenderPositionParams(nodeId1, result1);
    if (ret1 == SUCCESS) {
        EXPECT_FLOAT_EQ(result1.x, 1.0f);
    }

    AudioSpaceRenderPositionParams result2;
    int32_t ret2 = pipeline2->GetSpaceRenderPositionParams(nodeId2, result2);
    if (ret2 == SUCCESS) {
        EXPECT_FLOAT_EQ(result2.x, 4.0f);
    }
}

}  // namespace
