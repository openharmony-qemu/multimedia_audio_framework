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
    
    EXPECT_FALSE(handler.Detect(nullptr, ROUTER_TYPE_NONE, STREAM_USAGE_MUSIC, SOURCE_TYPE_INVALID, true));
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
    
    EXPECT_FALSE(handler.Detect(device, ROUTER_TYPE_NONE, STREAM_USAGE_MUSIC, SOURCE_TYPE_INVALID, true));
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
    EXPECT_FALSE(handler.Detect(currentDevice, ROUTER_TYPE_DEFAULT,
        STREAM_USAGE_MUSIC, SOURCE_TYPE_INVALID, true));
}

/**
 * @tc.name  : Test WirelessConflictHandler Detect conflict with different device.
 * @tc.number: WirelessConflictHandler_008
 * @tc.desc  : Test Detect when current device has different mac from highest priority device,
 *             and current priority is lower, should return true (conflict).
 */
HWTEST(AudioRouterCenterUnitTest, WirelessConflictHandler_008, TestSize.Level1)
{
    WirelessConflictHandler handler;
    
    std::vector<StreamDeviceInfo> outputDevices;
    StreamDeviceInfo info;
    info.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION; // priority PRIORITY_CALL_OUTPUT
    info.routerType = ROUTER_TYPE_DEFAULT;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->macAddress_ = "00:11:22:33:44:55";
    info.device = device;
    outputDevices.push_back(info);
    
    handler.SetSelectedDevices(outputDevices, {});
    
    // Different mac address, current is music (priority PRIORITY_NORMAL_OUTPUT),
    // call is priority PRIORITY_CALL_OUTPUT
    auto currentDevice = std::make_shared<AudioDeviceDescriptor>();
    currentDevice->macAddress_ = "00:11:22:33:44:66";
    EXPECT_TRUE(handler.Detect(currentDevice, ROUTER_TYPE_DEFAULT,
        STREAM_USAGE_MUSIC, SOURCE_TYPE_INVALID, true));
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
    EXPECT_FALSE(handler.Detect(currentDevice, ROUTER_TYPE_DEFAULT,
        STREAM_USAGE_VOICE_COMMUNICATION, SOURCE_TYPE_INVALID, true));
}

/**
 * @tc.name  : Test WirelessConflictHandler Detect with input stream.
 * @tc.number: WirelessConflictHandler_010
 * @tc.desc  : Test Detect with input stream devices.
 */
HWTEST(AudioRouterCenterUnitTest, WirelessConflictHandler_010, TestSize.Level1)
{
    WirelessConflictHandler handler;
    
    std::vector<StreamDeviceInfo> inputDevices;
    StreamDeviceInfo info;
    info.sourceType = SOURCE_TYPE_MIC; // priority PRIORITY_NORMAL_INPUT for input
    info.routerType = ROUTER_TYPE_USER_SELECT; // priority PRIORITY_FORCED_INPUT for forced input
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->macAddress_ = "00:11:22:33:44:55";
    info.device = device;
    inputDevices.push_back(info);
    
    handler.SetSelectedDevices({}, inputDevices);
    
    // Different mac, current is normal input (priority PRIORITY_NORMAL_INPUT),
    // existing is forced input (priority PRIORITY_FORCED_INPUT)
    auto currentDevice = std::make_shared<AudioDeviceDescriptor>();
    currentDevice->macAddress_ = "00:11:22:33:44:66";
    EXPECT_TRUE(handler.Detect(currentDevice, ROUTER_TYPE_DEFAULT,
        STREAM_USAGE_UNKNOWN, SOURCE_TYPE_MIC, false));
}

} // namespace AudioStandard
} // namespace OHOS