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
#define LOG_TAG "AudioCapturerProxyObj"
#endif

#include "audio_capturer_proxy_obj.h"
#include "audio_capturer_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
void AudioCapturerProxyObj::SaveCapturerObj(const AudioCapturer *capturerObj)
{
    capturer = capturerObj;
}


void AudioCapturerProxyObj::PausedStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal)
{
}

void AudioCapturerProxyObj::ResumeStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal)
{
}

void AudioCapturerProxyObj::SetLowPowerVolumeImpl(float volume)
{
}

void AudioCapturerProxyObj::GetLowPowerVolumeImpl(float &volume)
{
    volume = 1.0;
}

void AudioCapturerProxyObj::GetSingleStreamVolumeImpl(float &volume)
{
    volume = 0;
}
} // namespace AudioStandard
} // namespace OHOS
