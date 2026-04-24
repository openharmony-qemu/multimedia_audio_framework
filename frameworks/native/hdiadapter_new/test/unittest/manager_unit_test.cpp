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
class MockDeviceManager : public IDeviceManager {
public:
    MOCK_METHOD(int32_t, SetAudioParameter,
        (const std::string &adapterName, const AudioParamKey key, const std::string &condition,
        const std::string &value), (override));
    MOCK_METHOD(std::string, GetAudioParameter,
        (const std::string &adapterName, const AudioParamKey key, const std::string &condition), (override));

    int32_t LoadAdapter(const std::string &adapterName) override
    {
        (void)adapterName;
        return SUCCESS;
    }

    void UnloadAdapter(const std::string &adapterName, bool force = false) override
    {
        (void)adapterName;
        (void)force;
    }

    void AllAdapterSetMicMute(bool isMute) override
    {
        (void)isMute;
    }

    int32_t SetVoiceVolume(const std::string &adapterName, float volume) override
    {
        (void)adapterName;
        (void)volume;
        return SUCCESS;
    }

    int32_t SetOutputRoute(const std::string &adapterName, const std::vector<DeviceType> &devices,
        int32_t streamId) override
    {
        (void)adapterName;
        (void)devices;
        (void)streamId;
        return SUCCESS;
    }

    int32_t SetInputRoute(const std::string &adapterName, DeviceType device, int32_t streamId,
        int32_t inputType) override
    {
        (void)adapterName;
        (void)device;
        (void)streamId;
        (void)inputType;
        return SUCCESS;
    }

    void SetMicMute(const std::string &adapterName, bool isMute) override
    {
        (void)adapterName;
        (void)isMute;
    }

    void *CreateRender(const std::string &adapterName, void *param, void *deviceDesc,
        uint32_t &hdiRenderId) override
    {
        (void)adapterName;
        (void)param;
        (void)deviceDesc;
        (void)hdiRenderId;
        return nullptr;
    }

    void DestroyRender(const std::string &adapterName, uint32_t hdiRenderId) override
    {
        (void)adapterName;
        (void)hdiRenderId;
    }

    void *CreateCapture(const std::string &adapterName, void *param, void *deviceDesc,
        uint32_t &hdiCaptureId) override
    {
        (void)adapterName;
        (void)param;
        (void)deviceDesc;
        (void)hdiCaptureId;
        return nullptr;
    }

    void DestroyCapture(const std::string &adapterName, uint32_t hdiCaptureId) override
    {
        (void)adapterName;
        (void)hdiCaptureId;
    }

    void DumpInfo(std::string &dumpString) override
    {
        (void)dumpString;
    }

    void SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType) override
    {
        (void)dmDeviceType;
        (void)deviceType;
    }

    void SetAudioScene(const AudioScene scene) override
    {
        (void)scene;
    }
};

class ManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    virtual void SetUp()
    {
        HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
        manager.deviceManagers_[HDI_DEVICE_MANAGER_TYPE_LOCAL].reset();
        manager.deviceManagers_[HDI_DEVICE_MANAGER_TYPE_REMOTE].reset();
    }

    virtual void TearDown()
    {
        HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
        manager.deviceManagers_[HDI_DEVICE_MANAGER_TYPE_LOCAL].reset();
        manager.deviceManagers_[HDI_DEVICE_MANAGER_TYPE_REMOTE].reset();
    }

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
 * @tc.number : ManagerUnitTest_003
 * @tc.desc   : Test manager action
 */
HWTEST_F(ManagerUnitTest, ManagerUnitTest_003, TestSize.Level1)
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

/**
 * @tc.name   : SetExtraParameters_001
 * @tc.number : SetExtraParameters_001
 * @tc.desc   : Verify local extra parameter forwarding behavior.
 */
HWTEST_F(ManagerUnitTest, SetExtraParameters_001, TestSize.Level1)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    auto mockDeviceManager = std::make_shared<MockDeviceManager>();
    manager.deviceManagers_[HDI_DEVICE_MANAGER_TYPE_LOCAL] = mockDeviceManager;

    EXPECT_CALL(*mockDeviceManager, SetAudioParameter("primary", AudioParamKey::NONE, "", "k1=v1;k2=v2;"))
        .WillOnce(testing::Return(SUCCESS));

    int32_t ret = manager.SetExtraParameters(HDI_DEVICE_MANAGER_TYPE_LOCAL, "primary", "", "k1=v1;k2=v2;");
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : SetExtraParameters_002
 * @tc.number : SetExtraParameters_002
 * @tc.desc   : Verify remote extra parameter forwarding behavior.
 */
HWTEST_F(ManagerUnitTest, SetExtraParameters_002, TestSize.Level1)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    auto mockDeviceManager = std::make_shared<MockDeviceManager>();
    manager.deviceManagers_[HDI_DEVICE_MANAGER_TYPE_REMOTE] = mockDeviceManager;

    EXPECT_CALL(*mockDeviceManager, SetAudioParameter("networkId_001", AudioParamKey::NONE, "zone_id_change",
        "zone_001")).WillOnce(testing::Return(SUCCESS));

    int32_t ret = manager.SetExtraParameters(HDI_DEVICE_MANAGER_TYPE_REMOTE, "networkId_001",
        "zone_id_change", "zone_001");
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : SetAudioParameter_001
 * @tc.number : SetAudioParameter_001
 * @tc.desc   : Verify direct audio parameter forwarding behavior.
 */
HWTEST_F(ManagerUnitTest, SetAudioParameter_001, TestSize.Level1)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    auto mockDeviceManager = std::make_shared<MockDeviceManager>();
    manager.deviceManagers_[HDI_DEVICE_MANAGER_TYPE_LOCAL] = mockDeviceManager;

    EXPECT_CALL(*mockDeviceManager, SetAudioParameter("primary", AudioParamKey::MMI, "", "mmi=on"))
        .WillOnce(testing::Return(SUCCESS));

    int32_t ret = manager.SetAudioParameter(HDI_DEVICE_MANAGER_TYPE_LOCAL, "primary",
        AudioParamKey::MMI, "", "mmi=on");
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : GetAudioParameter_001
 * @tc.number : GetAudioParameter_001
 * @tc.desc   : Verify direct audio parameter query forwarding behavior.
 */
HWTEST_F(ManagerUnitTest, GetAudioParameter_001, TestSize.Level1)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    auto mockDeviceManager = std::make_shared<MockDeviceManager>();
    manager.deviceManagers_[HDI_DEVICE_MANAGER_TYPE_REMOTE] = mockDeviceManager;

    EXPECT_CALL(*mockDeviceManager, GetAudioParameter("networkId_001", AudioParamKey::PERF_INFO, "perf_info"))
        .WillOnce(testing::Return("perf_value"));

    std::string value = manager.GetAudioParameter(HDI_DEVICE_MANAGER_TYPE_REMOTE, "networkId_001",
        AudioParamKey::PERF_INFO, "perf_info");
    EXPECT_EQ(value, "perf_value");
}

/**
 * @tc.name   : GetAudioParameterByKey_001
 * @tc.number : GetAudioParameterByKey_001
 * @tc.desc   : Verify special key is forwarded to local device manager.
 */
HWTEST_F(ManagerUnitTest, GetAudioParameterByKey_001, TestSize.Level1)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    auto mockDeviceManager = std::make_shared<MockDeviceManager>();
    manager.deviceManagers_[HDI_DEVICE_MANAGER_TYPE_LOCAL] = mockDeviceManager;

    EXPECT_CALL(*mockDeviceManager, GetAudioParameter("primary", AudioParamKey::PERF_INFO, "perf_info"))
        .WillOnce(testing::Return("perf_value"));

    auto value = manager.GetAudioParameterByKey(HDI_DEVICE_MANAGER_TYPE_LOCAL, "perf_info");
    EXPECT_EQ(value, "perf_value");
}

/**
 * @tc.name   : GetAudioParameterByKey_002
 * @tc.number : GetAudioParameterByKey_002
 * @tc.desc   : Verify mmi prefix is converted before forwarding.
 */
HWTEST_F(ManagerUnitTest, GetAudioParameterByKey_002, TestSize.Level1)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    auto mockDeviceManager = std::make_shared<MockDeviceManager>();
    manager.deviceManagers_[HDI_DEVICE_MANAGER_TYPE_LOCAL] = mockDeviceManager;

    EXPECT_CALL(*mockDeviceManager, GetAudioParameter("primary", AudioParamKey::MMI, "speaker"))
        .WillOnce(testing::Return("mmi_value"));

    auto value = manager.GetAudioParameterByKey(HDI_DEVICE_MANAGER_TYPE_LOCAL, "mmi_speaker");
    EXPECT_EQ(value, "mmi_value");
}

/**
 * @tc.name   : GetAudioParameterByKey_003
 * @tc.number : GetAudioParameterByKey_003
 * @tc.desc   : Verify GetAudioParameterByKey return nullopt when deviceManager is nullptr.
 */
HWTEST_F(ManagerUnitTest, GetAudioParameterByKey_003, TestSize.Level1)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    manager.deviceManagers_[HDI_DEVICE_MANAGER_TYPE_LOCAL] = nullptr;

    auto result = manager.GetAudioParameterByKey(HDI_DEVICE_MANAGER_TYPE_LOCAL, "test_key");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result, std::nullopt);
}

/**
 * @tc.name   : GetExtraParameters_001
 * @tc.number : GetExtraParameters_001
 * @tc.desc   : Verify all subkeys are forwarded to local device manager.
 */
HWTEST_F(ManagerUnitTest, GetExtraParameters_001, TestSize.Level1)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    auto mockDeviceManager = std::make_shared<MockDeviceManager>();
    manager.deviceManagers_[HDI_DEVICE_MANAGER_TYPE_LOCAL] = mockDeviceManager;

    std::vector<std::string> subKeys = {"subkey_1", "subkey_2"};
    std::vector<std::pair<std::string, std::string>> result;

    EXPECT_CALL(*mockDeviceManager, GetAudioParameter("primary", AudioParamKey::NONE, "subkey_1"))
        .WillOnce(testing::Return("value_1"));
    EXPECT_CALL(*mockDeviceManager, GetAudioParameter("primary", AudioParamKey::NONE, "subkey_2"))
        .WillOnce(testing::Return("value_2"));

    int32_t ret = manager.GetExtraParameters(subKeys, result);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_THAT(result, testing::UnorderedElementsAre(
        std::make_pair(std::string("subkey_1"), std::string("value_1")),
        std::make_pair(std::string("subkey_2"), std::string("value_2"))));
}

/**
 * @tc.name   : GetExtraParameters_002
 * @tc.number : GetExtraParameters_002
 * @tc.desc   : Verify batch get forwards every requested subkey.
 */
HWTEST_F(ManagerUnitTest, GetExtraParameters_002, TestSize.Level1)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    auto mockDeviceManager = std::make_shared<MockDeviceManager>();
    manager.deviceManagers_[HDI_DEVICE_MANAGER_TYPE_LOCAL] = mockDeviceManager;

    std::vector<std::string> subKeys = {"subkey_3"};
    std::vector<std::pair<std::string, std::string>> result;

    EXPECT_CALL(*mockDeviceManager, GetAudioParameter("primary", AudioParamKey::NONE, "subkey_3"))
        .WillOnce(testing::Return("value_3"));

    int32_t ret = manager.GetExtraParameters(subKeys, result);
    EXPECT_EQ(ret, SUCCESS);
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ("subkey_3", result[0].first);
    EXPECT_EQ("value_3", result[0].second);
}
} // namespace AudioStandard
} // namespace OHOS
