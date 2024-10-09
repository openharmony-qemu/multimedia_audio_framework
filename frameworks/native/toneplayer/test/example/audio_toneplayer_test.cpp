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

constexpr int32_t REQ_ARG = 2;
int main(int argc, char *argv[])
{
    if (argc != REQ_ARG) {
        AUDIO_ERR_LOG("input parameter number error, argc: %{public}d", argc);
        return -1;
    }

    int32_t toneType = atoi(argv[1]);
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererInfo.rendererFlags = 0;
    shared_ptr<TonePlayer> lToneGen = TonePlayer::Create(rendererInfo);
    AUDIO_INFO_LOG("Load Tone for %{public}d ", toneType);
    lToneGen->LoadTone((ToneType)toneType);
    AUDIO_INFO_LOG("start Tone.");
    lToneGen->StartTone();
    usleep(30000); // 30ms sleep time
    AUDIO_INFO_LOG("stop Tone.");
    lToneGen->StopTone();
    AUDIO_INFO_LOG("release Tone.");
    lToneGen->Release();
    return 0;
}
