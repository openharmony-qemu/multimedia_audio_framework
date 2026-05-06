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
#include "adapter/local_device_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class LocalDeviceManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}

protected:
    std::string adapterName_ = "local_test";
};

/**
 * @tc.name   : LocalDeviceManager_UnloadAdapter_001
 * @tc.number : LocalDeviceManager_UnloadAdapter_001
 * @tc.desc   : Test LocalDeviceManager unload adapter busy branch.
 */
HWTEST_F(LocalDeviceManagerUnitTest, LocalDeviceManager_UnloadAdapter_001, TestSize.Level1)
{
    LocalDeviceManager manager;
    manager.audioManager_ = reinterpret_cast<struct IAudioManager *>(0x1);

    auto wrapper = std::make_shared<LocalAdapterWrapper>();
    wrapper->adapter_ = reinterpret_cast<struct IAudioAdapter *>(0x1);
    wrapper->hdiRenderIds_.insert(1);
    manager.adapters_[adapterName_] = wrapper;

    manager.UnloadAdapter(adapterName_);
    EXPECT_EQ(manager.adapters_.count(adapterName_), 1);
    EXPECT_NE(manager.adapters_[adapterName_], nullptr);
}

/**
 * @tc.name   : LocalDeviceManager_CreatePort_001
 * @tc.number : LocalDeviceManager_CreatePort_001
 * @tc.desc   : Test LocalDeviceManager create render/capture with null adapter.
 */
HWTEST_F(LocalDeviceManagerUnitTest, LocalDeviceManager_CreatePort_001, TestSize.Level1)
{
    LocalDeviceManager manager;
    auto wrapper = std::make_shared<LocalAdapterWrapper>();
    manager.adapters_[adapterName_] = wrapper;

    struct AudioSampleAttributes param = {};
    struct AudioDeviceDescriptor deviceDesc = {};
    uint32_t hdiId = 0;

    void *render = manager.CreateRender(adapterName_, &param, &deviceDesc, hdiId);
    EXPECT_EQ(render, nullptr);
    EXPECT_TRUE(wrapper->hdiRenderIds_.empty());

    void *capture = manager.CreateCapture(adapterName_, &param, &deviceDesc, hdiId);
    EXPECT_EQ(capture, nullptr);
    EXPECT_TRUE(wrapper->hdiCaptureIds_.empty());
}

/**
 * @tc.name   : LocalDeviceManager_DestroyPort_001
 * @tc.number : LocalDeviceManager_DestroyPort_001
 * @tc.desc   : Test LocalDeviceManager destroy render/capture with null adapter.
 */
HWTEST_F(LocalDeviceManagerUnitTest, LocalDeviceManager_DestroyPort_001, TestSize.Level1)
{
    LocalDeviceManager manager;
    auto wrapper = std::make_shared<LocalAdapterWrapper>();
    wrapper->hdiRenderIds_.insert(1);
    wrapper->hdiCaptureIds_.insert(2);
    manager.adapters_[adapterName_] = wrapper;

    manager.DestroyRender(adapterName_, 1);
    EXPECT_EQ(wrapper->hdiRenderIds_.count(1), 1);

    manager.DestroyCapture(adapterName_, 2);
    EXPECT_EQ(wrapper->hdiCaptureIds_.count(2), 1);
}

/**
 * @tc.name   : LocalDeviceManager_DumpInfo_001
 * @tc.number : LocalDeviceManager_DumpInfo_001
 * @tc.desc   : Test LocalDeviceManager dump info with wrapper and null wrapper.
 */
HWTEST_F(LocalDeviceManagerUnitTest, LocalDeviceManager_DumpInfo_001, TestSize.Level1)
{
    LocalDeviceManager manager;
    auto wrapper = std::make_shared<LocalAdapterWrapper>();
    wrapper->hdiRenderIds_.insert(11);
    wrapper->hdiCaptureIds_.insert(22);
    manager.adapters_["local_valid"] = wrapper;
    manager.adapters_["local_null"] = nullptr;

    std::string dumpString;
    manager.DumpInfo(dumpString);

    EXPECT_NE(dumpString.find("local/local_valid"), std::string::npos);
    EXPECT_NE(dumpString.find("renderNum: 1"), std::string::npos);
    EXPECT_NE(dumpString.find("captureNum: 1"), std::string::npos);
    EXPECT_NE(dumpString.find("local/local_null"), std::string::npos);
}
} // namespace AudioStandard
} // namespace OHOS
