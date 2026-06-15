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

#include "remote_adapter_aiut.h"
#include "aiut_log.h"
#include "manager/hdi_adapter_factory.h"
#include "adapter/remote_device_manager.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

// ==================== RemoteAdapterFactoryAiut ====================

void RemoteAdapterFactoryAiut::SetUpTestCase(void) {}
void RemoteAdapterFactoryAiut::TearDownTestCase(void) {}
void RemoteAdapterFactoryAiut::SetUp(void) {}
void RemoteAdapterFactoryAiut::TearDown(void) {}

// ==================== CreateDeviceManager Tests ====================

/**
 * @tc.name  : Test CreateDeviceManager with remote type
 * @tc.number: HdiAdapterFactory_CreateDeviceManager_Remote_001
 * @tc.desc  : Test CreateDeviceManager creates RemoteDeviceManager for remote type.
 */
HWTEST_F(RemoteAdapterFactoryAiut, HdiAdapterFactory_CreateDeviceManager_Remote_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &factory = HdiAdapterFactory::GetInstance();
    auto mgr = factory.CreateDeviceManager(HDI_DEVICE_MANAGER_TYPE_REMOTE);
    AIUT_ASSERT_NE_INFO_LOG(mgr, nullptr, "remote device manager should not be null");
}

// ==================== ReportHdiException REMOTE Tests ====================

/**
 * @tc.name  : Test ReportHdiException with REMOTE type
 * @tc.number: HdiMonitor_ReportHdiException_Remote_001
 * @tc.desc  : Test ReportHdiException does not crash with REMOTE HDI type.
 */
HWTEST_F(RemoteAdapterFactoryAiut, HdiMonitor_ReportHdiException_Remote_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    HdiMonitor::ReportHdiException(HdiType::REMOTE, ErrorCase::CHECK_HDI_FAILED, -3, "test_remote_error");
}

// ==================== RemoteDeviceManagerAiut ====================

void RemoteDeviceManagerAiut::SetUpTestCase(void) {}
void RemoteDeviceManagerAiut::TearDownTestCase(void) {}
void RemoteDeviceManagerAiut::SetUp(void) {}
void RemoteDeviceManagerAiut::TearDown(void) {}

// ==================== Construction / Destruction Tests ====================

/**
 * @tc.name  : Test RemoteDeviceManager construction
 * @tc.number: RemoteDeviceManager_Constructor_001
 * @tc.desc  : Test RemoteDeviceManager default constructor creates valid object.
 */
HWTEST_F(RemoteDeviceManagerAiut, RemoteDeviceManager_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<RemoteDeviceManager>();
    AIUT_ASSERT_NE_INFO_LOG(mgr, nullptr, "mgr is null");
}

/**
 * @tc.name  : Test RemoteDeviceManager destruction
 * @tc.number: RemoteDeviceManager_Destructor_001
 * @tc.desc  : Test RemoteDeviceManager destructor does not crash.
 */
HWTEST_F(RemoteDeviceManagerAiut, RemoteDeviceManager_Destructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<RemoteDeviceManager>();
    mgr.reset();
}

// ==================== Unsupported Operations Tests ====================

/**
 * @tc.name  : Test SetVoiceVolume returns not supported
 * @tc.number: RemoteDeviceManager_SetVoiceVolume_001
 * @tc.desc  : Test SetVoiceVolume returns ERR_NOT_SUPPORTED.
 */
HWTEST_F(RemoteDeviceManagerAiut, RemoteDeviceManager_SetVoiceVolume_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<RemoteDeviceManager>();
    int32_t ret = mgr->SetVoiceVolume("networkId_001", 0.5f);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED, "SetVoiceVolume should return not supported");
}

// ==================== SetOutputRoute Tests ====================

/**
 * @tc.name  : Test SetOutputRoute with empty devices
 * @tc.number: RemoteDeviceManager_SetOutputRoute_001
 * @tc.desc  : Test SetOutputRoute returns error when device list is empty.
 */
HWTEST_F(RemoteDeviceManagerAiut, RemoteDeviceManager_SetOutputRoute_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<RemoteDeviceManager>();
    std::vector<DeviceType> emptyDevices;
    int32_t ret = mgr->SetOutputRoute("networkId_001", emptyDevices, 0);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "SetOutputRoute should fail with empty devices");
}

// ==================== CreateRender / DestroyRender Tests ====================

/**
 * @tc.name  : Test CreateRender with null params
 * @tc.number: RemoteDeviceManager_CreateRender_001
 * @tc.desc  : Test CreateRender returns nullptr when param is null.
 */
HWTEST_F(RemoteDeviceManagerAiut, RemoteDeviceManager_CreateRender_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<RemoteDeviceManager>();
    uint32_t hdiRenderId = 0;
    void *result = mgr->CreateRender("networkId_001", nullptr, nullptr, hdiRenderId);
    AIUT_EXPECT_NULL_INFO_LOG(result, "CreateRender should return nullptr with null params");
}

// ==================== CreateCapture / DestroyCapture Tests ====================

/**
 * @tc.name  : Test CreateCapture with null params
 * @tc.number: RemoteDeviceManager_CreateCapture_001
 * @tc.desc  : Test CreateCapture returns nullptr when param is null.
 */
HWTEST_F(RemoteDeviceManagerAiut, RemoteDeviceManager_CreateCapture_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<RemoteDeviceManager>();
    uint32_t hdiCaptureId = 0;
    void *result = mgr->CreateCapture("networkId_001", nullptr, nullptr, hdiCaptureId);
    AIUT_EXPECT_NULL_INFO_LOG(result, "CreateCapture should return nullptr with null params");
}

// ==================== DumpInfo Tests ====================

/**
 * @tc.name  : Test DumpInfo
 * @tc.number: RemoteDeviceManager_DumpInfo_001
 * @tc.desc  : Test DumpInfo does not crash on empty adapter map.
 */
HWTEST_F(RemoteDeviceManagerAiut, RemoteDeviceManager_DumpInfo_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<RemoteDeviceManager>();
    std::string dumpString;
    mgr->DumpInfo(dumpString);
    AIUT_EXPECT_TRUE_INFO_LOG(dumpString.empty(), "DumpInfo should produce empty string for empty adapter map");
}

// ==================== HandleEvent Tests ====================

/**
 * @tc.name  : Test HandleEvent with unsupported key
 * @tc.number: RemoteDeviceManager_HandleEvent_001
 * @tc.desc  : Test HandleEvent returns error for unsupported key.
 */
HWTEST_F(RemoteDeviceManagerAiut, RemoteDeviceManager_HandleEvent_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<RemoteDeviceManager>();
    int32_t ret = mgr->HandleEvent("networkId_001", AudioParamKey::NONE, "condition", "value", nullptr);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS, "HandleEvent should fail for unsupported key");
}

} // namespace AudioStandard
} // namespace OHOS
