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

#include "render_adapter_aiut.h"
#include "aiut_log.h"
#include "manager/hdi_adapter_factory.h"
#include "adapter/local_device_manager.h"
#include "adapter/bluetooth_device_manager.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

// ==================== RenderAdapterFactoryAiut ====================

void RenderAdapterFactoryAiut::SetUpTestCase(void) {}
void RenderAdapterFactoryAiut::TearDownTestCase(void) {}
void RenderAdapterFactoryAiut::SetUp(void) {}
void RenderAdapterFactoryAiut::TearDown(void) {}

// ==================== CreateRenderSink Tests ====================

/**
 * @tc.name  : Test CreateRenderSink with invalid renderId
 * @tc.number: HdiAdapterFactory_CreateRenderSink_001
 * @tc.desc  : Test CreateRenderSink returns nullptr for invalid renderId.
 */
HWTEST_F(RenderAdapterFactoryAiut, HdiAdapterFactory_CreateRenderSink_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &factory = HdiAdapterFactory::GetInstance();
    auto sink = factory.CreateRenderSink(UINT32_MAX);
    AIUT_EXPECT_NULL_INFO_LOG(sink, "CreateRenderSink should return nullptr for invalid id");
}

// ==================== CreateAuxiliarySink Tests ====================

/**
 * @tc.name  : Test CreateAuxiliarySink
 * @tc.number: HdiAdapterFactory_CreateAuxiliarySink_001
 * @tc.desc  : Test CreateAuxiliarySink creates valid AuxiliarySink.
 */
HWTEST_F(RenderAdapterFactoryAiut, HdiAdapterFactory_CreateAuxiliarySink_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &factory = HdiAdapterFactory::GetInstance();
    auto sink = factory.CreateAuxiliarySink();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "auxiliary sink should not be null");
}

// ==================== LocalDeviceManagerRenderAiut ====================

void LocalDeviceManagerRenderAiut::SetUpTestCase(void) {}
void LocalDeviceManagerRenderAiut::TearDownTestCase(void) {}
void LocalDeviceManagerRenderAiut::SetUp(void) {}
void LocalDeviceManagerRenderAiut::TearDown(void) {}

// ==================== CreateRender / DestroyRender Tests ====================

/**
 * @tc.name  : Test LocalDeviceManager CreateRender with null params
 * @tc.number: LocalDeviceManager_CreateRender_001
 * @tc.desc  : Test CreateRender returns nullptr when param is null.
 */
HWTEST_F(LocalDeviceManagerRenderAiut, LocalDeviceManager_CreateRender_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<LocalDeviceManager>();
    uint32_t hdiRenderId = 0;
    void *result = mgr->CreateRender("primary", nullptr, nullptr, hdiRenderId);
    AIUT_EXPECT_NULL_INFO_LOG(result, "CreateRender should return nullptr with null params");
}

// ==================== SetOutputRoute Tests ====================

/**
 * @tc.name  : Test LocalDeviceManager SetOutputRoute with empty devices
 * @tc.number: LocalDeviceManager_SetOutputRoute_001
 * @tc.desc  : Test SetOutputRoute returns error when device list is empty.
 */
HWTEST_F(LocalDeviceManagerRenderAiut, LocalDeviceManager_SetOutputRoute_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<LocalDeviceManager>();
    std::vector<DeviceType> emptyDevices;
    int32_t ret = mgr->SetOutputRoute("primary", emptyDevices, 0);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "SetOutputRoute should fail with empty devices");
}

// ==================== CreateCognitionStream Tests ====================

/**
 * @tc.name  : Test LocalDeviceManager CreateCognitionStream with null params
 * @tc.number: LocalDeviceManager_CreateCognitionStream_001
 * @tc.desc  : Test CreateCognitionStream returns error when params are null.
 */
HWTEST_F(LocalDeviceManagerRenderAiut, LocalDeviceManager_CreateCognitionStream_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<LocalDeviceManager>();
    int32_t sinkId = 0;
    int32_t ret = mgr->CreateCognitionStream("primary", nullptr, sinkId, nullptr);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "CreateCognitionStream should fail with null params");
}

// ==================== DestroyCognitionStream Tests ====================

/**
 * @tc.name  : Test LocalDeviceManager DestroyCognitionStream with invalid sinkId
 * @tc.number: LocalDeviceManager_DestroyCognitionStream_001
 * @tc.desc  : Test DestroyCognitionStream returns error when sinkId is invalid.
 */
HWTEST_F(LocalDeviceManagerRenderAiut, LocalDeviceManager_DestroyCognitionStream_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<LocalDeviceManager>();
    int32_t sinkId = -1;
    int32_t ret = mgr->DestroyCognitionStream("primary", sinkId);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "DestroyCognitionStream should fail with invalid sinkId");
}

// ==================== BluetoothDeviceManagerRenderAiut ====================

void BluetoothDeviceManagerRenderAiut::SetUpTestCase(void) {}
void BluetoothDeviceManagerRenderAiut::TearDownTestCase(void) {}
void BluetoothDeviceManagerRenderAiut::SetUp(void) {}
void BluetoothDeviceManagerRenderAiut::TearDown(void) {}

// ==================== CreateRender Tests ====================

/**
 * @tc.name  : Test BluetoothDeviceManager CreateRender with null params
 * @tc.number: BluetoothDeviceManager_CreateRender_001
 * @tc.desc  : Test CreateRender returns nullptr when param is null.
 */
HWTEST_F(BluetoothDeviceManagerRenderAiut, BluetoothDeviceManager_CreateRender_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<BluetoothDeviceManager>();
    uint32_t hdiRenderId = 0;
    void *result = mgr->CreateRender("a2dp", nullptr, nullptr, hdiRenderId);
    AIUT_EXPECT_NULL_INFO_LOG(result, "CreateRender should return nullptr with null params");
}

// ==================== SetOutputRoute Tests ====================

/**
 * @tc.name  : Test BluetoothDeviceManager SetOutputRoute returns not supported
 * @tc.number: BluetoothDeviceManager_SetOutputRoute_001
 * @tc.desc  : Test SetOutputRoute returns ERR_NOT_SUPPORTED.
 */
HWTEST_F(BluetoothDeviceManagerRenderAiut, BluetoothDeviceManager_SetOutputRoute_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<BluetoothDeviceManager>();
    std::vector<DeviceType> devices = { DEVICE_TYPE_BLUETOOTH_A2DP };
    int32_t ret = mgr->SetOutputRoute("a2dp", devices, 0);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED, "SetOutputRoute should return not supported");
}

} // namespace AudioStandard
} // namespace OHOS
