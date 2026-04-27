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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "audio_loopback_manager_unit_test.h"
#include "audio_errors.h"
#include "audio_info.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

void LoopbackInServerUnitTest::SetUpTestCase(void) {}

void LoopbackInServerUnitTest::TearDownTestCase(void) {}

void LoopbackInServerUnitTest::SetUp()
{
    manager_ = std::make_shared<AudioLoopbackManager>();
}

void LoopbackInServerUnitTest::TearDown()
{
    manager_ = nullptr;
    loopbackInServer_ = nullptr;
}

HWTEST_F(LoopbackInServerUnitTest, Constructor_Normal_001, TestSize.Level1)
{
    loopbackInServer_ = new LoopbackInServer(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, 1000, manager_);
    EXPECT_NE(loopbackInServer_, nullptr);
    EXPECT_EQ(loopbackInServer_->GetMode(), LOOPBACK_HARDWARE);
    EXPECT_EQ(loopbackInServer_->GetType(), LOOPBACK_TYPE_NORMAL);
    EXPECT_EQ(loopbackInServer_->GetPid(), 1000);
}

HWTEST_F(LoopbackInServerUnitTest, Constructor_GlobalHandler_001, TestSize.Level1)
{
    loopbackInServer_ = new LoopbackInServer(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, 1001, manager_);
    EXPECT_NE(loopbackInServer_, nullptr);
    EXPECT_EQ(loopbackInServer_->GetMode(), LOOPBACK_HARDWARE);
    EXPECT_EQ(loopbackInServer_->GetType(), LOOPBACK_TYPE_GLOBAL_HANDLER);
    EXPECT_EQ(loopbackInServer_->GetPid(), 1001);
}

HWTEST_F(LoopbackInServerUnitTest, Constructor_GlobalControl_001, TestSize.Level1)
{
    loopbackInServer_ = new LoopbackInServer(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, 1002, manager_);
    EXPECT_NE(loopbackInServer_, nullptr);
    EXPECT_EQ(loopbackInServer_->GetMode(), LOOPBACK_HARDWARE);
    EXPECT_EQ(loopbackInServer_->GetType(), LOOPBACK_TYPE_GLOBAL_CONTROL);
    EXPECT_EQ(loopbackInServer_->GetPid(), 1002);
}

HWTEST_F(LoopbackInServerUnitTest, Destructor_001, TestSize.Level1)
{
    loopbackInServer_ = new LoopbackInServer(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, 1000, manager_);
    EXPECT_NE(loopbackInServer_, nullptr);
    loopbackInServer_ = nullptr;
    EXPECT_EQ(loopbackInServer_, nullptr);
}

HWTEST_F(LoopbackInServerUnitTest, Enable_ManagerValid_001, TestSize.Level1)
{
    loopbackInServer_ = new LoopbackInServer(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, 1000, manager_);
    int32_t ret = SUCCESS;
    int32_t result = loopbackInServer_->Enable(true, ret);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(LoopbackInServerUnitTest, Enable_ManagerValid_Disable_001, TestSize.Level1)
{
    loopbackInServer_ = new LoopbackInServer(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, 1000, manager_);
    int32_t ret = SUCCESS;
    int32_t result = loopbackInServer_->Enable(false, ret);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(LoopbackInServerUnitTest, Enable_ManagerNull_001, TestSize.Level1)
{
    loopbackInServer_ = new LoopbackInServer(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, 1000, nullptr);
    int32_t ret = SUCCESS;
    int32_t result = loopbackInServer_->Enable(true, ret);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(ret, ERR_NULL_POINTER);
}

HWTEST_F(LoopbackInServerUnitTest, Enable_ManagerExpired_001, TestSize.Level1)
{
    std::weak_ptr<AudioLoopbackManager> weakManager = manager_;
    manager_ = nullptr;
    loopbackInServer_ = new LoopbackInServer(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, 1000, weakManager.lock());
    int32_t ret = SUCCESS;
    int32_t result = loopbackInServer_->Enable(true, ret);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(ret, ERR_NULL_POINTER);
}

HWTEST_F(LoopbackInServerUnitTest, GetStatus_ManagerValid_001, TestSize.Level1)
{
    loopbackInServer_ = new LoopbackInServer(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, 1002, manager_);
    int32_t status = 0;
    int32_t ret = SUCCESS;
    int32_t result = loopbackInServer_->GetStatus(status, ret);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(ret, ERROR_LOOPBACK_HANDLER_NOT_EXIST);
}

HWTEST_F(LoopbackInServerUnitTest, GetStatus_ManagerNull_001, TestSize.Level1)
{
    loopbackInServer_ = new LoopbackInServer(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, 1002, nullptr);
    int32_t status = 0;
    int32_t ret = SUCCESS;
    int32_t result = loopbackInServer_->GetStatus(status, ret);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(ret, ERR_NULL_POINTER);
}

HWTEST_F(LoopbackInServerUnitTest, GetVolume_ManagerValid_001, TestSize.Level1)
{
    loopbackInServer_ = new LoopbackInServer(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, 1002, manager_);
    float volume = 0.0f;
    int32_t ret = SUCCESS;
    int32_t result = loopbackInServer_->GetVolume(volume, ret);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(ret, ERROR_LOOPBACK_HANDLER_NOT_EXIST);
}

HWTEST_F(LoopbackInServerUnitTest, GetVolume_ManagerNull_001, TestSize.Level1)
{
    loopbackInServer_ = new LoopbackInServer(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, 1002, nullptr);
    float volume = 0.0f;
    int32_t ret = SUCCESS;
    int32_t result = loopbackInServer_->GetVolume(volume, ret);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(ret, ERR_NULL_POINTER);
}

HWTEST_F(LoopbackInServerUnitTest, GetMode_001, TestSize.Level1)
{
    loopbackInServer_ = new LoopbackInServer(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, 1000, manager_);
    AudioLoopbackMode mode = loopbackInServer_->GetMode();
    EXPECT_EQ(mode, LOOPBACK_HARDWARE);
}

HWTEST_F(LoopbackInServerUnitTest, GetType_001, TestSize.Level1)
{
    loopbackInServer_ = new LoopbackInServer(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, 1001, manager_);
    LoopbackType type = loopbackInServer_->GetType();
    EXPECT_EQ(type, LOOPBACK_TYPE_GLOBAL_HANDLER);
}

HWTEST_F(LoopbackInServerUnitTest, GetPid_001, TestSize.Level1)
{
    loopbackInServer_ = new LoopbackInServer(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, 9999, manager_);
    int32_t pid = loopbackInServer_->GetPid();
    EXPECT_EQ(pid, 9999);
}

HWTEST_F(LoopbackInServerUnitTest, GetPid_Negative_001, TestSize.Level1)
{
    loopbackInServer_ = new LoopbackInServer(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, -1, manager_);
    int32_t pid = loopbackInServer_->GetPid();
    EXPECT_EQ(pid, -1);
}
}
}