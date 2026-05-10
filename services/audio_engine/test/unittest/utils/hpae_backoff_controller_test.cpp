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

#include "gtest/gtest.h"
#include <cstdint>

#include "hpae_backoff_controller.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class HpaeBackoffControllerTest : public ::testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name  : DefaultConstructor
 * @tc.type  : FUNC
 * @tc.number: BackoffController_001
 * @tc.desc  : Test default constructor initializes with default values.
 */
HWTEST_F(HpaeBackoffControllerTest, BackoffController_001, TestSize.Level0)
{
    HpaeBackoffController controller;
    EXPECT_EQ(controller.minDelay_, 0);
    EXPECT_EQ(controller.maxDelay_, 20);
    EXPECT_EQ(controller.increment_, 1);
    EXPECT_EQ(controller.delay_, 0);
}

/**
 * @tc.name  : CustomConstructor
 * @tc.type  : FUNC
 * @tc.number: BackoffController_002
 * @tc.desc  : Test custom constructor sets user-provided values.
 */
HWTEST_F(HpaeBackoffControllerTest, BackoffController_002, TestSize.Level0)
{
    HpaeBackoffController controller(5, 50, 10);
    EXPECT_EQ(controller.minDelay_, 5);
    EXPECT_EQ(controller.maxDelay_, 50);
    EXPECT_EQ(controller.increment_, 10);
    EXPECT_EQ(controller.delay_, 5);
}

/**
 * @tc.name  : HandleResultTrueResets
 * @tc.type  : FUNC
 * @tc.number: BackoffController_003
 * @tc.desc  : Test HandleResult(true) resets delay to minDelay.
 */
HWTEST_F(HpaeBackoffControllerTest, BackoffController_003, TestSize.Level0)
{
    HpaeBackoffController controller(0, 20, 1);
    controller.HandleResult(false);
    EXPECT_EQ(controller.delay_, 1);
    controller.HandleResult(true);
    EXPECT_EQ(controller.delay_, 0);
}

/**
 * @tc.name  : HandleResultFalseIncrements
 * @tc.type  : FUNC
 * @tc.number: BackoffController_004
 * @tc.desc  : Test HandleResult(false) increments delay by increment.
 */
HWTEST_F(HpaeBackoffControllerTest, BackoffController_004, TestSize.Level0)
{
    HpaeBackoffController controller(0, 20, 5);
    EXPECT_EQ(controller.delay_, 0);
    controller.HandleResult(false);
    EXPECT_EQ(controller.delay_, 5);
    controller.HandleResult(false);
    EXPECT_EQ(controller.delay_, 10);
}

/**
 * @tc.name  : HandleResultFalseSaturates
 * @tc.type  : FUNC
 * @tc.number: BackoffController_005
 * @tc.desc  : Test HandleResult(false) saturates at maxDelay.
 */
HWTEST_F(HpaeBackoffControllerTest, BackoffController_005, TestSize.Level0)
{
    HpaeBackoffController controller(0, 10, 5);
    controller.HandleResult(false);
    EXPECT_EQ(controller.delay_, 5);
    controller.HandleResult(false);
    EXPECT_EQ(controller.delay_, 10);
    controller.HandleResult(false);
    EXPECT_EQ(controller.delay_, 10);
}

/**
 * @tc.name  : HandleResultTrueAfterFailuresResets
 * @tc.type  : FUNC
 * @tc.number: BackoffController_006
 * @tc.desc  : Test HandleResult(true) after multiple failures resets delay to minDelay.
 */
HWTEST_F(HpaeBackoffControllerTest, BackoffController_006, TestSize.Level0)
{
    HpaeBackoffController controller(3, 20, 2);
    EXPECT_EQ(controller.delay_, 3);
    controller.HandleResult(false);
    EXPECT_EQ(controller.delay_, 5);
    controller.HandleResult(false);
    EXPECT_EQ(controller.delay_, 7);
    controller.HandleResult(false);
    EXPECT_EQ(controller.delay_, 9);
    controller.HandleResult(true);
    EXPECT_EQ(controller.delay_, 3);
}

/**
 * @tc.name  : IncrementExceedsMaxDelay
 * @tc.type  : FUNC
 * @tc.number: BackoffController_007
 * @tc.desc  : Test delay saturates when increment would exceed maxDelay.
 */
HWTEST_F(HpaeBackoffControllerTest, BackoffController_007, TestSize.Level0)
{
    HpaeBackoffController controller(0, 10, 100);
    EXPECT_EQ(controller.delay_, 0);
    controller.HandleResult(false);
    EXPECT_EQ(controller.delay_, 10);
}

/**
 * @tc.name  : RepeatedSuccessKeepsMinDelay
 * @tc.type  : FUNC
 * @tc.number: BackoffController_008
 * @tc.desc  : Test repeated HandleResult(true) keeps delay at minDelay.
 */
HWTEST_F(HpaeBackoffControllerTest, BackoffController_008, TestSize.Level0)
{
    HpaeBackoffController controller(5, 20, 3);
    EXPECT_EQ(controller.delay_, 5);
    controller.HandleResult(true);
    EXPECT_EQ(controller.delay_, 5);
    controller.HandleResult(true);
    EXPECT_EQ(controller.delay_, 5);
}

/**
 * @tc.name  : ZeroMinDelayAndIncrement
 * @tc.type  : FUNC
 * @tc.number: BackoffController_009
 * @tc.desc  : Test controller with zero minDelay and zero increment stays at zero.
 */
HWTEST_F(HpaeBackoffControllerTest, BackoffController_009, TestSize.Level0)
{
    HpaeBackoffController controller(0, 20, 0);
    EXPECT_EQ(controller.delay_, 0);
    controller.HandleResult(false);
    EXPECT_EQ(controller.delay_, 0);
    controller.HandleResult(false);
    EXPECT_EQ(controller.delay_, 0);
}

/**
 * @tc.name  : ResetDirectlySetsMinDelay
 * @tc.type  : FUNC
 * @tc.number: BackoffController_010
 * @tc.desc  : Test Reset method directly sets delay to minDelay.
 */
HWTEST_F(HpaeBackoffControllerTest, BackoffController_010, TestSize.Level0)
{
    HpaeBackoffController controller(7, 30, 5);
    controller.HandleResult(false);
    controller.HandleResult(false);
    controller.HandleResult(false);
    EXPECT_EQ(controller.delay_, 22);
    controller.Reset();
    EXPECT_EQ(controller.delay_, 7);
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
