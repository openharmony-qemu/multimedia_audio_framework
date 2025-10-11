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

#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "audio_utils.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "manager/hdi_adapter_factory.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class AdapterUnitTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    virtual void SetUp() {}
    virtual void TearDown() {}

    void TestAction(HdiDeviceManagerType type);
    void TestSetAndGet(HdiDeviceManagerType type);

protected:
    std::string adapterName_ = "test";
};

void AdapterUnitTest::TestAction(HdiDeviceManagerType type)
{
    std::shared_ptr<IDeviceManager> deviceManager = HdiAdapterManager::GetInstance().GetDeviceManager(type);
    ASSERT_NE(deviceManager, nullptr);

    auto ret = deviceManager->LoadAdapter(adapterName_);
    EXPECT_NE(ret, SUCCESS);

    uint32_t id = 0;
    void *render = deviceManager->CreateRender(adapterName_, nullptr, nullptr, id);
    EXPECT_EQ(render, nullptr);
    deviceManager->DestroyRender(adapterName_, id);

    void *capture = deviceManager->CreateCapture(adapterName_, nullptr, nullptr, id);
    EXPECT_EQ(capture, nullptr);
    deviceManager->DestroyCapture(adapterName_, id);

    deviceManager->UnloadAdapter(adapterName_);
}

void AdapterUnitTest::TestSetAndGet(HdiDeviceManagerType type)
{
    std::shared_ptr<IDeviceManager> deviceManager = HdiAdapterManager::GetInstance().GetDeviceManager(type);
    ASSERT_NE(deviceManager, nullptr);

    deviceManager->SetAudioParameter(adapterName_, AudioParamKey::NONE, "", "");

    std::string value = deviceManager->GetAudioParameter(adapterName_, AudioParamKey::NONE, "");
    EXPECT_EQ(value, "");

    auto ret = deviceManager->SetVoiceVolume(adapterName_, 0.0);
    EXPECT_NE(ret, SUCCESS);

    ret = deviceManager->SetOutputRoute(adapterName_, { DEVICE_TYPE_SPEAKER, DEVICE_TYPE_EARPIECE }, 0);
    EXPECT_NE(ret, SUCCESS);

    ret = deviceManager->SetInputRoute(adapterName_, DEVICE_TYPE_MIC, 0, 0);
    EXPECT_NE(ret, SUCCESS);

    deviceManager->SetMicMute(adapterName_, false);
}

/**
 * @tc.name   : Test Adapter API
 * @tc.number : AdapterUnitTest_001
 * @tc.desc   : Test local adapter action
 */
HWTEST_F(AdapterUnitTest, AdapterUnitTest_001, TestSize.Level1)
{
    TestAction(HDI_DEVICE_MANAGER_TYPE_LOCAL);
}

/**
 * @tc.name   : Test Adapter API
 * @tc.number : AdapterUnitTest_002
 * @tc.desc   : Test local adapter set/get operation
 */
HWTEST_F(AdapterUnitTest, AdapterUnitTest_002, TestSize.Level1)
{
    TestSetAndGet(HDI_DEVICE_MANAGER_TYPE_LOCAL);
}

/**
 * @tc.name   : Test Adapter API
 * @tc.number : AdapterUnitTest_003
 * @tc.desc   : Test bt adapter action
 */
HWTEST_F(AdapterUnitTest, AdapterUnitTest_003, TestSize.Level1)
{
    TestAction(HDI_DEVICE_MANAGER_TYPE_BLUETOOTH);
}

/**
 * @tc.name   : Test Adapter API
 * @tc.number : AdapterUnitTest_004
 * @tc.desc   : Test bt adapter set/get operation
 */
HWTEST_F(AdapterUnitTest, AdapterUnitTest_004, TestSize.Level1)
{
    TestSetAndGet(HDI_DEVICE_MANAGER_TYPE_BLUETOOTH);
}

/**
 * @tc.name   : Test Adapter API
 * @tc.number : AdapterUnitTest_005
 * @tc.desc   : Test remote adapter action
 */
HWTEST_F(AdapterUnitTest, AdapterUnitTest_005, TestSize.Level1)
{
    TestAction(HDI_DEVICE_MANAGER_TYPE_REMOTE);
}

/**
 * @tc.name   : Test Adapter API
 * @tc.number : AdapterUnitTest_006
 * @tc.desc   : Test remote adapter set/get operation
 */
HWTEST_F(AdapterUnitTest, AdapterUnitTest_006, TestSize.Level1)
{
    TestSetAndGet(HDI_DEVICE_MANAGER_TYPE_REMOTE);
}

/**
 * @tc.name   : Test Adapter API
 * @tc.number : AdapterUnitTest_007
 * @tc.desc   : Test CreateBluetoothRenderSink
 */
HWTEST_F(AdapterUnitTest, AdapterUnitTest_007, TestSize.Level1)
{
    HdiAdapterFactory &fac = HdiAdapterFactory::GetInstance();
    auto sink = fac.CreateBluetoothRenderSink(HDI_ID_INFO_HEARING_AID);
    ASSERT_TRUE(sink != nullptr);

    sink = fac.CreateBluetoothRenderSink(HDI_ID_INFO_MMAP);
    ASSERT_TRUE(sink != nullptr);

    sink = fac.CreateBluetoothRenderSink(HDI_ID_INFO_DEFAULT);
    ASSERT_TRUE(sink != nullptr);
}

/**
 * @tc.name   : Test DoSetSinkPrestoreInfo API
 * @tc.number : DoSetSinkPrestoreInfo_001
 * @tc.desc   : Test DoSetSinkPrestoreInfo_001
 */
HWTEST_F(AdapterUnitTest, DoSetSinkPrestoreInfo_001, TestSize.Level1)
{
    HdiAdapterFactory &fac = HdiAdapterFactory::GetInstance();
    auto sink = fac.CreateBluetoothRenderSink("bluetooth");

    HdiAdapterManager::GetInstance().
        UpdateSinkPrestoreInfo<std::pair<AudioParamKey, std::pair<std::string, std::string>>>(
            PRESTORE_INFO_AUDIO_BT_PARAM, {AudioParamKey::A2DP_SUSPEND_STATE, {"", "test"}});
    HdiAdapterManager::GetInstance().DoSetSinkPrestoreInfo(sink, HDI_ID_TYPE_PRIMARY);

    std::pair<AudioParamKey, std::pair<std::string, std::string>> param = {AudioParamKey::NONE, {"", ""}};
    int32_t ret = HdiAdapterManager::GetInstance().sinkPrestoreInfo_.Get(PRESTORE_INFO_AUDIO_BT_PARAM, param);
    EXPECT_EQ(param.second.second, "test");

    HdiAdapterManager::GetInstance().DoSetSinkPrestoreInfo(sink, HDI_ID_TYPE_BLUETOOTH);
    param = {AudioParamKey::NONE, {"", ""}};
    ret = HdiAdapterManager::GetInstance().sinkPrestoreInfo_.Get(PRESTORE_INFO_AUDIO_BT_PARAM, param);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

} // namespace AudioStandard
} // namespace OHOS
