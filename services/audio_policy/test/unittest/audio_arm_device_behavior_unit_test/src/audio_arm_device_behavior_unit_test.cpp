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

#include "audio_policy_utils.h"
#include "audio_core_service.h"
#include "audio_pipe_manager.h"
#include "audio_arm_device_behavior_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

namespace {
constexpr int32_t TEST_IO_HANDLE = 1;
constexpr DeviceType TEST_USB_DEVICE_TYPE = DEVICE_TYPE_USB_ARM_HEADSET;
const std::string TEST_USB_ADDR = "11:22:33:44:55:66";
const std::string TEST_USB_ADDR_ALT = "AA:BB:CC:DD:EE:FF";
const std::string TEST_USB_SINK_MODULE_NAME = "Usb_arm_speaker";
const std::string TEST_USB_SOURCE_MODULE_NAME = "Usb_arm_mic";

AudioModuleInfo MakeUsbSinkModuleInfo()
{
    AudioModuleInfo moduleInfo;
    moduleInfo.name = TEST_USB_SINK_MODULE_NAME;
    moduleInfo.role = "sink";
    moduleInfo.rate = "48000";
    moduleInfo.format = "s16le";
    moduleInfo.channels = "2";
    moduleInfo.bufferSize = "960";
    moduleInfo.channelLayout = "3";
    moduleInfo.adapterName = "usb_arm";
    moduleInfo.className = "USB_CLASS";
    return moduleInfo;
}

AudioModuleInfo MakeUsbSourceModuleInfo()
{
    AudioModuleInfo moduleInfo;
    moduleInfo.name = TEST_USB_SOURCE_MODULE_NAME;
    moduleInfo.role = "source";
    moduleInfo.rate = "48000";
    moduleInfo.format = "s16le";
    moduleInfo.channels = "2";
    moduleInfo.bufferSize = "960";
    moduleInfo.channelLayout = "3";
    moduleInfo.adapterName = "usb_arm";
    moduleInfo.className = "USB_CLASS";
    return moduleInfo;
}

std::shared_ptr<AudioStreamDescriptor> MakeStreamDesc(
    DeviceRole role, const std::string &address = TEST_USB_ADDR)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = TEST_USB_DEVICE_TYPE;
    deviceDesc->macAddress_ = address;
    if (role == INPUT_DEVICE) {
        deviceDesc->deviceRole_ = INPUT_DEVICE;
    } else {
        deviceDesc->deviceRole_ = OUTPUT_DEVICE;
    }
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    return streamDesc;
}

std::shared_ptr<AudioStreamDescriptor> MakeEmptyStreamDesc()
{
    return std::make_shared<AudioStreamDescriptor>();
}

std::shared_ptr<AudioStreamDescriptor> MakeNullDeviceDescStreamDesc()
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.push_back(nullptr);
    return streamDesc;
}

void InjectUsbModuleInfo()
{
    auto pipeManager = AudioPipeManager::GetPipeManager();
    std::list<AudioModuleInfo> usbModules = {MakeUsbSinkModuleInfo(), MakeUsbSourceModuleInfo()};
    pipeManager->audioConfigManager_.deviceClassInfo_[ClassType::TYPE_USB] = usbModules;
}

void ClearUsbModuleInfo()
{
    auto pipeManager = AudioPipeManager::GetPipeManager();
    pipeManager->audioConfigManager_.deviceClassInfo_.erase(ClassType::TYPE_USB);
}

void SetEcFeatureEnable(bool enable)
{
    auto coreService = AudioCoreService::GetCoreService();
    coreService->isEcFeatureEnable_ = enable;
}

void AddUsbIOHandle(const std::string &moduleName)
{
    AudioIOHandleMap::GetInstance().AddIOHandleInfo(moduleName, TEST_IO_HANDLE);
}

void RemoveUsbIOHandle(const std::string &moduleName)
{
    AudioIOHandleMap::GetInstance().DelIOHandleInfo(moduleName);
}
}

void AudioArmDeviceBehaviorUnitTest::SetUpTestCase(void) {}
void AudioArmDeviceBehaviorUnitTest::TearDownTestCase(void) {}

void AudioArmDeviceBehaviorUnitTest::SetUp(void)
{
    InjectUsbModuleInfo();
}

void AudioArmDeviceBehaviorUnitTest::TearDown(void)
{
    armDeviceBehavior_.activeArmInputAddr_ = "";
    armDeviceBehavior_.activeArmOutputAddr_ = "";
    RemoveUsbIOHandle(TEST_USB_SINK_MODULE_NAME);
    RemoveUsbIOHandle(TEST_USB_SOURCE_MODULE_NAME);
    SetEcFeatureEnable(false);
    ClearUsbModuleInfo();
}

/**
 * @tc.name: ActivateDevice_InputDeviceEcEnabled
 * @tc.desc: Test ActivateDevice with INPUT_DEVICE and EC enabled
 * @tc.type: FUNC
 */
HWTEST_F(AudioArmDeviceBehaviorUnitTest, ActivateDevice_InputDeviceEcEnabled, TestSize.Level1)
{
    SetEcFeatureEnable(true);
    auto streamDesc = MakeStreamDesc(INPUT_DEVICE);
    int32_t result = armDeviceBehavior_.ActivateDevice(streamDesc,
        AudioStreamDeviceChangeReasonExt(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE));
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: ActivateDevice_InputDeviceEcDisabled
 * @tc.desc: Test ActivateDevice with INPUT_DEVICE and EC disabled
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioArmDeviceBehaviorUnitTest, ActivateDevice_InputDeviceEcDisabled, TestSize.Level1)
{
    SetEcFeatureEnable(false);
    RemoveUsbIOHandle(TEST_USB_SOURCE_MODULE_NAME);
    auto streamDesc = MakeStreamDesc(INPUT_DEVICE);
    int32_t result = armDeviceBehavior_.ActivateDevice(streamDesc,
        AudioStreamDeviceChangeReasonExt(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE));
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: ActivateDevice_OutputDeviceEcEnabled_WithIOHandle
 * @tc.desc: Test ActivateDevice with OUTPUT_DEVICE, EC enabled
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioArmDeviceBehaviorUnitTest, ActivateDevice_OutputDeviceEcEnabled_WithIOHandle, TestSize.Level1)
{
    SetEcFeatureEnable(true);
    AddUsbIOHandle(TEST_USB_SINK_MODULE_NAME);
    auto streamDesc = MakeStreamDesc(OUTPUT_DEVICE);
    int32_t result = armDeviceBehavior_.ActivateDevice(streamDesc,
        AudioStreamDeviceChangeReasonExt(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE));
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name: ActivateDevice_OutputDeviceEcDisabled_WithIOHandle
 * @tc.desc: Test ActivateDevice with OUTPUT_DEVICE, EC disabled
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioArmDeviceBehaviorUnitTest, ActivateDevice_OutputDeviceEcDisabled_WithIOHandle, TestSize.Level1)
{
    SetEcFeatureEnable(false);
    AddUsbIOHandle(TEST_USB_SINK_MODULE_NAME);
    auto streamDesc = MakeStreamDesc(OUTPUT_DEVICE);
    int32_t result = armDeviceBehavior_.ActivateDevice(streamDesc,
        AudioStreamDeviceChangeReasonExt(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE));
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name: ActivateDevice_OutputDeviceEcEnabled_NoIOHandle
 * @tc.desc: Test ActivateDevice with OUTPUT_DEVICE, EC enabled
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioArmDeviceBehaviorUnitTest, ActivateDevice_OutputDeviceEcEnabled_NoIOHandle, TestSize.Level1)
{
    SetEcFeatureEnable(true);
    RemoveUsbIOHandle(TEST_USB_SINK_MODULE_NAME);
    auto streamDesc = MakeStreamDesc(OUTPUT_DEVICE);
    int32_t result = armDeviceBehavior_.ActivateDevice(streamDesc,
        AudioStreamDeviceChangeReasonExt(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE));
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: ActivateDevice_OutputDeviceEcDisabled_NoIOHandle
 * @tc.desc: Test ActivateDevice with OUTPUT_DEVICE, EC disabled
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioArmDeviceBehaviorUnitTest, ActivateDevice_OutputDeviceEcDisabled_NoIOHandle, TestSize.Level1)
{
    SetEcFeatureEnable(false);
    RemoveUsbIOHandle(TEST_USB_SINK_MODULE_NAME);
    auto streamDesc = MakeStreamDesc(OUTPUT_DEVICE);
    int32_t result = armDeviceBehavior_.ActivateDevice(streamDesc,
        AudioStreamDeviceChangeReasonExt(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE));
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: ActivateDevice_ConfigRoleMismatch
 * @tc.desc: Test ActivateDevice when module configRole does not match stream role
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioArmDeviceBehaviorUnitTest, ActivateDevice_ConfigRoleMismatch, TestSize.Level1)
{
    SetEcFeatureEnable(false);
    auto pipeManager = AudioPipeManager::GetPipeManager();
    AudioModuleInfo onlySinkModule = MakeUsbSinkModuleInfo();
    pipeManager->audioConfigManager_.deviceClassInfo_[ClassType::TYPE_USB] = {onlySinkModule};
    RemoveUsbIOHandle(TEST_USB_SINK_MODULE_NAME);
    auto streamDesc = MakeStreamDesc(INPUT_DEVICE);
    int32_t result = armDeviceBehavior_.ActivateDevice(streamDesc,
        AudioStreamDeviceChangeReasonExt(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE));
    EXPECT_EQ(result, SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS
