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
#define LOG_TAG "AudioInputThread"
#endif

#include "audio_input_thread.h"

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include "audio_errors.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
int32_t g_inputDevCnt = 0;
pollfd g_fdSets[INPUT_EVT_MAX_CNT];

AudioEvent AudioInputThread::audioInputEvent_ = {
    .eventType = AUDIO_EVENT_UNKNOWN,
    .deviceType = AUDIO_DEVICE_UNKNOWN,
};

int32_t AudioInputThread::AudioAnalogHeadsetDeviceCheck(input_event evt)
{
    audioInputEvent_.eventType = (evt.value == 0) ? AUDIO_DEVICE_REMOVE : AUDIO_DEVICE_ADD;
    switch (evt.code) {
        case SW_HEADPHONE_INSERT:
            audioInputEvent_.deviceType = AUDIO_HEADPHONE;
            break;
        case SW_MICROPHONE_INSERT:
            audioInputEvent_.deviceType = AUDIO_HEADSET;
            break;
        case SW_LINEOUT_INSERT:
            audioInputEvent_.deviceType = AUDIO_LINEOUT;
            break;
        default: // SW_JACK_PHYSICAL_INSERT = 0x7, SW_LINEIN_INSERT = 0xd and other.
            AUDIO_ERR_LOG("not surpport code = 0x%{public}x\n", evt.code);
            return ERROR;
    }
    return SUCCESS;
}

int32_t AudioInputThread::AudioPnpInputPollAndRead()
{
    int32_t num;
    int32_t ret;
    int32_t inputNum = g_inputDevCnt;
    input_event evt;

    ret = poll(g_fdSets, (nfds_t)inputNum, -1);
    if (ret < 0) {
        AUDIO_ERR_LOG("[poll] failed, %{public}d", errno);
        return ERROR;
    }

    for (num = 0; num < inputNum; num++) {
        if ((uint32_t)g_fdSets[num].revents & POLLIN) {
            if (read(g_fdSets[num].fd, (void *)&evt, sizeof(evt)) < 0) {
                AUDIO_ERR_LOG("[read] failed, %{public}d", errno);
                return ERROR;
            }
            switch (evt.type) {
                case EV_SYN:
                    AUDIO_DEBUG_LOG("evt.type = EV_SYN code = 0x%{public}d, value = %{public}d",
                        evt.code, evt.value);
                    break;
                case EV_SW:
                    AUDIO_DEBUG_LOG("evt.type = EV_SW5, code = 0x%{public}d, value = %{public}d\n",
                        evt.code, evt.value);
                    AudioAnalogHeadsetDeviceCheck(evt);
                    break;
                case EV_KEY:
                    AUDIO_DEBUG_LOG("evt.type = EV_KEY, code = 0x%{public}x, value = %{public}d.",
                        evt.code, evt.value);
                    break;
                case EV_REL: // mouse move event.
                case EV_MSC:
                default:
                    AUDIO_DEBUG_LOG("evt.type = EV_REL or EV_MSC code = 0x%{public}d, value = %{public}d",
                        evt.code, evt.value);
                    break;
            }
        }
    }
    return SUCCESS;
}

int32_t AudioInputThread::AudioPnpInputOpen()
{
    int32_t num;
    int32_t fdNum = 0;
    const char *devices[INPUT_EVT_MAX_CNT] = {
        "/dev/input/event1",
        "/dev/input/event2",
        "/dev/input/event3",
        "/dev/input/event4"
    };

    for (num = 0; num < INPUT_EVT_MAX_CNT; num++) {
        g_fdSets[fdNum].fd = open(devices[num], O_RDONLY);
        if (g_fdSets[fdNum].fd < 0) {
            AUDIO_ERR_LOG("[open] %{public}s failed!, fd %{public}d, errno: %{public}d",
                devices[num], g_fdSets[fdNum].fd, errno);
            continue;
        }
        g_fdSets[fdNum].events = POLLIN;
        fdNum++;
    }
    g_inputDevCnt = fdNum;

    return (fdNum == 0) ? ERROR : SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS