/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "i_audio_stream.h"
#include <map>

#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_utils.h"
#include "audio_policy_manager.h"
#include "capturer_in_client.h"
#include "renderer_in_client.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

/**
 * @tc.name  : Test OnOperationHandled API
 * @tc.type  : FUNC
 * @tc.number: OnOperationHandled_001
 * @tc.desc  : Test OnOperationHandled interface.
 */
HWTEST(CapturerInClientUnitTest, OnOperationHandled_001, TestSize.Level1)
{
    std::shared_ptr<CapturerInClientInner> capturerInClientInner_ =
        std::make_shared<CapturerInClientInner>(STREAM_MUSIC, getpid());
    Operation operation = Operation::UPDATE_STREAM;
    int64_t result = 1;
    int32_t ret = capturerInClientInner_->OnOperationHandled(operation, result);
    EXPECT_EQ(ret, SUCCESS);

    operation = Operation::BUFFER_OVERFLOW;
    ret = capturerInClientInner_->OnOperationHandled(operation, result);
    EXPECT_EQ(ret, SUCCESS);

    operation = Operation::RESTORE_SESSION;
    ret = capturerInClientInner_->OnOperationHandled(operation, result);
    EXPECT_EQ(ret, SUCCESS);

    operation = Operation::START_STREAM;
    ret = capturerInClientInner_->OnOperationHandled(operation, result);
    EXPECT_EQ(ret, SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS
