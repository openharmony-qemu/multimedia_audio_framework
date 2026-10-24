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
    AUDIO_INFO_LOG("INPUT EVENT: code = 0x%{public}x, value = %{public}d\n", evt.code, evt.value);
    audioInputEvent_.eventType = (evt.value == 0) ? AUDIO_DEVICE_REMOVE : AUDIO_DEVICE_ADD;
    switch (evt.code) {
        case SW_HEADPHONE_INSERT:
            audioInputEvent_.deviceType = AUDIO_HEADPHONE;
            AUDIO_INFO_LOG("AUDIO_HEADPHONE");
            break;
        case SW_MICROPHONE_INSERT:
            audioInputEvent_.deviceType = AUDIO_HEADSET;
            AUDIO_INFO_LOG("AUDIO_HEADSET");
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