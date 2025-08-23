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

#ifndef AUDIO_POLICY_CLIENT_HOLDER_UNIT_TEST_H
#define AUDIO_POLICY_CLIENT_HOLDER_UNIT_TEST_H

#include "gtest/gtest.h"
#include "audio_policy_client_holder.h"
#include "mock_audio_policy_client.h"

namespace OHOS {
namespace AudioStandard {

class AudioPolicyClientHolderUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before the first test case in this test suite
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after the last test case in this test suite
    static void TearDownTestCase(void);
    // SetUp: Called before each test case
    void SetUp(void);
    // TearDown: Called after each test case
    void TearDown(void);

public:
    sptr<MockAudioPolicyClient> mockClient_ = nullptr;
    std::shared_ptr<AudioPolicyClientHolder> clientHolder_ = nullptr;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_POLICY_CLIENT_HOLDER_UNIT_TEST_H