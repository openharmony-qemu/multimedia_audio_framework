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
#include "adapter/remote_device_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
using RemoteAudioSampleAttributes = OHOS::HDI::DistributedAudio::Audio::V2_0::AudioSampleAttributes;
using RemoteAudioDeviceDescriptor = OHOS::HDI::DistributedAudio::Audio::V2_0::AudioDeviceDescriptor;

class RemoteDeviceManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}

protected:
    std::string adapterName_ = "remote_test";
};

/**
 * @tc.name   : RemoteDeviceManager_UnloadAdapter_001
 * @tc.number : RemoteDeviceManager_UnloadAdapter_001
 * @tc.desc   : Test RemoteDeviceManager unload adapter busy branch.
 */
HWTEST_F(RemoteDeviceManagerUnitTest, RemoteDeviceManager_UnloadAdapter_001, TestSize.Level1)
{
    RemoteDeviceManager manager;
    manager.audioManager_.ForceSetRefPtr(reinterpret_cast<RemoteIAudioManager *>(0x1));

    auto wrapper = std::make_shared<RemoteAdapterWrapper>(adapterName_);
    wrapper->adapter_.ForceSetRefPtr(reinterpret_cast<RemoteIAudioAdapter *>(0x1));
    wrapper->hdiRenderIds_.insert(1);
    manager.adapters_[adapterName_] = wrapper;

    manager.UnloadAdapter(adapterName_);
    EXPECT_EQ(manager.adapters_.count(adapterName_), 1);

    wrapper->adapter_.ForceSetRefPtr(nullptr);
    manager.audioManager_.ForceSetRefPtr(nullptr);
}

/**
 * @tc.name   : RemoteDeviceManager_CreatePort_001
 * @tc.number : RemoteDeviceManager_CreatePort_001
 * @tc.desc   : Test RemoteDeviceManager create render/capture with null adapter.
 */
HWTEST_F(RemoteDeviceManagerUnitTest, RemoteDeviceManager_CreatePort_001, TestSize.Level1)
{
    RemoteDeviceManager manager;
    auto wrapper = std::make_shared<RemoteAdapterWrapper>(adapterName_);
    manager.adapters_[adapterName_] = wrapper;

    RemoteAudioSampleAttributes param = {};
    RemoteAudioDeviceDescriptor deviceDesc = {};
    uint32_t hdiId = 0;

    void *render = manager.CreateRender(adapterName_, &param, &deviceDesc, hdiId);
    EXPECT_EQ(render, nullptr);
    EXPECT_TRUE(wrapper->hdiRenderIds_.empty());

    void *capture = manager.CreateCapture(adapterName_, &param, &deviceDesc, hdiId);
    EXPECT_EQ(capture, nullptr);
    EXPECT_TRUE(wrapper->hdiCaptureIds_.empty());
}

/**
 * @tc.name   : RemoteDeviceManager_DestroyPort_001
 * @tc.number : RemoteDeviceManager_DestroyPort_001
 * @tc.desc   : Test RemoteDeviceManager destroy render/capture with null adapter.
 */
HWTEST_F(RemoteDeviceManagerUnitTest, RemoteDeviceManager_DestroyPort_001, TestSize.Level1)
{
    RemoteDeviceManager manager;
    auto wrapper = std::make_shared<RemoteAdapterWrapper>(adapterName_);
    wrapper->hdiRenderIds_.insert(1);
    wrapper->hdiCaptureIds_.insert(2);
    manager.adapters_[adapterName_] = wrapper;

    manager.DestroyRender(adapterName_, 1);
    EXPECT_EQ(wrapper->hdiRenderIds_.count(1), 1);

    manager.DestroyCapture(adapterName_, 2);
    EXPECT_EQ(wrapper->hdiCaptureIds_.count(2), 1);
}

/**
 * @tc.name   : RemoteDeviceManager_DestroyRenderNotExist_001
 * @tc.number : RemoteDeviceManager_DestroyRenderNotExist_001
 * @tc.desc   : Test RemoteDeviceManager render not exist branches.
 */
HWTEST_F(RemoteDeviceManagerUnitTest, RemoteDeviceManager_DestroyRenderNotExist_001, TestSize.Level1)
{
    RemoteDeviceManager manager;
    auto wrapper = std::make_shared<RemoteAdapterWrapper>(adapterName_);
    wrapper->adapter_.ForceSetRefPtr(reinterpret_cast<RemoteIAudioAdapter *>(0x1));
    manager.adapters_[adapterName_] = wrapper;

    wrapper->isValid_ = true;
    manager.DestroyRender(adapterName_, 10);
    EXPECT_EQ(manager.adapters_.count(adapterName_), 1);

    wrapper->isValid_ = false;
    manager.DestroyRender(adapterName_, 10);
    EXPECT_EQ(manager.adapters_.count(adapterName_), 1);

    wrapper->adapter_.ForceSetRefPtr(nullptr);
}

/**
 * @tc.name   : RemoteDeviceManager_DestroyCaptureNotExist_001
 * @tc.number : RemoteDeviceManager_DestroyCaptureNotExist_001
 * @tc.desc   : Test RemoteDeviceManager capture not exist branches.
 */
HWTEST_F(RemoteDeviceManagerUnitTest, RemoteDeviceManager_DestroyCaptureNotExist_001, TestSize.Level1)
{
    RemoteDeviceManager manager;
    auto wrapper = std::make_shared<RemoteAdapterWrapper>(adapterName_);
    wrapper->adapter_.ForceSetRefPtr(reinterpret_cast<RemoteIAudioAdapter *>(0x1));
    manager.adapters_[adapterName_] = wrapper;

    wrapper->isValid_ = true;
    manager.DestroyCapture(adapterName_, 20);
    EXPECT_EQ(manager.adapters_.count(adapterName_), 1);

    wrapper->isValid_ = false;
    manager.DestroyCapture(adapterName_, 20);
    EXPECT_EQ(manager.adapters_.count(adapterName_), 1);

    wrapper->adapter_.ForceSetRefPtr(nullptr);
}

/**
 * @tc.name   : RemoteDeviceManager_DumpInfo_001
 * @tc.number : RemoteDeviceManager_DumpInfo_001
 * @tc.desc   : Test RemoteDeviceManager dump info with wrapper and null wrapper.
 */
HWTEST_F(RemoteDeviceManagerUnitTest, RemoteDeviceManager_DumpInfo_001, TestSize.Level1)
{
    RemoteDeviceManager manager;
    auto wrapper = std::make_shared<RemoteAdapterWrapper>(adapterName_);
    wrapper->hdiRenderIds_.insert(11);
    wrapper->hdiCaptureIds_.insert(22);
    manager.adapters_["remote_valid"] = wrapper;
    manager.adapters_["remote_null"] = nullptr;

    std::string dumpString;
    manager.DumpInfo(dumpString);

    EXPECT_NE(dumpString.find("remote/remote_valid"), std::string::npos);
    EXPECT_NE(dumpString.find("renderNum: 1"), std::string::npos);
    EXPECT_NE(dumpString.find("captureNum: 1"), std::string::npos);
    EXPECT_NE(dumpString.find("remote/remote_null"), std::string::npos);
}
} // namespace AudioStandard
} // namespace OHOS
