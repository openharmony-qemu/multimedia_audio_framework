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
#include "audio_errors.h"
#include "securec.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
AudioEvent AudioSocketThread::audioSocketEvent_ = {
    .eventType = AUDIO_EVENT_UNKNOWN,
    .deviceType = AUDIO_DEVICE_UNKNOWN,
};

AudioDevBusUsbDevice g_audioUsbDeviceList[AUDIO_UEVENT_USB_DEVICE_COUNT] = {};

bool AudioSocketThread::IsUpdatePnpDeviceState(AudioEvent *pnpDeviceEvent)
{
    if (pnpDeviceEvent->eventType == audioSocketEvent_.eventType &&
        pnpDeviceEvent->deviceType == audioSocketEvent_.deviceType &&
        pnpDeviceEvent->name == audioSocketEvent_.name &&
        pnpDeviceEvent->address == audioSocketEvent_.address) {
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
        close(socketFd);
        return ERROR;
    }

    if (setsockopt(socketFd, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on)) != 0) {
        AUDIO_ERR_LOG("setsockopt SO_PASSCRED failed, %{public}d", errno);
        close(socketFd);
        return ERROR;
    }

    if (::bind(socketFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        AUDIO_ERR_LOG("bind socket failed, %{public}d", errno);
        close(socketFd);
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

int32_t AudioSocketThread::SetAudioPnpServerEventValue(AudioEvent *audioEvent, struct AudioPnpUevent *audioPnpUevent)
{
    if (strncmp(audioPnpUevent->subSystem, UEVENT_SUBSYSTEM_SWITCH, strlen(UEVENT_SUBSYSTEM_SWITCH)) == 0) {
        static uint32_t h2wTypeLast = AUDIO_HEADSET;
        if (strncmp(audioPnpUevent->switchName, UEVENT_SWITCH_NAME_H2W, strlen(UEVENT_SWITCH_NAME_H2W)) != 0) {
            AUDIO_ERR_LOG("the switch name of 'h2w' not found!");
            return ERROR;
        }
        switch (audioPnpUevent->switchState[0]) {
            case REMOVE_AUDIO_DEVICE:
                audioEvent->eventType = AUDIO_DEVICE_REMOVE;
                audioEvent->deviceType = h2wTypeLast;
                break;
            case ADD_DEVICE_HEADSET:
            case ADD_DEVICE_HEADSET_WITHOUT_MIC:
                audioEvent->eventType = AUDIO_DEVICE_ADD;
                audioEvent->deviceType = AUDIO_HEADSET;
                break;
            case ADD_DEVICE_ADAPTER:
                audioEvent->eventType = AUDIO_DEVICE_ADD;
                audioEvent->deviceType = AUDIO_ADAPTER_DEVICE;
                break;
            default:
                audioEvent->eventType = AUDIO_DEVICE_ADD;
                audioEvent->deviceType = AUDIO_DEVICE_UNKNOWN;
                break;
        }
        h2wTypeLast = audioEvent->deviceType;
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
            audioEvent->eventType = AUDIO_DEVICE_REMOVE;
        } else if (strstr(audioPnpUevent->state, UEVENT_STATE_ANALOG_HS1) != NULL) {
            audioEvent->eventType = AUDIO_DEVICE_ADD;
        } else {
            return ERROR;
        }
        audioEvent->deviceType = AUDIO_HEADSET;
    }
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
        audioEvent.deviceType == AUDIO_HEADSET ? "headset" : "headphone",
        audioEvent.eventType == AUDIO_DEVICE_ADD ? "add" : "removed");

    if (!IsUpdatePnpDeviceState(&audioEvent)) {
        AUDIO_ERR_LOG("audio analog device[%{public}u] state[%{public}u] not need flush !", audioEvent.deviceType,
            audioEvent.eventType);
        return SUCCESS;
    }
    UpdatePnpDeviceState(&audioEvent);
    return SUCCESS;
}

int32_t AudioSocketThread::CheckUsbDesc(struct UsbDevice *usbDevice)
{
    if (usbDevice->descLen > USB_DES_LEN_MAX) {
        AUDIO_ERR_LOG("usbDevice->descLen is more than USB_DES_LEN_MAX");
        return HDF_ERR_INVALID_PARAM;
    }
    for (size_t len = 0; len < usbDevice->descLen;) {
        size_t descLen = usbDevice->desc[len];
        if (descLen == 0) {
            AUDIO_ERR_LOG("descLen is 0");
            return HDF_ERR_INVALID_PARAM;
        }

        if (descLen < USB_IF_DESC_LEN) {
            len += descLen;
            continue;
        }

        int32_t descType = usbDevice->desc[len + 1];
        if (descType != USB_AUDIO_DESC_TYPE) {
            len += descLen;
            continue;
        }

        /* According to the 1.0 and 2.0 usb standard protocols, the audio field corresponding to the interface
         * description type is: offset=1 interface descriptor type is 4; offset=5 interface class,audio is 1; offset=6
         * interface subclass,audio control is 1 */
        int32_t usbClass = usbDevice->desc[len + USB_IF_CLASS_OFFSET];
        int32_t subClass = usbDevice->desc[len + USB_IF_SUBCLASS_OFFSET];
        if (usbClass == USB_AUDIO_CLASS && subClass == USB_AUDIO_SUBCLASS_CTRL) {
            AUDIO_INFO_LOG(
                "descType %{public}d, usbClass %{public}d, subClass %{public}d", descType, usbClass, subClass);
            return AUDIO_DEVICE_ONLINE;
        }
        len += descLen;
    }
    return SUCCESS;
}

int32_t AudioSocketThread::ReadAndScanUsbDev(const char *devPath)
{
    FILE *fp = NULL;
    struct UsbDevice usbDevice;
    size_t len;
    errno_t error;
    uint32_t tryTime = 0;
    char realpathRes[PATH_MAX + 1] = {'\0'};

    if (devPath == NULL) {
        AUDIO_ERR_LOG("audio devPath null");
        return ERROR;
    }

    while (tryTime < AUDIO_DEVICE_WAIT_TRY_TIME) {
        if (realpath(devPath, realpathRes) != NULL || (strlen(devPath) > PATH_MAX)) {
            AUDIO_INFO_LOG("audio try[%{public}d] realpath fail[%{public}d] realpathRes [%{public}s]",
                tryTime, errno, realpathRes);
            break;
        }
        tryTime++;
        OsalMSleep(AUDIO_DEVICE_WAIT_ONLINE);
    }

    fp = fopen(realpathRes, "r");
    if (fp == NULL) {
        AUDIO_ERR_LOG("audio realpath open fail[%{public}d]", errno);
        return ERROR;
    }

    len = fread(usbDevice.desc, 1, sizeof(usbDevice.desc) - 1, fp);
    if (len == 0) {
        AUDIO_ERR_LOG("audio realpath read fail");
        fclose(fp);
        return ERROR;
    }
    fclose(fp);

    error = strncpy_s((char *)usbDevice.devName, sizeof(usbDevice.devName), realpathRes,
        sizeof(usbDevice.devName) - 1);
    if (error != EOK) {
        AUDIO_ERR_LOG("audio realpath strncpy fail");
        return ERROR;
    }

    usbDevice.descLen = len;
    return CheckUsbDesc(&usbDevice);
}

bool AudioSocketThread::FindAudioUsbDevice(const char *devName)
{
    if (strlen(devName) > USB_DEV_NAME_LEN_MAX - 1) {
        AUDIO_ERR_LOG("find usb audio device name exceed max len");
        return false;
    }

    for (uint32_t count = 0; count < AUDIO_UEVENT_USB_DEVICE_COUNT; count++) {
        if (g_audioUsbDeviceList[count].isUsed &&
            (strncmp((char *)g_audioUsbDeviceList[count].devName, devName, strlen(devName)) == EOK)) {
            return true;
        }
    }
    return false;
}

bool AudioSocketThread::AddAudioUsbDevice(const char *devName)
{
    if (strlen(devName) > USB_DEV_NAME_LEN_MAX - 1) {
        AUDIO_ERR_LOG("add usb audio device name exceed max len");
        return false;
    }

    if (FindAudioUsbDevice(devName)) {
        AUDIO_ERR_LOG("find usb audio device name[%{public}s]", devName);
        return true;
    }

    for (uint32_t count = 0; count < AUDIO_UEVENT_USB_DEVICE_COUNT; count++) {
        if (g_audioUsbDeviceList[count].isUsed) {
            continue;
        }
        if (strncpy_s((char *)g_audioUsbDeviceList[count].devName, USB_DEV_NAME_LEN_MAX, devName, strlen(devName))
            != EOK) {
            AUDIO_ERR_LOG("add usb audio device name fail");
            return false;
        }
        g_audioUsbDeviceList[count].isUsed = true;
        return true;
    }
    AUDIO_ERR_LOG("add usb audio device name fail");
    return false;
}

bool AudioSocketThread::CheckAudioUsbDevice(const char *devName)
{
    int32_t state = 0;
    int32_t len;
    char subDir[USB_DEV_NAME_LEN_MAX] = {0};

    if (*devName == '\0') {
        return false;
    }
    len = snprintf_s(subDir, USB_DEV_NAME_LEN_MAX, USB_DEV_NAME_LEN_MAX - 1, "/dev/" "%s", devName);
    if (len < 0) {
        AUDIO_ERR_LOG("audio snprintf dev dir fail");
        return false;
    }
    AUDIO_INFO_LOG("CheckAudioUsbDevice: devName:%{public}s subDir:%{public}s len:%{public}d", devName, subDir, len);

    state = ReadAndScanUsbDev(subDir);
    if ((state == AUDIO_DEVICE_ONLINE) && AddAudioUsbDevice(devName)) {
        return true;
    }
    return false;
}

bool AudioSocketThread::DeleteAudioUsbDevice(const char *devName)
{
    if (strlen(devName) > USB_DEV_NAME_LEN_MAX - 1) {
        AUDIO_ERR_LOG("delete usb audio device name exceed max len");
        return false;
    }

    for (uint32_t count = 0; count < AUDIO_UEVENT_USB_DEVICE_COUNT; count++) {
        if (g_audioUsbDeviceList[count].isUsed &&
            strncmp((char *)g_audioUsbDeviceList[count].devName, devName, strlen(devName)) == EOK) {
            g_audioUsbDeviceList[count].isUsed = false;
            AUDIO_INFO_LOG("delete usb audio device name[%{public}s]", devName);
            return true;
        }
    }

    return false;
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
        audioEvent.eventType = AUDIO_DEVICE_ADD;
    } else if (strcmp(audioPnpUevent->switchState, "0") == 0) {
        audioEvent.eventType = AUDIO_DEVICE_REMOVE;
    } else {
        AUDIO_ERR_LOG("audio dp device [%{public}d]", audioEvent.eventType);
        return ERROR;
    }
    audioEvent.deviceType = AUDIO_DP_DEVICE;

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
    AUDIO_INFO_LOG("audio dp device [%{public}s]", audioEvent.eventType == AUDIO_DEVICE_ADD ? "add" : "removed");

    if (!IsUpdatePnpDeviceState(&audioEvent)) {
        AUDIO_ERR_LOG("audio usb device[%{public}u] state[%{public}u] not need flush !", audioEvent.deviceType,
            audioEvent.eventType);
        return SUCCESS;
    }
    UpdatePnpDeviceState(&audioEvent);
    return SUCCESS;
}

int32_t AudioSocketThread::AudioUsbHeadsetDetectDevice(struct AudioPnpUevent *audioPnpUevent)
{
    AudioEvent audioEvent = {0};

    if (audioPnpUevent == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (audioPnpUevent->action == NULL || audioPnpUevent->devName == NULL || audioPnpUevent->subSystem == NULL ||
        audioPnpUevent->devType == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    if ((strcmp(audioPnpUevent->subSystem, UEVENT_SUBSYSTEM_USB) != 0) ||
        (strcmp(audioPnpUevent->devType, UEVENT_SUBSYSTEM_USB_DEVICE) != 0) ||
        (strstr(audioPnpUevent->devName, BUS_USB_DIR) == NULL)) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (strcmp(audioPnpUevent->action, UEVENT_ACTION_ADD) == 0) {
        if (!CheckAudioUsbDevice(audioPnpUevent->devName)) {
            return HDF_ERR_INVALID_PARAM;
        }
        audioEvent.eventType = AUDIO_DEVICE_ADD;
    } else if (strcmp(audioPnpUevent->action, UEVENT_ACTION_REMOVE) == 0) {
        if (!DeleteAudioUsbDevice(audioPnpUevent->devName)) {
            return HDF_ERR_INVALID_PARAM;
        }
        audioEvent.eventType = AUDIO_DEVICE_REMOVE;
    } else {
        return ERROR;
    }

    audioEvent.deviceType = AUDIO_USB_HEADSET;
    AUDIO_DEBUG_LOG("audio usb headset [%{public}s]", audioEvent.eventType == AUDIO_DEVICE_ADD ? "add" : "removed");

    audioEvent.name = audioPnpUevent->name;
    audioEvent.address = audioPnpUevent->devName;

    if (!IsUpdatePnpDeviceState(&audioEvent)) {
        AUDIO_ERR_LOG("audio usb device[%{public}u] state[%{public}u] not need flush !", audioEvent.deviceType,
            audioEvent.eventType);
        return SUCCESS;
    }
    UpdatePnpDeviceState(&audioEvent);
    return SUCCESS;
}

bool AudioSocketThread::AudioPnpUeventParse(const char *msg, const ssize_t strLength)
{
    struct AudioPnpUevent audioPnpUevent = {"", "", "", "", "", "", "", "", ""};

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
        AUDIO_DEBUG_LOG("Param msgTmp:[%{public}s] len:[%{public}zu]", msgTmp, strlen(msgTmp));
        const char *arrStrTmp[UEVENT_ARR_SIZE] = {
            UEVENT_ACTION, UEVENT_DEV_NAME, UEVENT_NAME, UEVENT_STATE, UEVENT_DEVTYPE,
            UEVENT_SUBSYSTEM, UEVENT_SWITCH_NAME, UEVENT_SWITCH_STATE, UEVENT_HDI_NAME
        };
        const char **arrVarTmp[UEVENT_ARR_SIZE] = {
            &audioPnpUevent.action, &audioPnpUevent.devName, &audioPnpUevent.name,
            &audioPnpUevent.state, &audioPnpUevent.devType, &audioPnpUevent.subSystem,
            &audioPnpUevent.switchName, &audioPnpUevent.switchState, &audioPnpUevent.hidName
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

    if (AudioAnalogHeadsetDetectDevice(&audioPnpUevent) == SUCCESS) {
        return true;
    }
    if (AudioUsbHeadsetDetectDevice(&audioPnpUevent) == SUCCESS) {
        return true;
    }
    if (AudioDpDetectDevice(&audioPnpUevent) == SUCCESS) {
        return true;
    }

    return false;
}

int32_t AudioSocketThread::DetectAnalogHeadsetState(AudioEvent *audioEvent)
{
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

    if (state == '0') {
        audioEvent->eventType = AUDIO_DEVICE_REMOVE;
        audioEvent->deviceType = AUDIO_HEADSET;
    } else {
        audioEvent->eventType = AUDIO_DEVICE_ADD;
        audioEvent->deviceType = AUDIO_HEADSET;
    }

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

inline bool AudioSocketThread::IsBadName(const char *name)
{
    if (*name == '\0') {
        AUDIO_ERR_LOG("name is null");
        return true;
    }

    while (*name != '\0') {
        if (isdigit(*name++) == 0) {
            return true;
        }
    }

    return false;
}

int32_t AudioSocketThread::ScanUsbBusSubDir(const char *subDir)
{
    int32_t len;
    DIR *devDir = NULL;
    dirent *dirEnt = NULL;

    char devName[USB_DEV_NAME_LEN_MAX] = {0};

    devDir = opendir(subDir);
    if (devDir == NULL) {
        AUDIO_ERR_LOG("open usb sub dir failed");
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t state = SUCCESS;
    while (((dirEnt = readdir(devDir)) != NULL) && (state == SUCCESS)) {
        if (IsBadName(dirEnt->d_name)) {
            continue;
        }

        len = snprintf_s(devName, USB_DEV_NAME_LEN_MAX, USB_DEV_NAME_LEN_MAX - 1, "%s/%s", subDir,
            dirEnt->d_name);
        if (len < 0) {
            AUDIO_ERR_LOG("audio snprintf dev dir fail");
            state = ERROR;
            break;
        }

        AUDIO_DEBUG_LOG("audio usb dir[%{public}s]", devName);
        state = ReadAndScanUsbDev(devName);
        if (state == AUDIO_DEVICE_ONLINE) {
            char *subDevName = devName + strlen("/dev/");
            AUDIO_ERR_LOG("audio sub dev dir=[%{public}s]", subDevName);
            if (AddAudioUsbDevice(subDevName)) {
                AUDIO_ERR_LOG("audio add usb audio device success");
                break;
            }
        }
    }

    closedir(devDir);
    return state;
}

int32_t AudioSocketThread::DetectUsbHeadsetState(AudioEvent *audioEvent)
{
    int32_t len;
    DIR *busDir = NULL;
    dirent *dirEnt = NULL;

    char subDir[USB_DEV_NAME_LEN_MAX] = {0};

    busDir = opendir(DEV_BUS_USB_DIR);
    if (busDir == NULL) {
        AUDIO_ERR_LOG("open usb dir failed");
        return HDF_ERR_INVALID_PARAM;
    }

    int32_t state = SUCCESS;
    while (((dirEnt = readdir(busDir)) != NULL) && (state == SUCCESS)) {
        if (IsBadName(dirEnt->d_name)) {
            continue;
        }

        len = snprintf_s(subDir, USB_DEV_NAME_LEN_MAX, USB_DEV_NAME_LEN_MAX - 1, DEV_BUS_USB_DIR "/%s",
            dirEnt->d_name);
        if (len < 0) {
            AUDIO_ERR_LOG("audio snprintf dev dir fail");
            break;
        }
        state = ScanUsbBusSubDir(subDir);
        if (state == AUDIO_DEVICE_ONLINE) {
            audioEvent->eventType = AUDIO_DEVICE_ADD;
            audioEvent->deviceType = AUDIO_USB_HEADSET;
            closedir(busDir);
            return SUCCESS;
        }
    }

    closedir(busDir);
    return ERROR;
}
} // namespace AudioStandard
} // namespace OHOS