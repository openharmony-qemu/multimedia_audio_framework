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

void AudioLoopbackManagerUnitTest::SetUpTestCase(void) {}

void AudioLoopbackManagerUnitTest::TearDownTestCase(void) {}

void AudioLoopbackManagerUnitTest::SetUp()
{
    manager_ = std::make_shared<AudioLoopbackManager>();
    mockCallback_ = new MockILoopbackCallback();
}

void AudioLoopbackManagerUnitTest::TearDown()
{
    manager_ = nullptr;
    mockCallback_ = nullptr;
}

HWTEST_F(AudioLoopbackManagerUnitTest, CreateLoopback_ModeNotSupported_001, TestSize.Level1)
{
    sptr<IRemoteObject> loopback;
    int32_t result = manager_->CreateLoopback(static_cast<AudioLoopbackMode>(100), LOOPBACK_TYPE_NORMAL,
        mockCallback_, loopback, 1000);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);
}

HWTEST_F(AudioLoopbackManagerUnitTest, CreateLoopback_Normal_001, TestSize.Level1)
{
    sptr<IRemoteObject> loopback;
    int32_t result = manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL,
        mockCallback_, loopback, 1000);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_NE(loopback, nullptr);
}

HWTEST_F(AudioLoopbackManagerUnitTest, CreateLoopback_GlobalHandler_001, TestSize.Level1)
{
    sptr<IRemoteObject> loopback;
    int32_t result = manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        mockCallback_, loopback, 1001);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_NE(loopback, nullptr);
}

HWTEST_F(AudioLoopbackManagerUnitTest, CreateLoopback_GlobalHandler_AlreadyExist_001, TestSize.Level1)
{
    sptr<IRemoteObject> loopback1;
    sptr<IRemoteObject> loopback2;
    
    int32_t result1 = manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        mockCallback_, loopback1, 1001);
    EXPECT_EQ(result1, SUCCESS);
    int32_t result2 = manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        mockCallback_, loopback2, 1002);
    EXPECT_EQ(result2, ERROR_LOOPBACK_HANDLER_ALREADY_EXIST);
}

HWTEST_F(AudioLoopbackManagerUnitTest, CreateLoopback_GlobalControl_HandlerNotExist_001, TestSize.Level1)
{
    sptr<IRemoteObject> loopback;
    
    int32_t result = manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        mockCallback_, loopback, 1002);
    EXPECT_EQ(result, ERROR_LOOPBACK_HANDLER_NOT_EXIST);
}

HWTEST_F(AudioLoopbackManagerUnitTest, CreateLoopback_GlobalControl_001, TestSize.Level1)
{
    sptr<IRemoteObject> handlerLoopback;
    sptr<IRemoteObject> controlLoopback;
    sptr<MockILoopbackCallback> handlerCallback = new MockILoopbackCallback();
    sptr<MockILoopbackCallback> controlCallback = new MockILoopbackCallback();
    
    int32_t result1 = manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
        handlerCallback, handlerLoopback, 1001);
    EXPECT_EQ(result1, SUCCESS);
    int32_t result2 = manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL,
        controlCallback, controlLoopback, 1002);
    EXPECT_EQ(result2, SUCCESS);
    EXPECT_NE(controlLoopback, nullptr);
}

HWTEST_F(AudioLoopbackManagerUnitTest, CreateLoopback_UnknownType_001, TestSize.Level1)
{
    sptr<IRemoteObject> loopback;
    
    int32_t result = manager_->CreateLoopback(LOOPBACK_HARDWARE, static_cast<LoopbackType>(100),
        mockCallback_, loopback, 1000);
    EXPECT_EQ(result, ERROR_LOOPBACK_OPERATION_NOT_SUPPORTED);
}

HWTEST_F(AudioLoopbackManagerUnitTest, DestroyLoopback_Normal_001, TestSize.Level1)
{
    sptr<IRemoteObject> loopback;
    
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, mockCallback_, loopback, 1000);
    int32_t result = manager_->DestroyLoopback(LOOPBACK_TYPE_NORMAL, 1000);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioLoopbackManagerUnitTest, DestroyLoopback_Normal_NotExist_001, TestSize.Level1)
{
    int32_t result = manager_->DestroyLoopback(LOOPBACK_TYPE_NORMAL, 9999);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioLoopbackManagerUnitTest, DestroyLoopback_GlobalHandler_001, TestSize.Level1)
{
    sptr<IRemoteObject> loopback;
    
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, mockCallback_, loopback, 1001);
    int32_t result = manager_->DestroyLoopback(LOOPBACK_TYPE_GLOBAL_HANDLER, 1001);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioLoopbackManagerUnitTest, DestroyLoopback_GlobalHandler_WrongPid_001, TestSize.Level1)
{
    sptr<IRemoteObject> loopback;
    
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, mockCallback_, loopback, 1001);
    int32_t result = manager_->DestroyLoopback(LOOPBACK_TYPE_GLOBAL_HANDLER, 1002);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioLoopbackManagerUnitTest, DestroyLoopback_GlobalControl_001, TestSize.Level1)
{
    sptr<IRemoteObject> handlerLoopback;
    sptr<IRemoteObject> controlLoopback;
    sptr<MockILoopbackCallback> handlerCallback = new MockILoopbackCallback();
    sptr<MockILoopbackCallback> controlCallback = new MockILoopbackCallback();
    
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, handlerCallback, handlerLoopback, 1001);
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, controlCallback, controlLoopback, 1002);
    int32_t result = manager_->DestroyLoopback(LOOPBACK_TYPE_GLOBAL_CONTROL, 1002);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioLoopbackManagerUnitTest, DestroyLoopback_GlobalControl_NotExist_001, TestSize.Level1)
{
    int32_t result = manager_->DestroyLoopback(LOOPBACK_TYPE_GLOBAL_CONTROL, 9999);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioLoopbackManagerUnitTest, DestroyLoopback_UnknownType_001, TestSize.Level1)
{
    int32_t result = manager_->DestroyLoopback(static_cast<LoopbackType>(100), 1000);
    EXPECT_EQ(result, ERROR_LOOPBACK_OPERATION_NOT_SUPPORTED);
}

HWTEST_F(AudioLoopbackManagerUnitTest, Enable_GlobalControl_HandlerNotExist_001, TestSize.Level1)
{
    int32_t result = manager_->Enable(LOOPBACK_TYPE_GLOBAL_CONTROL, true, 1002);
    EXPECT_EQ(result, ERROR_LOOPBACK_HANDLER_NOT_EXIST);
}

HWTEST_F(AudioLoopbackManagerUnitTest, Enable_GlobalControl_AlreadyEnabled_001, TestSize.Level1)
{
    sptr<IRemoteObject> handlerLoopback;
    sptr<IRemoteObject> controlLoopback;
    sptr<MockILoopbackCallback> handlerCallback = new MockILoopbackCallback();
    sptr<MockILoopbackCallback> controlCallback = new MockILoopbackCallback();

    EXPECT_CALL(*handlerCallback, OnCommandResult(_, _, _)).Times(AnyNumber());
    EXPECT_CALL(*controlCallback, OnStatusChange(_, _)).Times(AnyNumber());
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, handlerCallback, handlerLoopback, 1001);
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, controlCallback, controlLoopback, 1002);
    manager_->Enable(LOOPBACK_TYPE_GLOBAL_CONTROL, true, 1002);
    int32_t result = manager_->Enable(LOOPBACK_TYPE_GLOBAL_CONTROL, true, 1002);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioLoopbackManagerUnitTest, Enable_GlobalControl_AlreadyDisabled_001, TestSize.Level1)
{
    sptr<IRemoteObject> handlerLoopback;
    sptr<IRemoteObject> controlLoopback;
    sptr<MockILoopbackCallback> handlerCallback = new MockILoopbackCallback();
    sptr<MockILoopbackCallback> controlCallback = new MockILoopbackCallback();

    EXPECT_CALL(*controlCallback, OnStatusChange(_, _)).Times(AnyNumber());
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, handlerCallback, handlerLoopback, 1001);
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, controlCallback, controlLoopback, 1002);
    int32_t result = manager_->Enable(LOOPBACK_TYPE_GLOBAL_CONTROL, false, 1002);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioLoopbackManagerUnitTest, Enable_GlobalControl_Enable_001, TestSize.Level1)
{
    sptr<IRemoteObject> handlerLoopback;
    sptr<IRemoteObject> controlLoopback;
    sptr<MockILoopbackCallback> handlerCallback = new MockILoopbackCallback();
    sptr<MockILoopbackCallback> controlCallback = new MockILoopbackCallback();
    
    EXPECT_CALL(*handlerCallback, OnCommandResult(LOOPBACK_CMD_ENABLE, true, SUCCESS)).Times(1);
    EXPECT_CALL(*controlCallback, OnStatusChange(LOOPBACK_AVAILABLE_RUNNING, CMD_FROM_SYSTEM)).Times(1);
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, handlerCallback, handlerLoopback, 1001);
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, controlCallback, controlLoopback, 1002);
    int32_t result = manager_->Enable(LOOPBACK_TYPE_GLOBAL_CONTROL, true, 1002);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioLoopbackManagerUnitTest, Enable_GlobalControl_Disable_001, TestSize.Level1)
{
    sptr<IRemoteObject> handlerLoopback;
    sptr<IRemoteObject> controlLoopback;
    sptr<MockILoopbackCallback> handlerCallback = new MockILoopbackCallback();
    sptr<MockILoopbackCallback> controlCallback = new MockILoopbackCallback();
    
    EXPECT_CALL(*handlerCallback, OnCommandResult(_, _, _)).Times(AnyNumber());
    EXPECT_CALL(*controlCallback, OnStatusChange(_, _)).Times(AnyNumber());
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, handlerCallback, handlerLoopback, 1001);
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, controlCallback, controlLoopback, 1002);
    manager_->Enable(LOOPBACK_TYPE_GLOBAL_CONTROL, true, 1002);
    EXPECT_CALL(*handlerCallback, OnCommandResult(LOOPBACK_CMD_DISABLE, true, SUCCESS)).Times(1);
    EXPECT_CALL(*controlCallback, OnStatusChange(LOOPBACK_AVAILABLE_IDLE, CMD_FROM_SYSTEM)).Times(1);
    int32_t result = manager_->Enable(LOOPBACK_TYPE_GLOBAL_CONTROL, false, 1002);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioLoopbackManagerUnitTest, Enable_GlobalHandler_Enable_001, TestSize.Level1)
{
    sptr<IRemoteObject> handlerLoopback;
    sptr<MockILoopbackCallback> handlerCallback = new MockILoopbackCallback();
    
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, handlerCallback, handlerLoopback, 1001);
    int32_t result = manager_->Enable(LOOPBACK_TYPE_GLOBAL_HANDLER, true, 1001);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioLoopbackManagerUnitTest, Enable_GlobalHandler_Disable_001, TestSize.Level1)
{
    sptr<IRemoteObject> handlerLoopback;
    sptr<MockILoopbackCallback> handlerCallback = new MockILoopbackCallback();
    
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, handlerCallback, handlerLoopback, 1001);
    manager_->Enable(LOOPBACK_TYPE_GLOBAL_HANDLER, true, 1001);
    int32_t result = manager_->Enable(LOOPBACK_TYPE_GLOBAL_HANDLER, false, 1001);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioLoopbackManagerUnitTest, Enable_Normal_001, TestSize.Level1)
{
    sptr<IRemoteObject> loopback;
    
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, mockCallback_, loopback, 1000);
    int32_t result = manager_->Enable(LOOPBACK_TYPE_NORMAL, true, 1000);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioLoopbackManagerUnitTest, GetStatus_GlobalControl_HandlerNotExist_001, TestSize.Level1)
{
    int status = 0;
    int32_t result = manager_->GetStatus(LOOPBACK_TYPE_GLOBAL_CONTROL, status, 1002);
    EXPECT_EQ(result, ERROR_LOOPBACK_HANDLER_NOT_EXIST);
}

HWTEST_F(AudioLoopbackManagerUnitTest, GetStatus_GlobalControl_001, TestSize.Level1)
{
    sptr<IRemoteObject> handlerLoopback;
    sptr<IRemoteObject> controlLoopback;
    sptr<MockILoopbackCallback> handlerCallback = new MockILoopbackCallback();
    sptr<MockILoopbackCallback> controlCallback = new MockILoopbackCallback();
    
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, handlerCallback, handlerLoopback, 1001);
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, controlCallback, controlLoopback, 1002);
    int status = 0;
    int32_t result = manager_->GetStatus(LOOPBACK_TYPE_GLOBAL_CONTROL, status, 1002);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(status, static_cast<int>(LOOPBACK_AVAILABLE_IDLE));
}

HWTEST_F(AudioLoopbackManagerUnitTest, GetStatus_GlobalControl_Running_001, TestSize.Level1)
{
    sptr<IRemoteObject> handlerLoopback;
    sptr<IRemoteObject> controlLoopback;
    sptr<MockILoopbackCallback> handlerCallback = new MockILoopbackCallback();
    sptr<MockILoopbackCallback> controlCallback = new MockILoopbackCallback();
    
    EXPECT_CALL(*handlerCallback, OnCommandResult(_, _, _)).Times(AnyNumber());
    EXPECT_CALL(*controlCallback, OnStatusChange(_, _)).Times(AnyNumber());
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, handlerCallback, handlerLoopback, 1001);
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, controlCallback, controlLoopback, 1002);
    manager_->Enable(LOOPBACK_TYPE_GLOBAL_CONTROL, true, 1002);
    int status = 0;
    int32_t result = manager_->GetStatus(LOOPBACK_TYPE_GLOBAL_CONTROL, status, 1002);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(status, static_cast<int>(LOOPBACK_AVAILABLE_RUNNING));
}

HWTEST_F(AudioLoopbackManagerUnitTest, GetStatus_NotSupported_001, TestSize.Level1)
{
    int status = 0;
    int32_t result = manager_->GetStatus(LOOPBACK_TYPE_NORMAL, status, 1000);
    EXPECT_EQ(result, ERROR_LOOPBACK_OPERATION_NOT_SUPPORTED);
}

HWTEST_F(AudioLoopbackManagerUnitTest, GetStatus_GlobalHandler_NotSupported_001, TestSize.Level1)
{
    sptr<IRemoteObject> handlerLoopback;
    sptr<MockILoopbackCallback> handlerCallback = new MockILoopbackCallback();
    
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, handlerCallback, handlerLoopback, 1001);
    int status = 0;
    int32_t result = manager_->GetStatus(LOOPBACK_TYPE_GLOBAL_HANDLER, status, 1001);
    EXPECT_EQ(result, ERROR_LOOPBACK_OPERATION_NOT_SUPPORTED);
}

HWTEST_F(AudioLoopbackManagerUnitTest, GetVolume_GlobalControl_HandlerNotExist_001, TestSize.Level1)
{
    float volume = 0.0f;
    int32_t result = manager_->GetVolume(LOOPBACK_TYPE_GLOBAL_CONTROL, volume, 1002);
    EXPECT_EQ(result, ERROR_LOOPBACK_HANDLER_NOT_EXIST);
}

HWTEST_F(AudioLoopbackManagerUnitTest, GetVolume_GlobalControl_001, TestSize.Level1)
{
    sptr<IRemoteObject> handlerLoopback;
    sptr<IRemoteObject> controlLoopback;
    sptr<MockILoopbackCallback> handlerCallback = new MockILoopbackCallback();
    sptr<MockILoopbackCallback> controlCallback = new MockILoopbackCallback();

    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, handlerCallback, handlerLoopback, 1001);
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, controlCallback, controlLoopback, 1002);
    float volume = 0.0f;
    int32_t result = manager_->GetVolume(LOOPBACK_TYPE_GLOBAL_CONTROL, volume, 1002);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(volume, 0.0f);
}

HWTEST_F(AudioLoopbackManagerUnitTest, GetVolume_NotSupported_001, TestSize.Level1)
{
    float volume = 0.0f;
    int32_t result = manager_->GetVolume(LOOPBACK_TYPE_NORMAL, volume, 1000);
    EXPECT_EQ(result, ERROR_LOOPBACK_OPERATION_NOT_SUPPORTED);
}

HWTEST_F(AudioLoopbackManagerUnitTest, OnHandlerDied_001, TestSize.Level1)
{
    sptr<IRemoteObject> handlerLoopback;
    sptr<IRemoteObject> controlLoopback;
    sptr<MockILoopbackCallback> handlerCallback = new MockILoopbackCallback();
    sptr<MockILoopbackCallback> controlCallback = new MockILoopbackCallback();
    
    EXPECT_CALL(*controlCallback, OnStatusChange(LOOPBACK_AVAILABLE_IDLE, CMD_FROM_SYSTEM)).Times(1);
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, handlerCallback, handlerLoopback, 1001);
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, controlCallback, controlLoopback, 1002);
    manager_->OnHandlerDied();
    int status = 0;
    int32_t result = manager_->GetStatus(LOOPBACK_TYPE_GLOBAL_CONTROL, status, 1002);
    EXPECT_EQ(result, ERROR_LOOPBACK_HANDLER_NOT_EXIST);
}

HWTEST_F(AudioLoopbackManagerUnitTest, OnControlDied_001, TestSize.Level1)
{
    sptr<IRemoteObject> handlerLoopback;
    sptr<IRemoteObject> controlLoopback;
    sptr<MockILoopbackCallback> handlerCallback = new MockILoopbackCallback();
    sptr<MockILoopbackCallback> controlCallback = new MockILoopbackCallback();
    
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, handlerCallback, handlerLoopback, 1001);
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, controlCallback, controlLoopback, 1002);
    manager_->OnControlDied(1002);
    int status = 0;
    int32_t result = manager_->GetStatus(LOOPBACK_TYPE_GLOBAL_CONTROL, status, 1002);
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(AudioLoopbackManagerUnitTest, OnControlDied_NotExist_001, TestSize.Level1)
{
    manager_->OnControlDied(9999);
    EXPECT_TRUE(true);
}

HWTEST_F(AudioLoopbackManagerUnitTest, OnNormalDied_001, TestSize.Level1)
{
    sptr<IRemoteObject> loopback;
    
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, mockCallback_, loopback, 1000);
    manager_->OnNormalDied(1000);
    EXPECT_TRUE(true);
}

HWTEST_F(AudioLoopbackManagerUnitTest, OnNormalDied_NotExist_001, TestSize.Level1)
{
    manager_->OnNormalDied(9999);
    EXPECT_TRUE(true);
}

HWTEST_F(AudioLoopbackManagerUnitTest, BroadcastToControls_Enable_001, TestSize.Level1)
{
    sptr<IRemoteObject> handlerLoopback;
    sptr<IRemoteObject> controlLoopback;
    sptr<MockILoopbackCallback> handlerCallback = new MockILoopbackCallback();
    sptr<MockILoopbackCallback> controlCallback = new MockILoopbackCallback();
    
    EXPECT_CALL(*handlerCallback, OnCommandResult(_, _, _)).Times(AnyNumber());
    EXPECT_CALL(*controlCallback, OnStatusChange(LOOPBACK_AVAILABLE_RUNNING, CMD_FROM_SYSTEM)).Times(1);
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, handlerCallback, handlerLoopback, 1001);
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, controlCallback, controlLoopback, 1002);
    manager_->Enable(LOOPBACK_TYPE_GLOBAL_CONTROL, true, 1002);
}

HWTEST_F(AudioLoopbackManagerUnitTest, BroadcastToControls_Disable_001, TestSize.Level1)
{
    sptr<IRemoteObject> handlerLoopback;
    sptr<IRemoteObject> controlLoopback;
    sptr<MockILoopbackCallback> handlerCallback = new MockILoopbackCallback();
    sptr<MockILoopbackCallback> controlCallback = new MockILoopbackCallback();
    
    EXPECT_CALL(*handlerCallback, OnCommandResult(_, _, _)).Times(AnyNumber());
    EXPECT_CALL(*controlCallback, OnStatusChange(_, _)).Times(AnyNumber());
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, handlerCallback, handlerLoopback, 1001);
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, controlCallback, controlLoopback, 1002);
    manager_->Enable(LOOPBACK_TYPE_GLOBAL_CONTROL, true, 1002);
    EXPECT_CALL(*handlerCallback, OnCommandResult(LOOPBACK_CMD_DISABLE, _, _)).Times(1);
    EXPECT_CALL(*controlCallback, OnStatusChange(LOOPBACK_AVAILABLE_IDLE, CMD_FROM_SYSTEM)).Times(1);
    manager_->Enable(LOOPBACK_TYPE_GLOBAL_CONTROL, false, 1002);
}

HWTEST_F(AudioLoopbackManagerUnitTest, Destructor_001, TestSize.Level1)
{
    auto manager = std::make_shared<AudioLoopbackManager>();
    sptr<IRemoteObject> loopback;
    sptr<MockILoopbackCallback> callback = new MockILoopbackCallback();
    
    manager->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, callback, loopback, 1000);
    manager = nullptr;
    EXPECT_EQ(manager, nullptr);
}

HWTEST_F(AudioLoopbackManagerUnitTest, MultipleControls_001, TestSize.Level1)
{
    sptr<IRemoteObject> handlerLoopback;
    sptr<IRemoteObject> controlLoopback1;
    sptr<IRemoteObject> controlLoopback2;
    sptr<MockILoopbackCallback> handlerCallback = new MockILoopbackCallback();
    sptr<MockILoopbackCallback> controlCallback1 = new MockILoopbackCallback();
    sptr<MockILoopbackCallback> controlCallback2 = new MockILoopbackCallback();
    
    EXPECT_CALL(*handlerCallback, OnCommandResult(_, _, _)).Times(AnyNumber());
    EXPECT_CALL(*controlCallback1, OnStatusChange(_, _)).Times(AnyNumber());
    EXPECT_CALL(*controlCallback2, OnStatusChange(_, _)).Times(AnyNumber());
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, handlerCallback, handlerLoopback, 1001);
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, controlCallback1, controlLoopback1, 1002);
    manager_->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, controlCallback2, controlLoopback2, 1003);
    int32_t result = manager_->Enable(LOOPBACK_TYPE_GLOBAL_CONTROL, true, 1002);
    EXPECT_EQ(result, SUCCESS);
}
}
}