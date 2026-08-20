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
#include "securec.h"
using namespace std;
using namespace std::chrono;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    const int32_t HDF_ERR_INVALID_PARAM = -3;
    const int32_t HDF_FAILURE = -1;

    std::vector<char> BuildUeventMsg(const std::vector<const char *> &segs)
    {
        std::vector<char> msg;
        for (const char *s : segs) {
            msg.insert(msg.end(), s, s + strlen(s) + 1); // include trailing '\0'
        }
        return msg;
    }
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

    //Test events where both eventType and deviceType are different
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

    //Test events with different eventType, deviceType, and name
    AudioEvent event2;
    event2.eventType = 2;
    event2.deviceType = 2;
    event2.name = "Device2";
    event2.address = "Address1";
    event2.anahsName = "AnahsName1";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event2), true);

    //Test events with different eventType, name, and address
    AudioEvent event3;
    event3.eventType = 2;
    event3.deviceType = 2;
    event3.name = "Device2";
    event3.address = "Address1";
    event3.anahsName = "AnahsName1";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event3), true);

    //Test for events where name, address, and anahsName are different
    AudioEvent event4;
    event4.eventType = 2;
    event4.deviceType = 2;
    event4.name = "Device2";
    event4.address = "Address1";
    event4.anahsName = "AnahsName1";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event4), true);

    // Test foraddress,anahsName and deviceType are different
    AudioEvent event5;
    event5.eventType = 2;
    event5.deviceType = 1;
    event5.name = "Device1";
    event5.address = "Address2";
    event5.anahsName = "AnahsName2";
    EXPECT_EQ(audioSocketThread_.IsUpdatePnpDeviceState(&event5), true);

    //Test for events where address, anahsName, and eventType are different
    AudioEvent event6;
    event6.eventType = 2;
    event6.deviceType = 2;
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

    //Test events that are different for eventType, deviceType, name, and address
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
* @tc.desc  : Test SetAudioPnpUevent with non-switch subsystem
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
    EXPECT_EQ(event.deviceType, PNP_DEVICE_HEADPHONE);
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
 * @tc.name  : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_017
 * @tc.desc  : Test SetAudioPnpUevent REMOVE by ANALOG_HS0 in non-switch subsystem
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_017, TestSize.Level4)
{
    AudioPnpUevent initUevent = {
        .action = "change",
        .name = "headset",
        .state = "MICROPHONE=0",
        .devType = "extcon3",
        .subSystem = UEVENT_SUBSYSTEM_SWITCH,
        .switchName = "h2w",
        .switchState = "2",
        .hidName = "hid",
        .devName = "TestDevName0",
        .anahsName = "anahs"
    };
    AudioEvent event;
    int32_t initResult = AudioSocketThread::SetAudioPnpServerEventValue(&event, &initUevent);

    AudioPnpUevent uevent = {
        .action = "change",
        .name = "headset",
        .state = "MICROPHONE=0",
        .devType = "extcon3",
        .subSystem = UEVENT_SUBSYSTEM_SWITCH,
        .switchName = "h2w",
        .switchState = "0",
        .hidName = "hid",
        .devName = "TestDevName0",
        .anahsName = "anahs"
    };

    int32_t result = AudioSocketThread::SetAudioPnpServerEventValue(&event, &uevent);

    EXPECT_EQ(event.eventType, PNP_EVENT_DEVICE_REMOVE);
    EXPECT_EQ(event.deviceType, PNP_DEVICE_HEADPHONE);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_018
 * @tc.desc  : Test SetAudioPnpUevent ADD by ANALOG_HS1 in non-switch subsystem
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_018, TestSize.Level4)
{
    AudioPnpUevent uevent = {
        .action = "change",
        .name = "headset",
        .state = "MICROPHONE=1",
        .devType = "extcon3",
        .subSystem = UEVENT_SUBSYSTEM_SWITCH,
        .switchName = "h2w",
        .switchState = "1",
        .hidName = "hid",
        .devName = "TestDevName1",
        .anahsName = "anahs"
    };

    AudioEvent event;
    int32_t result = AudioSocketThread::SetAudioPnpServerEventValue(&event, &uevent);

    EXPECT_EQ(event.eventType, PNP_EVENT_DEVICE_ADD);
    EXPECT_EQ(event.deviceType, PNP_DEVICE_HEADSET);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_019
 * @tc.desc  : Test SetAudioPnpUevent with unsupported ANALOG_HSx state
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_019, TestSize.Level4)
{
    AudioPnpUevent uevent = {
        .action = "change",
        .name = "headset",
        .state = "unsupported",
        .devType = "extcon3",
        .subSystem = "audio",
        .switchName = "h2w",
        .switchState = "1",
        .hidName = "hid",
        .devName = "TestDevNameInvalid",
        .anahsName = "anahs"
    };

    AudioEvent event;
    int32_t result = AudioSocketThread::SetAudioPnpServerEventValue(&event, &uevent);

    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_020
 * @tc.desc  : Test SetAudioPnpUevent with switchState = '0' (REMOVE_AUDIO_DEVICE)
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_020, TestSize.Level4)
{
    AudioPnpUevent uevent = {
        .action = "remove",
        .name = "TestDevice_Remove",
        .state = "removed",
        .devType = "headset",
        .subSystem = "switch",
        .switchName = "h2w",
        .switchState = "0",
        .hidName = "hid",
        .devName = "TestDevName_Remove",
        .anahsName = "anahs"
    };

    AudioEvent event;
    int32_t result = AudioSocketThread::SetAudioPnpServerEventValue(&event, &uevent);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(event.name, "TestDevice_Remove");
    EXPECT_EQ(event.address, "TestDevName_Remove");
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
 * @tc.name  : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_032
 * @tc.desc  : Test AudioDpDetectDevice when same device state is reported again
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_032, TestSize.Level4)
{
    AudioSocketThread audioSocketThread;

    AudioPnpUevent uevent = {
        .subSystem = "switch",
        .switchName = "hdmi_audio1device_port=HDMI-0",
        .action = "change",
        .switchState = "1"};

    auto ret = audioSocketThread.AudioDpDetectDevice(&uevent);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(audioSocketThread.AudioDpDetectDevice(&uevent), SUCCESS);
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
    audioSocketThread.UpdateDeviceState(noUpdateEvent);
    //Test: Successful update
    AudioEvent successUpdateEvent = {1, 2, "device", "address", "anahs"};
    audioSocketThread.UpdateDeviceState(successUpdateEvent);
    //Test：snprintf_s failed
    AudioEvent snprintfFailEvent = {1, 2, "device", "address", "anahs"};
    audioSocketThread.UpdateDeviceState(snprintfFailEvent);
    AudioEvent snprintfSuccessEvent = {1, 2, "device", "address", "anahs"};
    audioSocketThread.UpdateDeviceState(snprintfSuccessEvent);
    EXPECT_TRUE(AudioSocketThread::audioSocketEvent_.eventType == 1);
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
 * @tc.name  : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_043
 * @tc.desc  : Test AudioAnalogHeadsetDetectDevice when device state is the same, should early return.
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_043, TestSize.Level4)
{
    AudioSocketThread audioSocketThread;

    AudioPnpUevent ueventInit = {
        .action = "add",
        .name = "SameDevice",
        .state = "added",
        .devType = "headset",
        .subSystem = "switch",
        .switchName = "h2w",
        .switchState = "1",
        .hidName = "hid",
        .devName = "SameDevAddress",
        .anahsName = "SameAnahs"
    };

    auto ret = audioSocketThread.AudioAnalogHeadsetDetectDevice(&ueventInit);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(audioSocketThread.AudioAnalogHeadsetDetectDevice(&ueventInit), SUCCESS);
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
    EXPECT_EQ(ret, SUCCESS);
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
 * @tc.name  : ReadAndScanDpState_Failure_WhenFileContainsInvalidChar
 * @tc.number: Audio_AudioSocketThread_ReadAndScanDpState_002
 * @tc.desc  : Test ReadAndScanDpState function when file contains invalid character (not '1' or '0')
 */
HWTEST_F(AudioSocketThreadUnitTest, ReadAndScanDpState_Failure_WhenFileContainsInvalidChar, TestSize.Level4)
{
    std::string testPath = "/tmp/test_path_invalid";

    std::ofstream file(testPath);
    file << 'x';
    file.close();

    uint32_t eventType = 0;
    int32_t ret = audioSocketThread_.ReadAndScanDpState(testPath, eventType);

    EXPECT_EQ(ret, ERROR);
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
 * @tc.name  : AudioPnpUeventParse_Test_05
 * @tc.number: Audio_AudioPnpUeventParse_005
 * @tc.desc  : Cover branch where *msgTmp == '\0' to test skip behavior
 */
HWTEST_F(AudioSocketThreadUnitTest, Audio_AudioPnpUeventParse_005, TestSize.Level4)
{
    AudioSocketThread audioSocketThread;

    std::vector<char> msgVec;
    const char *seg1 = "ACTION=change";
    const char *seg2 = "";
    const char *seg3 = "NAME=mic_blocked";
    msgVec.insert(msgVec.end(), seg1, seg1 + strlen(seg1) + 1);
    msgVec.insert(msgVec.end(), seg2, seg2 + strlen(seg2) + 1);
    msgVec.insert(msgVec.end(), seg3, seg3 + strlen(seg3) + 1);

    ssize_t strLength = msgVec.size();
    const char *msg = msgVec.data();

    EXPECT_EQ(audioSocketThread.AudioPnpUeventParse(msg, strLength), true);
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
    AudioEvent audioEvent;
    audioSocketThread_.SetAudioPnpUevent(&audioEvent, REMOVE_AUDIO_DEVICE);
    EXPECT_EQ(audioEvent.eventType, PNP_EVENT_DEVICE_REMOVE);
}

/**
 * @tc.name  : DetectAnalogHeadsetState_Headset_Add_004
 * @tc.number: Audio_AudioSocketThread_DetectAnalogHeadsetState_004
 * @tc.desc  : Test DetectAnalogHeadsetState function when headset is added.
 */
HWTEST_F(AudioSocketThreadUnitTest, DetectAnalogHeadsetState_Headset_Add_004, TestSize.Level0)
{
    AudioEvent audioEvent;
    audioSocketThread_.SetAudioPnpUevent(&audioEvent, ADD_DEVICE_HEADSET);
    EXPECT_EQ(audioEvent.eventType, PNP_EVENT_DEVICE_ADD);
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

/**
 * @tc.name  : ReadAndScanDpName_Success_WhenDevicePortFound
 * @tc.number: Audio_AudioSocketThread_ReadAndScanDpName_005
 * @tc.desc  : Test ReadAndScanDpName function when device port string exists and ends with '\n'.
 */
HWTEST_F(AudioSocketThreadUnitTest, ReadAndScanDpName_Success_WhenDevicePortFound, TestSize.Level4)
{
    std::string testPath = "/tmp/test_path";
    std::string testContent = "some_prefix" + std::string(DEVICE_PORT) + "dp-name\n";
    std::ofstream file(testPath);
    file << testContent;
    file.close();

    std::string name;
    int32_t ret = audioSocketThread_.ReadAndScanDpName(testPath, name);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(name, "dp-name");
    remove(testPath.c_str());
}

/**
 * @tc.name  : Test AudioSocketThread
 * @tc.number: AudioSocketThread_101
 * @tc.desc  : Test AudioSocketThread
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_101, TestSize.Level0)
{
    auto audioSocketThread = std::make_shared<AudioSocketThread>();
    EXPECT_NE(audioSocketThread, nullptr);

    struct AudioPnpUevent *audioPnpUevent = nullptr;
    auto ret = audioSocketThread->AudioMicBlockDevice(audioPnpUevent);
    EXPECT_EQ(ret, HDF_ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test AudioSocketThread
 * @tc.number: AudioSocketThread_102
 * @tc.desc  : Test AudioSocketThread
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_102, TestSize.Level0)
{
    auto audioSocketThread = std::make_shared<AudioSocketThread>();
    EXPECT_NE(audioSocketThread, nullptr);

    struct AudioPnpUevent *audioPnpUevent = new AudioPnpUevent();
    EXPECT_NE(audioPnpUevent, nullptr);
    audioPnpUevent->name = "mic_blocked";
    auto ret = audioSocketThread->AudioMicBlockDevice(audioPnpUevent);
    EXPECT_EQ(ret, SUCCESS);

    if (audioPnpUevent != nullptr) {
        delete audioPnpUevent;
        audioPnpUevent = nullptr;
    }
}

/**
 * @tc.name  : Test AudioSocketThread
 * @tc.number: AudioSocketThread_103
 * @tc.desc  : Test AudioMicBlockDevice with "mic_un_blocked"
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_103, TestSize.Level4)
{
    auto audioSocketThread = std::make_shared<AudioSocketThread>();
    EXPECT_NE(audioSocketThread, nullptr);

    struct AudioPnpUevent *audioPnpUevent = new AudioPnpUevent();
    EXPECT_NE(audioPnpUevent, nullptr);
    audioPnpUevent->name = "mic_un_blocked";
    int32_t ret = audioSocketThread->AudioMicBlockDevice(audioPnpUevent);
    EXPECT_EQ(ret, SUCCESS);
    delete audioPnpUevent;
}

/**
 * @tc.name  : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_104
 * @tc.desc  : Test IsUpdatePnpDeviceState with same event values
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_104, TestSize.Level4)
{
    AudioEvent event1;
    event1.eventType = AUDIO_EVENT_UNKNOWN;
    event1.deviceType = AUDIO_DEVICE_UNKNOWN;
    event1.name = "test";
    event1.address = "addr";
    event1.anahsName = "anahs";
    AudioSocketThread::audioSocketEvent_ = event1;
    bool result = AudioSocketThread::IsUpdatePnpDeviceState(&event1);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_105
 * @tc.desc  : Test IsUpdatePnpDeviceState with different action
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_105, TestSize.Level4)
{
    AudioEvent event1;
    event1.eventType = AUDIO_EVENT_UNKNOWN;
    event1.deviceType = AUDIO_DEVICE_UNKNOWN;
    event1.name = "test";
    event1.address = "addr";
    event1.anahsName = "anahs";
    AudioSocketThread::audioSocketEvent_ = event1;
    AudioEvent event2;
    event2.eventType = PNP_EVENT_DEVICE_ADD;
    event2.deviceType = AUDIO_DEVICE_UNKNOWN;
    event2.name = "test2";
    event2.address = "addr2";
    event2.anahsName = "anahs2";
    bool result = AudioSocketThread::IsUpdatePnpDeviceState(&event2);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_106
 * @tc.desc  : Test UpdateDeviceState with invalid action event type
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_106, TestSize.Level4)
{
    AudioEvent audioEvent;
    audioEvent.eventType = AUDIO_EVENT_UNKNOWN;
    audioEvent.deviceType = AUDIO_DEVICE_UNKNOWN;
    AudioSocketThread::UpdateDeviceState(audioEvent);
    EXPECT_TRUE(AudioSocketThread::audioSocketEvent_.deviceType != AUDIO_HEARING_AID_DEVICE);
}

/**
 * @tc.name  : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_107
 * @tc.desc  : Test AudioPnpUeventParse with valid message
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_107, TestSize.Level4)
{
    std::string message;
    message += "ACTION=add";
    message += '\0';
    message += "SUBSYSTEM=switch";
    message += '\0';
    message += "SWITCH_NAME=h2w";
    message += '\0';
    message += "SWITCH_STATE=1";
    message += '\0';
    message += "NAME=headset";
    bool result = AudioSocketThread::AudioPnpUeventParse(message.c_str(), message.length());
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_108
 * @tc.desc  : Test AudioPnpUeventParse with empty message
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_108, TestSize.Level4)
{
    std::string message = "";
    bool result = AudioSocketThread::AudioPnpUeventParse(message.c_str(), message.length());
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test AudioSocketThread.
 * @tc.number: AudioSocketThread_109
 * @tc.desc  : Test AudioMicBlockDevice with mic_blocked
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_109, TestSize.Level4)
{
    auto audioSocketThread = std::make_shared<AudioSocketThread>();
    EXPECT_NE(audioSocketThread, nullptr);
    struct AudioPnpUevent *audioPnpUevent = new AudioPnpUevent();
    EXPECT_NE(audioPnpUevent, nullptr);
    audioPnpUevent->name = "mic_blocked";
    int32_t ret = audioSocketThread->AudioMicBlockDevice(audioPnpUevent);
    EXPECT_EQ(ret, SUCCESS);
    delete audioPnpUevent;
}

/**
 * @tc.name  : Test SetAudioPnpUevent_UnknownSwitchState
 * @tc.number: AudioSocketThread_110
 * @tc.desc  : Test SetAudioPnpUevent with switchState '3' (not 0/1/2/4), should hit default case
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_110, TestSize.Level1)
{
    AudioPnpUevent uevent = {
        .action = "change",
        .name = "TestDevice",
        .state = "unknown",
        .devType = "extcon",
        .subSystem = "switch",
        .switchName = "h2w",
        .switchState = "3",  // Invalid - not 0/1/2/4
        .hidName = "hid",
        .devName = "TestDevName",
        .anahsName = "anahs"
    };

    AudioEvent event;
    int32_t result = AudioSocketThread::SetAudioPnpServerEventValue(&event, &uevent);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(event.eventType, PNP_EVENT_DEVICE_ADD);
    EXPECT_EQ(event.deviceType, PNP_DEVICE_UNKNOWN);
}

/**
 * @tc.name  : Test ProcessSwitchDeviceConfig_UnknownSwitchName
 * @tc.number: AudioSocketThread_111
 * @tc.desc  : Test ProcessSwitchDeviceConfig with unknown switchName, should hit else branch
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_111, TestSize.Level1)
{
    AudioPnpUevent uevent = {
        .action = "change",
        .name = "TestDevice",
        .state = "added",
        .devType = "extcon",
        .subSystem = "switch",
        .switchName = "unknown_switch",  // Not in SWITCH_DEVICE_CONFIGS
        .switchState = "1",
        .hidName = "hid",
        .devName = "TestDevName",
        .anahsName = "anahs"
    };

    AudioEvent event;
    int32_t result = AudioSocketThread::SetAudioPnpServerEventValue(&event, &uevent);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : Test ProcessSwitchDeviceConfig_InvalidSwitchState
 * @tc.number: AudioSocketThread_112
 * @tc.desc  : Test ProcessSwitchDeviceConfig with switchState '9', should hit invalid branch
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_112, TestSize.Level1)
{
    AudioPnpUevent uevent = {
        .action = "change",
        .name = "TestDevice",
        .state = "added",
        .devType = "extcon",
        .subSystem = "switch",
        .switchName = UEVENT_NAME_H2W_MICIN,
        .switchState = "9",  // Invalid switchState
        .hidName = "hid",
        .devName = "TestDevName",
        .anahsName = "anahs"
    };

    AudioEvent event;
    int32_t result = AudioSocketThread::SetAudioPnpServerEventValue(&event, &uevent);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : Test AudioAnahsDetectDevice_SetAudioAnahsEventValueError
 * @tc.number: AudioSocketThread_113
 * @tc.desc  : Test AudioAnahsDetectDevice when SetAudioAnahsEventValue returns ERROR
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_113, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    struct AudioPnpUevent audioPnpUevent = {
        .subSystem = "invalid_subsystem",  // Not "platform"
        .anahsName = "INVALID"
    };
    int32_t ret = audioSocketThread.AudioAnahsDetectDevice(&audioPnpUevent);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test AudioDpDetectDevice_InvalidSwitchState
 * @tc.number: AudioSocketThread_116
 * @tc.desc  : Test AudioDpDetectDevice with switchState not '0' or '1'
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_116, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    AudioPnpUevent uevent = {
        .subSystem = "switch",
        .switchName = "hdmi_audio1device_port=DP-0",
        .action = "change",
        .switchState = "2"  // Invalid - not '0' or '1'
    };
    int32_t result = audioSocketThread.AudioDpDetectDevice(&uevent);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : Test AudioDpDetectDevice_IsUpdatePnpDeviceStateFalse
 * @tc.number: AudioSocketThread_117
 * @tc.desc  : Test AudioDpDetectDevice when IsUpdatePnpDeviceState returns false (same state)
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_117, TestSize.Level4)
{
    AudioSocketThread audioSocketThread;
    // First call to set the state
    AudioPnpUevent uevent1 = {
        .subSystem = "switch",
        .switchName = "hdmi_audio1device_port=DP-0",
        .action = "change",
        .switchState = "1"
    };
    int32_t ret1 = audioSocketThread.AudioDpDetectDevice(&uevent1);
    EXPECT_EQ(ret1, SUCCESS);

    // Second call with same state - should return SUCCESS but not update
    AudioPnpUevent uevent2 = {
        .subSystem = "switch",
        .switchName = "hdmi_audio1device_port=DP-0",
        .action = "change",
        .switchState = "1"
    };
    int32_t ret2 = audioSocketThread.AudioDpDetectDevice(&uevent2);
    EXPECT_EQ(ret2, SUCCESS);
}

/**
 * @tc.name  : Test AudioHDMIDetectDevice_InvalidSwitchState
 * @tc.number: AudioSocketThread_118
 * @tc.desc  : Test AudioHDMIDetectDevice with switchState not '0' or '1'
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_118, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    AudioPnpUevent uevent = {
        .subSystem = "switch",
        .switchName = "hdmi_mipi_audio,device_port=HDMI-0",
        .action = "change",
        .state = "INVALID",  // non-null, neither HDMI=1 nor HDMI=0
        .switchState = "X"  // Invalid - neither '0' nor '1'
    };
    int32_t result = audioSocketThread.AudioHDMIDetectDevice(&uevent);
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name  : Test AudioHDMIDetectDevice_IsUpdatePnpDeviceStateFalse
 * @tc.number: AudioSocketThread_119
 * @tc.desc  : Test AudioHDMIDetectDevice when IsUpdatePnpDeviceState returns false
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_119, TestSize.Level4)
{
    AudioSocketThread audioSocketThread;
    // First call to set the state
    AudioPnpUevent uevent1 = {
        .subSystem = "switch",
        .switchName = "hdmi_mipi_audio,device_port=HDMI-0",
        .action = "change",
        .state = "HDMI=1",
        .switchState = "1"
    };
    int32_t ret1 = audioSocketThread.AudioHDMIDetectDevice(&uevent1);
    EXPECT_EQ(ret1, SUCCESS);

    // Second call with same state - should return SUCCESS but not update
    AudioPnpUevent uevent2 = {
        .subSystem = "switch",
        .switchName = "hdmi_mipi_audio,device_port=HDMI-0",
        .action = "change",
        .state = "HDMI=1",
        .switchState = "1"
    };
    int32_t ret2 = audioSocketThread.AudioHDMIDetectDevice(&uevent2);
    EXPECT_EQ(ret2, SUCCESS);
}

/**
 * @tc.name  : Test AudioMicBlockDevice_UnknownName
 * @tc.number: AudioSocketThread_120
 * @tc.desc  : Test AudioMicBlockDevice with name not "mic_blocked" or "mic_un_blocked"
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_120, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    struct AudioPnpUevent audioPnpUevent;
    audioPnpUevent.name = "unknown_name";  // Not "mic_blocked" or "mic_un_blocked"
    int32_t ret = audioSocketThread.AudioMicBlockDevice(&audioPnpUevent);
    EXPECT_EQ(ret, HDF_ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test ProcessOtherSubsystem_AllDetectReturnError
 * @tc.number: AudioSocketThread_121
 * @tc.desc  : Test ProcessOtherSubsystem when all DetectDevice functions return ERROR
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_121, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    // A message whose keys do not match any UEVENT_* prefix; all parsed fields stay
    // empty, so ProcessSwitchSubsystem and ProcessOtherSubsystem both return false.
    const char *msg = "UNKNOWN_KEY=unknown_value";
    ssize_t strLength = strlen(msg);
    bool result = audioSocketThread.AudioPnpUeventParse(msg, strLength);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test AudioPnpUeventParse_LibudevPrefix
 * @tc.number: AudioSocketThread_122
 * @tc.desc  : Test AudioPnpUeventParse with message starting with "libudev"
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_122, TestSize.Level1)
{
    const char *msg = "libudevabc123";
    ssize_t strLength = strlen(msg);
    bool result = audioSocketThread_.AudioPnpUeventParse(msg, strLength);
    EXPECT_FALSE(result);  // Should return false for libudev prefix
}

/**
 * @tc.name  : Test UpdateDeviceState_IsUpdatePnpDeviceStateFalse
 * @tc.number: AudioSocketThread_123
 * @tc.desc  : Test UpdateDeviceState when IsUpdatePnpDeviceState returns false
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_123, TestSize.Level1)
{
    // First set a state
    AudioEvent event1 = {1, 1, "device", "address", "anahs"};
    audioSocketThread_.UpdatePnpDeviceState(&event1);

    // Then try to update with the same state - should early return
    AudioEvent event2 = {1, 1, "device", "address", "anahs"};
    audioSocketThread_.UpdateDeviceState(event2);
    // The function should return early without updating
    EXPECT_EQ(AudioSocketThread::audioSocketEvent_.eventType, 1);
}

/**
 * @tc.name  : Test ReadAndScanDpState_InvalidStateChar
 * @tc.number: AudioSocketThread_124
 * @tc.desc  : Test ReadAndScanDpState with invalid state char (not '0' or '1')
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_124, TestSize.Level1)
{
    std::string testPath = "/tmp/test_dp_state";
    std::ofstream file(testPath);
    file << '3';  // Invalid state
    file.close();

    uint32_t eventType = 0;
    int32_t ret = audioSocketThread_.ReadAndScanDpState(testPath, eventType);
    EXPECT_NE(ret, SUCCESS);
    remove(testPath.c_str());
}

/**
 * @tc.name  : Test ReadAndScanDpName_PortNotFound
 * @tc.number: AudioSocketThread_125
 * @tc.desc  : Test ReadAndScanDpName when device_port is not in the content
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_125, TestSize.Level1)
{
    std::string testPath = "/tmp/test_dp_name";
    std::ofstream file(testPath);
    file << "some_content_without_device_port";  // No device_port found
    file.close();

    std::string name;
    int32_t ret = audioSocketThread_.ReadAndScanDpName(testPath, name);
    EXPECT_NE(ret, SUCCESS);
    remove(testPath.c_str());
}

/**
 * @tc.name  : Test ReadAndScanDpName_EmptyAddress
 * @tc.number: AudioSocketThread_126
 * @tc.desc  : Test ReadAndScanDpName parses device_port name successfully
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_126, TestSize.Level1)
{
    std::string testPath = "/tmp/test_dp_name_empty";
    std::ofstream file(testPath);
    file << "device_port=HDMI-0";  // device_port present, name parsed as HDMI-0
    file.close();

    std::string name;
    int32_t ret = audioSocketThread_.ReadAndScanDpName(testPath, name);
    EXPECT_NE(name, "HDMI-1");
    remove(testPath.c_str());
}

/**
 * @tc.name  : Test DetectSwitchDeviceState_OpenFileFail
 * @tc.number: AudioSocketThread_127
 * @tc.desc  : Test DetectSwitchDeviceState when fopen fails
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_127, TestSize.Level1)
{
    AudioEvent audioEvent;
    // Using a path that doesn't exist
    int32_t ret = audioSocketThread_.DetectSwitchDeviceState(
        "/nonexistent/path/state", "h2w", &audioEvent, "TestPrefix");
    EXPECT_EQ(ret, HDF_ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test DetectSwitchDeviceState_PathNotAbsolute
 * @tc.number: AudioSocketThread_128
 * @tc.desc  : Test DetectSwitchDeviceState when path is not absolute
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_128, TestSize.Level1)
{
    AudioEvent audioEvent;
    int32_t ret = audioSocketThread_.DetectSwitchDeviceState(
        "relative/path", "h2w", &audioEvent, "TestPrefix");
    EXPECT_EQ(ret, HDF_ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test DetectSwitchDeviceState_ReadFail
 * @tc.number: AudioSocketThread_129
 * @tc.desc  : Test DetectSwitchDeviceState when fread returns 0
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_129, TestSize.Level1)
{
    std::string testPath = "/tmp/test_switch_state_empty";
    std::ofstream file(testPath);
    file.close();  // Empty file

    AudioEvent audioEvent;
    int32_t ret = audioSocketThread_.DetectSwitchDeviceState(
        testPath.c_str(), "h2w", &audioEvent, "TestPrefix");
    EXPECT_NE(ret, SUCCESS);
    remove(testPath.c_str());
}

/**
 * @tc.name  : Test DetectDPState_Port0StateFail
 * @tc.number: AudioSocketThread_130
 * @tc.desc  : Test DetectDPState when port 0 state read fails
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_130, TestSize.Level1)
{
    AudioEvent audioEvent;
    audioEvent.eventType = PNP_EVENT_DEVICE_ADD;
    audioEvent.name = "testName";
    int32_t result = audioSocketThread_.DetectDPState(&audioEvent);
    // Expected to fail because /sys/class/switch/hdmi_audio/state doesn't exist
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name  : Test SetAudioPnpServerEventValue_NullSwitchName
 * @tc.number: AudioSocketThread_131
 * @tc.desc  : Test SetAudioPnpServerEventValue with null switchName
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_131, TestSize.Level1)
{
    AudioPnpUevent uevent = {
        .action = "change",
        .name = "headset",
        .state = "analog_hs1",
        .devType = "extcon",
        .subSystem = "switch",
        .switchName = nullptr,  // NULL switchName
        .switchState = "1",
        .hidName = "hid",
        .devName = "dev_name",
        .anahsName = "anahs_name"
    };
    AudioEvent audioEvent;
    EXPECT_EQ(audioSocketThread_.SetAudioPnpServerEventValue(&audioEvent, &uevent), ERROR);
}

/**
 * @tc.name  : Test SetAudioPnpServerEventValue_EmptySwitchState
 * @tc.number: AudioSocketThread_132
 * @tc.desc  : Test SetAudioPnpServerEventValue with empty switchState
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_132, TestSize.Level1)
{
    AudioPnpUevent uevent = {
        .action = "change",
        .name = "headset",
        .state = "analog_hs1",
        .devType = "extcon",
        .subSystem = "switch",
        .switchName = "h2w",
        .switchState = "",  // Empty switchState
        .hidName = "hid",
        .devName = "dev_name",
        .anahsName = "anahs_name"
    };
    AudioEvent audioEvent;
    EXPECT_EQ(audioSocketThread_.SetAudioPnpServerEventValue(&audioEvent, &uevent), ERROR);
}

/**
 * @tc.name  : Test SetAudioAnahsEventValue_ValidInsert
 * @tc.number: AudioSocketThread_133
 * @tc.desc  : Test SetAudioAnahsEventValue with valid INSERT event
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_133, TestSize.Level1)
{
    struct AudioPnpUevent audioPnpUevent;
    audioPnpUevent.subSystem = UEVENT_PLATFORM;
    audioPnpUevent.anahsName = UEVENT_INSERT;
    AudioEvent audioEvent;
    int32_t ret = AudioSocketThread::SetAudioAnahsEventValue(&audioEvent, &audioPnpUevent);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(audioEvent.anahsName, UEVENT_INSERT);
}

/**
 * @tc.name  : Test SetAudioAnahsEventValue_ValidRemove
 * @tc.number: AudioSocketThread_134
 * @tc.desc  : Test SetAudioAnahsEventValue with valid REMOVE event
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_134, TestSize.Level1)
{
    struct AudioPnpUevent audioPnpUevent;
    audioPnpUevent.subSystem = UEVENT_PLATFORM;
    audioPnpUevent.anahsName = UEVENT_REMOVE;
    AudioEvent audioEvent;
    int32_t ret = AudioSocketThread::SetAudioAnahsEventValue(&audioEvent, &audioPnpUevent);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(audioEvent.anahsName, UEVENT_REMOVE);
}

/**
 * @tc.name  : Test SetAudioAnahsEventValue_InvalidAnahsName
 * @tc.number: AudioSocketThread_135
 * @tc.desc  : Test SetAudioAnahsEventValue with invalid anahsName (not INSERT/REMOVE)
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_135, TestSize.Level1)
{
    struct AudioPnpUevent audioPnpUevent;
    audioPnpUevent.subSystem = UEVENT_PLATFORM;
    audioPnpUevent.anahsName = "INVALID";
    AudioEvent audioEvent;
    int32_t ret = AudioSocketThread::SetAudioAnahsEventValue(&audioEvent, &audioPnpUevent);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test SetAudioAnahsEventValue_InvalidSubSystem
 * @tc.number: AudioSocketThread_136
 * @tc.desc  : Test SetAudioAnahsEventValue with invalid subSystem
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_136, TestSize.Level1)
{
    struct AudioPnpUevent audioPnpUevent;
    audioPnpUevent.subSystem = "invalid";
    audioPnpUevent.anahsName = UEVENT_INSERT;
    AudioEvent audioEvent;
    int32_t ret = AudioSocketThread::SetAudioAnahsEventValue(&audioEvent, &audioPnpUevent);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test ProcessSwitchSubsystem_NonSwitchSubSystem
 * @tc.number: AudioSocketThread_137
 * @tc.desc  : Test ProcessSwitchSubsystem with non-switch subsystem
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_137, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    struct AudioPnpUevent uevent = {
        .action = "change",
        .name = "headset",
        .state = "analog_hs1",
        .devType = "extcon",
        .subSystem = "other",  // Not "switch"
        .switchName = "h2w",
        .switchState = "1",
        .hidName = "hid",
        .devName = "dev_name",
        .anahsName = "anahs_name"
    };
    bool ret = audioSocketThread.ProcessSwitchSubsystem(&uevent);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test ProcessOtherSubsystem_AnahsSuccess
 * @tc.number: AudioSocketThread_138
 * @tc.desc  : Test ProcessOtherSubsystem when AudioAnahsDetectDevice returns SUCCESS
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_138, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    struct AudioPnpUevent uevent = {
        .action = "change",
        .name = "headset",
        .state = "analog_hs1",
        .devType = "extcon",
        .subSystem = UEVENT_PLATFORM,
        .switchName = "h2w",
        .switchState = "1",
        .hidName = "hid",
        .devName = "dev_name",
        .anahsName = UEVENT_INSERT
    };
    // AudioAnahsDetectDevice should succeed for platform subsystem with INSERT/REMOVE
    int32_t ret = audioSocketThread.AudioAnahsDetectDevice(&uevent);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test ReadAndScanDpState_ReadZeroBytes
 * @tc.number: AudioSocketThread_140
 * @tc.desc  : Test ReadAndScanDpState when fread returns 0 (empty file)
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_140, TestSize.Level1)
{
    std::string testPath = "/tmp/test_dp_state_empty";
    std::ofstream file(testPath);
    file.close();  // Create empty file

    uint32_t eventType = 0;
    int32_t ret = audioSocketThread_.ReadAndScanDpState(testPath, eventType);
    EXPECT_NE(ret, SUCCESS);
    remove(testPath.c_str());
}

/**
 * @tc.name  : Test AudioMicInDetectDevice_NonH2wMicIn
 * @tc.number: AudioSocketThread_141
 * @tc.desc  : Test AudioMicInDetectDevice with switchName not h2w_micin
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_141, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    struct AudioPnpUevent audioPnpUevent;
    audioPnpUevent.switchName = "other_switch";
    int32_t ret = audioSocketThread.AudioMicInDetectDevice(&audioPnpUevent);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test AudioLineInDetectDevice_NonH2wLineIn
 * @tc.number: AudioSocketThread_142
 * @tc.desc  : Test AudioLineInDetectDevice with switchName not h2w_linein
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_142, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    struct AudioPnpUevent audioPnpUevent;
    audioPnpUevent.switchName = "other_switch";
    int32_t ret = audioSocketThread.AudioLineInDetectDevice(&audioPnpUevent);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test AudioLineOutDetectDevice_NonH2wLineOut
 * @tc.number: AudioSocketThread_143
 * @tc.desc  : Test AudioLineOutDetectDevice with switchName not h2w_lineout
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_143, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    struct AudioPnpUevent audioPnpUevent;
    audioPnpUevent.switchName = "other_switch";
    int32_t ret = audioSocketThread.AudioLineOutDetectDevice(&audioPnpUevent);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test DetectSwitchDeviceState_NullParams
 * @tc.number: AudioSocketThread_144
 * @tc.desc  : Test DetectSwitchDeviceState with null parameters
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_144, TestSize.Level1)
{
    AudioEvent audioEvent;
    // Null switchPath
    int32_t ret1 = audioSocketThread_.DetectSwitchDeviceState(nullptr, "h2w", &audioEvent, "Test");
    EXPECT_EQ(ret1, ERROR);
    // Null switchName
    int32_t ret2 = audioSocketThread_.DetectSwitchDeviceState("/path", nullptr, &audioEvent, "Test");
    EXPECT_EQ(ret2, ERROR);
    // Null audioEvent
    int32_t ret3 = audioSocketThread_.DetectSwitchDeviceState("/path", "h2w", nullptr, "Test");
    EXPECT_EQ(ret3, ERROR);
}

/**
 * @tc.name  : Test DetectAnalogHeadsetState_FreadZero
 * @tc.number: AudioSocketThread_145
 * @tc.desc  : Test DetectAnalogHeadsetState when h2w state node open/read fails
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_145, TestSize.Level1)
{
    std::ofstream ofs(SWITCH_STATE_PATH);
    ofs.close();  // may fail: /sys node is usually not writable in UT env
    AudioEvent audioEvent;
    int32_t ret = audioSocketThread_.DetectAnalogHeadsetState(&audioEvent);
    // fopen fail -> HDF_ERR_INVALID_PARAM; fread 0 on empty writable node -> ERROR
    EXPECT_TRUE(ret == HDF_ERR_INVALID_PARAM || ret == ERROR);
}

/**
 * @tc.name  : Test UpdatePnpDeviceState_UpdateFields
 * @tc.number: AudioSocketThread_146
 * @tc.desc  : Test UpdatePnpDeviceState updates audioSocketEvent_ fields
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_146, TestSize.Level4)
{
    AudioEvent event;
    event.eventType = PNP_EVENT_DEVICE_ADD;
    event.deviceType = PNP_DEVICE_MIC;
    event.name = "mic_test";
    event.address = "addr_0";
    event.anahsName = "anahs_test";
    AudioSocketThread::UpdatePnpDeviceState(&event);
    EXPECT_EQ(AudioSocketThread::audioSocketEvent_.eventType, PNP_EVENT_DEVICE_ADD);
    EXPECT_EQ(AudioSocketThread::audioSocketEvent_.deviceType, PNP_DEVICE_MIC);
    EXPECT_EQ(AudioSocketThread::audioSocketEvent_.name, "mic_test");
    EXPECT_EQ(AudioSocketThread::audioSocketEvent_.address, "addr_0");
    EXPECT_EQ(AudioSocketThread::audioSocketEvent_.anahsName, "anahs_test");
}

/**
 * @tc.name  : Test SetAudioPnpServerEventValue_NonSwitchSubsystem
 * @tc.number: AudioSocketThread_147
 * @tc.desc  : Test SetAudioPnpServerEventValue with non-switch subsystem
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_147, TestSize.Level1)
{
    AudioPnpUevent uevent = {
        .action = "change",
        .name = "headset",
        .state = "analog_hs1",
        .devType = "extcon",
        .subSystem = "audio",  // Not "switch"
        .switchName = "h2w",
        .switchState = "1",
        .hidName = "hid",
        .devName = "dev_name",
        .anahsName = "anahs_name"
    };
    AudioEvent audioEvent;
    int32_t ret = audioSocketThread_.SetAudioPnpServerEventValue(&audioEvent, &uevent);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test ProcessSwitchDeviceConfig_H2wMicIn_Add
 * @tc.number: AudioSocketThread_148
 * @tc.desc  : Test ProcessSwitchDeviceConfig with h2w_micin and switchState '1'
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_148, TestSize.Level1)
{
    AudioEvent audioEvent;
    AudioPnpUevent audioPnpUevent{
        .subSystem = "switch",
        .switchName = UEVENT_NAME_H2W_MICIN,
        .switchState = "1",
        .devName = "test_dev",
        .name = "test_name"
    };
    int32_t ret = AudioSocketThread::SetAudioPnpServerEventValue(&audioEvent,
        &audioPnpUevent);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(audioEvent.eventType, PNP_EVENT_DEVICE_ADD);
    EXPECT_EQ(audioEvent.deviceType, PNP_DEVICE_MICIN);
}

/**
 * @tc.name  : Test ProcessSwitchDeviceConfig_H2wLineIn_Remove
 * @tc.number: AudioSocketThread_149
 * @tc.desc  : Test ProcessSwitchDeviceConfig with h2w_linein and switchState '0'
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_149, TestSize.Level1)
{
    AudioEvent audioEvent;
    AudioPnpUevent audioPnpUevent{
        .subSystem = "switch",
        .switchName = UEVENT_NAME_H2W_LINEIN,
        .switchState = "0",
        .devName = "test_dev",
        .name = "test_name"
    };
    int32_t ret = AudioSocketThread::SetAudioPnpServerEventValue(&audioEvent,
        &audioPnpUevent);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(audioEvent.eventType, PNP_EVENT_DEVICE_REMOVE);
    EXPECT_EQ(audioEvent.deviceType, PNP_DEVICE_LINEIN);
}

/**
 * @tc.name  : Test ProcessSwitchDeviceConfig_H2wLineOut_Add
 * @tc.number: AudioSocketThread_150
 * @tc.desc  : Test ProcessSwitchDeviceConfig with h2w_lineout and switchState '1'
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_150, TestSize.Level1)
{
    AudioEvent audioEvent;
    AudioPnpUevent audioPnpUevent{
        .subSystem = "switch",
        .switchName = UEVENT_NAME_H2W_LINEOUT,
        .switchState = "1",
        .devName = "test_dev",
        .name = "test_name"
    };
    int32_t ret = AudioSocketThread::SetAudioPnpServerEventValue(&audioEvent,
        &audioPnpUevent);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(audioEvent.eventType, PNP_EVENT_DEVICE_ADD);
    EXPECT_EQ(audioEvent.deviceType, PNP_DEVICE_LINEOUT);
}

/**
 * @tc.name  : Test AudioPnpUeventParse_StrLengthExceed
 * @tc.number: AudioSocketThread_151
 * @tc.desc  : Test AudioPnpUeventParse when strLength > UEVENT_MSG_LEN + 1
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_151, TestSize.Level1)
{
    std::string msg = "ACTION=change";
    ssize_t strLength = UEVENT_MSG_LEN + 10;  // Exceed limit
    bool result = audioSocketThread_.AudioPnpUeventParse(msg.c_str(), strLength);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test AudioPnpUeventParse_WithNullChars
 * @tc.number: AudioSocketThread_152
 * @tc.desc  : Test AudioPnpUeventParse with embedded null characters
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_152, TestSize.Level4)
{
    std::vector<char> msgVec;
    const char *segs[] = {
        "ACTION=change",
        "SUBSYSTEM=switch",
        "SWITCH_NAME=h2w",
        "SWITCH_STATE=1",
        "NAME=headset"
    };
    for (const char *seg : segs) {
        msgVec.insert(msgVec.end(), seg, seg + strlen(seg) + 1);  // include trailing '\0'
    }

    ssize_t strLength = msgVec.size();
    const char *msg = msgVec.data();
    bool result = audioSocketThread_.AudioPnpUeventParse(msg, strLength);
    // Null-separated valid switch event should be parsed and handled
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test AudioPnpUeventParse_AnahsHandled
 * @tc.number: AudioSocketThread_153
 * @tc.desc  : Cover ProcessOtherSubsystem ANAHS success branch via AudioPnpUeventParse
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_153, TestSize.Level1)
{
    auto msg = BuildUeventMsg({"ACTION=change", "SUBSYSTEM=platform", "ANAHS=INSERT"});
    bool result = audioSocketThread_.AudioPnpUeventParse(msg.data(), msg.size());
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test AudioPnpUeventParse_DpHandled
 * @tc.number: AudioSocketThread_154
 * @tc.desc  : Cover ProcessOtherSubsystem DP success branch via AudioPnpUeventParse
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_154, TestSize.Level1)
{
    auto msg = BuildUeventMsg({"ACTION=change", "SUBSYSTEM=switch",
        "SWITCH_NAME=hdmi_audio1device_port=DP-0", "SWITCH_STATE=1"});
    bool result = audioSocketThread_.AudioPnpUeventParse(msg.data(), msg.size());
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test AudioPnpUeventParse_HdmiHandled
 * @tc.number: AudioSocketThread_155
 * @tc.desc  : Cover ProcessOtherSubsystem HDMI success branch via AudioPnpUeventParse
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_155, TestSize.Level1)
{
    auto msg = BuildUeventMsg({"ACTION=change", "SUBSYSTEM=switch",
        "SWITCH_NAME=hdmi_mipi_audio,device_port=HDMI-0", "STATE=HDMI=1", "SWITCH_STATE=1"});
    bool result = audioSocketThread_.AudioPnpUeventParse(msg.data(), msg.size());
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test AudioPnpUeventParse_SwitchMicInHandled
 * @tc.number: AudioSocketThread_156
 * @tc.desc  : Cover ProcessSwitchSubsystem MicIn SUCCESS short-circuit branch
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_156, TestSize.Level1)
{
    auto msg = BuildUeventMsg({"ACTION=change", "SUBSYSTEM=switch",
        "SWITCH_NAME=h2w_micin", "SWITCH_STATE=1"});
    bool result = audioSocketThread_.AudioPnpUeventParse(msg.data(), msg.size());
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test AudioPnpUeventParse_SwitchLineInHandled
 * @tc.number: AudioSocketThread_157
 * @tc.desc  : Cover ProcessSwitchSubsystem LineIn SUCCESS short-circuit branch
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_157, TestSize.Level1)
{
    auto msg = BuildUeventMsg({"ACTION=change", "SUBSYSTEM=switch",
        "SWITCH_NAME=h2w_linein", "SWITCH_STATE=1"});
    bool result = audioSocketThread_.AudioPnpUeventParse(msg.data(), msg.size());
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test AudioPnpUeventParse_SwitchLineOutHandled
 * @tc.number: AudioSocketThread_158
 * @tc.desc  : Cover ProcessSwitchSubsystem LineOut SUCCESS short-circuit branch
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_158, TestSize.Level1)
{
    auto msg = BuildUeventMsg({"ACTION=change", "SUBSYSTEM=switch",
        "SWITCH_NAME=h2w_lineout", "SWITCH_STATE=1"});
    bool result = audioSocketThread_.AudioPnpUeventParse(msg.data(), msg.size());
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test AudioMicInDetectDevice_NullAndMatch
 * @tc.number: AudioSocketThread_159
 * @tc.desc  : Cover AudioMicInDetectDevice null param and SUCCESS path
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_159, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    EXPECT_EQ(audioSocketThread.AudioMicInDetectDevice(nullptr), HDF_ERR_INVALID_PARAM);
    struct AudioPnpUevent uevent;
    uevent.switchName = nullptr;
    EXPECT_EQ(audioSocketThread.AudioMicInDetectDevice(&uevent), HDF_ERR_INVALID_PARAM);
    uevent.switchName = UEVENT_NAME_H2W_MICIN;
    EXPECT_EQ(audioSocketThread.AudioMicInDetectDevice(&uevent), SUCCESS);
}

/**
 * @tc.name  : Test AudioLineInDetectDevice_NullAndMatch
 * @tc.number: AudioSocketThread_160
 * @tc.desc  : Cover AudioLineInDetectDevice null param and SUCCESS path
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_160, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    EXPECT_EQ(audioSocketThread.AudioLineInDetectDevice(nullptr), HDF_ERR_INVALID_PARAM);
    struct AudioPnpUevent uevent;
    uevent.switchName = nullptr;
    EXPECT_EQ(audioSocketThread.AudioLineInDetectDevice(&uevent), HDF_ERR_INVALID_PARAM);
    uevent.switchName = UEVENT_NAME_H2W_LINEIN;
    EXPECT_EQ(audioSocketThread.AudioLineInDetectDevice(&uevent), SUCCESS);
}

/**
 * @tc.name  : Test AudioLineOutDetectDevice_NullAndMatch
 * @tc.number: AudioSocketThread_161
 * @tc.desc  : Cover AudioLineOutDetectDevice null param and SUCCESS path
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_161, TestSize.Level1)
{
    AudioSocketThread audioSocketThread;
    EXPECT_EQ(audioSocketThread.AudioLineOutDetectDevice(nullptr), HDF_ERR_INVALID_PARAM);
    struct AudioPnpUevent uevent;
    uevent.switchName = nullptr;
    EXPECT_EQ(audioSocketThread.AudioLineOutDetectDevice(&uevent), HDF_ERR_INVALID_PARAM);
    uevent.switchName = UEVENT_NAME_H2W_LINEOUT;
    EXPECT_EQ(audioSocketThread.AudioLineOutDetectDevice(&uevent), SUCCESS);
}

/**
 * @tc.name  : Test DetectSwitchDeviceState_FreadSuccess
 * @tc.number: AudioSocketThread_162
 * @tc.desc  : Cover DetectSwitchDeviceState fread>0 path (ADD and REMOVE)
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSocketThreadUnitTest, AudioSocketThread_162, TestSize.Level1)
{
    std::string pathAdd = "/tmp/test_switch_h2w_add";
    {
        std::ofstream f(pathAdd);
        f << '1';
    }
    AudioEvent evAdd;
    audioSocketThread_.DetectSwitchDeviceState(pathAdd.c_str(), "h2w", &evAdd, "T");
    remove(pathAdd.c_str());

    std::string pathRemove = "/tmp/test_switch_h2w_remove";
    {
        std::ofstream f(pathRemove);
        f << '0';
    }
    AudioEvent evRemove;
    audioSocketThread_.DetectSwitchDeviceState(pathRemove.c_str(), "h2w", &evRemove, "T");
    EXPECT_NE(evRemove.eventType, PNP_EVENT_MIC_UNBLOCKED);
    remove(pathRemove.c_str());
}

} // namespace AudioStandard
} // namespace OHOS
