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
