/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioTonePlayerTest"
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

#include "tone_player_impl.h"
#include "audio_common_log.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AudioStandard;

// usgae audio_toneplayer_test 3 1000000
constexpr int32_t REQ_ARG = 3;
constexpr int32_t MAX_SLEEP_TIME_US = 10000000;
int main(int argc, char *argv[])
{
    if (argc != REQ_ARG) {
        AUDIO_ERR_LOG("input parameter number error, argc: %{public}d", argc);
        return -1;
    }

    char *endptr = nullptr;
    int32_t toneType = atoi(argv[1]);
    long sleepTimeTemp = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0' || sleepTimeTemp <= 0 || sleepTimeTemp > MAX_SLEEP_TIME_US) {
        AUDIO_ERR_LOG("Invalid Time: %ld", sleepTimeTemp);
        return -1;
    }
    int32_t sleepTime = static_cast<int32_t>(sleepTimeTemp);
    AudioRendererInfo rendererInfo = {};
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_RINGTONE;
    rendererInfo.rendererFlags = 0;
    shared_ptr<TonePlayer> lToneGen = TonePlayer::Create(rendererInfo);
    AUDIO_INFO_LOG("Load Tone for %{public}d ", toneType);
    lToneGen->LoadTone((ToneType)toneType);
    AUDIO_INFO_LOG("start Tone.");
    lToneGen->StartTone();
    usleep(sleepTime);
    AUDIO_INFO_LOG("stop Tone.");
    lToneGen->StopTone();
    AUDIO_INFO_LOG("release Tone.");
    lToneGen->Release();
    return 0;
}
