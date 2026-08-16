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

#include "opensles_obj_aiut.h"
#include "aiut_log.h"
#include "common.h"

using namespace testing::ext;

namespace {
constexpr int32_t TEST_MALLOC_SIZE = 64;
}

namespace OHOS {
namespace AudioStandard {

void OpenslesObjAiut::SetUpTestCase(void) {}
void OpenslesObjAiut::TearDownTestCase(void) {}
void OpenslesObjAiut::SetUp(void) {}
void OpenslesObjAiut::TearDown(void) {}

// ==================== EngineDestory Tests ====================

/**
 * @tc.name  : Test EngineDestory with valid self pointer.
 * @tc.number: EngineDestory_ValidSelf
 * @tc.desc  : Test EngineDestory interface. Frees memory and returns SL_RESULT_SUCCESS.
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OpenslesObjAiut, EngineDestory_ValidSelf, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    void *self = malloc(TEST_MALLOC_SIZE);
    AIUT_ASSERT_NE_INFO_LOG(self, nullptr, "malloc failed");

    SLresult result = EngineDestory(self);
    AIUT_EXPECT_EQ_INFO_LOG(result, SL_RESULT_SUCCESS, "expected SL_RESULT_SUCCESS, got %d", result);
}

/**
 * @tc.name  : Test EngineDestory with null self pointer.
 * @tc.number: EngineDestory_NullSelf
 * @tc.desc  : Test EngineDestory interface. Returns SL_RESULT_SUCCESS when self is null.
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OpenslesObjAiut, EngineDestory_NullSelf, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    SLresult result = EngineDestory(nullptr);
    AIUT_EXPECT_EQ_INFO_LOG(result, SL_RESULT_SUCCESS, "expected SL_RESULT_SUCCESS, got %d", result);
}

// ==================== OutputMixDestroy Tests ====================

/**
 * @tc.name  : Test OutputMixDestroy with valid self pointer.
 * @tc.number: OutputMixDestroy_ValidSelf
 * @tc.desc  : Test OutputMixDestroy interface. Frees memory and returns SL_RESULT_SUCCESS.
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OpenslesObjAiut, OutputMixDestroy_ValidSelf, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    void *self = malloc(TEST_MALLOC_SIZE);
    AIUT_ASSERT_NE_INFO_LOG(self, nullptr, "malloc failed");

    SLresult result = OutputMixDestroy(self);
    AIUT_EXPECT_EQ_INFO_LOG(result, SL_RESULT_SUCCESS, "expected SL_RESULT_SUCCESS, got %d", result);
}

/**
 * @tc.name  : Test OutputMixDestroy with null self pointer.
 * @tc.number: OutputMixDestroy_NullSelf
 * @tc.desc  : Test OutputMixDestroy interface. Returns SL_RESULT_SUCCESS when self is null.
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OpenslesObjAiut, OutputMixDestroy_NullSelf, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    SLresult result = OutputMixDestroy(nullptr);
    AIUT_EXPECT_EQ_INFO_LOG(result, SL_RESULT_SUCCESS, "expected SL_RESULT_SUCCESS, got %d", result);
}

} // namespace AudioStandard
} // namespace OHOS