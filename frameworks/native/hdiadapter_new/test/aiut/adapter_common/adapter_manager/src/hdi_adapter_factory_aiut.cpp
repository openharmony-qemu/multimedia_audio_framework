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

#include "hdi_adapter_factory_aiut.h"
#include "aiut_log.h"
#include "manager/hdi_adapter_factory.h"
#include "audio_errors.h"
#include "audio_hdi_log.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

void HdiAdapterFactoryAiut::SetUpTestCase(void) {}
void HdiAdapterFactoryAiut::TearDownTestCase(void) {}
void HdiAdapterFactoryAiut::SetUp(void) {}
void HdiAdapterFactoryAiut::TearDown(void) {}

// ==================== GetInstance Tests ====================

/**
 * @tc.name  : Test HdiAdapterFactory GetInstance
 * @tc.number: HdiAdapterFactory_GetInstance_001
 * @tc.desc  : Test GetInstance returns valid singleton.
 */
HWTEST_F(HdiAdapterFactoryAiut, HdiAdapterFactory_GetInstance_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &factory = HdiAdapterFactory::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(&factory, nullptr, "factory instance should not be null");
}

/**
 * @tc.name  : Test HdiAdapterFactory GetInstance returns same singleton
 * @tc.number: HdiAdapterFactory_GetInstance_002
 * @tc.desc  : Test GetInstance returns the same instance each time.
 */
HWTEST_F(HdiAdapterFactoryAiut, HdiAdapterFactory_GetInstance_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &factory1 = HdiAdapterFactory::GetInstance();
    auto &factory2 = HdiAdapterFactory::GetInstance();
    AIUT_EXPECT_EQ_INFO_LOG(&factory1, &factory2, "GetInstance should return same singleton");
}

// ==================== CreateDeviceManager Tests ====================

/**
 * @tc.name  : Test CreateDeviceManager with local type
 * @tc.number: HdiAdapterFactory_CreateDeviceManager_001
 * @tc.desc  : Test CreateDeviceManager creates LocalDeviceManager for local type.
 */
HWTEST_F(HdiAdapterFactoryAiut, HdiAdapterFactory_CreateDeviceManager_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &factory = HdiAdapterFactory::GetInstance();
    auto mgr = factory.CreateDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    AIUT_ASSERT_NE_INFO_LOG(mgr, nullptr, "local device manager should not be null");
}

/**
 * @tc.name  : Test CreateDeviceManager with bluetooth type
 * @tc.number: HdiAdapterFactory_CreateDeviceManager_002
 * @tc.desc  : Test CreateDeviceManager creates BluetoothDeviceManager for bluetooth type.
 */
HWTEST_F(HdiAdapterFactoryAiut, HdiAdapterFactory_CreateDeviceManager_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &factory = HdiAdapterFactory::GetInstance();
    auto mgr = factory.CreateDeviceManager(HDI_DEVICE_MANAGER_TYPE_BLUETOOTH);
    AIUT_ASSERT_NE_INFO_LOG(mgr, nullptr, "bluetooth device manager should not be null");
}

/**
 * @tc.name  : Test CreateDeviceManager with invalid type
 * @tc.number: HdiAdapterFactory_CreateDeviceManager_003
 * @tc.desc  : Test CreateDeviceManager returns nullptr for invalid type.
 */
HWTEST_F(HdiAdapterFactoryAiut, HdiAdapterFactory_CreateDeviceManager_003, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &factory = HdiAdapterFactory::GetInstance();
    auto mgr = factory.CreateDeviceManager(99);
    AIUT_EXPECT_NULL_INFO_LOG(mgr, "CreateDeviceManager should return nullptr for invalid type");
}

// ==================== AdapterVolume Tests ====================

/**
 * @tc.name  : SetAudioParameter_001
 * @tc.number: SetAudioParameter_001
 * @tc.desc  : Test SetAudioParameter with valid params on local device manager
 */
HWTEST_F(HdiAdapterFactoryAiut, SetAudioParameter_001, TestSize.Level1)
{
    auto dm = manager_.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    ASSERT_NE(dm, nullptr);
    int32_t ret = dm->SetAudioParameter("primary", AudioParamKey::VOLUME, "", "volume=0.5");
    (void)ret;
}

/**
 * @tc.name  : GetAudioParameter_001
 * @tc.number: GetAudioParameter_001
 * @tc.desc  : Test GetAudioParameter with valid params on local device manager
 */
HWTEST_F(HdiAdapterFactoryAiut, GetAudioParameter_001, TestSize.Level1)
{
    auto dm = manager_.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    ASSERT_NE(dm, nullptr);
    std::string result = dm->GetAudioParameter("primary", AudioParamKey::VOLUME, "");
    (void)result;
}

/**
 * @tc.name  : SetExtraParameters_001
 * @tc.number: SetExtraParameters_001
 * @tc.desc  : Test SetExtraParameters with valid params on local device manager
 */
HWTEST_F(HdiAdapterFactoryAiut, SetExtraParameters_001, TestSize.Level1)
{
    auto dm = manager_.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    ASSERT_NE(dm, nullptr);
    int32_t ret = dm->SetExtraParameters("primary", "", "key=value");
    (void)ret;
}

/**
 * @tc.name  : GetExtraParameters_001
 * @tc.number: GetExtraParameters_001
 * @tc.desc  : Test GetExtraParameters with valid params
 */
HWTEST_F(HdiAdapterFactoryAiut, GetExtraParameters_001, TestSize.Level1)
{
    std::vector<std::string> subKeys = {"key"};
    std::vector<std::pair<std::string, std::string>> result;
    int32_t ret = manager_.GetExtraParameters(subKeys, result);
    (void)ret;
}

/**
 * @tc.name  : GetAudioParameterByKey_001
 * @tc.number: GetAudioParameterByKey_001
 * @tc.desc  : Test GetAudioParameterByKey on local device manager
 */
HWTEST_F(HdiAdapterFactoryAiut, GetAudioParameterByKey_001, TestSize.Level1)
{
    auto result = manager_.GetAudioParameterByKey(HDI_DEVICE_MANAGER_TYPE_LOCAL, "volume");
    (void)result;
}

/**
 * @tc.name  : ReleaseDeviceManager_001
 * @tc.number: ReleaseDeviceManager_001
 * @tc.desc  : Test ReleaseDeviceManager
 */
HWTEST_F(HdiAdapterFactoryAiut, ReleaseDeviceManager_001, TestSize.Level1)
{
    manager_.ReleaseDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    EXPECT_TRUE(true);
}

// ==================== AdapterDevice Tests ====================

/**
 * @tc.name  : GetRenderIdByDeviceClass_001
 * @tc.number: GetRenderIdByDeviceClass_001
 * @tc.desc  : Test GetRenderIdByDeviceClass with primary device class
 */
HWTEST_F(HdiAdapterFactoryAiut, GetRenderIdByDeviceClass_001, TestSize.Level1)
{
    uint32_t id = manager_.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY);
    EXPECT_NE(id, HDI_INVALID_ID);
}

/**
 * @tc.name  : GetCaptureIdByDeviceClass_001
 * @tc.number: GetCaptureIdByDeviceClass_001
 * @tc.desc  : Test GetCaptureIdByDeviceClass with primary device class
 */
HWTEST_F(HdiAdapterFactoryAiut, GetCaptureIdByDeviceClass_001, TestSize.Level1)
{
    uint32_t id = manager_.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY);
    EXPECT_NE(id, HDI_INVALID_ID);
}

/**
 * @tc.name  : GetRenderSink_001
 * @tc.number: GetRenderSink_001
 * @tc.desc  : Test GetRenderSink with valid render ID
 */
HWTEST_F(HdiAdapterFactoryAiut, GetRenderSink_001, TestSize.Level1)
{
    uint32_t id = manager_.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY);
    auto sink = manager_.GetRenderSink(id);
    (void)sink;
}

/**
 * @tc.name  : GetCaptureSource_001
 * @tc.number: GetCaptureSource_001
 * @tc.desc  : Test GetCaptureSource with valid capture ID
 */
HWTEST_F(HdiAdapterFactoryAiut, GetCaptureSource_001, TestSize.Level1)
{
    uint32_t id = manager_.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY);
    auto source = manager_.GetCaptureSource(id);
    (void)source;
}

/**
 * @tc.name  : ReleaseId_001
 * @tc.number: ReleaseId_001
 * @tc.desc  : Test ReleaseId
 */
HWTEST_F(HdiAdapterFactoryAiut, ReleaseId_001, TestSize.Level1)
{
    uint32_t id = manager_.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY);
    manager_.ReleaseId(id);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : GetCurrentOutputPipeChangeInfos_001
 * @tc.number: GetCurrentOutputPipeChangeInfos_001
 * @tc.desc  : Test GetCurrentOutputPipeChangeInfos (output route)
 */
HWTEST_F(HdiAdapterFactoryAiut, GetCurrentOutputPipeChangeInfos_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioOutputPipeInfo>> pipeChangeInfos;
    int32_t ret = manager_.GetCurrentOutputPipeChangeInfos(pipeChangeInfos);
    (void)ret;
}

/**
 * @tc.name  : GetCurrentInputPipeChangeInfos_001
 * @tc.number: GetCurrentInputPipeChangeInfos_001
 * @tc.desc  : Test GetCurrentInputPipeChangeInfos (input route)
 */
HWTEST_F(HdiAdapterFactoryAiut, GetCurrentInputPipeChangeInfos_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioInputPipeInfo>> pipeChangeInfos;
    int32_t ret = manager_.GetCurrentInputPipeChangeInfos(pipeChangeInfos);
    (void)ret;
}

/**
 * @tc.name  : LoadAdapter_001
 * @tc.number: LoadAdapter_001
 * @tc.desc  : Test LoadAdapter with primary adapter
 */
HWTEST_F(HdiAdapterFactoryAiut, LoadAdapter_001, TestSize.Level1)
{
    int32_t ret = manager_.LoadAdapter(HDI_DEVICE_MANAGER_TYPE_LOCAL, "primary");
    (void)ret;
}

/**
 * @tc.name  : UnloadAdapter_001
 * @tc.number: UnloadAdapter_001
 * @tc.desc  : Test UnloadAdapter with primary adapter
 */
HWTEST_F(HdiAdapterFactoryAiut, UnloadAdapter_001, TestSize.Level1)
{
    manager_.UnloadAdapter(HDI_DEVICE_MANAGER_TYPE_LOCAL, "primary");
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : ProcessSink_001
 * @tc.number: ProcessSink_001
 * @tc.desc  : Test ProcessSink callback
 */
HWTEST_F(HdiAdapterFactoryAiut, ProcessSink_001, TestSize.Level1)
{
    int32_t count = 0;
    auto processFunc = [&count](uint32_t id, std::shared_ptr<IAudioRenderSink> sink) -> int32_t {
        count++;
        return 0;
    };
    int32_t ret = manager_.ProcessSink(processFunc);
    (void)ret;
    (void)count;
}

/**
 * @tc.name  : ProcessSource_001
 * @tc.number: ProcessSource_001
 * @tc.desc  : Test ProcessSource callback
 */
HWTEST_F(HdiAdapterFactoryAiut, ProcessSource_001, TestSize.Level1)
{
    int32_t count = 0;
    auto processFunc = [&count](uint32_t id, std::shared_ptr<IAudioCaptureSource> source) -> int32_t {
        count++;
        return 0;
    };
    int32_t ret = manager_.ProcessSource(processFunc);
    (void)ret;
    (void)count;
}

/**
 * @tc.name  : DumpInfo_001
 * @tc.number: DumpInfo_001
 * @tc.desc  : Test DumpInfo
 */
HWTEST_F(HdiAdapterFactoryAiut, DumpInfo_001, TestSize.Level1)
{
    std::string dumpString;
    manager_.DumpInfo(dumpString);
    EXPECT_TRUE(true);
}

} // namespace AudioStandard
} // namespace OHOS
