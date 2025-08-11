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

namespace OHOS {
namespace AudioStandard {
using namespace std;
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
    audioSocketEvent_.eventType = pnpDeviceEvent->eventType;
    audioSocketEvent_.deviceType = pnpDeviceEvent->deviceType;
    audioSocketEvent_.name = pnpDeviceEvent->name;
    audioSocketEvent_.address = pnpDeviceEvent->address;
    audioSocketEvent_.anahsName = pnpDeviceEvent->anahsName;
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

int32_t AudioSocketThread::SetAudioAnahsEventValue(AudioEvent *audioEvent, struct AudioPnpUevent *audioPnpUevent)
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
            break;
        case ADD_DEVICE_HEADSET:
            audioEvent->eventType = PNP_EVENT_DEVICE_ADD;
            audioEvent->deviceType = PNP_DEVICE_HEADSET;
            break;
        case ADD_DEVICE_HEADSET_WITHOUT_MIC:
            audioEvent->eventType = PNP_EVENT_DEVICE_ADD;
            audioEvent->deviceType = PNP_DEVICE_HEADPHONE;
            break;
        case ADD_DEVICE_ADAPTER:
            audioEvent->eventType = PNP_EVENT_DEVICE_ADD;
            audioEvent->deviceType = PNP_DEVICE_ADAPTER_DEVICE;
            break;
        default:
            audioEvent->eventType = PNP_EVENT_DEVICE_ADD;
            audioEvent->deviceType = PNP_DEVICE_UNKNOWN;
            break;
    }
    h2wTypeLast = audioEvent->deviceType;
}

int32_t AudioSocketThread::SetAudioPnpServerEventValue(AudioEvent *audioEvent, struct AudioPnpUevent *audioPnpUevent)
{
    if (audioEvent == nullptr) {
        AUDIO_ERR_LOG("audioEvent is null!");
        return ERROR;
    }
    if (strncmp(audioPnpUevent->subSystem, UEVENT_SUBSYSTEM_SWITCH, strlen(UEVENT_SUBSYSTEM_SWITCH)) == 0) {
        if (strncmp(audioPnpUevent->switchName, UEVENT_SWITCH_NAME_H2W, strlen(UEVENT_SWITCH_NAME_H2W)) != 0) {
            AUDIO_ERR_LOG("the switch name of 'h2w' not found!");
            return ERROR;
        }
        AudioSocketThread::SetAudioPnpUevent(audioEvent, audioPnpUevent->switchState[0]);
        audioEvent->name = audioPnpUevent->name;
        audioEvent->address = audioPnpUevent->devName;
    } else {
        if (strncmp(audioPnpUevent->action, UEVENT_ACTION_CHANGE, strlen(UEVENT_ACTION_CHANGE)) != 0) {
            return ERROR;
        }
        if (strstr(audioPnpUevent->name, UEVENT_NAME_HEADSET) == NULL) {
            return ERROR;
        }
        if (strncmp(audioPnpUevent->devType, UEVENT_TYPE_EXTCON, strlen(UEVENT_TYPE_EXTCON)) != 0) {
            return ERROR;
        }
        if (strstr(audioPnpUevent->state, UEVENT_STATE_ANALOG_HS0) != NULL) {
            audioEvent->eventType = PNP_EVENT_DEVICE_REMOVE;
        } else if (strstr(audioPnpUevent->state, UEVENT_STATE_ANALOG_HS1) != NULL) {
            audioEvent->eventType = PNP_EVENT_DEVICE_ADD;
        } else {
            return ERROR;
        }
        audioEvent->deviceType = PNP_DEVICE_HEADSET;
    }
    return SUCCESS;
}

int32_t AudioSocketThread::AudioAnahsDetectDevice(struct AudioPnpUevent *audioPnpUevent)
{
    AudioEvent audioEvent;
    if (audioPnpUevent == NULL) {
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
    audioSocketEvent_.anahsName = audioEvent.anahsName;
    return SUCCESS;
}

int32_t AudioSocketThread::AudioAnalogHeadsetDetectDevice(struct AudioPnpUevent *audioPnpUevent)
{
    AudioEvent audioEvent;
    if (audioPnpUevent == NULL) {
        AUDIO_ERR_LOG("audioPnpUevent is null!");
        return HDF_ERR_INVALID_PARAM;
    }
 
    if (SetAudioPnpServerEventValue(&audioEvent, audioPnpUevent) != SUCCESS) {
        return ERROR;
    }
    AUDIO_DEBUG_LOG("audio analog [%{public}s][%{public}s]",
        audioEvent.deviceType == PNP_DEVICE_HEADSET ? "headset" : "headphone",
        audioEvent.eventType == PNP_EVENT_DEVICE_ADD ? "add" : "removed");

    if (!IsUpdatePnpDeviceState(&audioEvent)) {
        AUDIO_ERR_LOG("audio analog device[%{public}u] state[%{public}u] not need flush !", audioEvent.deviceType,
            audioEvent.eventType);
        return SUCCESS;
    }
    UpdatePnpDeviceState(&audioEvent);
    return SUCCESS;
}

int32_t AudioSocketThread::AudioNnDetectDevice(struct AudioPnpUevent *audioPnpUevent)
{
    if (audioPnpUevent == NULL) {
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

int32_t AudioSocketThread::AudioDpDetectDevice(struct AudioPnpUevent *audioPnpUevent)
{
    AudioEvent audioEvent = {0};
    if (audioPnpUevent == NULL) {
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

int32_t AudioSocketThread::AudioMicBlockDevice(struct AudioPnpUevent *audioPnpUevent)
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

int32_t AudioSocketThread::AudioHDMIDetectDevice(struct AudioPnpUevent *audioPnpUevent)
{
    AudioEvent audioEvent = {0};
    if (audioPnpUevent == NULL) {
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
            UEVENT_ANAHS
        };
        const char **arrVarTmp[UEVENT_ARR_SIZE] = {
            &audioPnpUevent.action, &audioPnpUevent.devName, &audioPnpUevent.name,
            &audioPnpUevent.state, &audioPnpUevent.devType, &audioPnpUevent.subSystem,
            &audioPnpUevent.switchName, &audioPnpUevent.switchState, &audioPnpUevent.hidName,
            &audioPnpUevent.anahsName
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

    if ((AudioAnalogHeadsetDetectDevice(&audioPnpUevent) == SUCCESS) ||
        (AudioHDMIDetectDevice(&audioPnpUevent) == SUCCESS) ||
        (AudioDpDetectDevice(&audioPnpUevent) == SUCCESS) ||
        (AudioAnahsDetectDevice(&audioPnpUevent) == SUCCESS) ||
        (AudioNnDetectDevice(&audioPnpUevent) == SUCCESS) ||
        (AudioMicBlockDevice(&audioPnpUevent) == SUCCESS)) {
        return true;
    }

    return false;
}

int32_t AudioSocketThread::DetectAnalogHeadsetState(AudioEvent *audioEvent)
{
    if (audioEvent == nullptr) {
        AUDIO_ERR_LOG("audioEvent is null!");
        return ERROR;
    }
    int8_t state = 0;
    FILE *fp = fopen(SWITCH_STATE_PATH, "r");
    if (fp == NULL) {
        AUDIO_ERR_LOG("audio open switch state node fail, %{public}d", errno);
        return HDF_ERR_INVALID_PARAM;
    }

    size_t ret = fread(&state, STATE_PATH_ITEM_SIZE, STATE_PATH_ITEM_SIZE, fp);
    if (ret == 0) {
        fclose(fp);
        AUDIO_ERR_LOG("audio read switch state node fail, %{public}d", errno);
        return ERROR;
    }

    AudioSocketThread::SetAudioPnpUevent(audioEvent, state);

    fclose(fp);
    return SUCCESS;
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