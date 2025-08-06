/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "audio_service.h"
#include "audio_service_log.h"
#include "audio_errors.h"
#include "audio_system_manager.h"

#include "audio_manager_listener_stub_impl.h"
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class AudioManagerListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

/**
 * @tc.name  : Test OnCapturerState API
 * @tc.type  : FUNC
 * @tc.number: OnCapturerState_001
 * @tc.desc  : Test OnCapturerState interface.
 */
HWTEST(AudioManagerListenerTest, OnCapturerState_001, TestSize.Level1)
{
    auto audioManagerListenerStub = std::make_unique<AudioManagerListenerStubImpl>();
    EXPECT_EQ(audioManagerListenerStub->OnCapturerState(true), SUCCESS);
}

/**
 * @tc.name  : Test OnCapturerState API
 * @tc.type  : FUNC
 * @tc.number: OnCapturerState_002
 * @tc.desc  : Test OnCapturerState interface.
 */
HWTEST(AudioManagerListenerTest, OnCapturerState_002, TestSize.Level1)
{
    auto audioManagerListenerStub = std::make_unique<AudioManagerListenerStubImpl>();
    EXPECT_EQ(audioManagerListenerStub->OnCapturerState(false), SUCCESS);
}

/**
 * @tc.name  : Test OnWakeupClose API
 * @tc.type  : FUNC
 * @tc.number: OnWakeupClose_001
 * @tc.desc  : Test OnWakeupClose interface.
 */
HWTEST(AudioManagerListenerTest, OnWakeupClose_001, TestSize.Level1)
{
    auto audioManagerListenerStub = std::make_unique<AudioManagerListenerStubImpl>();
    EXPECT_EQ(audioManagerListenerStub->OnWakeupClose(), SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS