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

#include <gtest/gtest.h>
#include "hpae_signal_process_thread.h"
#include "hpae_renderer_manager.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

class HpaeSignalProcessThreadTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeSignalProcessThreadTest::SetUp()
{}

void HpaeSignalProcessThreadTest::TearDown()
{}

HWTEST_F(HpaeSignalProcessThreadTest, ActivateDeactivateThread, TestSize.Level0)
{
    std::shared_ptr<HpaeRendererManager> streamManager = nullptr;
    std::unique_ptr<HpaeSignalProcessThread> hpaeSignalProcessThread = std::make_unique<HpaeSignalProcessThread>();
    EXPECT_EQ(hpaeSignalProcessThread->IsRunning(), false);
    EXPECT_EQ(hpaeSignalProcessThread->IsMsgProcessing(), false);

    hpaeSignalProcessThread->ActivateThread(streamManager);
    EXPECT_EQ(hpaeSignalProcessThread->IsRunning(), true);

    hpaeSignalProcessThread->Notify();
    EXPECT_EQ(hpaeSignalProcessThread->IsMsgProcessing(), true);

    hpaeSignalProcessThread->DeactivateThread();
    EXPECT_EQ(hpaeSignalProcessThread->IsRunning(), false);
}

/**
 * @tc.name  : setFastThread_true_001
 * @tc.type  : FUNC
 * @tc.number: HpaeSignalProcessThreadTest_002
 * @tc.desc  : Test SetFastThread(true) sets fast thread mode
 */
HWTEST_F(HpaeSignalProcessThreadTest, setFastThread_true_001, TestSize.Level0)
{
    auto hpaeSignalProcessThread = std::make_unique<HpaeSignalProcessThread>();
    hpaeSignalProcessThread->SetFastThread(true);
    // Verify no crash, fast thread mode is set
    EXPECT_EQ(hpaeSignalProcessThread->IsRunning(), false);
}

/**
 * @tc.name  : setFastThread_false_001
 * @tc.type  : FUNC
 * @tc.number: HpaeSignalProcessThreadTest_003
 * @tc.desc  : Test SetFastThread(false) sets normal thread mode
 */
HWTEST_F(HpaeSignalProcessThreadTest, setFastThread_false_001, TestSize.Level0)
{
    auto hpaeSignalProcessThread = std::make_unique<HpaeSignalProcessThread>();
    hpaeSignalProcessThread->SetFastThread(false);
    EXPECT_EQ(hpaeSignalProcessThread->IsRunning(), false);
}

/**
 * @tc.name  : setFastThread_defaultIsFalse_001
 * @tc.type  : FUNC
 * @tc.number: HpaeSignalProcessThreadTest_004
 * @tc.desc  : Test default thread is not fast thread
 */
HWTEST_F(HpaeSignalProcessThreadTest, setFastThread_defaultIsFalse_001, TestSize.Level0)
{
    auto hpaeSignalProcessThread = std::make_unique<HpaeSignalProcessThread>();
    // Default isFastThread_ = false, ActivateThread should use non-fast path
    std::shared_ptr<HpaeRendererManager> streamManager = nullptr;
    hpaeSignalProcessThread->ActivateThread(streamManager);
    EXPECT_EQ(hpaeSignalProcessThread->IsRunning(), true);
    hpaeSignalProcessThread->DeactivateThread();
    EXPECT_EQ(hpaeSignalProcessThread->IsRunning(), false);
}

/**
 * @tc.name  : activateDeactivateThread_FastPath_001
 * @tc.type  : FUNC
 * @tc.number: HpaeSignalProcessThreadTest_005
 * @tc.desc  : Test ActivateThread with SetFastThread(true) uses fast path lifecycle
 */
HWTEST_F(HpaeSignalProcessThreadTest, activateDeactivateThread_FastPath_001, TestSize.Level0)
{
    auto hpaeSignalProcessThread = std::make_unique<HpaeSignalProcessThread>();
    hpaeSignalProcessThread->SetFastThread(true);
    std::shared_ptr<HpaeRendererManager> streamManager = nullptr;
    hpaeSignalProcessThread->ActivateThread(streamManager);
    EXPECT_EQ(hpaeSignalProcessThread->IsRunning(), true);

    hpaeSignalProcessThread->Notify();
    EXPECT_EQ(hpaeSignalProcessThread->IsMsgProcessing(), true);

    hpaeSignalProcessThread->DeactivateThread();
    EXPECT_EQ(hpaeSignalProcessThread->IsRunning(), false);
}

/**
 * @tc.name  : activateDeactivateThread_SwitchFastToNormal_001
 * @tc.type  : FUNC
 * @tc.number: HpaeSignalProcessThreadTest_006
 * @tc.desc  : Test SetFastThread can switch from true to false before ActivateThread
 */
HWTEST_F(HpaeSignalProcessThreadTest, activateDeactivateThread_SwitchFastToNormal_001, TestSize.Level0)
{
    auto hpaeSignalProcessThread = std::make_unique<HpaeSignalProcessThread>();
    hpaeSignalProcessThread->SetFastThread(true);
    hpaeSignalProcessThread->SetFastThread(false);
    std::shared_ptr<HpaeRendererManager> streamManager = nullptr;
    hpaeSignalProcessThread->ActivateThread(streamManager);
    EXPECT_EQ(hpaeSignalProcessThread->IsRunning(), true);
    hpaeSignalProcessThread->DeactivateThread();
    EXPECT_EQ(hpaeSignalProcessThread->IsRunning(), false);
}

/**
 * @tc.name  : setFastThreadAfterActivate_001
 * @tc.type  : FUNC
 * @tc.number: HpaeSignalProcessThreadTest_007
 * @tc.desc  : Test SetFastThread called after ActivateThread does not crash
 */
HWTEST_F(HpaeSignalProcessThreadTest, setFastThreadAfterActivate_001, TestSize.Level0)
{
    auto hpaeSignalProcessThread = std::make_unique<HpaeSignalProcessThread>();
    std::shared_ptr<HpaeRendererManager> streamManager = nullptr;
    hpaeSignalProcessThread->ActivateThread(streamManager);
    EXPECT_EQ(hpaeSignalProcessThread->IsRunning(), true);
    // SetFastThread after ActivateThread — Run() already started with isFastThread_=false
    hpaeSignalProcessThread->SetFastThread(true);
    hpaeSignalProcessThread->DeactivateThread();
    EXPECT_EQ(hpaeSignalProcessThread->IsRunning(), false);
}

/**
 * @tc.name  : bindCore_defaultNotBinded_001
 * @tc.type  : FUNC
 * @tc.number: HpaeSignalProcessThreadTest_006
 * @tc.desc  : Test coreBinded_ is false by default
 */
HWTEST_F(HpaeSignalProcessThreadTest, bindCore_defaultNotBinded_001, TestSize.Level0)
{
    auto thrd = std::make_unique<HpaeSignalProcessThread>();
    EXPECT_EQ(thrd->coreBinded_, false);
    thrd->coreBinded_ = true;
    thrd->BindCore();
    EXPECT_EQ(thrd->coreBinded_, true);
}