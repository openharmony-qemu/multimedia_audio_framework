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
#include <thread>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "audio_utils.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "manager/hdi_monitor.h"
#include "util/id_handler.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class ManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void ThreadFunc(bool *isDone);
};

void ManagerUnitTest::ThreadFunc(bool *isDone)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::unordered_map<uint32_t, uint32_t> renderMap;
    std::unordered_map<uint32_t, uint32_t> captureMap;
    int32_t num = 10;
    int32_t mod = 2;

    for (int i = 0; i < num; i++) {
        if (i % mod == 0) {
            uint32_t renderId = manager.GetRenderIdByDeviceClass("remote", "info", true);
            manager.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_REMOTE, "info", false);
            renderMap[renderId]++;
        } else {
            uint32_t captureId = manager.GetCaptureIdByDeviceClass("remote", SOURCE_TYPE_MIC, "info", true);
            captureMap[captureId]++;
        }
    }

    for (auto &item : renderMap) {
        uint32_t cnt = item.second;
        for (uint32_t i = 0; i < cnt; i++) {
            uint32_t id = item.first;
            manager.ReleaseId(id);
        }
    }
    for (auto &item : captureMap) {
        uint32_t cnt = item.second;
        for (uint32_t i = 0; i < cnt; i++) {
            uint32_t id = item.first;
            manager.ReleaseId(id);
        }
    }
    *isDone = true;
}

/**
 * @tc.name   : Test Manager API
 * @tc.number : ManagerUnitTest_001
 * @tc.desc   : Test manager action
 */
HWTEST_F(ManagerUnitTest, ManagerUnitTest_001, TestSize.Level1)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    uint32_t id = manager.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY);
    EXPECT_NE(id, HDI_INVALID_ID);

    id = manager.GetRenderIdByDeviceClass("");
    EXPECT_EQ(id, HDI_INVALID_ID);

    id = manager.GetCaptureIdByDeviceClass("", SOURCE_TYPE_MIC);
    EXPECT_EQ(id, HDI_INVALID_ID);

    id = manager.GetCaptureIdByDeviceClass("offload", SOURCE_TYPE_MIC);
    EXPECT_EQ(id, 0X1700);

    id = manager.GetCaptureIdByDeviceClass("invalid", SOURCE_TYPE_MIC);
    EXPECT_EQ(id, HDI_INVALID_ID);

    manager.ReleaseId(id);

    std::shared_ptr<IAudioRenderSink> sink = manager.GetRenderSink(id);
    EXPECT_EQ(sink, nullptr);

    std::shared_ptr<IAudioCaptureSource> source = manager.GetCaptureSource(id);
    EXPECT_EQ(source, nullptr);

    std::function<int32_t(uint32_t, std::shared_ptr<IAudioRenderSink>)> sinkProcessFunc =
        [](uint32_t id, std::shared_ptr<IAudioRenderSink> sink) -> bool { return SUCCESS; };
    auto ret = manager.ProcessSink(sinkProcessFunc);
    EXPECT_EQ(ret, SUCCESS);

    std::function<int32_t(uint32_t, std::shared_ptr<IAudioCaptureSource>)> sourceProcessFunc =
        [](uint32_t id, std::shared_ptr<IAudioCaptureSource> source) -> bool { return SUCCESS; };
    ret = manager.ProcessSource(sourceProcessFunc);
    EXPECT_EQ(ret, SUCCESS);

    manager.UpdateSinkPrestoreInfo<bool>("test", true);

    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    EXPECT_NE(deviceManager, nullptr);

    manager.ReleaseDeviceManager(HDI_DEVICE_MANAGER_TYPE_NUM);

    HdiMonitor::ReportHdiException(LOCAL, CALL_HDI_FAILED, 0, "test report hdi");
}

/**
 * @tc.name   : Test Manager API
 * @tc.number : ManagerUnitTest_002
 * @tc.desc   : Test manager action
 */
HWTEST_F(ManagerUnitTest, ManagerUnitTest_002, TestSize.Level1)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    uint32_t id = manager.GetRenderIdByDeviceClass("remote", "info", true);
    uint32_t oldId = id;
    manager.ReleaseId(id);

    int32_t num = 5;
    for (int i = 0; i < num; i++) {
        bool isDone0 = false;
        bool isDone1 = false;

        std::thread t0(ManagerUnitTest::ThreadFunc, &isDone0);
        std::thread t1(ManagerUnitTest::ThreadFunc, &isDone1);

        t0.join();
        t1.join();

        EXPECT_TRUE(isDone0);
        EXPECT_TRUE(isDone1);
    }

    uint32_t newId = manager.GetRenderIdByDeviceClass("remote", "info1", true);
    EXPECT_EQ(newId, oldId);
}

/**
 * @tc.name   : Test Manager API
 * @tc.number : GetId_001
 * @tc.desc   : Test GetId action
 */
HWTEST_F(ManagerUnitTest, GetId_001, TestSize.Level1)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    uint32_t id = manager.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_REMOTE, "test", false, false);
    std::shared_ptr<IAudioRenderSink> sink = manager.GetRenderSink(id, false);
    IdHandler &idHandler = IdHandler::GetInstance();
    uint32_t infoId = id & idHandler.HDI_ID_INFO_MASK;
    EXPECT_EQ(idHandler.infoIdMap_[infoId].useIdSet_.size(), 0);
}

/**
 * @tc.name   : Test Manager API
 * @tc.number : GetId_002
 * @tc.desc   : Test GetId action
 */
HWTEST_F(ManagerUnitTest, GetId_002, TestSize.Level1)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    uint32_t id = manager.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_REMOTE, "test", false);
    std::shared_ptr<IAudioRenderSink> sink = manager.GetRenderSink(id, true);
    id = manager.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_REMOTE, "test", false, false);
    IdHandler &idHandler = IdHandler::GetInstance();
    uint32_t infoId = id & idHandler.HDI_ID_INFO_MASK;
    EXPECT_NE(idHandler.infoIdMap_[infoId].useIdSet_.size(), 0);
    manager.ReleaseId(id);
}
} // namespace AudioStandard
} // namespace OHOS
