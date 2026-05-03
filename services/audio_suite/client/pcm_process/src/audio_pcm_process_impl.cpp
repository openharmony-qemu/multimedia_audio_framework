/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "audio_pcm_process_impl.h"

AudioConverterHandle AudioConverterCreate(const AudioFormatConfig* inCfg, const AudioFormatConfig* outCfg)
{
    return nullptr;
}

AudioConvertResult AudioConverterProcess(AudioConverterHandle handle, const uint8_t* inData, int inSampleCount)
{
    AudioConvertResult result;
    return result;
}

void AudioConverterDestroy(AudioConverterHandle handle)
{
    (void)handle;
}

AudioMixHandle AudioMixerCreate(const AudioFormatConfig* inCfg)
{
    return nullptr;
}

AudioMixResult AudioMixerProcess(AudioMixHandle handle, const AudioMixStream* streams, int streamCount)
{
    AudioMixResult result;
    return result;
}

void AudioMixerDestroy(AudioMixHandle hanlde)
{
    (void)hanlde;
}