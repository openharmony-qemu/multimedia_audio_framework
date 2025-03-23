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
#include <thread>
#include <string>
#include <memory>
#include <vector>
#include <sys/socket.h>
#include <cerrno>
#include <fstream>
#include <algorithm>
#include <unistd.h>
#include "audio_policy_log.h"
#include "audio_errors.h"
#include "audio_pnp_server.h"
using namespace std;
using namespace std::chrono;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    const int32_t HDF_ERR_INVALID_PARAM = -3;
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
    EXPECT_EQ(event.eventType, PNP_EVENT_DEVICE_ADD);
    EXPECT_EQ(event.deviceType, PNP_DEVICE_UNKNOWN);
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
    EXPECT_EQ(event.eventType, PNP_EVENT_DEVICE_ADD);
    EXPECT_EQ(event.deviceType, PNP_DEVICE_UNKNOWN);
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
        EXPECT_EQ(result, HDF_ERR_INVALID_PARAM);
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
        EXPECT_EQ(result, HDF_ERR_INVALID_PARAM);
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
        EXPECT_EQ(result, HDF_ERR_INVALID_PARAM);
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
        EXPECT_EQ(result, HDF_ERR_INVALID_PARAM);
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
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, audioSocketThread.AudioAnahsDetectDevice(nullptr));

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
 * @tc.name : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_040
 * @tc.desc : Test AudioHDMIDetectDevice
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_040, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    // Test case 1: Invalid parameter (NULL audioPnpUevent)
    {
        int32_t result = audioSocketThread.AudioHDMIDetectDevice(nullptr);
        EXPECT_EQ(result, HDF_ERR_INVALID_PARAM);
    }
    // Test case 2: Invalid subSystem
    {
        AudioPnpUevent uevent = {
            .subSystem = "invalid",
            .switchName = "hdmi_mipi_audio",
            .action = "change",
            .switchState = "1"
        };
        int32_t result = audioSocketThread.AudioHDMIDetectDevice(&uevent);
        EXPECT_EQ(result, HDF_ERR_INVALID_PARAM);
    }
    // Test case 3: Invalid switchName
    {
        AudioPnpUevent uevent = {
            .subSystem = "switch",
            .switchName = "invalid",
            .action = "change",
            .switchState = "1"
        };
        int32_t result = audioSocketThread.AudioHDMIDetectDevice(&uevent);
        EXPECT_EQ(result, HDF_ERR_INVALID_PARAM);
    }
}

/**
 * @tc.name : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_041
 * @tc.desc : Test AudioHDMIDetectDevice
 */
 HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_041, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    // Test case 4: Invalid action
    {
        AudioPnpUevent uevent = {
            .subSystem = "switch",
            .switchName = "hdmi_mipi_audio",
            .action = "invalid",
            .switchState = "1"
        };
        int32_t result = audioSocketThread.AudioHDMIDetectDevice(&uevent);
        EXPECT_EQ(result, HDF_ERR_INVALID_PARAM);
    }
    // Test case 5: Device Add Event
    {
        AudioPnpUevent uevent = {
            .subSystem = "switch",
            .switchName = "hdmi_mipi_audio,device_port=HDMI-0",
            .action = "change",
            .switchState = "1"
        };
        int32_t result = audioSocketThread.AudioHDMIDetectDevice(&uevent);
        EXPECT_EQ(result, SUCCESS);
        // Additional checks can be added here to verify the internal state
    }
    // Test case 6: Device Remove Event
    {
        AudioPnpUevent uevent = {
            .subSystem = "switch",
            .switchName = "hdmi_mipi_audio,device_port=HDMI-0",
            .action = "change",
            .switchState = "0"
        };
        int32_t result = audioSocketThread.AudioHDMIDetectDevice(&uevent);
        EXPECT_EQ(result, SUCCESS);
        // Additional checks can be added here to verify the internal state
    }
}

/**
 * @tc.name : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_042
 * @tc.desc : Test AudioHDMIDetectDevice
 */
 HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_042, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    // Test case 7: Invalid switchState
    {
        AudioPnpUevent uevent = {
            .subSystem = "switch",
            .switchName = "hdmi_mipi_audio,device_port=HDMI-0",
            .action = "change",
            .switchState = "invalid"
        };
        int32_t result = audioSocketThread.AudioHDMIDetectDevice(&uevent);
        EXPECT_EQ(result, ERROR);
    }
    // Test case 8: No device_port in switchName
    {
        AudioPnpUevent uevent = {
            .subSystem = "switch",
            .switchName = "hdmi_mipi_audio,",
            .action = "change",
            .switchState = "1"
        };
        int32_t result = audioSocketThread.AudioHDMIDetectDevice(&uevent);
        EXPECT_EQ(result, SUCCESS);
    }
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
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(eventType, PNP_EVENT_DEVICE_ADD);
}

/**
 * @tc.name  : AudioNnDetectDevice_InvalidParam_Test
 * @tc.number: Audio_AudioSocketThread_AudioNnDetectDevice_001
 * @tc.desc  : Test AudioNnDetectDevice function with invalid parameters.
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioNnDetectDevice_InvalidParam_Test, TestSize.Level2)
{
    EXPECT_EQ(audioSocketThread_.AudioNnDetectDevice(NULL), -3);
    struct AudioPnpUevent audioPnpUevent;
    audioPnpUevent.action = "add";
    audioPnpUevent.name = "send_nn_state1";
    EXPECT_EQ(audioSocketThread_.AudioNnDetectDevice(&audioPnpUevent), -3);
    audioPnpUevent.action = "change";
    audioPnpUevent.name = "test_send_nn_state1";
    EXPECT_EQ(audioSocketThread_.AudioNnDetectDevice(&audioPnpUevent), -3);
}

/**
 * @tc.name  : AudioNnDetectDevice_ValidParam_Test
 * @tc.number: Audio_AudioSocketThread_AudioNnDetectDevice_002
 * @tc.desc  : Test AudioNnDetectDevice function with valid parameters.
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioNnDetectDevice_ValidParam_Test, TestSize.Level2)
{
    struct AudioPnpUevent audioPnpUevent;
    audioPnpUevent.action = "change";
    audioPnpUevent.name = "send_nn_state1";
    EXPECT_NE(audioSocketThread_.AudioNnDetectDevice(&audioPnpUevent), SUCCESS);
}

/**
 * @tc.name  : AudioNnDetectDevice_StateNotSupported_Test
 * @tc.number: Audio_AudioSocketThread_AudioNnDetectDevice_003
 * @tc.desc  : Test AudioNnDetectDevice function with state not supported.
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioNnDetectDevice_StateNotSupported_Test, TestSize.Level2)
{
    struct AudioPnpUevent audioPnpUevent;
    audioPnpUevent.action = "change";
    audioPnpUevent.name = "send_nn_state00";
    EXPECT_NE(audioSocketThread_.AudioNnDetectDevice(&audioPnpUevent), SUCCESS);
}

/**
 * @tc.name  : AudioNnDetectDevice_StateNnOff_Test
 * @tc.number: Audio_AudioSocketThread_AudioNnDetectDevice_004
 * @tc.desc  : Test AudioNnDetectDevice function with state nn off.
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioNnDetectDevice_StateNnOff_Test, TestSize.Level2)
{
    struct AudioPnpUevent audioPnpUevent;
    audioPnpUevent.action = "change";
    audioPnpUevent.name = "send_nn_state01";
    EXPECT_EQ(audioSocketThread_.AudioNnDetectDevice(&audioPnpUevent), STATE_NN_OFF);
}

/**
 * @tc.name  : AudioNnDetectDevice_StateNnOn_Test
 * @tc.number: Audio_AudioSocketThread_AudioNnDetectDevice_005
 * @tc.desc  : Test AudioNnDetectDevice function with state nn on.
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioNnDetectDevice_StateNnOn_Test, TestSize.Level2)
{
    struct AudioPnpUevent audioPnpUevent;
    audioPnpUevent.action = "change";
    audioPnpUevent.name = "send_nn_state02";
    EXPECT_NE(audioSocketThread_.AudioNnDetectDevice(&audioPnpUevent), SUCCESS);
}

/**
 * @tc.name  : AudioNnDetectDevice_InvalidState_Test
 * @tc.number: Audio_AudioSocketThread_AudioNnDetectDevice_006
 * @tc.desc  : Test AudioNnDetectDevice function with invalid state.
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioNnDetectDevice_InvalidState_Test, TestSize.Level2)
{
    struct AudioPnpUevent audioPnpUevent;
    audioPnpUevent.action = "change";
    audioPnpUevent.name = "send_nn_state04";
    EXPECT_EQ(audioSocketThread_.AudioNnDetectDevice(&audioPnpUevent), HDF_ERR_INVALID_PARAM);
}

/**
 * @tc.name  : AudioNnDetectDevice_SendNnStateChangeCallback_Failed_Test
 * @tc.number: Audio_AudioSocketThread_AudioNnDetectDevice_007
 * @tc.desc  : Test AudioNnDetectDevice function with failed SendNnStateChangeCallback.
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioNnDetectDevice_SendNnStateChangeCallback_Failed_Test, TestSize.Level2)
{
    struct AudioPnpUevent audioPnpUevent;
    audioPnpUevent.action = "change";
    audioPnpUevent.name = "send_nn_state03";
    EXPECT_EQ(audioSocketThread_.AudioNnDetectDevice(&audioPnpUevent), HDF_ERR_INVALID_PARAM);
}

/**
 * @tc.name  : AudioPnpUeventParse_Test_01
 * @tc.number: Audio_AudioPnpUeventParse_001
 * @tc.desc  : Test AudioPnpUeventParse function when msg starts with "libudev"
 */
HWTEST_F(AudioSocketThreadUnitTest, Audio_AudioPnpUeventParse_001, TestSize.Level0)
{
    const char *msg = "libudev";
    ssize_t strLength = strlen(msg);
    EXPECT_FALSE(audioSocketThread_.AudioPnpUeventParse(msg, strLength));
}

/**
 * @tc.name  : AudioPnpUeventParse_Test_02
 * @tc.number: Audio_AudioPnpUeventParse_002
 * @tc.desc  : Test AudioPnpUeventParse function when strLength > UEVENT_MSG_LEN + 1
 */
HWTEST_F(AudioSocketThreadUnitTest, Audio_AudioPnpUeventParse_002, TestSize.Level0)
{
    const char *msg = "test message";
    ssize_t strLength = UEVENT_MSG_LEN + 2;
    EXPECT_FALSE(audioSocketThread_.AudioPnpUeventParse(msg, strLength));
}

/**
 * @tc.name  : AudioPnpUeventParse_Test_03
 * @tc.number: Audio_AudioPnpUeventParse_003
 * @tc.desc  : Test AudioPnpUeventParse function when no matching UEVENT_ARR_SIZE is found
 */
HWTEST_F(AudioSocketThreadUnitTest, Audio_AudioPnpUeventParse_003, TestSize.Level0)
{
    const char *msg = "unmatched event";
    ssize_t strLength = strlen(msg);
    EXPECT_FALSE(audioSocketThread_.AudioPnpUeventParse(msg, strLength));
}

/**
 * @tc.name  : AudioPnpUeventParse_Test_04
 * @tc.number: Audio_AudioPnpUeventParse_004
 * @tc.desc  : Test AudioPnpUeventParse function when all detect devices return SUCCESS
 */
HWTEST_F(AudioSocketThreadUnitTest, Audio_AudioPnpUeventParse_004, TestSize.Level0)
{
    const char *msg = "matched event";
    ssize_t strLength = strlen(msg);
    EXPECT_FALSE(audioSocketThread_.AudioPnpUeventParse(msg, strLength));
}

/**
 * @tc.name  : DetectAnalogHeadsetState_Fail_OpenFile
 * @tc.number: Audio_AudioSocketThread_DetectAnalogHeadsetState_001
 * @tc.desc  : Test DetectAnalogHeadsetState function when open file fail.
 */
HWTEST_F(AudioSocketThreadUnitTest, DetectAnalogHeadsetState_Fail_OpenFile, TestSize.Level0)
{
    std::ofstream ofs;
    ofs.open(SWITCH_STATE_PATH, std::ofstream::out);
    ofs.close();
    unlink(SWITCH_STATE_PATH);
    AudioEvent audioEvent;
    int32_t ret = audioSocketThread_.DetectAnalogHeadsetState(&audioEvent);
    EXPECT_EQ(ret, HDF_ERR_INVALID_PARAM);
}

/**
 * @tc.name  : DetectAnalogHeadsetState_Fail_ReadFile
 * @tc.number: Audio_AudioSocketThread_DetectAnalogHeadsetState_002
 * @tc.desc  : Test DetectAnalogHeadsetState function when read file fail.
 */
HWTEST_F(AudioSocketThreadUnitTest, DetectAnalogHeadsetState_Fail_ReadFile, TestSize.Level0)
{
    std::ofstream ofs(SWITCH_STATE_PATH);
    ofs.close();
    AudioEvent audioEvent;
    int32_t ret = audioSocketThread_.DetectAnalogHeadsetState(&audioEvent);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : DetectAnalogHeadsetState_Headset_Remove_003
 * @tc.number: Audio_AudioSocketThread_DetectAnalogHeadsetState_003
 * @tc.desc  : Test DetectAnalogHeadsetState function when headset is removed.
 */
HWTEST_F(AudioSocketThreadUnitTest, DetectAnalogHeadsetState_Headset_Remove_003, TestSize.Level0)
{
    std::ofstream ofs(SWITCH_STATE_PATH);
    ofs << '0';
    ofs.close();
    AudioEvent audioEvent;
    int32_t hdf_err_invalid_param = -3;
    int32_t ret = audioSocketThread_.DetectAnalogHeadsetState(&audioEvent);
    EXPECT_EQ(ret, hdf_err_invalid_param);
}

/**
 * @tc.name  : DetectAnalogHeadsetState_Headset_Add_004
 * @tc.number: Audio_AudioSocketThread_DetectAnalogHeadsetState_004
 * @tc.desc  : Test DetectAnalogHeadsetState function when headset is added.
 */
HWTEST_F(AudioSocketThreadUnitTest, DetectAnalogHeadsetState_Headset_Add_004, TestSize.Level0)
{
    std::ofstream ofs(SWITCH_STATE_PATH);
    ofs << '1';
    ofs.close();
    AudioEvent audioEvent;
    int32_t ret = audioSocketThread_.DetectAnalogHeadsetState(&audioEvent);
    int32_t hdf_err_invalid_param = -3;
    EXPECT_EQ(ret, hdf_err_invalid_param);
}

/**
 * @tc.name  : ReadAndScanDpName_Success_WhenFileExists
 * @tc.number: Audio_AudioSocketThread_ReadAndScanDpName_001
 * @tc.desc  : Test ReadAndScanDpName function when file exists and can be read successfully.
 */
HWTEST_F(AudioSocketThreadUnitTest, ReadAndScanDpName_Success_WhenFileExists, TestSize.Level0)
{
    std::string testPath = "/tmp/test_path";
    std::string testName = "test_name";
    std::ofstream file(testPath);
    file << testName;
    file.close();
    std::string name;
    int32_t ret = audioSocketThread_.ReadAndScanDpName(testPath, name);
    EXPECT_EQ(ret, ERROR);
    remove(testPath.c_str());
}

/**
 * @tc.name  : ReadAndScanDpName_Fail_WhenFileNotExists
 * @tc.number: Audio_AudioSocketThread_ReadAndScanDpName_002
 * @tc.desc  : Test ReadAndScanDpName function when file does not exist.
 */
HWTEST_F(AudioSocketThreadUnitTest, ReadAndScanDpName_Fail_WhenFileNotExists, TestSize.Level0)
{
    std::string testPath = "/tmp/test_path";
    std::string testName = "test_name";
    std::ofstream file(testPath);
    file << testName;
    file.close();
    std::string name;
    int32_t ret = audioSocketThread_.ReadAndScanDpName("/invalid/path", name);
    EXPECT_EQ(ret, HDF_ERR_INVALID_PARAM);
    remove(testPath.c_str());
}

/**
 * @tc.name  : ReadAndScanDpName_Fail_WhenFileReadFails
 * @tc.number: Audio_AudioSocketThread_ReadAndScanDpName_003
 * @tc.desc  : Test ReadAndScanDpName function when file read fails.
 */
HWTEST_F(AudioSocketThreadUnitTest, ReadAndScanDpName_Fail_WhenFileReadFails, TestSize.Level0)
{
    std::string testPath = "/tmp/test_path";
    std::string testName = "test_name";
    std::ofstream file(testPath);
    file << testName;
    file.close();
    std::string name;
    int32_t ret = audioSocketThread_.ReadAndScanDpName("/dev/null", name);
    EXPECT_EQ(ret, ERROR);
    remove(testPath.c_str());
}

/**
 * @tc.name  : ReadAndScanDpName_Fail_WhenDevicePortNotFound
 * @tc.number: Audio_AudioSocketThread_ReadAndScanDpName_004
 * @tc.desc  : Test ReadAndScanDpName function when device port not found in file content.
 */
HWTEST_F(AudioSocketThreadUnitTest, ReadAndScanDpName_Fail_WhenDevicePortNotFound, TestSize.Level0)
{
    std::string testName = "invalid_name";
    std::string testPath = "/tmp/test_path";
    std::ofstream file(testPath);
    file << testName;
    file.close();

    std::string name;
    int32_t ret = audioSocketThread_.ReadAndScanDpName(testPath, name);
    EXPECT_NE(ret, SUCCESS);
    remove(testPath.c_str());
}
} // namespace AudioStandard
} // namespace OHOS
