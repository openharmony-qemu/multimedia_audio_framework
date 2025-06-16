/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef AUDIO_PNP_PARAM
#define AUDIO_PNP_PARAM

#define ADD_DEVICE_HEADSET '1'
#define ADD_DEVICE_HEADSET_WITHOUT_MIC '2'
#define ADD_DEVICE_ADAPTER '4'
#define ADD_DEVICE_MIC_BLOCKED '5'
#define ADD_DEVICE_MIC_UN_BLOCKED '6'
#define AUDIO_EVENT_INFO_LEN_MAX 256
#define AUDIO_PNP_INFO_LEN_MAX  256
#define AUDIO_DEVICE_WAIT_USB_EVENT_UPDATE 100

#define INPUT_EVT_MAX_CNT       4
#define MOVE_NUM 16
#define REMOVE_AUDIO_DEVICE '0'
#define SWITCH_STATE_PATH    "/sys/class/switch/h2w/state"
#define STATE_PATH_ITEM_SIZE 1

#define STATE_NOT_SUPPORTED     0
#define STATE_NN_OFF            1
#define STATE_NN_ON             2

#define UEVENT_ACTION           "ACTION="
#define UEVENT_NAME             "NAME="
#define UEVENT_STATE            "STATE="
#define UEVENT_DEV_NAME         "DEVNAME="
#define UEVENT_DEVTYPE          "DEVTYPE="
#define UEVENT_SUBSYSTEM        "SUBSYSTEM="
#define UEVENT_SWITCH_NAME      "SWITCH_NAME="
#define UEVENT_SWITCH_STATE     "SWITCH_STATE="
#define UEVENT_HDI_NAME         "HID_NAME="
#define UEVENT_ANAHS            "ANAHS="
#define UEVENT_ACTION_CHANGE    "change"
#define UEVENT_TYPE_EXTCON      "extcon3"
#define UEVENT_NAME_HEADSET     "headset"
#define UEVENT_STATE_ANALOG_HS0 "MICROPHONE=0"
#define UEVENT_STATE_ANALOG_HS1 "MICROPHONE=1"
#define UEVENT_SUBSYSTEM_SWITCH "switch"
#define UEVENT_SWITCH_NAME_H2W  "h2w"
#define UEVENT_SOCKET_BUFF_SIZE (64 * 1024)
#define UEVENT_SOCKET_GROUPS    0xffffffff
#define UEVENT_MSG_LEN          2048
#define UEVENT_ID_MODEL         "ID_MODEL="
#define UEVENT_PLATFORM         "platform"
#define UEVENT_INSERT           "INSERT"
#define UEVENT_REMOVE           "REMOVE"
#define UEVENT_ARR_SIZE 10
#define UEVENT_POLL_WAIT_TIME 100
#define WAIT_THREAD_END_TIME_MS 1

#define DP_PORT_COUNT           5
#define DP_PATH                 "/sys/class/switch/hdmi_audio"
#define DEVICE_PORT             "device_port="

#include <string>

namespace OHOS {
namespace AudioStandard {
using namespace std;

struct AudioPnpUevent {
    const char *action;
    const char *name;
    const char *state;
    const char *devType;
    const char *subSystem;
    const char *switchName;
    const char *switchState;
    const char *hidName;
    const char *devName;
    const char *anahsName;
};

struct AudioEvent {
    uint32_t eventType;
    uint32_t deviceType;
    std::string name;
    std::string address;
    std::string anahsName;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PNP_PARAM