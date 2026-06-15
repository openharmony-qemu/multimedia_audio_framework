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

#include "bluetooth_device_manager_aiut.h"
#include "aiut_log.h"
#include "adapter/bluetooth_device_manager.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

void BluetoothDeviceManagerAiut::SetUpTestCase(void) {}
void BluetoothDeviceManagerAiut::TearDownTestCase(void) {}
void BluetoothDeviceManagerAiut::SetUp(void) {}
void BluetoothDeviceManagerAiut::TearDown(void) {}

// ==================== Construction / Destruction Tests ====================

/**
 * @tc.name  : Test BluetoothDeviceManager construction
 * @tc.number: BluetoothDeviceManager_Constructor_001
 * @tc.desc  : Test BluetoothDeviceManager default constructor creates valid object.
 */
HWTEST_F(BluetoothDeviceManagerAiut, BluetoothDeviceManager_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<BluetoothDeviceManager>();
    AIUT_ASSERT_NE_INFO_LOG(mgr, nullptr, "mgr is null");
}

/**
 * @tc.name  : Test BluetoothDeviceManager destruction
 * @tc.number: BluetoothDeviceManager_Destructor_001
 * @tc.desc  : Test BluetoothDeviceManager destructor does not crash.
 */
HWTEST_F(BluetoothDeviceManagerAiut, BluetoothDeviceManager_Destructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<BluetoothDeviceManager>();
    mgr.reset();
}

// ==================== Unsupported Operations Tests ====================

/**
 * @tc.name  : Test SetAudioParameter returns not supported
 * @tc.number: BluetoothDeviceManager_SetAudioParameter_001
 * @tc.desc  : Test SetAudioParameter returns ERR_DEVICE_NOT_SUPPORTED.
 */
HWTEST_F(BluetoothDeviceManagerAiut, BluetoothDeviceManager_SetAudioParameter_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<BluetoothDeviceManager>();
    int32_t ret = mgr->SetAudioParameter("a2dp", AudioParamKey::NONE, "", "value");
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_DEVICE_NOT_SUPPORTED, "SetAudioParameter should return not supported");
}

/**
 * @tc.name  : Test SetVoiceVolume returns not supported
 * @tc.number: BluetoothDeviceManager_SetVoiceVolume_001
 * @tc.desc  : Test SetVoiceVolume returns ERR_NOT_SUPPORTED.
 */
HWTEST_F(BluetoothDeviceManagerAiut, BluetoothDeviceManager_SetVoiceVolume_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<BluetoothDeviceManager>();
    int32_t ret = mgr->SetVoiceVolume("a2dp", 0.5f);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED, "SetVoiceVolume should return not supported");
}

// ==================== DumpInfo Tests ====================

/**
 * @tc.name  : Test DumpInfo
 * @tc.number: BluetoothDeviceManager_DumpInfo_001
 * @tc.desc  : Test DumpInfo does not crash on empty adapter map.
 */
HWTEST_F(BluetoothDeviceManagerAiut, BluetoothDeviceManager_DumpInfo_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto mgr = std::make_shared<BluetoothDeviceManager>();
    std::string dumpString;
    mgr->DumpInfo(dumpString);
    AIUT_EXPECT_TRUE_INFO_LOG(dumpString.empty(), "DumpInfo should produce empty string for empty adapter map");
}

} // namespace AudioStandard
} // namespace OHOS
