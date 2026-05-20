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

#include "audio_router_center_unit_test.h"
#include "wireless_conflict_handler.h"
#include "audio_errors.h"
#include "audio_policy_log.h"
#include <gtest/gtest.h>
#include <memory>
#include <vector>

using namespace testing::ext;
using namespace OHOS::AudioStandard;

namespace OHOS {
namespace AudioStandard {

/**
 * @tc.name  : Test WirelessConflictHandler IsCallStreamUsage.
 * @tc.number: WirelessConflictHandler_001
 * @tc.desc  : Test IsCallStreamUsage interface with various stream usages.
 */
HWTEST(AudioRouterCenterUnitTest, WirelessConflictHandler_001, TestSize.Level1)
{
    WirelessConflictHandler handler;
    
    EXPECT_TRUE(handler.IsCallStreamUsage(STREAM_USAGE_VOICE_COMMUNICATION));
    EXPECT_TRUE(handler.IsCallStreamUsage(STREAM_USAGE_VOICE_MODEM_COMMUNICATION));
    EXPECT_TRUE(handler.IsCallStreamUsage(STREAM_USAGE_VIDEO_COMMUNICATION));
    EXPECT_TRUE(handler.IsCallStreamUsage(STREAM_USAGE_RINGTONE));
    EXPECT_TRUE(handler.IsCallStreamUsage(STREAM_USAGE_VOICE_RINGTONE));
    EXPECT_FALSE(handler.IsCallStreamUsage(STREAM_USAGE_MUSIC));
    EXPECT_FALSE(handler.IsCallStreamUsage(STREAM_USAGE_MEDIA));
    EXPECT_FALSE(handler.IsCallStreamUsage(STREAM_USAGE_UNKNOWN));
}

/**
 * @tc.name  : Test WirelessConflictHandler IsForcedRouterType.
 * @tc.number: WirelessConflictHandler_002
 * @tc.desc  : Test IsForcedRouterType interface with various router types.
 */
HWTEST(AudioRouterCenterUnitTest, WirelessConflictHandler_002, TestSize.Level1)
{
    WirelessConflictHandler handler;
    
    EXPECT_TRUE(handler.IsForcedRouterType(ROUTER_TYPE_USER_SELECT));
    EXPECT_TRUE(handler.IsForcedRouterType(ROUTER_TYPE_APP_SELECT));
    EXPECT_FALSE(handler.IsForcedRouterType(ROUTER_TYPE_DEFAULT));
    EXPECT_FALSE(handler.IsForcedRouterType(ROUTER_TYPE_PAIR_DEVICE));
    EXPECT_FALSE(handler.IsForcedRouterType(ROUTER_TYPE_NONE));
}

/**
 * @tc.name  : Test WirelessConflictHandler GetPriority.
 * @tc.number: WirelessConflictHandler_003
 * @tc.desc  : Test GetPriority interface with various parameters.
 *             Priority order: call > forced input > input > forced output > output
 */
HWTEST(AudioRouterCenterUnitTest, WirelessConflictHandler_003, TestSize.Level1)
{
    WirelessConflictHandler handler;
    
    // Call output stream: priority PRIORITY_CALL_OUTPUT
    EXPECT_EQ(handler.GetPriority(STREAM_USAGE_VOICE_COMMUNICATION,
        SOURCE_TYPE_INVALID, ROUTER_TYPE_NONE, true), PRIORITY_CALL_OUTPUT);
    EXPECT_EQ(handler.GetPriority(STREAM_USAGE_RINGTONE,
        SOURCE_TYPE_INVALID, ROUTER_TYPE_NONE, true), PRIORITY_CALL_OUTPUT);
    
    // Forced input: priority PRIORITY_FORCED_INPUT
    EXPECT_EQ(handler.GetPriority(STREAM_USAGE_UNKNOWN,
        SOURCE_TYPE_INVALID, ROUTER_TYPE_USER_SELECT, false), PRIORITY_FORCED_INPUT);
    EXPECT_EQ(handler.GetPriority(STREAM_USAGE_UNKNOWN,
        SOURCE_TYPE_INVALID, ROUTER_TYPE_APP_SELECT, false), PRIORITY_FORCED_INPUT);
    
    // Normal input: priority PRIORITY_NORMAL_INPUT
    EXPECT_EQ(handler.GetPriority(STREAM_USAGE_UNKNOWN,
        SOURCE_TYPE_MIC, ROUTER_TYPE_NONE, false), PRIORITY_NORMAL_INPUT);
    
    // Forced output: priority PRIORITY_FORCED_OUTPUT
    EXPECT_EQ(handler.GetPriority(STREAM_USAGE_MUSIC,
        SOURCE_TYPE_INVALID, ROUTER_TYPE_USER_SELECT, true), PRIORITY_FORCED_OUTPUT);
    EXPECT_EQ(handler.GetPriority(STREAM_USAGE_MUSIC,
        SOURCE_TYPE_INVALID, ROUTER_TYPE_APP_SELECT, true), PRIORITY_FORCED_OUTPUT);
    
    // Normal output: priority PRIORITY_NORMAL_OUTPUT
    EXPECT_EQ(handler.GetPriority(STREAM_USAGE_MUSIC,
        SOURCE_TYPE_INVALID, ROUTER_TYPE_NONE, true), PRIORITY_NORMAL_OUTPUT);
}

/**
 * @tc.name  : Test WirelessConflictHandler Detect with nullptr device.
 * @tc.number: WirelessConflictHandler_004
 * @tc.desc  : Test Detect interface with nullptr device, should return false.
 */
HWTEST(AudioRouterCenterUnitTest, WirelessConflictHandler_004, TestSize.Level1)
{
    WirelessConflictHandler handler;
    
    EXPECT_FALSE(handler.Detect(nullptr, ROUTER_TYPE_NONE));
}

/**
 * @tc.name  : Test WirelessConflictHandler Detect with empty mac address.
 * @tc.number: WirelessConflictHandler_005
 * @tc.desc  : Test Detect interface with empty mac address, should return false.
 */
HWTEST(AudioRouterCenterUnitTest, WirelessConflictHandler_005, TestSize.Level1)
{
    WirelessConflictHandler handler;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->macAddress_ = "";
    
    EXPECT_FALSE(handler.Detect(device, ROUTER_TYPE_NONE));
}

/**
 * @tc.name  : Test WirelessConflictHandler SetSelectedDevices and ClearSelectedDevices.
 * @tc.number: WirelessConflictHandler_006
 * @tc.desc  : Test SetSelectedDevices and ClearSelectedDevices interfaces.
 */
HWTEST(AudioRouterCenterUnitTest, WirelessConflictHandler_006, TestSize.Level1)
{
    WirelessConflictHandler handler;
    
    std::vector<StreamDeviceInfo> outputDevices;
    std::vector<StreamDeviceInfo> inputDevices;
    
    StreamDeviceInfo outputInfo;
    outputInfo.uid = 1001;
    outputInfo.sessionId = 1;
    outputInfo.streamUsage = STREAM_USAGE_MUSIC;
    outputInfo.routerType = ROUTER_TYPE_DEFAULT;
    auto outputDevice = std::make_shared<AudioDeviceDescriptor>();
    outputDevice->macAddress_ = "00:11:22:33:44:55";
    outputInfo.device = outputDevice;
    outputDevices.push_back(outputInfo);
    
    StreamDeviceInfo inputInfo;
    inputInfo.uid = 1002;
    inputInfo.sessionId = 2;
    inputInfo.sourceType = SOURCE_TYPE_MIC;
    inputInfo.routerType = ROUTER_TYPE_DEFAULT;
    auto inputDevice = std::make_shared<AudioDeviceDescriptor>();
    inputDevice->macAddress_ = "00:11:22:33:44:56";
    inputInfo.device = inputDevice;
    inputDevices.push_back(inputInfo);
    
    handler.SetSelectedDevices(outputDevices, inputDevices);
    
    // Clear selected devices
    handler.ClearSelectedDevices();
}

/**
 * @tc.name  : Test WirelessConflictHandler Detect no conflict with same device.
 * @tc.number: WirelessConflictHandler_007
 * @tc.desc  : Test Detect when current device has same mac as highest priority device,
 *             should return false (no conflict).
 */
HWTEST(AudioRouterCenterUnitTest, WirelessConflictHandler_007, TestSize.Level1)
{
    WirelessConflictHandler handler;
    
    std::vector<StreamDeviceInfo> outputDevices;
    StreamDeviceInfo info;
    info.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    info.routerType = ROUTER_TYPE_DEFAULT;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->macAddress_ = "00:11:22:33:44:55";
    info.device = device;
    outputDevices.push_back(info);
    
    handler.SetSelectedDevices(outputDevices, {});
    
    // Same mac address, no conflict
    auto currentDevice = std::make_shared<AudioDeviceDescriptor>();
    currentDevice->macAddress_ = "00:11:22:33:44:55";
    EXPECT_FALSE(handler.Detect(currentDevice, ROUTER_TYPE_DEFAULT));
}

/**
 * @tc.name  : Test WirelessConflictHandler Detect no conflict when priority is higher.
 * @tc.number: WirelessConflictHandler_009
 * @tc.desc  : Test Detect when current stream has higher priority than existing,
 *             should return false (no conflict).
 */
HWTEST(AudioRouterCenterUnitTest, WirelessConflictHandler_009, TestSize.Level1)
{
    WirelessConflictHandler handler;
    
    std::vector<StreamDeviceInfo> outputDevices;
    StreamDeviceInfo info;
    info.streamUsage = STREAM_USAGE_MUSIC; // priority 1
    info.routerType = ROUTER_TYPE_DEFAULT;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->macAddress_ = "00:11:22:33:44:55";
    info.device = device;
    outputDevices.push_back(info);
    
    handler.SetSelectedDevices(outputDevices, {});
    
    // Current is call (priority PRIORITY_CALL_OUTPUT),
    // existing is music (priority PRIORITY_NORMAL_OUTPUT), no conflict
    auto currentDevice = std::make_shared<AudioDeviceDescriptor>();
    currentDevice->macAddress_ = "00:11:22:33:44:66";
    EXPECT_FALSE(handler.Detect(currentDevice, ROUTER_TYPE_DEFAULT));
}

/**
 * @tc.name  : Test WirelessConflictHandler IsNearlinkDevice.
 * @tc.number: WirelessConflictHandler_011
 * @tc.desc  : Test IsNearlinkDevice interface with various device types.
 */
HWTEST(AudioRouterCenterUnitTest, WirelessConflictHandler_011, TestSize.Level1)
{
    WirelessConflictHandler handler;
    
    auto nearlinkDevice = std::make_shared<AudioDeviceDescriptor>();
    nearlinkDevice->deviceType_ = DEVICE_TYPE_NEARLINK;
    EXPECT_TRUE(handler.IsNearlinkDevice(nearlinkDevice));
    
    auto nearlinkInDevice = std::make_shared<AudioDeviceDescriptor>();
    nearlinkInDevice->deviceType_ = DEVICE_TYPE_NEARLINK_IN;
    EXPECT_TRUE(handler.IsNearlinkDevice(nearlinkInDevice));
    
    auto scoDevice = std::make_shared<AudioDeviceDescriptor>();
    scoDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    EXPECT_FALSE(handler.IsNearlinkDevice(scoDevice));
    
    auto speakerDevice = std::make_shared<AudioDeviceDescriptor>();
    speakerDevice->deviceType_ = DEVICE_TYPE_SPEAKER;
    EXPECT_FALSE(handler.IsNearlinkDevice(speakerDevice));
    
    EXPECT_FALSE(handler.IsNearlinkDevice(nullptr));
}

/**
 * @tc.name  : Test WirelessConflictHandler IsWirelessDevice.
 * @tc.number: WirelessConflictHandler_012
 * @tc.desc  : Test IsWirelessDevice interface with various device types.
 */
HWTEST(AudioRouterCenterUnitTest, WirelessConflictHandler_012, TestSize.Level1)
{
    WirelessConflictHandler handler;
    
    auto scoDevice = std::make_shared<AudioDeviceDescriptor>();
    scoDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    EXPECT_TRUE(handler.IsWirelessDevice(scoDevice));
    
    auto a2dpDevice = std::make_shared<AudioDeviceDescriptor>();
    a2dpDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    EXPECT_TRUE(handler.IsWirelessDevice(a2dpDevice));
    
    auto a2dpInDevice = std::make_shared<AudioDeviceDescriptor>();
    a2dpInDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;
    EXPECT_TRUE(handler.IsWirelessDevice(a2dpInDevice));
    
    auto nearlinkDevice = std::make_shared<AudioDeviceDescriptor>();
    nearlinkDevice->deviceType_ = DEVICE_TYPE_NEARLINK;
    EXPECT_TRUE(handler.IsWirelessDevice(nearlinkDevice));
    
    auto nearlinkInDevice = std::make_shared<AudioDeviceDescriptor>();
    nearlinkInDevice->deviceType_ = DEVICE_TYPE_NEARLINK_IN;
    EXPECT_TRUE(handler.IsWirelessDevice(nearlinkInDevice));
    
    auto speakerDevice = std::make_shared<AudioDeviceDescriptor>();
    speakerDevice->deviceType_ = DEVICE_TYPE_SPEAKER;
    EXPECT_FALSE(handler.IsWirelessDevice(speakerDevice));
    
    auto wiredDevice = std::make_shared<AudioDeviceDescriptor>();
    wiredDevice->deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    EXPECT_FALSE(handler.IsWirelessDevice(wiredDevice));
    
    EXPECT_FALSE(handler.IsWirelessDevice(nullptr));
}

/**
 * @tc.name  : Test WirelessConflictHandler Detect conflict with different wireless device.
 * @tc.number: WirelessConflictHandler_014
 * @tc.desc  : Test Detect when output tries to select different wireless device from input.
 */
HWTEST(AudioRouterCenterUnitTest, WirelessConflictHandler_014, TestSize.Level1)
{
    WirelessConflictHandler handler;
    
    std::vector<StreamDeviceInfo> inputDevices;
    StreamDeviceInfo inputInfo;
    inputInfo.routerType = ROUTER_TYPE_USER_SELECT;
    inputInfo.sourceType = SOURCE_TYPE_MIC;
    auto inputDevice = std::make_shared<AudioDeviceDescriptor>();
    inputDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    inputDevice->macAddress_ = "00:11:22:33:44:55";
    inputInfo.device = inputDevice;
    inputDevices.push_back(inputInfo);
    
    handler.SetSelectedDevices({}, inputDevices);
    
    auto outputDevice = std::make_shared<AudioDeviceDescriptor>();
    outputDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    outputDevice->macAddress_ = "00:11:22:33:44:66";
    
    EXPECT_TRUE(handler.Detect(outputDevice, ROUTER_TYPE_USER_SELECT));
}

/**
 * @tc.name  : Test WirelessConflictHandler no conflict with same wireless device pair.
 * @tc.number: WirelessConflictHandler_015
 * @tc.desc  : Test Detect when output and input have same macAddress (same device pair).
 */
HWTEST(AudioRouterCenterUnitTest, WirelessConflictHandler_015, TestSize.Level1)
{
    WirelessConflictHandler handler;
    
    std::vector<StreamDeviceInfo> inputDevices;
    StreamDeviceInfo inputInfo;
    inputInfo.routerType = ROUTER_TYPE_USER_SELECT;
    inputInfo.sourceType = SOURCE_TYPE_MIC;
    auto inputDevice = std::make_shared<AudioDeviceDescriptor>();
    inputDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    inputDevice->macAddress_ = "00:11:22:33:44:55";
    inputInfo.device = inputDevice;
    inputDevices.push_back(inputInfo);
    
    handler.SetSelectedDevices({}, inputDevices);
    
    auto outputDevice = std::make_shared<AudioDeviceDescriptor>();
    outputDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    outputDevice->macAddress_ = "00:11:22:33:44:55";
    
    EXPECT_FALSE(handler.Detect(outputDevice, ROUTER_TYPE_USER_SELECT));
}

/**
 * @tc.name  : Test WirelessConflictHandler nearlink device conflict.
 * @tc.number: WirelessConflictHandler_018
 * @tc.desc  : Test Detect with nearlink device conflict.
 */
HWTEST(AudioRouterCenterUnitTest, WirelessConflictHandler_018, TestSize.Level1)
{
    WirelessConflictHandler handler;
    
    std::vector<StreamDeviceInfo> inputDevices;
    StreamDeviceInfo inputInfo;
    inputInfo.routerType = ROUTER_TYPE_USER_SELECT;
    inputInfo.sourceType = SOURCE_TYPE_MIC;
    auto inputDevice = std::make_shared<AudioDeviceDescriptor>();
    inputDevice->deviceType_ = DEVICE_TYPE_NEARLINK_IN;
    inputDevice->macAddress_ = "00:11:22:33:44:55";
    inputInfo.device = inputDevice;
    inputDevices.push_back(inputInfo);
    
    handler.SetSelectedDevices({}, inputDevices);
    
    auto outputDevice = std::make_shared<AudioDeviceDescriptor>();
    outputDevice->deviceType_ = DEVICE_TYPE_NEARLINK;
    outputDevice->macAddress_ = "00:11:22:33:44:66";
    
    EXPECT_TRUE(handler.Detect(outputDevice, ROUTER_TYPE_USER_SELECT));
    
    outputDevice->macAddress_ = "00:11:22:33:44:55";
    EXPECT_FALSE(handler.Detect(outputDevice, ROUTER_TYPE_USER_SELECT));
}

} // namespace AudioStandard
} // namespace OHOS