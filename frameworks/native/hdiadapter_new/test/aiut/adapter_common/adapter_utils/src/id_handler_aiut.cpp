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

#include "id_handler_aiut.h"
#include "aiut_log.h"
#include "util/id_handler.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

void IdHandlerAiut::SetUpTestCase(void) {}
void IdHandlerAiut::TearDownTestCase(void) {}
void IdHandlerAiut::SetUp(void) {}
void IdHandlerAiut::TearDown(void) {}

// ==================== GetInstance Tests ====================

/**
 * @tc.name  : Test IdHandler GetInstance
 * @tc.number: IdHandler_GetInstance_001
 * @tc.desc  : Test GetInstance returns valid singleton.
 */
HWTEST_F(IdHandlerAiut, IdHandler_GetInstance_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &handler = IdHandler::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(&handler, nullptr, "IdHandler instance should not be null");
}

// ==================== GetId Tests ====================

/**
 * @tc.name  : Test IdHandler GetId for primary render
 * @tc.number: IdHandler_GetId_001
 * @tc.desc  : Test GetId returns valid id for primary render.
 */
HWTEST_F(IdHandlerAiut, IdHandler_GetId_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &handler = IdHandler::GetInstance();
    uint32_t id = handler.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY);
    AIUT_EXPECT_NE_INFO_LOG(id, 0u, "GetId should return non-zero id");
}

/**
 * @tc.name  : Test IdHandler GetId for primary capture
 * @tc.number: IdHandler_GetId_002
 * @tc.desc  : Test GetId returns valid id for primary capture.
 */
HWTEST_F(IdHandlerAiut, IdHandler_GetId_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &handler = IdHandler::GetInstance();
    uint32_t id = handler.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY);
    AIUT_EXPECT_NE_INFO_LOG(id, 0u, "GetId should return non-zero id");
}

// ==================== Parse Tests ====================

/**
 * @tc.name  : Test IdHandler ParseBase
 * @tc.number: IdHandler_ParseBase_001
 * @tc.desc  : Test ParseBase returns correct base type.
 */
HWTEST_F(IdHandlerAiut, IdHandler_ParseBase_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &handler = IdHandler::GetInstance();
    uint32_t id = handler.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY);
    uint32_t base = handler.ParseBase(id);
    AIUT_EXPECT_EQ_INFO_LOG(base, HDI_ID_BASE_RENDER, "ParseBase should return correct base");
}

/**
 * @tc.name  : Test IdHandler ParseType
 * @tc.number: IdHandler_ParseType_001
 * @tc.desc  : Test ParseType returns correct type.
 */
HWTEST_F(IdHandlerAiut, IdHandler_ParseType_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &handler = IdHandler::GetInstance();
    uint32_t id = handler.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY);
    uint32_t type = handler.ParseType(id);
    AIUT_EXPECT_EQ_INFO_LOG(type, HDI_ID_TYPE_PRIMARY, "ParseType should return correct type");
}

// ==================== CheckId Tests ====================

/**
 * @tc.name  : Test IdHandler CheckId for valid render id
 * @tc.number: IdHandler_CheckId_001
 * @tc.desc  : Test CheckId returns true for valid render id.
 */
HWTEST_F(IdHandlerAiut, IdHandler_CheckId_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &handler = IdHandler::GetInstance();
    uint32_t id = handler.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY);
    bool result = handler.CheckId(id, HDI_ID_BASE_RENDER);
    AIUT_EXPECT_TRUE_INFO_LOG(result, "CheckId should return true for valid render id");
}

/**
 * @tc.name  : Test IdHandler CheckId for mismatched base
 * @tc.number: IdHandler_CheckId_002
 * @tc.desc  : Test CheckId returns false for id with wrong base.
 */
HWTEST_F(IdHandlerAiut, IdHandler_CheckId_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &handler = IdHandler::GetInstance();
    uint32_t id = handler.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY);
    bool result = handler.CheckId(id, HDI_ID_BASE_CAPTURE);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "CheckId should return false for mismatched base");
}

// ==================== DecRefCount / IncRefCount Tests ====================

/**
 * @tc.name  : Test IdHandler ref count lifecycle
 * @tc.number: IdHandler_RefCount_001
 * @tc.desc  : Test IncRefCount and DecRefCount do not crash.
 */
HWTEST_F(IdHandlerAiut, IdHandler_RefCount_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &handler = IdHandler::GetInstance();
    uint32_t id = handler.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY);
    handler.IncInfoIdUseCount(id);
    handler.DecInfoIdUseCount(id);
}

} // namespace AudioStandard
} // namespace OHOS
