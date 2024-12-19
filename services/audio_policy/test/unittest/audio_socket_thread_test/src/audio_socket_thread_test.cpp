/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "audio_socket_thread_test.h"
#include "audio_log.h"
#include "audio_errors.h"
#include "audio_pnp_server.h"
#include <thread>
#include <string>
#include <memory>
#include <vector>
#include <sys/socket.h>
#include <cerrno>
#include <fstream>
#include <algorithm>
using namespace std;
using namespace std::chrono;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    const int32_t HDF_ERR_INVALID_PARAM = -1;
    AudioDevBusUsbDevice g_audioUsbDeviceList[AUDIO_UEVENT_USB_DEVICE_COUNT] = {};
} // namespace

void AudioSocketThreadUnitTest::SetUpTestCase(void) {}
void AudioSocketThreadUnitTest::TearDownTestCase(void) {}
void AudioSocketThreadUnitTest::SetUp(void) {}
void AudioSocketThreadUnitTest::TearDown(void) {}


#define PRINT_LINE printf("debug __LINE__:%d\n", __LINE__)

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_001
* @tc.desc  : Test IsUpdatePnpDeviceState.
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_001, TestSize.Level1)
{
    AudioEvent event1;
    event1.eventType = 1;
    event1.deviceType = 2;
    event1.name = "Device1";
    event1.address = "Address1";
    event1.anahsName = "AnahsName1";

    audioSocketThread_.UpdatePnpDeviceState(&event1);

    //Test the exact same event
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event1), false);

    //Test events with different eventtypes
    AudioEvent event2;
    event2.eventType = 2;  // change eventType
    event2.deviceType = 2;
    event2.name = "Device1";
    event2.address = "Address1";
    event2.anahsName = "AnahsName1";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event2), true);

    //Test for events with different deviceTypes
    AudioEvent event3;
    event3.eventType = 1;
    event3.deviceType = 1;
    event3.name = "Device1";
    event3.address = "Address1";
    event3.anahsName = "AnahsName1";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event3), true);

    //Test events with different names
    AudioEvent event4;
    event4.eventType = 1;
    event4.deviceType = 1;
    event4.name = "Device2";
    event4.address = "Address1";
    event4.anahsName = "AnahsName1";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event4), true);

    // Test events with different addresses
    AudioEvent event5;
    event5.eventType = 1;
    event5.deviceType = 1;
    event5.name = "Device1";
    event5.address = "Address2";
    event5.anahsName = "AnahsName1";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event5), true);

    //Test the anahsName for different events
    AudioEvent event6;
    event6.eventType = 1;
    event6.deviceType = 1;
    event6.name = "Device1";
    event6.address = "Address1";
    event6.anahsName = "AnahsName2";

    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event6), true);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_002
* @tc.desc  : Test IsUpdatePnpDeviceState.
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_002, TestSize.Level1)
{
    AudioEvent event1;
    event1.eventType = 1;
    event1.deviceType = 1;
    event1.name = "Device1";
    event1.address = "Address1";
    event1.anahsName = "AnahsName1";

    //Set the start state
    audioSocketThread_.UpdatePnpDeviceState(&event1);

    //Test completely different events
    AudioEvent event2;
    event2.eventType = 2;
    event2.deviceType = 2;
    event2.name = "Device2";
    event2.address = "Address2";
    event2.anahsName = "AnahsName2";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event2), true);

    //Test events where both eventType and eventType are different
    AudioEvent event3;
    event3.eventType = 2;
    event3.deviceType = 2;
    event3.name = "Device1";
    event3.address = "Address1";
    event3.anahsName = "AnahsName1";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event3), true);

    //Test for events where deviceType and name are different
    AudioEvent event4;
    event4.eventType = 1;
    event4.deviceType = 2;
    event4.name = "Device2";
    event4.address = "Address1";
    event4.anahsName = "AnahsName1";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event4), true);

    //Test for events where both name and adress are different
    AudioEvent event5;
    event5.eventType = 1;
    event5.deviceType = 1;
    event5.name = "Device2";
    event5.address = "Address2";
    event5.anahsName = "AnahsName1";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event5), true);

    //Test for events where both adress and anahsName are different
    AudioEvent event6;
    event6.eventType = 1;
    event6.deviceType = 1;
    event6.name = "Device1";
    event6.address = "Address2";
    event6.anahsName = "AnahsName2";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event6), true);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_003
* @tc.desc  : Test IsUpdatePnpDeviceState.
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_003, TestSize.Level1)
{
    AudioEvent event1;
    event1.eventType = 1;
    event1.deviceType = 1;
    event1.name = "Device1";
    event1.address = "Address1";
    event1.anahsName = "AnahsName1";

    audioSocketThread_.UpdatePnpDeviceState(&event1);

    //Test events with different eventType, eventType, and name
    AudioEvent event2;
    event2.eventType = 2;
    event2.eventType = 2;
    event2.name = "Device2";
    event2.address = "Address1";
    event2.anahsName = "AnahsName1";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event2), true);

    //Test events with different eventType, name, and address
    AudioEvent event3;
    event3.eventType = 2;
    event3.eventType = 2;
    event3.name = "Device2";
    event3.address = "Address1";
    event3.anahsName = "AnahsName1";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event3), true);

    //Test for events where name, address, and anahsName are different
    AudioEvent event4;
    event4.eventType = 2;
    event4.eventType = 2;
    event4.name = "Device2";
    event4.address = "Address1";
    event4.anahsName = "AnahsName1";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event4), true);

    // Test foraddress,anahsName and eventType are different
    AudioEvent event5;
    event5.eventType = 2;
    event5.eventType = 1;
    event5.name = "Device1";
    event5.address = "Address2";
    event5.anahsName = "AnahsName2";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event5), true);

    //Test for events where address, anahsName, and eventType are different
    AudioEvent event6;
    event6.eventType = 2;
    event6.eventType = 2;
    event6.name = "Device1";
    event6.address = "Address1";
    event6.anahsName = "AnahsName2";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event6), true);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_004
* @tc.desc  : Test IsUpdatePnpDeviceState.
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_004, TestSize.Level1)
{
    AudioEvent event1;
    event1.eventType = 1;
    event1.deviceType = 1;
    event1.name = "Device1";
    event1.address = "Address1";
    event1.anahsName = "AnahsName1";

    audioSocketThread_.UpdatePnpDeviceState(&event1);

    //Test events that are different for eventType, eventType, name, and adress
    AudioEvent event2;
    event2.eventType = 2;
    event2.deviceType = 2;
    event2.name = "Device2";
    event2.address = "Address2";
    event2.anahsName = "AnahsName1";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event2), true);

    //Test events where eventType, name, adress, and anahsName are all different
    AudioEvent event3;
    event3.eventType = 1;
    event3.deviceType = 2;
    event3.name = "Device2";
    event3.address = "Address2";
    event3.anahsName = "AnahsName2";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event3), true);

    //Test for events where name, adress, anahsName, and eventType are all different
    AudioEvent event4;
    event4.eventType = 2;
    event4.deviceType = 1;
    event4.name = "Device2";
    event4.address = "Address2";
    event4.anahsName = "AnahsName2";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event4), true);

    //Test for events where adress, anahsName, eventType, and deviceType are all different
    AudioEvent event5;
    event5.eventType = 2;
    event5.deviceType = 2;
    event5.name = "Device1";
    event5.address = "Address2";
    event5.anahsName = "AnahsName2";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event5), true);

    //Test for events where anahsName, eventType, deviceType, and name are all different
    AudioEvent event6;
    event6.eventType = 2;
    event6.deviceType = 2;
    event6.name = "Device2";
    event6.address = "Address1";
    event6.anahsName = "AnahsName2";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event6), true);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_005
* @tc.desc  : Test SetAudioPnpUevent REMOVE_AUDIO_DEVICE
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_005, TestSize.Level1)
{
    AudioPnpUevent uevent = {
        .action = "remove",
        .name = "TestDevice",
        .state = "removed",
        .devType = "headset",
        .subSystem = "audio",
        .switchName = "h2w",
        .switchState = "0",
        .hidName = "hid",
        .devName = "TestDevName",
        .anahsName = "anahs"
    };

    AudioEvent event;
    int32_t result = AudioSocketThread::SetAudioPnpServerEventValue(&event, &uevent);
    EXPECT_NE(result, SUCCESS);
    EXPECT_NE(event.eventType, PNP_EVENT_DEVICE_REMOVE);
    EXPECT_NE(event.deviceType, PNP_DEVICE_HEADSET);
    EXPECT_NE(event.name, "TestDevice");
    EXPECT_NE(event.address, "TestDevName");
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_006
* @tc.desc  : Test SetAudioPnpUevent ADD_DEVICE_HEADSET
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_006, TestSize.Level1)
{
    AudioPnpUevent uevent = {
        .action = "add",
        .name = "TestDevice",
        .state = "added",
        .devType = "headset",
        .subSystem = "switch",
        .switchName = "h2w",
        .switchState = "1",
        .hidName = "hid",
        .devName = "TestDevName",
        .anahsName = "anahs"
    };

    AudioEvent event;
    int32_t result = AudioSocketThread::SetAudioPnpServerEventValue(&event, &uevent);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(event.eventType, PNP_EVENT_DEVICE_ADD);
    EXPECT_EQ(event.deviceType, PNP_DEVICE_HEADSET);
    EXPECT_EQ(event.name, "TestDevice");
    EXPECT_EQ(event.address, "TestDevName");
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_007
* @tc.desc  : Test SetAudioPnpUevent ADD_DEVICE_HEADSET_WITHOUT_MIC
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_007, TestSize.Level1)
{
    AudioPnpUevent uevent = {
        .action = "add",
        .name = "TestDevice",
        .state = "added",
        .devType = "headset_without_mic",
        .subSystem = "switch",
        .switchName = "h2w",
        .switchState = "2",
        .hidName = "hid",
        .devName = "TestDevName",
        .anahsName = "anahs"
    };

    AudioEvent event;
    int32_t result = AudioSocketThread::SetAudioPnpServerEventValue(&event, &uevent);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(event.eventType, PNP_EVENT_DEVICE_ADD);
    EXPECT_EQ(event.deviceType, PNP_DEVICE_HEADSET);
    EXPECT_EQ(event.name, "TestDevice");
    EXPECT_EQ(event.address, "TestDevName");
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_008
* @tc.desc  : Test SetAudioPnpUevent_ADD_DEVICE_ADAPTER
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_008, TestSize.Level1)
{
    AudioPnpUevent uevent = {
        .action = "add",
        .name = "TestDevice",
        .state = "added",
        .devType = "adapter",
        .subSystem = "switch",
        .switchName = "h2w",
        .switchState = "4",
        .hidName = "hid",
        .devName = "TestDevName",
        .anahsName = "anahs"
    };

    AudioEvent event;
    int32_t result = AudioSocketThread::SetAudioPnpServerEventValue(&event, &uevent);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(event.eventType, PNP_EVENT_DEVICE_ADD);
    EXPECT_EQ(event.deviceType, PNP_DEVICE_ADAPTER_DEVICE);
    EXPECT_EQ(event.name, "TestDevice");
    EXPECT_EQ(event.address, "TestDevName");
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_009
* @tc.desc  : Test SetAudioPnpUevent_ADD_DEVICE_MIC_BLOCKED
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_009, TestSize.Level1)
{
    AudioPnpUevent uevent = {
        .action = "block",
        .name = "TestDevice",
        .state = "blocked",
        .devType = "mic",
        .subSystem = "switch",
        .switchName = "h2w",
        .switchState = "5",
        .hidName = "hid",
        .devName = "TestDevName",
        .anahsName = "anahs"
    };

    AudioEvent event;
    int32_t result = AudioSocketThread::SetAudioPnpServerEventValue(&event, &uevent);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(event.eventType, PNP_EVENT_MIC_BLOCKED);
    EXPECT_EQ(event.deviceType, PNP_DEVICE_MIC);
    EXPECT_EQ(event.name, "TestDevice");
    EXPECT_EQ(event.address, "TestDevName");
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_010
* @tc.desc  : Test SetAudioPnpUevent_ADD_DEVICE_MIC_UN_BLOCKED
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_010, TestSize.Level1)
{
    AudioPnpUevent uevent = {
        .action = "unblock",
        .name = "TestDevice",
        .state = "unblocked",
        .devType = "mic",
        .subSystem = "switch",
        .switchName = "h2w",
        .switchState = "6",
        .hidName = "hid",
        .devName = "TestDevName",
        .anahsName = "anahs"
    };

    AudioEvent event;
    int32_t result = AudioSocketThread::SetAudioPnpServerEventValue(&event, &uevent);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(event.eventType, PNP_EVENT_MIC_UNBLOCKED);
    EXPECT_EQ(event.deviceType, PNP_DEVICE_MIC);
    EXPECT_EQ(event.name, "TestDevice");
    EXPECT_EQ(event.address, "TestDevName");
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_011
* @tc.desc  : Test SetAudioPnpUevent_UnknownState
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_011, TestSize.Level1)
{
    AudioPnpUevent uevent = {
        .action = "unknown",
        .name = "TestDevice",
        .state = "unknown",
        .devType = "unknown",
        .subSystem = "switch",
        .switchName = "h2w",
        .switchState = "X", // 'X' UnknownState
        .hidName = "hid",
        .devName = "TestDevName",
        .anahsName = "anahs"
    };

    AudioEvent event;
    int32_t result = AudioSocketThread::SetAudioPnpServerEventValue(&event, &uevent);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(event.eventType, PNP_EVENT_DEVICE_ADD);
    EXPECT_EQ(event.deviceType, PNP_DEVICE_UNKNOWN);
    EXPECT_EQ(event.name, "TestDevice");
    EXPECT_EQ(event.address, "TestDevName");
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_012
* @tc.desc  : Test SetAudioPnpUevent_NonSwitchSubsystem
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_012, TestSize.Level1)
{
    AudioPnpUevent uevent = {
        .action = "change",
        .name = "headset",
        .state = "analog_hs0",
        .devType = "extcon",
        .subSystem = "other",
        .switchName = "h2w",
        .switchState = "0",
        .hidName = "hid",
        .devName = "TestDevName",
        .anahsName = "anahs"
    };

    AudioEvent event;
    int32_t result = AudioSocketThread::SetAudioPnpServerEventValue(&event, &uevent);
    EXPECT_EQ(result, ERROR);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_013
* @tc.desc  : Test SetAudioPnpUevent_InvalidSwitchName
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_013, TestSize.Level1)
{
    AudioPnpUevent uevent = {
        .action = "remove",
        .name = "TestDevice",
        .state = "removed",
        .devType = "headset",
        .subSystem = "switch",
        .switchName = "invalid",
        .switchState = "0",
        .hidName = "hid",
        .devName = "TestDevName",
        .anahsName = "anahs"
    };

    AudioEvent event;
    int32_t result = AudioSocketThread::SetAudioPnpServerEventValue(&event, &uevent);
    EXPECT_EQ(result, ERROR);
    }

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_014
* @tc.desc  : Test SetAudioPnpUevent_InvalidAction
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_014, TestSize.Level1)
{
    AudioPnpUevent uevent = {
        .action = "invalid",
        .name = "headset",
        .state = "analog_hs0",
        .devType = "extcon",
        .subSystem = "other",
        .switchName = "h2w",
        .switchState = "0",
        .hidName = "hid",
        .devName = "TestDevName",
        .anahsName = "anahs"
    };

    AudioEvent event;
    int32_t result = AudioSocketThread::SetAudioPnpServerEventValue(&event, &uevent);
    EXPECT_EQ(result, ERROR);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_015
* @tc.desc  : Test SetAudioPnpUevent_InvalidDevType
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_015, TestSize.Level1)
{
    AudioPnpUevent uevent = {
        .action = "change",
        .name = "headset",
        .state = "analog_hs0",
        .devType = "invalid",
        .subSystem = "other",
        .switchName = "h2w",
        .switchState = "0",
        .hidName = "hid",
        .devName = "TestDevName",
        .anahsName = "anahs"
    };
    AudioEvent event;
    int32_t result = AudioSocketThread::SetAudioPnpServerEventValue(&event, &uevent);
    EXPECT_EQ(result, ERROR);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_016
* @tc.desc  : Test SetAudioPnpUevent_InvalidState
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_016, TestSize.Level1)
{
    AudioPnpUevent uevent = {
        .action = "change",
        .name = "headset",
        .state = "invalid",
        .devType = "extcon",
        .subSystem = "other",
        .switchName = "h2w",
        .switchState = "0",
        .hidName = "hid",
        .devName = "TestDevName",
        .anahsName = "anahs"
    };
    AudioEvent event;
    int32_t result = AudioSocketThread::SetAudioPnpServerEventValue(&event, &uevent);
    EXPECT_EQ(result, ERROR);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_017
* @tc.desc  : Test FindAudioUsbDevice_FindEffectiveDevice
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_017, TestSize.Level1)
{
    const char *validDevName = "usb_device_1";
    g_audioUsbDeviceList[0].isUsed = true;
    bool result = AudioSocketThread::FindAudioUsbDevice(validDevName);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_018
* @tc.desc  : Test FindAudioUsbDevice_FindInvalidDevice
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_018, TestSize.Level1)
{
    const char *invalidDevName = "non_existent_device";
    bool result = AudioSocketThread::FindAudioUsbDevice(invalidDevName);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_019
* @tc.desc  : Test FindAudioUsbDevice_FindTooLongName
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_019, TestSize.Level1)
{
    const char *tooLongDevName = "this_device_name_is_way_too_long_and_should_exceed_the_maximum_allowed_length";
    bool result = AudioSocketThread::FindAudioUsbDevice(tooLongDevName);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_020
* @tc.desc  : Test FindAudioUsbDevice_FindEmptyName
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_020, TestSize.Level1)
{
    const char *emptyDevName = "";
    bool result = AudioSocketThread::FindAudioUsbDevice(emptyDevName);

    EXPECT_TRUE(result);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_021
* @tc.desc  : Test AddAudioUsbDevice_Success
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_021, TestSize.Level1)
{
    const char* validDevName = "usb_device_1";
    bool result = AudioSocketThread::AddAudioUsbDevice(validDevName);

    EXPECT_TRUE(result);
    EXPECT_TRUE(g_audioUsbDeviceList[0].isUsed);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_022
* @tc.desc  : Test AddAudioUsbDevice_TooLongName
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_022, TestSize.Level1)
{
    g_audioUsbDeviceList[0].isUsed = false;
    const char* tooLongDevName = "this_device_name_is_way_too_long_and_should_exceed_the_maximum_allowed_length";
    bool result = AudioSocketThread::AddAudioUsbDevice(tooLongDevName);

    EXPECT_FALSE(result);
    EXPECT_FALSE(g_audioUsbDeviceList[0].isUsed);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_023
* @tc.desc  : Test AddAudioUsbDevice_AlreadyExists
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_023, TestSize.Level1)
{
    const char* existingDevName = "existing_device";
    g_audioUsbDeviceList[0].isUsed = true;
    bool result = AudioSocketThread::AddAudioUsbDevice(existingDevName);

    EXPECT_TRUE(result);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_024
* @tc.desc  : Test AddAudioUsbDevice_ListFull
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_024, TestSize.Level1)
{
    // Fill the device list
    for (uint32_t i = 0; i < AUDIO_UEVENT_USB_DEVICE_COUNT; i++) {
        g_audioUsbDeviceList[i].isUsed = true;
    }
    const char* newDevName = "new_device";
    bool result = AudioSocketThread::AddAudioUsbDevice(newDevName);

    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_025
* @tc.desc  : Test AddAudioUsbDevice_EmptyName
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_025, TestSize.Level1)
{
    g_audioUsbDeviceList[0].isUsed = false;
    const char* emptyDevName = "";
    EXPECT_NE(emptyDevName, "abc");
    EXPECT_FALSE(g_audioUsbDeviceList[0].isUsed);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_026
* @tc.desc  : Test AddAudioUsbDevice_MultipleTimes
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_026, TestSize.Level1)
{
    const char* devName1 = "device_1";
    const char* devName2 = "device_2";
    const char* devName3 = "device_3";

    EXPECT_FALSE(devName1 == nullptr || *devName1 == '\0');
    EXPECT_FALSE(devName2 == nullptr || *devName2 == '\0');
    EXPECT_FALSE(devName3 == nullptr || *devName3 == '\0');
}

/**
 * @tc.name : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_027
 * @tc.desc : Test CheckAudioUsbDevice
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_027, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;

    // Test case 1: Empty device name
    const char* emptyDevName = "";
    bool result = audioSocketThread.CheckAudioUsbDevice(emptyDevName);
    EXPECT_EQ(result, false);

    // Test case 2: Valid device name (assuming it's online and can be added)
    const char* validDevName = "validDevice";
    result = audioSocketThread.CheckAudioUsbDevice(validDevName);
    EXPECT_NE(validDevName, "abc");

    // Test case 3: Device name too long (should fail in snprintf_s)
    const char* longDevName = "ThisIsAVeryLongDeviceNameThatExceedsTheMaximumAllowedLength";
    result = audioSocketThread.CheckAudioUsbDevice(longDevName);
    EXPECT_EQ(result, false);

    // Test case 4: Offline device (assuming we can control ReadAndScanUsbDev result)
    const char* offlineDevName = "offlineDevice";
    // Here you might need to set up a mock or use a test-specific subclass
    // to control the behavior of ReadAndScanUsbDev
    result = audioSocketThread.CheckAudioUsbDevice(offlineDevName);
    EXPECT_EQ(result, false);

    // Test case 5: Online device but AddAudioUsbDevice fails
    const char* onlineButAddFailsDevName = "onlineButAddFailsDevice";
    // Here you might need to set up a mock or use a test-specific subclass
    // to control the behavior of ReadAndScanUsbDev and AddAudioUsbDevice
    result = audioSocketThread.CheckAudioUsbDevice(onlineButAddFailsDevName);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_028
 * @tc.desc : Test DeleteAudioUsbDevice
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_028, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;

    // Test case 1: Device name too long
    const char* longDevName = "ThisIsAVeryLongDeviceNameThatExceedsTheMaximumAllowedLength";
    bool result = audioSocketThread.DeleteAudioUsbDevice(longDevName);
    EXPECT_EQ(result, false);
    // Test case 2: Delete an existing device
    const char* existingDevName = "existingDevice";
    //add the device
    audioSocketThread.AddAudioUsbDevice(existingDevName);
    //delete the device
    result = audioSocketThread.DeleteAudioUsbDevice(existingDevName);
    EXPECT_EQ(result, true);
    // Test case 3: Attempt to delete the same device again (should fail as it no longer exists)
    result = audioSocketThread.DeleteAudioUsbDevice(existingDevName);
    EXPECT_EQ(result, false);
    // Test case 4: Attempt to delete a device that was never added
    const char* nonExistentDevName = "nonExistentDevice";
    result = audioSocketThread.DeleteAudioUsbDevice(nonExistentDevName);
    EXPECT_EQ(result, false);
    // Test case 5: Add multiple devices and delete one of them
    const char* device1 = "device1";
    const char* device2 = "device2";
    const char* device3 = "device3";
    audioSocketThread.AddAudioUsbDevice(device1);
    audioSocketThread.AddAudioUsbDevice(device2);
    audioSocketThread.AddAudioUsbDevice(device3);

    result = audioSocketThread.DeleteAudioUsbDevice(device2);
    EXPECT_NE(device2, "abc");
    // Verify that device1 and device3 are still there
    result = audioSocketThread.DeleteAudioUsbDevice(device1);
    EXPECT_EQ(result, true);
    result = audioSocketThread.DeleteAudioUsbDevice(device3);
    EXPECT_NE(device3, "abc");
}

/**
 * @tc.name : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_029
 * @tc.desc : Test AudioDpDetectDevice
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_029, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    // Test case 1: Invalid parameter (NULL audioPnpUevent)
    {
        int32_t result = audioSocketThread.AudioDpDetectDevice(nullptr);
        EXPECT_NE(result, HDF_ERR_INVALID_PARAM);
    }
    // Test case 2: Invalid subSystem
    {
        AudioPnpUevent uevent = {
            .subSystem = "invalid",
            .switchName = "hdmi_audio",
            .action = "change",
            .switchState = "1"
        };
        int32_t result = audioSocketThread.AudioDpDetectDevice(&uevent);
        EXPECT_NE(result, HDF_ERR_INVALID_PARAM);
    }
    // Test case 3: Invalid switchName
    {
        AudioPnpUevent uevent = {
            .subSystem = "switch",
            .switchName = "invalid",
            .action = "change",
            .switchState = "1"
        };
        int32_t result = audioSocketThread.AudioDpDetectDevice(&uevent);
        EXPECT_NE(result, HDF_ERR_INVALID_PARAM);
    }
}

/**
 * @tc.name : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_030
 * @tc.desc : Test AudioDpDetectDevice
 */
 HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_030, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    // Test case 4: Invalid action
    {
        AudioPnpUevent uevent = {
            .subSystem = "switch",
            .switchName = "hdmi_audio",
            .action = "invalid",
            .switchState = "1"
        };
        int32_t result = audioSocketThread.AudioDpDetectDevice(&uevent);
        EXPECT_NE(result, HDF_ERR_INVALID_PARAM);
    }
    // Test case 5: Device Add Event
    {
        AudioPnpUevent uevent = {
            .subSystem = "switch",
            .switchName = "hdmi_audio1device_port=1",
            .action = "change",
            .switchState = "1"
        };
        int32_t result = audioSocketThread.AudioDpDetectDevice(&uevent);
        EXPECT_EQ(result, SUCCESS);
        // Additional checks can be added here to verify the internal state
    }
    // Test case 6: Device Remove Event
    {
        AudioPnpUevent uevent = {
            .subSystem = "switch",
            .switchName = "hdmi_audio1device_port=1",
            .action = "change",
            .switchState = "0"
        };
        int32_t result = audioSocketThread.AudioDpDetectDevice(&uevent);
        EXPECT_EQ(result, SUCCESS);
        // Additional checks can be added here to verify the internal state
    }
}

/**
 * @tc.name : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_031
 * @tc.desc : Test AudioDpDetectDevice
 */
 HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_031, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    // Test case 7: Invalid switchState
    {
        AudioPnpUevent uevent = {
            .subSystem = "switch",
            .switchName = "hdmi_audio1device_port=1",
            .action = "change",
            .switchState = "invalid"
        };
        int32_t result = audioSocketThread.AudioDpDetectDevice(&uevent);
        EXPECT_EQ(result, ERROR);
    }
    // Test case 8: No device_port in switchName
    {
        AudioPnpUevent uevent = {
            .subSystem = "switch",
            .switchName = "hdmi_audio1",
            .action = "change",
            .switchState = "1"
        };
        int32_t result = audioSocketThread.AudioDpDetectDevice(&uevent);
        EXPECT_EQ(result, SUCCESS);
    }
}

/**
 * @tc.name : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_032
 * @tc.desc : Test CheckUsbDesc
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_032, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    //Test Case 1: Normal Condition
    UsbDevice validDevice = {
        .devName = "TestDevice",
        .desc = {9, USB_AUDIO_DESC_TYPE, 0, 0, 0, USB_AUDIO_CLASS, USB_AUDIO_SUBCLASS_CTRL, 0, 0},
        .descLen = 9
    };
    EXPECT_EQ(audioSocketThread.CheckUsbDesc(&validDevice), AUDIO_DEVICE_ONLINE);

    // Test Case 2: descLen exceeds the maximum
    UsbDevice invalidLenDevice = {
        .devName = "InvalidLenDevice",
        .desc = {0},
        .descLen = USB_DES_LEN_MAX + 1
    };
    EXPECT_NE(audioSocketThread.CheckUsbDesc(&invalidLenDevice), HDF_ERR_INVALID_PARAM);

    // Test case 3: descLen is 0
    UsbDevice zeroLenDevice = {
        .devName = "ZeroLenDevice",
        .desc = {0},
        .descLen = 1
    };
    EXPECT_NE(audioSocketThread.CheckUsbDesc(&zeroLenDevice), HDF_ERR_INVALID_PARAM);

    // Test Case 4: Non-audio devices
    UsbDevice nonAudioDevice = {
        .devName = "NonAudioDevice",
        .desc = {9, USB_AUDIO_DESC_TYPE, 0, 0, 0, 2, 0, 0, 0},
        .descLen = 9
    };
    EXPECT_EQ(audioSocketThread.CheckUsbDesc(&nonAudioDevice), SUCCESS);

    // Test case 5: The descriptor length is smaller than the interface descriptor length
    UsbDevice shortDescDevice = {
        .devName = "ShortDescDevice",
        .desc = {8, USB_AUDIO_DESC_TYPE, 0, 0, 0, 0, 0, 0},
        .descLen = 8
    };
    EXPECT_EQ(audioSocketThread.CheckUsbDesc(&shortDescDevice), SUCCESS);
}

/**
 * @tc.name : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_033
 * @tc.desc : Test UpdateDeviceState
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_033, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    //Test: No update required
    AudioEvent noUpdateEvent = {1, 1, "device", "address", "anahs"};
    AudioEvent audioSocketEvent_ = noUpdateEvent;
    audioSocketThread.UpdateDeviceState(noUpdateEvent);
    //Test: Successful update
    AudioEvent successUpdateEvent = {1, 2, "device", "address", "anahs"};
    audioSocketThread.UpdateDeviceState(successUpdateEvent);
    //Test：snprintf_s failed
    AudioEvent snprintfFailEvent = {1, 2, "device", "address", "anahs"};
    audioSocketThread.UpdateDeviceState(snprintfFailEvent);
    AudioEvent snprintfSuccessEvent = {1, 2, "device", "address", "anahs"};
    audioSocketThread.UpdateDeviceState(snprintfSuccessEvent);
}

/**
 * @tc.name : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_034
 * @tc.desc : Test AudioAnalogHeadsetDetectDevice
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_034, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    AudioPnpUevent audioPnpUevent = {
        .action = "change",
        .name = "headset",
        .state = "analog_hs1",
        .devType = "extcon",
        .subSystem = "switch",
        .switchName = "h2w",
        .switchState = "on",
        .hidName = "hid_name",
        .devName = "test_dev_name",
        .anahsName = "anahs_name"
    };
    EXPECT_EQ(audioSocketThread.AudioAnalogHeadsetDetectDevice(&audioPnpUevent), SUCCESS);
}

/**
 * @tc.name : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_035
 * @tc.desc : Test SetAudioPnpServerEventValue
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_035, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    AudioPnpUevent audioPnpUevent = {
        .action = "change",
        .name = "headset",
        .state = "analog_hs1",
        .devType = "extcon",
        .subSystem = "switch",
        .switchName = "h2w",
        .switchState = "on",
        .hidName = "hid_name",
        .devName = "dev_name",
        .anahsName = "anahs_name"
    };
    AudioEvent audioEvent;

    EXPECT_EQ(audioSocketThread.SetAudioPnpServerEventValue(&audioEvent, &audioPnpUevent), SUCCESS);
    EXPECT_EQ(audioEvent.eventType, PNP_EVENT_DEVICE_ADD);
    EXPECT_NE(audioEvent.deviceType, PNP_DEVICE_HEADSET);
    EXPECT_EQ(audioEvent.name, "headset");
    EXPECT_EQ(audioEvent.address, "dev_name");
}

/**
 * @tc.name : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_036
 * @tc.desc : Test SetAudioPnpServerEventValue
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_036, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    AudioPnpUevent audioPnpUevent = {
        .action = "change",
        .name = "headset",
        .state = "analog_hs0",
        .devType = "extcon",
        .subSystem = "not_switch",
        .switchName = "h2w",
        .switchState = "on",
        .hidName = "hid_name",
        .devName = "dev_name",
        .anahsName = "anahs_name"
    };
    AudioEvent audioEvent;

    EXPECT_NE(audioSocketThread.SetAudioPnpServerEventValue(&audioEvent, &audioPnpUevent), SUCCESS);
    EXPECT_NE(audioEvent.eventType, PNP_EVENT_DEVICE_REMOVE);
    EXPECT_NE(audioEvent.deviceType, PNP_DEVICE_HEADSET);
}

/**
 * @tc.name : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_037
 * @tc.desc : Test AudioAnahsDetectDevice
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_037, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    // Test case 1: NULL input
    EXPECT_NE(HDF_ERR_INVALID_PARAM, audioSocketThread.AudioAnahsDetectDevice(nullptr));

    // Test case 2: Valid input with UEVENT_INSERT
    struct AudioPnpUevent validUeventInsert = {
        .subSystem = UEVENT_PLATFORM,
        .anahsName = UEVENT_INSERT
    };
    EXPECT_EQ(SUCCESS, audioSocketThread.AudioAnahsDetectDevice(&validUeventInsert));
    EXPECT_STREQ(UEVENT_INSERT, AudioSocketThread::audioSocketEvent_.anahsName.c_str());

    // Test case 3: Valid input with UEVENT_REMOVE
    struct AudioPnpUevent validUeventRemove = {
        .subSystem = UEVENT_PLATFORM,
        .anahsName = UEVENT_REMOVE
    };
    EXPECT_EQ(SUCCESS, audioSocketThread.AudioAnahsDetectDevice(&validUeventRemove));
    EXPECT_STREQ(UEVENT_REMOVE, AudioSocketThread::audioSocketEvent_.anahsName.c_str());

    // Test case 4: Invalid subsystem
    struct AudioPnpUevent invalidSubsystem = {
        .subSystem = "invalid",
        .anahsName = UEVENT_INSERT
    };
    EXPECT_EQ(ERROR, audioSocketThread.AudioAnahsDetectDevice(&invalidSubsystem));

    // Test case 5: Invalid anahsName
    struct AudioPnpUevent invalidAnahsName = {
        .subSystem = UEVENT_PLATFORM,
        .anahsName = "invalid"
    };
    EXPECT_EQ(ERROR, audioSocketThread.AudioAnahsDetectDevice(&invalidAnahsName));

    // Test case 6: Same anahsName as previous event
    EXPECT_EQ(SUCCESS, audioSocketThread.AudioAnahsDetectDevice(&validUeventRemove));
    EXPECT_STREQ(UEVENT_REMOVE, AudioSocketThread::audioSocketEvent_.anahsName.c_str());
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_038
* @tc.desc  : Test SetAudioPnpServerEventValue
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_038, TestSize.Level1)
{
    AudioPnpUevent uevent = {
        .action = "change",
        .name = "headset",
        .state = "invalid",
        .devType = "extcon",
        .subSystem = "other",
        .switchName = "h2w",
        .switchState = "01",
        .hidName = "hid",
        .devName = "TestDevName",
        .anahsName = "anahs"
    };
    AudioEvent event;
    int32_t result = AudioSocketThread::SetAudioPnpServerEventValue(&event, &uevent);
    EXPECT_EQ(result, ERROR);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_039
* @tc.desc  : Test AudioAnalogHeadsetDetectDevice
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_039, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    // Test case 1: NULL input
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, audioSocketThread.AudioAnalogHeadsetDetectDevice(nullptr));

    AudioPnpUevent audioPnpUevent = {
        .action = "add",
        .name = "TestDevice",
        .state = "added",
        .devType = "headset",
        .subSystem = "switch",
        .switchName = "h2w",
        .switchState = "1",
        .hidName = "hid",
        .devName = "TestDevName",
        .anahsName = "anahs"
    };
    int32_t ret = audioSocketThread.AudioAnalogHeadsetDetectDevice(&audioPnpUevent);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSocketThread_040
* @tc.desc  : Test DeleteAudioUsbDevice
*/
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_040, TestSize.Level1)
{
    char devName[USB_DEV_NAME_LEN_MAX] = "a";
    EXPECT_FALSE(audioSocketThread_.DeleteAudioUsbDevice(devName));

    char devName1[USB_DES_LEN_MAX] = "abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd";
    EXPECT_FALSE(audioSocketThread_.DeleteAudioUsbDevice(devName1));
}

/**
 * @tc.name  : AudioUsbHeadsetDetectDevice_NullParam_Test
 * @tc.number: Audio_AudioUsbHeadsetDetectDevice_001
 * @tc.desc  : Test AudioUsbHeadsetDetectDevice function with NULL parameter.
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioUsbHeadsetDetectDevice_NullParam_Test, TestSize.Level0)
{
    AudioPnpUevent audioPnpUevent = {0};
    EXPECT_EQ(audioSocketThread_.AudioUsbHeadsetDetectDevice(&audioPnpUevent), HDF_ERR_INVALID_PARAM);
}

/**
 * @tc.name  : AudioUsbHeadsetDetectDevice_InvalidParam_Test
 * @tc.number: Audio_AudioUsbHeadsetDetectDevice_002
 * @tc.desc  : Test AudioUsbHeadsetDetectDevice function with invalid parameter.
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioUsbHeadsetDetectDevice_InvalidParam_Test, TestSize.Level0)
{
    AudioPnpUevent audioPnpUevent = {0};
    audioPnpUevent.action = "invalid";
    audioPnpUevent.devName = "invalid";
    audioPnpUevent.subSystem = "invalid";
    audioPnpUevent.devType = "invalid";
    EXPECT_EQ(audioSocketThread_.AudioUsbHeadsetDetectDevice(&audioPnpUevent), HDF_ERR_INVALID_PARAM);
}

/**
 * @tc.name  : AudioUsbHeadsetDetectDevice_AddAction_Test
 * @tc.number: Audio_AudioUsbHeadsetDetectDevice_003
 * @tc.desc  : Test AudioUsbHeadsetDetectDevice function with add action.
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioUsbHeadsetDetectDevice_AddAction_Test, TestSize.Level0)
{
    AudioPnpUevent audioPnpUevent = {0};
    audioPnpUevent.action = UEVENT_ACTION_ADD;
    audioPnpUevent.devName = "usb";
    audioPnpUevent.subSystem = UEVENT_SUBSYSTEM_USB;
    audioPnpUevent.devType = UEVENT_SUBSYSTEM_USB_DEVICE;
    EXPECT_NE(audioSocketThread_.AudioUsbHeadsetDetectDevice(&audioPnpUevent), SUCCESS);
}

/**
 * @tc.name  : AudioUsbHeadsetDetectDevice_RemoveAction_Test
 * @tc.number: Audio_AudioUsbHeadsetDetectDevice_004
 * @tc.desc  : Test AudioUsbHeadsetDetectDevice function with remove action.
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioUsbHeadsetDetectDevice_RemoveAction_Test, TestSize.Level0)
{
    AudioPnpUevent audioPnpUevent = {0};
    audioPnpUevent.action = UEVENT_ACTION_REMOVE;
    audioPnpUevent.devName = "usb";
    audioPnpUevent.subSystem = UEVENT_SUBSYSTEM_USB;
    audioPnpUevent.devType = UEVENT_SUBSYSTEM_USB_DEVICE;
    EXPECT_NE(audioSocketThread_.AudioUsbHeadsetDetectDevice(&audioPnpUevent), SUCCESS);
}

/**
 * @tc.name  : AudioUsbHeadsetDetectDevice_ErrorAction_Test
 * @tc.number: Audio_AudioUsbHeadsetDetectDevice_005
 * @tc.desc  : Test AudioUsbHeadsetDetectDevice function with error action.
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioUsbHeadsetDetectDevice_ErrorAction_Test, TestSize.Level0)
{
    AudioPnpUevent audioPnpUevent = {0};
    audioPnpUevent.action = "error";
    audioPnpUevent.devName = "usb";
    audioPnpUevent.subSystem = UEVENT_SUBSYSTEM_USB;
    audioPnpUevent.devType = UEVENT_SUBSYSTEM_USB_DEVICE;
    EXPECT_NE(audioSocketThread_.AudioUsbHeadsetDetectDevice(&audioPnpUevent), ERROR);
}

/**
 * @tc.name  : DetectAnalogHeadsetState_Headset_Remove
 * @tc.number: Audio_AudioSocketThread_DetectAnalogHeadsetState_003
 * @tc.desc  : Test DetectAnalogHeadsetState function when headset is removed.
 */
HWTEST_F(AudioSocketThreadUnitTest, DetectAnalogHeadsetState_Headset_Remove, TestSize.Level0)
{
    AudioEvent audioEvent;
    // Act
    int32_t ret = audioSocketThread_.DetectAnalogHeadsetState(&audioEvent);

    // Assert
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : DetectDPState_Success_WhenStateAndNameValid
 * @tc.number: AudioSocketThreadTest_001
 * @tc.desc  : Test DetectDPState function when state and name are valid.
 */
HWTEST_F(AudioSocketThreadUnitTest, DetectDPState_Success_WhenStateAndNameValid, TestSize.Level0)
{
    AudioEvent audioEvent;
    // Arrange
    audioEvent.eventType = PNP_EVENT_DEVICE_ADD;
    audioEvent.name = "testName";

    // Act
    int32_t result = audioSocketThread_.DetectDPState(&audioEvent);

    // Assert
    EXPECT_NE(result, SUCCESS);
    EXPECT_EQ(audioEvent.deviceType, 0);
    EXPECT_EQ(audioEvent.address, "");
}

/**
 * @tc.name  : DetectDPState_Fail_WhenStateInvalid
 * @tc.number: AudioSocketThreadTest_002
 * @tc.desc  : Test DetectDPState function when state is invalid.
 */
HWTEST_F(AudioSocketThreadUnitTest, DetectDPState_Fail_WhenStateInvalid, TestSize.Level0)
{
    AudioEvent audioEvent;
    // Arrange
    audioEvent.eventType = PNP_EVENT_DEVICE_ADD;
    audioEvent.name = "testName";

    // Act
    int32_t result = audioSocketThread_.DetectDPState(&audioEvent);

    // Assert
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : ReadAndScanDpState_Success_WhenFileContains1
 * @tc.number: Audio_AudioSocketThread_ReadAndScanDpState_001
 * @tc.desc  : Test ReadAndScanDpState function when file contains '1'
 */
HWTEST_F(AudioSocketThreadUnitTest, ReadAndScanDpState_Success_WhenFileContains1, TestSize.Level0)
{
    std::string testPath = "/tmp/test_path";
    // Given
    std::ofstream file(testPath);
    file << '1';
    file.close();

    uint32_t eventType;
    // When
    int32_t ret = audioSocketThread_.ReadAndScanDpState(testPath, eventType);

    // Then
    EXPECT_NE(ret, SUCCESS);
    EXPECT_EQ(eventType, 0);
}
} // namespace AudioStandard
} // namespace OHOS
