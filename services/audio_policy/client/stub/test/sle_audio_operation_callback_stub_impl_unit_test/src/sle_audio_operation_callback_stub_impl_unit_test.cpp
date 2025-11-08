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

#include "sle_audio_operation_callback_stub_impl_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"
#include "sle_audio_device_manager.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void SleAudioOperationCallbackStubImplUnitTest::SetUpTestCase(void) {}
void SleAudioOperationCallbackStubImplUnitTest::TearDownTestCase(void) {}
void SleAudioOperationCallbackStubImplUnitTest::SetUp(void) {}
void SleAudioOperationCallbackStubImplUnitTest::TearDown(void) {}

/**
* @tc.name  : Test SleAudioOperationCallbackStubImplUnitTest.
* @tc.number: CallbackStubUnitTest_001
* @tc.desc  : Test SleAudioOperationCallbackStubImpl
*/
HWTEST(SleAudioOperationCallbackStubImplUnitTest, CallbackStubUnitTest_001, TestSize.Level4)
{
    auto audioSleCb = new (std::nothrow) SleAudioOperationCallbackStubImpl();
    EXPECT_NE(audioSleCb, nullptr);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback;
    auto ret = audioSleCb->SetSleAudioOperationCallback(sleAudioOperationCallback);
    EXPECT_NE(-1, ret);
    sleAudioOperationCallback = nullptr;
    ret = audioSleCb->SetSleAudioOperationCallback(sleAudioOperationCallback);
    EXPECT_NE(SUCCESS, ret);
    delete audioSleCb;
}

/**
* @tc.name  : Test SleAudioOperationCallbackStubImplUnitTest.
* @tc.number: CallbackStubUnitTest_002
* @tc.desc  : Test GetSleAudioDeviceList
*/
HWTEST(SleAudioOperationCallbackStubImplUnitTest, CallbackStubUnitTest_002, TestSize.Level4)
{
    auto audioSleCb = new (std::nothrow) SleAudioOperationCallbackStubImpl();
    std::vector<AudioDeviceDescriptor> devices;
    EXPECT_NE(audioSleCb, nullptr);
    auto ret = audioSleCb->GetSleAudioDeviceList(devices);
    EXPECT_NE(SUCCESS, ret);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback;
    audioSleCb->sleAudioOperationCallback_ = sleAudioOperationCallback;
    ret = audioSleCb->GetSleAudioDeviceList(devices);
    EXPECT_NE(-1, ret);
    delete audioSleCb;
}

/**
* @tc.name  : Test SleAudioOperationCallbackStubImplUnitTest.
* @tc.number: CallbackStubUnitTest_003
* @tc.desc  : Test IsInBandRingOpen
*/
HWTEST(SleAudioOperationCallbackStubImplUnitTest, CallbackStubUnitTest_003, TestSize.Level4)
{
    auto audioSleCb = new (std::nothrow) SleAudioOperationCallbackStubImpl();
    EXPECT_NE(audioSleCb, nullptr);
    std::string device = "test";
    bool result = true;
    auto ret = audioSleCb->IsInBandRingOpen(device, result);
    EXPECT_EQ(SUCCESS, ret);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback;
    audioSleCb->sleAudioOperationCallback_ = sleAudioOperationCallback;
    ret = audioSleCb->IsInBandRingOpen(device, result);
    EXPECT_NE(-1, ret);
    delete audioSleCb;
}

/**
* @tc.name  : Test SleAudioOperationCallbackStubImplUnitTest.
* @tc.number: CallbackStubUnitTest_004
* @tc.desc  : Test GetSupportStreamType
*/
HWTEST(SleAudioOperationCallbackStubImplUnitTest, CallbackStubUnitTest_004, TestSize.Level4)
{
    auto audioSleCb = new (std::nothrow) SleAudioOperationCallbackStubImpl();
    EXPECT_NE(audioSleCb, nullptr);
    std::string device = "test";
    uint32_t retType = 0;
    auto ret = audioSleCb->GetSupportStreamType(device, retType);
    EXPECT_EQ(SUCCESS, ret);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback;
    audioSleCb->sleAudioOperationCallback_ = sleAudioOperationCallback;
    ret = audioSleCb->GetSupportStreamType(device, retType);
    EXPECT_NE(-1, ret);
    delete audioSleCb;
}

/**
* @tc.name  : Test SleAudioOperationCallbackStubImplUnitTest.
* @tc.number: CallbackStubUnitTest_005
* @tc.desc  : Test SetActiveSinkDevice
*/
HWTEST(SleAudioOperationCallbackStubImplUnitTest, CallbackStubUnitTest_005, TestSize.Level4)
{
    auto audioSleCb = new (std::nothrow) SleAudioOperationCallbackStubImpl();
    EXPECT_NE(audioSleCb, nullptr);
    std::string device = "test";
    uint32_t streamType = 0;
    int32_t result = 0;
    auto ret = audioSleCb->SetActiveSinkDevice(device, streamType, result);
    EXPECT_NE(SUCCESS, ret);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback;
    audioSleCb->sleAudioOperationCallback_ = sleAudioOperationCallback;
    ret = audioSleCb->SetActiveSinkDevice(device, streamType, result);
    EXPECT_NE(-1, ret);
    delete audioSleCb;
}

/**
* @tc.name  : Test SleAudioOperationCallbackStubImplUnitTest.
* @tc.number: CallbackStubUnitTest_006
* @tc.desc  : Test StartPlaying
*/
HWTEST(SleAudioOperationCallbackStubImplUnitTest, CallbackStubUnitTest_006, TestSize.Level4)
{
    auto audioSleCb = new (std::nothrow) SleAudioOperationCallbackStubImpl();
    EXPECT_NE(audioSleCb, nullptr);
    std::string device = "test";
    uint32_t streamType = 0;
    int32_t result = 0;
    int32_t timeoutMs = 1000;
    auto ret = audioSleCb->StartPlaying(device, streamType, timeoutMs, result);
    EXPECT_NE(SUCCESS, ret);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback;
    audioSleCb->sleAudioOperationCallback_ = sleAudioOperationCallback;
    ret = audioSleCb->StartPlaying(device, streamType, timeoutMs, result);
    EXPECT_NE(-1, ret);
    delete audioSleCb;
}

/**
* @tc.name  : Test SleAudioOperationCallbackStubImplUnitTest.
* @tc.number: CallbackStubUnitTest_007
* @tc.desc  : Test StopPlaying
*/
HWTEST(SleAudioOperationCallbackStubImplUnitTest, CallbackStubUnitTest_007, TestSize.Level4)
{
    auto audioSleCb = new (std::nothrow) SleAudioOperationCallbackStubImpl();
    EXPECT_NE(audioSleCb, nullptr);
    std::string device = "test";
    uint32_t streamType = 0;
    int32_t result = 0;
    auto ret = audioSleCb->StopPlaying(device, streamType, result);
    EXPECT_NE(SUCCESS, ret);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback;
    audioSleCb->sleAudioOperationCallback_ = sleAudioOperationCallback;
    ret = audioSleCb->StopPlaying(device, streamType, result);
    EXPECT_NE(-1, ret);
    delete audioSleCb;
}

/**
* @tc.name  : Test SleAudioOperationCallbackStubImplUnitTest.
* @tc.number: CallbackStubUnitTest_008
* @tc.desc  : Test ConnectAllowedProfiles
*/
HWTEST(SleAudioOperationCallbackStubImplUnitTest, CallbackStubUnitTest_008, TestSize.Level4)
{
    auto audioSleCb = new (std::nothrow) SleAudioOperationCallbackStubImpl();
    EXPECT_NE(audioSleCb, nullptr);
    std::string device = "test";
    int32_t result = 0;
    auto ret = audioSleCb->ConnectAllowedProfiles(device, result);
    EXPECT_NE(SUCCESS, ret);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback;
    audioSleCb->sleAudioOperationCallback_ = sleAudioOperationCallback;
    ret = audioSleCb->ConnectAllowedProfiles(device, result);
    EXPECT_NE(-1, ret);
    delete audioSleCb;
}

/**
* @tc.name  : Test SleAudioOperationCallbackStubImplUnitTest.
* @tc.number: CallbackStubUnitTest_009
* @tc.desc  : Test SetDeviceAbsVolume
*/
HWTEST(SleAudioOperationCallbackStubImplUnitTest, CallbackStubUnitTest_009, TestSize.Level4)
{
    auto audioSleCb = new (std::nothrow) SleAudioOperationCallbackStubImpl();
    EXPECT_NE(audioSleCb, nullptr);
    std::string remoteAddr = "test";
    uint32_t volume = 100;
    uint32_t streamType = 0;
    int32_t result = 0;
    auto ret = audioSleCb->SetDeviceAbsVolume(remoteAddr, volume, streamType, result);
    EXPECT_NE(SUCCESS, ret);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback;
    audioSleCb->sleAudioOperationCallback_ = sleAudioOperationCallback;
    ret = audioSleCb->SetDeviceAbsVolume(remoteAddr, volume, streamType, result);
    EXPECT_NE(-1, ret);
    delete audioSleCb;
}

/**
* @tc.name  : Test SleAudioOperationCallbackStubImplUnitTest.
* @tc.number: CallbackStubUnitTest_010
* @tc.desc  : Test SendUserSelection
*/
HWTEST(SleAudioOperationCallbackStubImplUnitTest, CallbackStubUnitTest_010, TestSize.Level4)
{
    auto audioSleCb = new (std::nothrow) SleAudioOperationCallbackStubImpl();
    EXPECT_NE(audioSleCb, nullptr);
    std::string device = "test";
    uint32_t streamType = 0;
    int32_t result = 0;
    int32_t eventType = 2;
    auto ret = audioSleCb->SendUserSelection(device, streamType, eventType, result);
    EXPECT_NE(SUCCESS, ret);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback;
    audioSleCb->sleAudioOperationCallback_ = sleAudioOperationCallback;
    ret = audioSleCb->SendUserSelection(device, streamType, eventType, result);
    EXPECT_NE(-1, ret);
    delete audioSleCb;
}

/**
* @tc.name  : Test SleAudioOperationCallbackStubImplUnitTest.
* @tc.number: CallbackStubUnitTest_011
* @tc.desc  : Test GetRenderPosition
*/
HWTEST(SleAudioOperationCallbackStubImplUnitTest, CallbackStubUnitTest_011, TestSize.Level4)
{
    auto audioSleCb = new (std::nothrow) SleAudioOperationCallbackStubImpl();
    EXPECT_NE(audioSleCb, nullptr);
    std::string device = "test";
    uint32_t delayValue = 0;
    auto ret = audioSleCb->GetRenderPosition(device, delayValue);
    EXPECT_NE(SUCCESS, ret);
    std::shared_ptr<SleAudioOperationCallback> sleAudioOperationCallback;
    audioSleCb->sleAudioOperationCallback_ = sleAudioOperationCallback;
    ret = audioSleCb->GetRenderPosition(device, delayValue);
    EXPECT_NE(-1, ret);
    delete audioSleCb;
}
} // namespace AudioStandard
} // namespace OHOS
