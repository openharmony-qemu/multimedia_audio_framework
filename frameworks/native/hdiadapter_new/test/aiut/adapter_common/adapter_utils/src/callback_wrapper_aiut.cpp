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

#include "callback_wrapper_aiut.h"
#include "aiut_log.h"
#include "util/callback_wrapper.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

void CallbackWrapperAiut::SetUpTestCase(void) {}
void CallbackWrapperAiut::TearDownTestCase(void) {}
void CallbackWrapperAiut::SetUp(void) {}
void CallbackWrapperAiut::TearDown(void) {}

// ==================== SinkCallbackWrapper Tests ====================

/**
 * @tc.name  : Test SinkCallbackWrapper construction
 * @tc.number: SinkCallbackWrapper_Constructor_001
 * @tc.desc  : Test SinkCallbackWrapper default constructor creates valid object.
 */
HWTEST_F(CallbackWrapperAiut, SinkCallbackWrapper_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    SinkCallbackWrapper wrapper;
}

/**
 * @tc.name  : Test SinkCallbackWrapper GetCallback with no registered callback
 * @tc.number: SinkCallbackWrapper_GetCallback_001
 * @tc.desc  : Test GetCallback returns nullptr when no callback registered.
 */
HWTEST_F(CallbackWrapperAiut, SinkCallbackWrapper_GetCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    SinkCallbackWrapper wrapper;
    auto cb = wrapper.GetCallback(0, 0);
    AIUT_EXPECT_NULL_INFO_LOG(cb, "GetCallback should return nullptr when no callback registered");
}

/**
 * @tc.name  : Test SinkCallbackWrapper GetRawCallback with no registered callback
 * @tc.number: SinkCallbackWrapper_GetRawCallback_001
 * @tc.desc  : Test GetRawCallback returns nullptr when no callback registered.
 */
HWTEST_F(CallbackWrapperAiut, SinkCallbackWrapper_GetRawCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    SinkCallbackWrapper wrapper;
    auto *cb = wrapper.GetRawCallback(0);
    AIUT_EXPECT_NULL_INFO_LOG(cb, "GetRawCallback should return nullptr when no callback registered");
}

// ==================== SourceCallbackWrapper Tests ====================

/**
 * @tc.name  : Test SourceCallbackWrapper construction
 * @tc.number: SourceCallbackWrapper_Constructor_001
 * @tc.desc  : Test SourceCallbackWrapper default constructor creates valid object.
 */
HWTEST_F(CallbackWrapperAiut, SourceCallbackWrapper_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    SourceCallbackWrapper wrapper;
}

/**
 * @tc.name  : Test SourceCallbackWrapper GetCallback with no registered callback
 * @tc.number: SourceCallbackWrapper_GetCallback_001
 * @tc.desc  : Test GetCallback returns nullptr when no callback registered.
 */
HWTEST_F(CallbackWrapperAiut, SourceCallbackWrapper_GetCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    SourceCallbackWrapper wrapper;
    auto cb = wrapper.GetCallback(0, 0);
    AIUT_EXPECT_NULL_INFO_LOG(cb, "GetCallback should return nullptr when no callback registered");
}

} // namespace AudioStandard
} // namespace OHOS
