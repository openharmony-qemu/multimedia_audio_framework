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
#ifndef LOG_TAG
#define LOG_TAG "AudioSocketThread"
#endif

#include "audio_socket_thread.h"
#include <cctype>
#include <cstdlib>
#include <dirent.h>
#include <linux/netlink.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include "osal_time.h"
#include "audio_utils.h"
#include "audio_errors.h"
#include "securec.h"
#include "singleton.h"
#include "audio_policy_log.h"
#include "audio_pnp_server.h"
#include "audio_policy_server_handler.h"
#ifdef USB_ENABLE
#include "audio_usb_manager.h"
#endif

namespace OHOS {
namespace AudioStandard {
using namespace std;
std::mutex AudioSocketThread::eventMutex_;
struct SwitchDeviceConfig {
    const char* switchName;
    PnpDeviceType deviceType;
    int32_t deviceRole;
    const char* logInsert;
    const char* logRemove;
};

static const SwitchDeviceConfig SWITCH_DEVICE_CONFIGS[] = {
    { UEVENT_NAME_H2W_MICIN, PNP_DEVICE_MICIN, INPUT_DEVICE, "POST-MIC IN: INSERTED (h2w_micin)",
        "POST-MIC IN: REMOVE (h2w_micin)"},
    { UEVENT_NAME_H2W_LINEIN, PNP_DEVICE_LINEIN, INPUT_DEVICE, "POST-LINE IN: INSERTED (h2w_linein)",
        "POST-LINE IN: REMOVE (h2w_linein)"},
    { UEVENT_NAME_H2W_LINEOUT, PNP_DEVICE_LINEOUT, OUTPUT_DEVICE, "POST-MIC OUT: INSERTED (h2w_lineout)",
        "POST-LINE OUT: REMOVE (h2w_lineout)"},
};
static char g_switchStateBuffer[2] = {0};
static constexpr size_t SWITCH_DEVICE_CONFIG_COUNT = sizeof(SWITCH_DEVICE_CONFIGS) / sizeof(SWITCH_DEVICE_CONFIGS[0]);

AudioEvent AudioSocketThread::audioSocketEvent_ = {
    .eventType = AUDIO_EVENT_UNKNOWN,
    .deviceType = AUDIO_DEVICE_UNKNOWN,
};

bool AudioSocketThread::IsUpdatePnpDeviceState(AudioEvent *pnpDeviceEvent)
{
    if (pnpDeviceEvent->eventType == audioSocketEvent_.eventType &&
        pnpDeviceEvent->deviceType == audioSocketEvent_.deviceType &&
        pnpDeviceEvent->name == audioSocketEvent_.name &&
        pnpDeviceEvent->address == audioSocketEvent_.address &&
        pnpDeviceEvent->anahsName == audioSocketEvent_.anahsName) {
        return false;
    }
    return true;
}

void AudioSocketThread::UpdatePnpDeviceState(AudioEvent *pnpDeviceEvent)
{
    std::lock_guard<std::mutex> lock(eventMutex_);
    audioSocketEvent_.eventType = pnpDeviceEvent->eventType;
    audioSocketEvent_.deviceType = pnpDeviceEvent->deviceType;
    audioSocketEvent_.name = pnpDeviceEvent->name;
    audioSocketEvent_.address = pnpDeviceEvent->address;
    audioSocketEvent_.anahsName = pnpDeviceEvent->anahsName;
    audioSocketEvent_.deviceRole = pnpDeviceEvent->deviceRole;
}

int AudioSocketThread::AudioPnpUeventOpen(int *fd)
{
    int socketFd = -1;
    int buffSize = UEVENT_SOCKET_BUFF_SIZE;
    const int32_t on = 1; // turn on passcred
    sockaddr_nl addr;

    if (memset_s(&addr, sizeof(addr), 0, sizeof(addr)) != EOK) {
        AUDIO_ERR_LOG("addr memset_s failed!");
        return ERROR;
    }
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = ((uint32_t)gettid() << MOVE_NUM) | (uint32_t)getpid();
    addr.nl_groups = UEVENT_SOCKET_GROUPS;

    socketFd = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (socketFd < 0) {
        AUDIO_ERR_LOG("socket failed, %{public}d", errno);
        return ERROR;
    }

    if (setsockopt(socketFd, SOL_SOCKET, SO_RCVBUF, &buffSize, sizeof(buffSize)) != 0) {
        AUDIO_ERR_LOG("setsockopt SO_RCVBUF failed, %{public}d", errno);
        CloseFd(socketFd);
        return ERROR;
    }

    if (setsockopt(socketFd, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on)) != 0) {
        AUDIO_ERR_LOG("setsockopt SO_PASSCRED failed, %{public}d", errno);
        CloseFd(socketFd);
        return ERROR;
    }

    if (::bind(socketFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        AUDIO_ERR_LOG("bind socket failed, %{public}d", errno);
        CloseFd(socketFd);
        return ERROR;
    }

    *fd = socketFd;
    return SUCCESS;
}

ssize_t AudioSocketThread::AudioPnpReadUeventMsg(int sockFd, char *buffer, size_t length)
{
    char credMsg[CMSG_SPACE(sizeof(struct ucred))] = {0};
    iovec iov;
    sockaddr_nl addr;
    msghdr msghdr = {0};

    memset_s(&addr, sizeof(addr), 0, sizeof(addr));

    iov.iov_base = buffer;
    iov.iov_len = length;

    msghdr.msg_name = &addr;
    msghdr.msg_namelen = sizeof(addr);
    msghdr.msg_iov = &iov;
    msghdr.msg_iovlen = 1;
    msghdr.msg_control = credMsg;
    msghdr.msg_controllen = sizeof(credMsg);

    ssize_t len = recvmsg(sockFd, &msghdr, 0);
    if (len <= 0) {
        return ERROR;
    }
    cmsghdr *hdr = CMSG_FIRSTHDR(&msghdr);
    if (hdr == NULL || hdr->cmsg_type != SCM_CREDENTIALS) {
        AUDIO_ERR_LOG("Unexpected control message, ignored");
        *buffer = '\0';
        return ERROR;
    }
    return len;
}

int32_t AudioSocketThread::SetAudioAnahsEventValue(AudioEvent *audioEvent, const struct AudioPnpUevent *audioPnpUevent)
{
    if (strncmp(audioPnpUevent->subSystem, UEVENT_PLATFORM, strlen(UEVENT_PLATFORM)) == 0) {
        if (strncmp(audioPnpUevent->anahsName, UEVENT_INSERT, strlen(UEVENT_INSERT)) == 0) {
            AUDIO_INFO_LOG("set anahs event to insert.");
            audioEvent->anahsName = UEVENT_INSERT;
            return SUCCESS;
        } else if (strncmp(audioPnpUevent->anahsName, UEVENT_REMOVE, strlen(UEVENT_REMOVE)) == 0) {
            AUDIO_INFO_LOG("set anahs event to remove.");
            audioEvent->anahsName = UEVENT_REMOVE;
            return SUCCESS;
        } else {
            return ERROR;
        }
    }
    return ERROR;
}

void AudioSocketThread::SetAudioPnpUevent(AudioEvent *audioEvent, char switchState)
{
    if (audioEvent == nullptr) {
        AUDIO_ERR_LOG("audioEvent is null!");
        return;
    }
    static uint32_t h2wTypeLast = PNP_DEVICE_HEADSET;
    switch (switchState) {
        case REMOVE_AUDIO_DEVICE:
            audioEvent->eventType = PNP_EVENT_DEVICE_REMOVE;
            audioEvent->deviceType = h2wTypeLast;
            AUDIO_INFO_LOG("FRONT-HEADSET: REMOVED");
            break;
        case ADD_DEVICE_HEADSET:
            audioEvent->eventType = PNP_EVENT_DEVICE_ADD;
            audioEvent->deviceType = PNP_DEVICE_HEADSET;
            AUDIO_INFO_LOG("FRONT-HEADSET: INSERTED (4-pole, HEADSET)");
            break;
        case ADD_DEVICE_HEADSET_WITHOUT_MIC:
            audioEvent->eventType = PNP_EVENT_DEVICE_ADD;
            audioEvent->deviceType = PNP_DEVICE_HEADPHONE;
            AUDIO_INFO_LOG("FRONT-HEADSET: INSERTED (3-pole, HEADPHONE)");
            break;
        case ADD_DEVICE_ADAPTER:
            audioEvent->eventType = PNP_EVENT_DEVICE_ADD;
            audioEvent->deviceType = PNP_DEVICE_ADAPTER_DEVICE;
            break;
        default:
            audioEvent->eventType = PNP_EVENT_DEVICE_ADD;
            audioEvent->deviceType = PNP_DEVICE_UNKNOWN;
            AUDIO_ERR_LOG("Unknown h2w switchState: %{public}c", switchState);
            break;
    }
    h2wTypeLast = audioEvent->deviceType;
}

int32_t AudioSocketThread::ProcessSwitchDeviceConfig(AudioEvent *audioEvent, const char *switchName,
    char switchState, const char *devName, const char *eventName)
{
    for (size_t i = 0; i < SWITCH_DEVICE_CONFIG_COUNT; ++i) {
        const auto &cfg = SWITCH_DEVICE_CONFIGS[i];
        if (strcmp(switchName, cfg.switchName) == 0) {
            audioEvent->deviceType = cfg.deviceType;
            audioEvent->deviceRole = cfg.deviceRole;
            if (switchState == '1') {
                audioEvent->eventType = PNP_EVENT_DEVICE_ADD;
                AUDIO_INFO_LOG("%{public}s", cfg.logInsert);
            } else if (switchState == '0') {
                audioEvent->eventType = PNP_EVENT_DEVICE_REMOVE;
                AUDIO_INFO_LOG("%{public}s", cfg.logRemove);
            } else {
                AUDIO_ERR_LOG("Invalid switchState for %{public}s: %{public}c", switchName, switchState);
                return ERROR;
            }

            audioEvent->name = switchName;
            audioEvent->address = devName ? devName : "";
            return SUCCESS;
        }
    }

    AUDIO_ERR_LOG("Unknown switchName: %{public}s", switchName);
    return ERROR;
}

int32_t AudioSocketThread::SetAudioPnpServerEventValue(AudioEvent *audioEvent,
    const struct AudioPnpUevent *audioPnpUevent)
{
    if (audioEvent == nullptr || audioPnpUevent == nullptr) {
        AUDIO_ERR_LOG("audioEvent or audioPnpUevent is null!");
        return ERROR;
    }

    AUDIO_INFO_LOG("SetAudioPnpServerEventValue: "
        "subSystem=[%{public}s] "
        "switchName=[%{public}s] "
        "switchState=[%{public}s] "
        "name=[%{public}s] ",
        audioPnpUevent->subSystem ? audioPnpUevent->subSystem : "null",
        audioPnpUevent->switchName ? audioPnpUevent->switchName : "null",
        audioPnpUevent->switchState ? audioPnpUevent->switchState : "null",
        audioPnpUevent->name ? audioPnpUevent->name : "null");

        if (audioPnpUevent->subSystem == nullptr ||
            strncmp(audioPnpUevent->subSystem, UEVENT_SUBSYSTEM_SWITCH, strlen(UEVENT_SUBSYSTEM_SWITCH)) != 0) {
            AUDIO_DEBUG_LOG("Not a switch subsustem event, skip");
            return ERROR;
        }

        const char *switchName = audioPnpUevent->switchName;
        if (switchName == nullptr) {
            AUDIO_ERR_LOG("switchName is null");
            return ERROR;
        }

        if (audioPnpUevent->switchState == nullptr || audioPnpUevent->switchState[0] == '\0') {
            AUDIO_ERR_LOG("switchState is invalid");
            return ERROR;
        }

        char switchState = audioPnpUevent->switchState[0];
        if (strcmp(switchName, UEVENT_SWITCH_NAME_H2W) == 0) {
            SetAudioPnpUevent(audioEvent, switchState);
            audioEvent->name = audioPnpUevent->name ? audioPnpUevent->name : "";
            audioEvent->address = audioPnpUevent->devName ? audioPnpUevent->devName : "";
            return SUCCESS;
        }

        return ProcessSwitchDeviceConfig(audioEvent, switchName, switchState,
            audioPnpUevent->devName, audioPnpUevent->name);
}

int32_t AudioSocketThread::AudioAnahsDetectDevice(const struct AudioPnpUevent *audioPnpUevent)
{
    AudioEvent audioEvent;
    if (audioPnpUevent == nullptr) {
        AUDIO_ERR_LOG("audioPnpUevent is null!");
        return HDF_ERR_INVALID_PARAM;
    }
    if (SetAudioAnahsEventValue(&audioEvent, audioPnpUevent) != SUCCESS) {
        return ERROR;
    }

    if (audioEvent.anahsName == audioSocketEvent_.anahsName) {
        AUDIO_ERR_LOG("audio anahs device[%{public}u] state[%{public}u] not need flush !", audioEvent.deviceType,
            audioEvent.eventType);
        return SUCCESS;
    }
    UpdatePnpDeviceState(&audioEvent);
    return SUCCESS;
}

int32_t AudioSocketThread::AudioAnalogHeadsetDetectDevice(const struct AudioPnpUevent *audioPnpUevent)
{
    if (audioPnpUevent == nullptr || audioPnpUevent->switchName == nullptr) {
        AUDIO_ERR_LOG("audioPnpUevent is null");
        return HDF_ERR_INVALID_PARAM;
    }

    if (strcmp(audioPnpUevent->switchName, UEVENT_SWITCH_NAME_H2W) != 0) {
        AUDIO_DEBUG_LOG("AudioAnalogHeadsetDetectDevice: not h2w, skip");
        return ERROR;
    }
    AUDIO_INFO_LOG("AudioAnalogHeadsetDetectDevice: h2w event handled by parsed");
    return SUCCESS;
}

int32_t AudioSocketThread::AudioMicInDetectDevice(const struct AudioPnpUevent *audioPnpUevent)
{
    if (audioPnpUevent == nullptr || audioPnpUevent->switchName == nullptr) {
        AUDIO_ERR_LOG("audioPnpUevent is null");
        return HDF_ERR_INVALID_PARAM;
    }

    if (strcmp(audioPnpUevent->switchName, UEVENT_NAME_H2W_MICIN) != 0) {
        AUDIO_DEBUG_LOG("AudioMicInDetectDevice: not h2w_micin, skip");
        return ERROR;
    }
    AUDIO_INFO_LOG("AudioMicInDetectDevice: h2w_micin event parsed");
    return SUCCESS;
}

int32_t AudioSocketThread::AudioLineInDetectDevice(const struct AudioPnpUevent *audioPnpUevent)
{
    if (audioPnpUevent == nullptr || audioPnpUevent->switchName == nullptr) {
        AUDIO_ERR_LOG("audioPnpUevent is null");
        return HDF_ERR_INVALID_PARAM;
    }

    if (strcmp(audioPnpUevent->switchName, UEVENT_NAME_H2W_LINEIN) != 0) {
        AUDIO_DEBUG_LOG("AudioLineInDetectDevice: not h2w_linein, skip");
        return ERROR;
    }
    AUDIO_INFO_LOG("AudioLineInDetectDevice: h2w_linein event parsed");
    return SUCCESS;
}

int32_t AudioSocketThread::AudioLineOutDetectDevice(const struct AudioPnpUevent *audioPnpUevent)
{
    if (audioPnpUevent == nullptr || audioPnpUevent->switchName == nullptr) {
        AUDIO_ERR_LOG("audioPnpUevent is null");
        return HDF_ERR_INVALID_PARAM;
    }

    if (strcmp(audioPnpUevent->switchName, UEVENT_NAME_H2W_LINEOUT) != 0) {
        AUDIO_DEBUG_LOG("AudioLineOutDetectDevice: not h2w_lineout, skip");
        return ERROR;
    }
    AUDIO_INFO_LOG("AudioLineOutDetectDevice: h2w_lineout parsed");
    return SUCCESS;
}

int32_t AudioSocketThread::AudioNnDetectDevice(const struct AudioPnpUevent *audioPnpUevent)
{
    if (audioPnpUevent == nullptr) {
        return HDF_ERR_INVALID_PARAM;
    }

    if ((strcmp(audioPnpUevent->action, "change") != 0) ||
        (strncmp(audioPnpUevent->name, "send_nn_state", strlen("send_nn_state")) != 0)) {
        return HDF_ERR_INVALID_PARAM;
    }

    std::string ueventStr = audioPnpUevent->name;
    auto state = ueventStr.substr(ueventStr.find("send_nn_state") + strlen("send_nn_state") + 1);
    int32_t nnState;
    switch (atoi(state.c_str())) {
        case STATE_NOT_SUPPORTED:
            nnState = STATE_NOT_SUPPORTED;
            break;
        case STATE_NN_OFF:
            nnState = STATE_NN_OFF;
            break;
        case STATE_NN_ON:
            nnState = STATE_NN_ON;
            break;
        default:
            AUDIO_ERR_LOG("NN state is invalid");
            return HDF_ERR_INVALID_PARAM;
    }

    // callback of bluetooth
    auto handle = DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    if (handle == nullptr) {
        AUDIO_ERR_LOG("get AudioPolicyServerHandler instance failed");
        return HDF_ERR_INVALID_PARAM;
    }
    bool ret = handle->SendNnStateChangeCallback(nnState);
    AUDIO_INFO_LOG("NN state change callback ret is [%{public}d]", ret);
    return ret;
}

int32_t AudioSocketThread::AudioDpDetectDevice(const struct AudioPnpUevent *audioPnpUevent)
{
    AudioEvent audioEvent = {0};
    if (audioPnpUevent == nullptr) {
        return HDF_ERR_INVALID_PARAM;
    }
    if ((strcmp(audioPnpUevent->subSystem, "switch") != 0) ||
        (strstr(audioPnpUevent->switchName, "hdmi_audio") == NULL) ||
        (strcmp(audioPnpUevent->action, "change") != 0)) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (strcmp(audioPnpUevent->switchState, "1") == 0) {
        audioEvent.eventType = PNP_EVENT_DEVICE_ADD;
    } else if (strcmp(audioPnpUevent->switchState, "0") == 0) {
        audioEvent.eventType = PNP_EVENT_DEVICE_REMOVE;
    } else {
        AUDIO_ERR_LOG("audio dp device [%{public}d]", audioEvent.eventType);
        return ERROR;
    }
    audioEvent.deviceType = PNP_DEVICE_DP_DEVICE;

    std::string switchNameStr = audioPnpUevent->switchName;

    auto portBegin = switchNameStr.find("device_port=");
    if (portBegin != switchNameStr.npos) {
        audioEvent.name = switchNameStr.substr(portBegin + std::strlen("device_port="),
            switchNameStr.length() - portBegin - std::strlen("device_port="));
    }

    auto addressBegin = switchNameStr.find("hdmi_audio");
    auto addressEnd = switchNameStr.find_first_of("device_port", portBegin);
    if (addressEnd != switchNameStr.npos) {
        std::string portId = switchNameStr.substr(addressBegin + std::strlen("hdmi_audio"),
            addressEnd - addressBegin - std::strlen("hdmi_audio")-1);
        audioEvent.address = portId;
        AUDIO_INFO_LOG("audio dp device portId:[%{public}s]", portId.c_str());
    }

    if (audioEvent.address.empty()) {
        audioEvent.address = '0';
    }
    AUDIO_INFO_LOG("audio dp device [%{public}s]", audioEvent.eventType == PNP_EVENT_DEVICE_ADD ? "add" : "removed");

    if (!IsUpdatePnpDeviceState(&audioEvent)) {
        AUDIO_ERR_LOG("audio usb device[%{public}u] state[%{public}u] not need flush !", audioEvent.deviceType,
            audioEvent.eventType);
        return SUCCESS;
    }
    UpdatePnpDeviceState(&audioEvent);
    return SUCCESS;
}

int32_t AudioSocketThread::AudioMicBlockDevice(const struct AudioPnpUevent *audioPnpUevent)
{
    if (audioPnpUevent == nullptr) {
        AUDIO_ERR_LOG("mic blocked audioPnpUevent is null");
        return HDF_ERR_INVALID_PARAM;
    }
    AudioEvent audioEvent = {0};
    if (strncmp(audioPnpUevent->name, "mic_blocked", strlen("mic_blocked")) == 0) {
        audioEvent.eventType = PNP_EVENT_MIC_BLOCKED;
    } else if (strncmp(audioPnpUevent->name, "mic_un_blocked", strlen("mic_un_blocked")) == 0) {
        audioEvent.eventType = PNP_EVENT_MIC_UNBLOCKED;
    } else {
        return HDF_ERR_INVALID_PARAM;
    }
    audioEvent.deviceType = PNP_DEVICE_MIC;

    AUDIO_INFO_LOG("mic blocked uevent info recv: %{public}s", audioPnpUevent->name);
    UpdatePnpDeviceState(&audioEvent);
    return SUCCESS;
}

#ifdef USB_ENABLE
int32_t AudioSocketThread::AudioDetectUsbSoundCard(const AudioPnpUevent &audioPnpUevent)
{
    if (!audioPnpUevent.devPath || !audioPnpUevent.subSystem || !audioPnpUevent.action) {
        return HDF_ERR_INVALID_PARAM;
    }
    CHECK_AND_RETURN_RET(strcmp(audioPnpUevent.subSystem, "sound") == 0, HDF_ERR_INVALID_PARAM);
    string devPath{audioPnpUevent.devPath};
    const char *strs[] {"/usb", "/sound/card", "/pcmC"};
    size_t pos = 0;
    for (const char *str : strs) {
        pos = devPath.find(str, pos);
        CHECK_AND_RETURN_RET(pos != string::npos, HDF_ERR_INVALID_PARAM);
        pos += strlen(str);
    }
    AUDIO_INFO_LOG("devPath=%{public}s, action=%{public}s", audioPnpUevent.devPath, audioPnpUevent.action);
    auto cardNumStr = devPath.substr(pos, devPath.find('D', pos) - pos);
    if (strcmp(audioPnpUevent.action, "add") == 0) {
        AudioUsbManager::GetInstance().NotifySoundCardChange(cardNumStr, true);
    } else if (strcmp(audioPnpUevent.action, "remove") == 0) {
        AudioUsbManager::GetInstance().NotifySoundCardChange(cardNumStr, false);
    } else {
        AUDIO_ERR_LOG("Invalid Action[%{public}s]", audioPnpUevent.action);
        return ERROR;
    }
    AudioEvent audioEvent;
    UpdatePnpDeviceState(&audioEvent);
    return SUCCESS;
}
#endif

int32_t AudioSocketThread::AudioHDMIDetectDevice(const struct AudioPnpUevent *audioPnpUevent)
{
    AudioEvent audioEvent = {0};
    if (audioPnpUevent == nullptr) {
        return HDF_ERR_INVALID_PARAM;
    }
    if ((strcmp(audioPnpUevent->subSystem, "switch") != 0) ||
        (strstr(audioPnpUevent->switchName, "hdmi_mipi_audio") == NULL) ||
        (strcmp(audioPnpUevent->action, "change") != 0)) {
        AUDIO_DEBUG_LOG("AudioHDMIDetectDevice fail");
        return HDF_ERR_INVALID_PARAM;
    }

    if (strcmp(audioPnpUevent->switchState, "1") == 0) {
        audioEvent.eventType = PNP_EVENT_DEVICE_ADD;
    } else if (strcmp(audioPnpUevent->switchState, "0") == 0) {
        audioEvent.eventType = PNP_EVENT_DEVICE_REMOVE;
    } else {
        AUDIO_ERR_LOG("audio hdmi device [%{public}d]", audioEvent.eventType);
        return ERROR;
    }
    audioEvent.deviceType = PNP_DEVICE_HDMI_DEVICE;

    std::string switchNameStr = audioPnpUevent->switchName;

    auto portBegin = switchNameStr.find("device_port=");
    if (portBegin != switchNameStr.npos) {
        audioEvent.name = switchNameStr.substr(portBegin + std::strlen("device_port="),
            switchNameStr.length() - portBegin - std::strlen("device_port="));
    }

    auto addressBegin = switchNameStr.find("hdmi_mipi_audio");
    auto addressEnd = switchNameStr.find_first_of("device_port", portBegin);
    if (addressEnd != switchNameStr.npos) {
        std::string portId = switchNameStr.substr(addressBegin + std::strlen("hdmi_mipi_audio"),
            addressEnd - addressBegin - std::strlen("hdmi_mipi_audio")-1);
        audioEvent.address = portId;
    }

    if (audioEvent.address.empty()) {
        audioEvent.address = '0';
    }
    AUDIO_INFO_LOG("audio hdmi device [%{public}s]", audioEvent.eventType == PNP_EVENT_DEVICE_ADD ? "add" : "removed");

    if (!IsUpdatePnpDeviceState(&audioEvent)) {
        AUDIO_ERR_LOG("audio device[%{public}u] state[%{public}u] not need flush !", audioEvent.deviceType,
            audioEvent.eventType);
        return SUCCESS;
    }
    UpdatePnpDeviceState(&audioEvent);
    return SUCCESS;
}

bool AudioSocketThread::ProcessSwitchSubsystem(const struct AudioPnpUevent *uevent)
{
    bool handled = false;
    AudioEvent parsedEvent = {};

    if (uevent->subSystem == nullptr ||
        strncmp(uevent->subSystem, UEVENT_SUBSYSTEM_SWITCH, strlen(UEVENT_SUBSYSTEM_SWITCH)) != 0) {
            return false;
        }

    if (AudioAnalogHeadsetDetectDevice(uevent) == SUCCESS ||
        AudioMicInDetectDevice(uevent) == SUCCESS ||
        AudioLineInDetectDevice(uevent) == SUCCESS ||
        AudioLineOutDetectDevice(uevent) == SUCCESS) {
            int32_t ret = SetAudioPnpServerEventValue(&parsedEvent, uevent);
            if (ret == SUCCESS) {
                AUDIO_INFO_LOG("Parsed switch event: eventType=%{public}d, deviceType=%{public}d, role=%{public}d",
                    parsedEvent.eventType, parsedEvent.deviceType, parsedEvent.deviceRole);
                UpdateDeviceState(parsedEvent);
                handled = true;
            }
        }
        return handled;
}

bool AudioSocketThread::ProcessOtherSubsystem(const struct AudioPnpUevent *uevent)
{
    bool handled = false;
    if (AudioAnahsDetectDevice(uevent) == SUCCESS) {
        AUDIO_INFO_LOG("Handled ANAHS event");
        handled = true;
    } else if (AudioNnDetectDevice(uevent) == SUCCESS) {
        AUDIO_INFO_LOG("Handled NN event");
        handled = true;
    } else if (AudioMicBlockDevice(uevent) == SUCCESS) {
        AUDIO_INFO_LOG("Handled Mic_Block event");
        handled = true;
    } else if (AudioDpDetectDevice(uevent) == SUCCESS) {
        AUDIO_INFO_LOG("Handled DP event");
        handled = true;
    } else if (AudioHDMIDetectDevice(uevent) == SUCCESS) {
        AUDIO_INFO_LOG("Handled HDMI event");
        handled = true;
    }
#ifdef USB_ENABLE
    {
        if (AudioDetectUsbSoundCard(*uevent) == SUCCESS) {
        AUDIO_INFO_LOG("Handled USB SoundCard event");
        handled = true;
        }
    }
#endif
    return handled;
}

bool AudioSocketThread::AudioPnpUeventParse(const char *msg, const ssize_t strLength)
{
    struct AudioPnpUevent audioPnpUevent = {"", "", "", "", "", "", "", "", "", ""};

    if (strncmp(msg, "libudev", strlen("libudev")) == 0) {
        return false;
    }

    if (strLength > UEVENT_MSG_LEN + 1) {
        AUDIO_ERR_LOG("strLength > UEVENT_MSG_LEN + 1");
        return false;
    }
    AUDIO_DEBUG_LOG("Param strLength: %{public}zu msg:[%{public}s] len:[%{public}zu]", strLength, msg, strlen(msg));
    for (const char *msgTmp = msg; msgTmp < (msg + strLength);) {
        if (*msgTmp == '\0') {
            msgTmp++;
            continue;
        }
        AUDIO_DEBUG_LOG("Param msgTmp:[%{private}s] len:[%{public}zu]", msgTmp, strlen(msgTmp));
        const char *arrStrTmp[UEVENT_ARR_SIZE] = {
            UEVENT_ACTION, UEVENT_DEV_NAME, UEVENT_NAME, UEVENT_STATE, UEVENT_DEVTYPE,
            UEVENT_SUBSYSTEM, UEVENT_SWITCH_NAME, UEVENT_SWITCH_STATE, UEVENT_HDI_NAME,
            UEVENT_ANAHS, UEVENT_DEVPATH,
        };
        const char **arrVarTmp[UEVENT_ARR_SIZE] = {
            &audioPnpUevent.action, &audioPnpUevent.devName, &audioPnpUevent.name,
            &audioPnpUevent.state, &audioPnpUevent.devType, &audioPnpUevent.subSystem,
            &audioPnpUevent.switchName, &audioPnpUevent.switchState, &audioPnpUevent.hidName,
            &audioPnpUevent.anahsName, &audioPnpUevent.devPath,
        };
        for (int count = 0; count < UEVENT_ARR_SIZE; count++) {
            if (strncmp(msgTmp, arrStrTmp[count], strlen(arrStrTmp[count])) == 0) {
                msgTmp += strlen(arrStrTmp[count]);
                *arrVarTmp[count] = msgTmp;
                break;
            }
        }
        msgTmp += strlen(msgTmp) + 1;
    }

    bool handled = ProcessSwitchSubsystem(&audioPnpUevent);
    if (!handled) {
        handled = ProcessOtherSubsystem(&audioPnpUevent);
    }

    return handled;
}

int32_t AudioSocketThread::DetectSwitchDeviceState(const char* switchPath, const char* switchName,
    AudioEvent* audioEvent, const char* logPrefix)
{
    if (audioEvent == nullptr || switchPath == nullptr || switchName == nullptr) {
        AUDIO_ERR_LOG("DetectSwitchDeviceState: null param! path=%{public}s, name=%{public}s",
            switchPath ? switchPath : "null", switchName);
        return ERROR;
    }
    if (switchPath[0] != '/') {
        AUDIO_ERR_LOG("DetectSwitchDeviceState: path is not absolute: %{public}s", switchPath);
        return HDF_ERR_INVALID_PARAM;
    }
    FILE* fp = fopen(switchPath, "r");
    if (fp == nullptr) {
        AUDIO_ERR_LOG("DetectSwitchDeviceState: open file failed: %{public}s, err=%{public}d",
            switchPath, errno);
        return HDF_ERR_INVALID_PARAM;
    }
    char state = '0';
    size_t ret = fread(&state, STATE_PATH_ITEM_SIZE, STATE_PATH_ITEM_SIZE, fp);
    if (ret == 0) {
        fclose(fp);
        AUDIO_ERR_LOG("DetectSwitchDeviceState: read file failed: %{public}s, err=%{public}d",
            switchPath, errno);
        return ERROR;
    }
    int rets = fclose(fp);
    if (rets != 0) {
        AUDIO_ERR_LOG("DetectSwitchDeviceState: close file failed: %{public}s, err=%{public}d", switchPath, errno);
        return ERROR;
    }
    g_switchStateBuffer[0] = state;
    g_switchStateBuffer[1] = '\0';
    struct AudioPnpUevent tempUevent = {
        .subSystem = UEVENT_SUBSYSTEM_SWITCH,
        .switchName = switchName,
        .switchState = g_switchStateBuffer,
    };
    int32_t parseRet = SetAudioPnpServerEventValue(audioEvent, &tempUevent);
    if (parseRet != SUCCESS) {
        AUDIO_ERR_LOG("DetectSwitchDeviceState: parse evvent failed for %{public}s", switchName);
        return ERROR;
    }
    if (audioEvent->eventType == PNP_EVENT_DEVICE_ADD) {
        AUDIO_INFO_LOG("%s: detected state=%{public}c", logPrefix, state);
    } else {
        AUDIO_INFO_LOG("%s: detected state=%{public}c", logPrefix, state);
    }
    return SUCCESS;
}

int32_t AudioSocketThread::DetectAnalogHeadsetState(AudioEvent *audioEvent)
{
    if (audioEvent == nullptr) {
        AUDIO_ERR_LOG("audioEvent is null!");
        return ERROR;
    }
    char state = '0';
    FILE *fp = fopen(SWITCH_STATE_PATH, "r");
    if (fp == NULL) {
        AUDIO_ERR_LOG("DetectAnalogHeadsetState: open h2w state node fail, %{public}d", errno);
        return HDF_ERR_INVALID_PARAM;
    }

    size_t ret = fread(&state, STATE_PATH_ITEM_SIZE, STATE_PATH_ITEM_SIZE, fp);
    if (ret == 0) {
        fclose(fp);
        AUDIO_ERR_LOG("DetectAnalogHeadsetState: read h2w state node fail, %{public}d", errno);
        return ERROR;
    }

    SetAudioPnpUevent(audioEvent, state);

    fclose(fp);
    if (audioEvent->eventType == PNP_EVENT_DEVICE_ADD) {
        AUDIO_INFO_LOG("DetectAnalogHeadsetState: h2w state= %{public}c", state);
    }
    return SUCCESS;
}

int32_t AudioSocketThread::DetectPostAudioDevices(AudioEvent *audioEvent)
{
    if (audioEvent == nullptr) {
        AUDIO_ERR_LOG("DetectPostAudioDevices: audioEvent is null!");
        return ERROR;
    }

    int32_t ret = ERROR;
    ret = DetectSwitchDeviceState(SWITCH_STATE_PATH_MICIN, UEVENT_NAME_H2W_MICIN,
        audioEvent, "DetectPostAudioDevices-MICIN");
    if (ret == SUCCESS && audioEvent->eventType == PNP_EVENT_DEVICE_ADD) {
        AUDIO_INFO_LOG("DetectPostAudioDevices: MicIn detected and added");
        return SUCCESS;
    }

    ret = DetectSwitchDeviceState(SWITCH_STATE_PATH_LINEIN, UEVENT_NAME_H2W_LINEIN,
        audioEvent, "DetectPostAudioDevices-LINEIN");
    if (ret == SUCCESS && audioEvent->eventType == PNP_EVENT_DEVICE_ADD) {
        AUDIO_INFO_LOG("DetectPostAudioDevices: LineIn detected and added");
        return SUCCESS;
    }

    ret = DetectSwitchDeviceState(SWITCH_STATE_PATH_LINEOUT, UEVENT_NAME_H2W_LINEOUT,
        audioEvent, "DetectPostAudioDevices-LINEOUT");
    if (ret == SUCCESS && audioEvent->eventType == PNP_EVENT_DEVICE_ADD) {
        AUDIO_INFO_LOG("DetectPostAudioDevices: LineOut detected and added");
        return SUCCESS;
    }

    AUDIO_DEBUG_LOG("DetectPostAudioDevices: No post audio device detected or not added");
    return ERROR;
}

void AudioSocketThread::UpdateDeviceState(AudioEvent audioEvent)
{
    char pnpInfo[AUDIO_EVENT_INFO_LEN_MAX] = {0};
    int32_t ret;
    if (!IsUpdatePnpDeviceState(&audioEvent)) {
        AUDIO_ERR_LOG("audio first pnp device[%{public}u] state[%{public}u] not need flush !", audioEvent.deviceType,
            audioEvent.eventType);
        return;
    }
    ret = snprintf_s(pnpInfo, AUDIO_EVENT_INFO_LEN_MAX, AUDIO_EVENT_INFO_LEN_MAX - 1, "EVENT_TYPE=%u;DEVICE_TYPE=%u",
        audioEvent.eventType, audioEvent.deviceType);
    if (ret < 0) {
        AUDIO_ERR_LOG("snprintf_s fail!");
        return;
    }

    UpdatePnpDeviceState(&audioEvent);
    return;
}

int32_t AudioSocketThread::DetectDPState(AudioEvent *audioEvent)
{
    for (size_t i = 0; i <= DP_PORT_COUNT; ++i) {
        std::string statePath = DP_PATH;
        std::string namePath = DP_PATH;

        if (i == 0) {
            statePath.append("/state");
            namePath.append("/name");
        } else {
            statePath.append(std::to_string(i) + "/state");
            namePath.append(std::to_string(i) + "/name");
        }

        int32_t ret = ReadAndScanDpState(statePath, audioEvent->eventType);
        if (ret != SUCCESS || audioEvent->eventType != PNP_EVENT_DEVICE_ADD) continue;

        ret = ReadAndScanDpName(namePath, audioEvent->name);
        if (ret != SUCCESS) continue;

        audioEvent->deviceType = PNP_DEVICE_DP_DEVICE;
        audioEvent->address = std::to_string(i);

        AUDIO_INFO_LOG("dp device reconnect when server start");
        return SUCCESS;
    }
    return ERROR;
}

int32_t AudioSocketThread::ReadAndScanDpState(const std::string &path, uint32_t &eventType)
{
    int8_t state = 0;

    FILE *fp = fopen(path.c_str(), "r");
    if (fp == nullptr) {
        AUDIO_ERR_LOG("audio open dp state node fail, %{public}d", errno);
        return HDF_ERR_INVALID_PARAM;
    }
    size_t ret = fread(&state, STATE_PATH_ITEM_SIZE, STATE_PATH_ITEM_SIZE, fp);
    if (ret == 0) {
        fclose(fp);
        AUDIO_ERR_LOG("audio read dp state node fail, %{public}d", errno);
        return ERROR;
    }
    int32_t closeRet = fclose(fp);
    if (closeRet != 0) {
        AUDIO_ERR_LOG("something wrong when fclose! err:%{public}d", errno);
    }

    if (state == '1') {
        eventType = PNP_EVENT_DEVICE_ADD;
    } else if (state == '0') {
        eventType = PNP_EVENT_DEVICE_REMOVE;
        return ERROR;
    } else {
        AUDIO_ERR_LOG("audio dp device [%{public}d]", eventType);
        return ERROR;
    }
    AUDIO_DEBUG_LOG("audio read dp state path: %{public}s, event type: %{public}d",
        path.c_str(), eventType);
    return SUCCESS;
}

int32_t AudioSocketThread::ReadAndScanDpName(const std::string &path, std::string &name)
{
    char deviceName[AUDIO_PNP_INFO_LEN_MAX];

    FILE *fp = fopen(path.c_str(), "r");
    if (fp == nullptr) {
        AUDIO_ERR_LOG("audio open dp name node fail, %{public}d", errno);
        return HDF_ERR_INVALID_PARAM;
    }
    size_t ret = fread(&deviceName, STATE_PATH_ITEM_SIZE, AUDIO_PNP_INFO_LEN_MAX, fp);
    if (ret == 0) {
        fclose(fp);
        AUDIO_ERR_LOG("audio read dp name node fail, %{public}d", errno);
        return ERROR;
    }
    int32_t closeRet = fclose(fp);
    if (closeRet != 0) {
        AUDIO_ERR_LOG("something wrong when fclose! err:%{public}d", errno);
    }
    AUDIO_DEBUG_LOG("audio read dp name path: %{public}s, name:%{public}s",
        path.c_str(), deviceName);

    name = deviceName;
    auto portPos = name.find(DEVICE_PORT);
    if (portPos == std::string::npos) {
        name.clear();
        AUDIO_ERR_LOG("audio read dp name node device port not find, %{public}d", errno);
        return ERROR;
    }
    name = name.substr(portPos + std::strlen(DEVICE_PORT));
    name.erase(name.find_last_not_of('\n') + 1);
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS