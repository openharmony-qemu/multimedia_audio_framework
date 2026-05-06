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
#include "adapter/bluetooth_device_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
using BtAudioSampleAttributes = OHOS::HDI::Audio_Bluetooth::AudioSampleAttributes;
using BtAudioDeviceDescriptor = OHOS::HDI::Audio_Bluetooth::AudioDeviceDescriptor;

class BluetoothDeviceManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}

protected:
    std::string adapterName_ = "bt_test";
};

/**
 * @tc.name   : BluetoothDeviceManager_GetHdiId_001
 * @tc.number : BluetoothDeviceManager_GetHdiId_001
 * @tc.desc   : Test BluetoothDeviceManager helper branches for render/capture ids.
 */
HWTEST_F(BluetoothDeviceManagerUnitTest, BluetoothDeviceManager_GetHdiId_001, TestSize.Level1)
{
    BluetoothDeviceManager manager;
    EXPECT_EQ(manager.GetHdiRenderId(nullptr), 0);
    EXPECT_EQ(manager.GetHdiCaptureId(nullptr), 0);

    auto wrapper = std::make_shared<BluetoothAdapterWrapper>();
    wrapper->renders_[0] = reinterpret_cast<BtAudioRender *>(0x1);
    wrapper->captures_[1] = reinterpret_cast<BtAudioCapture *>(0x2);
    EXPECT_EQ(manager.GetHdiRenderId(wrapper), 1);
    EXPECT_EQ(manager.GetHdiCaptureId(wrapper), 1);

    wrapper->freeHdiRenderIdSet_.insert(7);
    wrapper->freeHdiCaptureIdSet_.insert(9);
    EXPECT_EQ(manager.GetHdiRenderId(wrapper), 7);
    EXPECT_EQ(wrapper->freeHdiRenderIdSet_.count(7), 0);
    EXPECT_EQ(manager.GetHdiCaptureId(wrapper), 9);
    EXPECT_EQ(wrapper->freeHdiCaptureIdSet_.count(9), 0);
}

/**
 * @tc.name   : BluetoothDeviceManager_UnloadAdapter_001
 * @tc.number : BluetoothDeviceManager_UnloadAdapter_001
 * @tc.desc   : Test BluetoothDeviceManager unload adapter busy branch.
 */
HWTEST_F(BluetoothDeviceManagerUnitTest, BluetoothDeviceManager_UnloadAdapter_001, TestSize.Level1)
{
    BluetoothDeviceManager manager;
    manager.audioManager_ = reinterpret_cast<BtAudioProxyManager *>(0x1);

    auto wrapper = std::make_shared<BluetoothAdapterWrapper>();
    wrapper->adapter_ = reinterpret_cast<BtAudioAdapter *>(0x1);
    wrapper->captures_[1] = reinterpret_cast<BtAudioCapture *>(0x1);
    manager.adapters_[adapterName_] = wrapper;

    manager.UnloadAdapter(adapterName_);
    EXPECT_EQ(manager.adapters_.count(adapterName_), 1);
    EXPECT_NE(manager.adapters_[adapterName_], nullptr);
}

/**
 * @tc.name   : BluetoothDeviceManager_CreatePort_001
 * @tc.number : BluetoothDeviceManager_CreatePort_001
 * @tc.desc   : Test BluetoothDeviceManager create render/capture with null adapter.
 */
HWTEST_F(BluetoothDeviceManagerUnitTest, BluetoothDeviceManager_CreatePort_001, TestSize.Level1)
{
    BluetoothDeviceManager manager;
    auto wrapper = std::make_shared<BluetoothAdapterWrapper>();
    manager.adapters_[adapterName_] = wrapper;

    BtAudioSampleAttributes param = {};
    BtAudioDeviceDescriptor deviceDesc = {};
    uint32_t hdiId = 0;

    void *render = manager.CreateRender(adapterName_, &param, &deviceDesc, hdiId);
    EXPECT_EQ(render, nullptr);
    EXPECT_TRUE(wrapper->renders_.empty());

    void *capture = manager.CreateCapture(adapterName_, &param, &deviceDesc, hdiId);
    EXPECT_EQ(capture, nullptr);
    EXPECT_TRUE(wrapper->captures_.empty());
}

/**
 * @tc.name   : BluetoothDeviceManager_DestroyPort_001
 * @tc.number : BluetoothDeviceManager_DestroyPort_001
 * @tc.desc   : Test BluetoothDeviceManager destroy render/capture with null adapter.
 */
HWTEST_F(BluetoothDeviceManagerUnitTest, BluetoothDeviceManager_DestroyPort_001, TestSize.Level1)
{
    BluetoothDeviceManager manager;
    auto wrapper = std::make_shared<BluetoothAdapterWrapper>();
    wrapper->renders_[1] = reinterpret_cast<BtAudioRender *>(0x1);
    wrapper->captures_[2] = reinterpret_cast<BtAudioCapture *>(0x2);
    manager.adapters_[adapterName_] = wrapper;

    manager.DestroyRender(adapterName_, 1);
    EXPECT_EQ(wrapper->renders_.count(1), 1);

    manager.DestroyCapture(adapterName_, 2);
    EXPECT_EQ(wrapper->captures_.count(2), 1);
}

/**
 * @tc.name   : BluetoothDeviceManager_DumpInfo_001
 * @tc.number : BluetoothDeviceManager_DumpInfo_001
 * @tc.desc   : Test BluetoothDeviceManager dump info with wrapper and null wrapper.
 */
HWTEST_F(BluetoothDeviceManagerUnitTest, BluetoothDeviceManager_DumpInfo_001, TestSize.Level1)
{
    BluetoothDeviceManager manager;
    auto wrapper = std::make_shared<BluetoothAdapterWrapper>();
    wrapper->renders_[3] = reinterpret_cast<BtAudioRender *>(0x3);
    wrapper->captures_[4] = reinterpret_cast<BtAudioCapture *>(0x4);
    manager.adapters_["bt_valid"] = wrapper;
    manager.adapters_["bt_null"] = nullptr;

    std::string dumpString;
    manager.DumpInfo(dumpString);

    EXPECT_NE(dumpString.find("bt/bt_valid"), std::string::npos);
    EXPECT_NE(dumpString.find("renderNum: 1"), std::string::npos);
    EXPECT_NE(dumpString.find("captureNum: 1"), std::string::npos);
    EXPECT_NE(dumpString.find("bt/bt_null"), std::string::npos);
}
} // namespace AudioStandard
} // namespace OHOS
