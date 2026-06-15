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

#include "capture_adapter_aiut.h"
#include "aiut_log.h"
#include "manager/hdi_adapter_factory.h"
#include "adapter/local_device_manager.h"
#include "adapter/bluetooth_device_manager.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

// ==================== CaptureAdapterFactoryAiut ====================

void CaptureAdapterFactoryAiut::SetUpTestCase(void) {}
void CaptureAdapterFactoryAiut::TearDownTestCase(void) {}
void CaptureAdapterFactoryAiut::SetUp(void) {}
void CaptureAdapterFactoryAiut::TearDown(void) {}

// ==================== CreateCaptureSource Tests ====================

/**
 * @tc.name  : Test CreateCaptureSource with invalid captureId
 * @tc.number: HdiAdapterFactory_CreateCaptureSource_001
 * @tc.desc  : Test CreateCaptureSource returns nullptr for invalid captureId.
 */
HWTEST_F(CaptureAdapterFactoryAiut, HdiAdapterFactory_CreateCaptureSource_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &factory = HdiAdapterFactory::GetInstance();
    auto source = factory.CreateCaptureSource(UINT32_MAX);
    AIUT_EXPECT_NULL_INFO_LOG(source, "CreateCaptureSource should return nullptr for invalid id");
}

// ==================== LocalDeviceManagerCaptureAiut ====================

void LocalDeviceManagerCaptureAiut::SetUpTestCase(void) {}
void LocalDeviceManagerCaptureAiut::TearDownTestCase(void) {}
void LocalDeviceManagerCaptureAiut::SetUp(void) {}
void LocalDeviceManagerCaptureAiut::TearDown(void) {}

// ==================== CreateCapture / DestroyCapture Tests ====================

/**
 * @tc.name  : Test LocalDeviceManager CreateCapture with null params
 * @tc.number: LocalDeviceManager_CreateCapture_001
 * @tc.desc  : Test CreateCapture returns nullptr when param is null.
 */
HWTEST_F(LocalDeviceManagerCaptureAiut, LocalDeviceManager_CreateCapture_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<LocalDeviceManager>();
    uint32_t hdiCaptureId = 0;
    void *result = mgr->CreateCapture("primary", nullptr, nullptr, hdiCaptureId);
    AIUT_EXPECT_NULL_INFO_LOG(result, "CreateCapture should return nullptr with null params");
}

// ==================== BluetoothDeviceManagerCaptureAiut ====================

void BluetoothDeviceManagerCaptureAiut::SetUpTestCase(void) {}
void BluetoothDeviceManagerCaptureAiut::TearDownTestCase(void) {}
void BluetoothDeviceManagerCaptureAiut::SetUp(void) {}
void BluetoothDeviceManagerCaptureAiut::TearDown(void) {}

// ==================== CreateCapture / DestroyCapture Tests ====================

/**
 * @tc.name  : Test BluetoothDeviceManager CreateCapture with null params
 * @tc.number: BluetoothDeviceManager_CreateCapture_001
 * @tc.desc  : Test CreateCapture returns nullptr when param is null.
 */
HWTEST_F(BluetoothDeviceManagerCaptureAiut, BluetoothDeviceManager_CreateCapture_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<BluetoothDeviceManager>();
    uint32_t hdiCaptureId = 0;
    void *result = mgr->CreateCapture("a2dp", nullptr, nullptr, hdiCaptureId);
    AIUT_EXPECT_NULL_INFO_LOG(result, "CreateCapture should return nullptr with null params");
}

// ==================== SetInputRoute Tests ====================

/**
 * @tc.name  : Test BluetoothDeviceManager SetInputRoute returns not supported
 * @tc.number: BluetoothDeviceManager_SetInputRoute_001
 * @tc.desc  : Test SetInputRoute returns ERR_NOT_SUPPORTED.
 */
HWTEST_F(BluetoothDeviceManagerCaptureAiut, BluetoothDeviceManager_SetInputRoute_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<BluetoothDeviceManager>();
    int32_t ret = mgr->SetInputRoute("a2dp", DEVICE_TYPE_BLUETOOTH_SCO, 0, 0);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED, "SetInputRoute should return not supported");
}

} // namespace AudioStandard
} // namespace OHOS
