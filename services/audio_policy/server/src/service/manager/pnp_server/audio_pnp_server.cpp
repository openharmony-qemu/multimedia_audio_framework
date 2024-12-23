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
#define LOG_TAG "AudioPnpServer"
#endif

#include "audio_pnp_server.h"

#include <poll.h>
#include "securec.h"
#include "osal_time.h"
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_input_thread.h"
#include "audio_policy_log.h"
#include "audio_socket_thread.h"

using namespace std;
namespace OHOS {
namespace AudioStandard {
static bool g_socketRunThread = false;
static bool g_inputRunThread = false;
#ifdef AUDIO_DOUBLE_PNP_DETECT
AudioEvent g_usbHeadset = {0};
#endif

static std::string GetAudioEventInfo(const AudioEvent audioEvent)
{
    int32_t ret;
    char event[AUDIO_PNP_INFO_LEN_MAX] = {0};
    if (audioEvent.eventType == PNP_EVENT_UNKNOWN || audioEvent.deviceType == PNP_DEVICE_UNKNOWN) {
        AUDIO_ERR_LOG("audio event is not updated");
        return event;
    }
    ret = snprintf_s(event, AUDIO_PNP_INFO_LEN_MAX, AUDIO_PNP_INFO_LEN_MAX - 1,
        "EVENT_TYPE=%u;DEVICE_TYPE=%u;EVENT_NAME=%s;DEVICE_ADDRESS=%s",
        audioEvent.eventType, audioEvent.deviceType, audioEvent.name.c_str(), audioEvent.address.c_str());
    if (ret < 0) {
        AUDIO_ERR_LOG("snprintf_s failed");
        return event;
    }

    return event;
}

AudioPnpServer::~AudioPnpServer()
{
    AUDIO_INFO_LOG("~AudioPnpServer");
    g_socketRunThread = false;
    g_inputRunThread = false;

    if (socketThread_ && socketThread_->joinable()) {
        socketThread_->detach();
    }

    if (inputThread_ && inputThread_->joinable()) {
        inputThread_->detach();
    }
}

bool AudioPnpServer::init(void)
{
    AUDIO_INFO_LOG("Init");
    g_socketRunThread = true;
    g_inputRunThread = true;

    socketThread_ = std::make_unique<std::thread>([this] { this->OpenAndReadWithSocket(); });
    pthread_setname_np(socketThread_->native_handle(), "OS_SocketEvent");
    inputThread_ = std::make_unique<std::thread>([this] { this->OpenAndReadInput(); });
    pthread_setname_np(inputThread_->native_handle(), "OS_InputEvent");
    return true;
}

int32_t AudioPnpServer::RegisterPnpStatusListener(std::shared_ptr<AudioPnpDeviceChangeCallback> callback)
{
    AUDIO_INFO_LOG("Enter");
    {
        std::lock_guard<std::mutex> lock(pnpMutex_);
        pnpCallback_ = callback;
    }

    DetectAudioDevice();
    return SUCCESS;
}

int32_t AudioPnpServer::UnRegisterPnpStatusListener()
{
    std::lock_guard<std::mutex> lock(pnpMutex_);
    pnpCallback_ = nullptr;
    return SUCCESS;
}

void AudioPnpServer::OnPnpDeviceStatusChanged(const std::string &info)
{
    std::lock_guard<std::mutex> lock(pnpMutex_);
    if (pnpCallback_ != nullptr) {
        pnpCallback_->OnPnpDeviceStatusChanged(info);
    }
}

void MicrophoneBlocked::OnMicrophoneBlocked(const std::string &info, AudioPnpServer &audioPnpServer)
{
    std::lock_guard<std::mutex> lock(audioPnpServer.pnpMutex_);
    if (audioPnpServer.pnpCallback_ != nullptr) {
        audioPnpServer.pnpCallback_->OnMicrophoneBlocked(info);
    }
}

void AudioPnpServer::OpenAndReadInput()
{
    int32_t ret = -1;
    int32_t status = AudioInputThread::AudioPnpInputOpen();
    if (status != SUCCESS) {
        return;
    }

    do {
        ret = AudioInputThread::AudioPnpInputPollAndRead();
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("[AudioPnpInputPollAndRead] failed");
            return;
        }
        eventInfo_ = GetAudioEventInfo(AudioInputThread::audioInputEvent_);
        CHECK_AND_RETURN_LOG(!eventInfo_.empty(), "invalid input info");
        if (AudioSocketThread::audioSocketEvent_.eventType == PNP_EVENT_MIC_BLOCKED ||
            AudioSocketThread::audioSocketEvent_.eventType == PNP_EVENT_MIC_UNBLOCKED) {
            MicrophoneBlocked::GetInstance().OnMicrophoneBlocked(eventInfo_, GetAudioPnpServer());
        } else {
            OnPnpDeviceStatusChanged(eventInfo_);
        }
    } while (g_inputRunThread);
    return;
}

void AudioPnpServer::OpenAndReadWithSocket()
{
    ssize_t rcvLen;
    int32_t socketFd = -1;
    struct pollfd fd;
    char msg[UEVENT_MSG_LEN + 1] = {0};

    int32_t ret = AudioSocketThread::AudioPnpUeventOpen(&socketFd);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("open audio pnp socket failed");
        return;
    }

    fd.fd = socketFd;
    fd.events = POLLIN | POLLERR;
    fd.revents = 0;

    while (g_socketRunThread) {
        if (poll(&fd, 1, -1) <= 0) {
            AUDIO_ERR_LOG("audio event poll fail %{public}d", errno);
            OsalMSleep(UEVENT_POLL_WAIT_TIME);
            continue;
        }

        if (((uint32_t)fd.revents & POLLIN) == POLLIN) {
            memset_s(&msg, sizeof(msg), 0, sizeof(msg));
            rcvLen = AudioSocketThread::AudioPnpReadUeventMsg(socketFd, msg, UEVENT_MSG_LEN);
            if (rcvLen <= 0) {
                continue;
            }
            bool status = AudioSocketThread::AudioPnpUeventParse(msg, rcvLen);
            if (!status) {
                continue;
            }
            eventInfo_ = GetAudioEventInfo(AudioSocketThread::audioSocketEvent_);
            CHECK_AND_RETURN_LOG(!eventInfo_.empty(), "invalid socket info");
            if (AudioSocketThread::audioSocketEvent_.eventType == PNP_EVENT_MIC_BLOCKED ||
                AudioSocketThread::audioSocketEvent_.eventType == PNP_EVENT_MIC_UNBLOCKED) {
                MicrophoneBlocked::GetInstance().OnMicrophoneBlocked(eventInfo_, GetAudioPnpServer());
            } else {
                OnPnpDeviceStatusChanged(eventInfo_);
            }
        }
    }
    CloseFd(socketFd);
    return;
}

#ifdef AUDIO_DOUBLE_PNP_DETECT
void AudioPnpServer::UpdateUsbHeadset()
{
    char pnpInfo[AUDIO_EVENT_INFO_LEN_MAX] = {0};
    int32_t ret;
    bool status = AudioSocketThread::IsUpdatePnpDeviceState(&g_usbHeadset);
    if (!status) {
        AUDIO_ERR_LOG("audio first pnp device[%{public}u] state[%{public}u] not need flush !",
            g_usbHeadset.deviceType, g_usbHeadset.eventType);
        return;
    }
    ret = snprintf_s(pnpInfo, AUDIO_EVENT_INFO_LEN_MAX, AUDIO_EVENT_INFO_LEN_MAX - 1, "EVENT_TYPE=%u;DEVICE_TYPE=%u",
        g_usbHeadset.eventType, g_usbHeadset.deviceType);
    if (ret < 0) {
        AUDIO_ERR_LOG("snprintf_s fail!");
        return;
    }
    AUDIO_DEBUG_LOG("g_usbHeadset.eventType [%{public}u], g_usbHeadset.deviceType [%{public}u]",
        g_usbHeadset.eventType, g_usbHeadset.deviceType);
    AudioSocketThread::UpdatePnpDeviceState(&g_usbHeadset);
    return;
}
#endif

void AudioPnpServer::DetectAudioDevice()
{
    AUDIO_INFO_LOG("Enter");
    int32_t ret;
    AudioEvent audioEvent = {0};

    OsalMSleep(AUDIO_DEVICE_WAIT_USB_ONLINE);
    ret = AudioSocketThread::DetectAnalogHeadsetState(&audioEvent);
    if ((ret == SUCCESS) && (audioEvent.eventType == AUDIO_DEVICE_ADD)) {
        AUDIO_INFO_LOG("audio detect analog headset");
        AudioSocketThread::UpdateDeviceState(audioEvent);

        eventInfo_ = GetAudioEventInfo(AudioSocketThread::audioSocketEvent_);
        CHECK_AND_RETURN_LOG(!eventInfo_.empty(), "invalid detect info");
        OnPnpDeviceStatusChanged(eventInfo_);
#ifndef AUDIO_DOUBLE_PNP_DETECT
        return;
#endif
    }
#ifdef AUDIO_DOUBLE_PNP_DETECT
    ret = AudioSocketThread::DetectUsbHeadsetState(&g_usbHeadset);
    if ((ret == SUCCESS) && (g_usbHeadset.eventType == AUDIO_DEVICE_ADD)) {
        AUDIO_INFO_LOG("audio detect usb headset");
        std::unique_ptr<std::thread> bootupThread_ = nullptr;
        bootupThread_ = std::make_unique<std::thread>([this] { this->UpdateUsbHeadset(); });
        pthread_setname_np(bootupThread_->native_handle(), "OS_BootupEvent");
        OsalMSleep(AUDIO_DEVICE_WAIT_USB_EVENT_UPDATE);
        if (AudioSocketThread::audioSocketEvent_.eventType != AUDIO_EVENT_UNKNOWN &&
            AudioSocketThread::audioSocketEvent_.deviceType != AUDIO_DEVICE_UNKNOWN) {
            eventInfo_ = GetAudioEventInfo(AudioSocketThread::audioSocketEvent_);
            CHECK_AND_RETURN_LOG(!eventInfo_.empty(), "invalid detect info");
            OnPnpDeviceStatusChanged(eventInfo_);
        }
        if (bootupThread_ && bootupThread_->joinable()) {
            bootupThread_->join();
        }
    }
    return;
#else
    audioEvent.eventType = AUDIO_EVENT_UNKNOWN;
    audioEvent.deviceType = AUDIO_DEVICE_UNKNOWN;
    ret = AudioSocketThread::DetectUsbHeadsetState(&audioEvent);
    if ((ret == SUCCESS) && (audioEvent.eventType == AUDIO_DEVICE_ADD)) {
        AUDIO_INFO_LOG("audio detect usb headset");
        AudioSocketThread::UpdateDeviceState(audioEvent);
        eventInfo_ = GetAudioEventInfo(AudioSocketThread::audioSocketEvent_);
        CHECK_AND_RETURN_LOG(!eventInfo_.empty(), "invalid detect info");
        OnPnpDeviceStatusChanged(eventInfo_);
    }
#endif
    AUDIO_INFO_LOG("Done");
}

void AudioPnpServer::StopPnpServer()
{
    g_socketRunThread = false;
    g_inputRunThread = false;
    if (socketThread_ && socketThread_->joinable()) {
        socketThread_->detach();
    }

    if (inputThread_ && inputThread_->joinable()) {
        inputThread_->detach();
    }
}
} // namespace AudioStandard
} // namespace OHOS