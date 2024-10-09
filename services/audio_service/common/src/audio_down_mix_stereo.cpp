/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioDownMixStereo"
#endif

#include "audio_down_mix_stereo.h"
#include "audio_service_log.h"
#include "audio_errors.h"
#include <dlfcn.h>
#include <cinttypes>

namespace OHOS {
namespace AudioStandard {
#if (defined(__aarch64__) || defined(__x86_64__))
const char *DOWN_MIXER_FILE_NAME = "/system/lib64/libaudio_downmixer_mcr.z.so";
#else
const char *DOWN_MIXER_FILE_NAME = "/system/lib/libaudio_downmixer_mcr.z.so";
#endif
const char *DOWN_MIX_FUNC_NAME = "AudioMcrMixerClassCreate";

AudioDownMixStereo::AudioDownMixStereo() : handle_(nullptr), mixer_(nullptr)
{
    if (access(DOWN_MIXER_FILE_NAME, R_OK) != 0) {
        AUDIO_ERR_LOG("so file not exist.");
        return;
    }
    handle_ = ::dlopen(DOWN_MIXER_FILE_NAME, RTLD_NOW);
    if (!handle_) {
        AUDIO_ERR_LOG("dlopen failed check so file exists");
        return;
    }
    AudioMcrMixerClassCreateFunc *createMixerFunc =
        reinterpret_cast<AudioMcrMixerClassCreateFunc *>(dlsym(handle_, DOWN_MIX_FUNC_NAME));
    if (!createMixerFunc) {
        AUDIO_ERR_LOG("dlsym failed.check so has this function.");
        return;
    }
    createMixerFunc(&mixer_);
}

AudioDownMixStereo::~AudioDownMixStereo()
{
    if (mixer_) {
        free(mixer_);
    }
    if (handle_) {
#ifndef TEST_COVERAGE
        dlclose(handle_);
#endif
        handle_ = nullptr;
    }
}

int32_t AudioDownMixStereo::InitMixer(AudioChannelLayout mode, int32_t channels)
{
    if (!mixer_) {
        AUDIO_ERR_LOG("mixer is nullptr.");
        return ERR_INVALID_HANDLE;
    }
    AUDIO_INFO_LOG("channel layout:%{public}" PRIu64 ".", mode);
    int32_t ret = mixer_->InitMixer(mode, channels);
    if (ret != 0) {
        AUDIO_ERR_LOG("init mixer failed.ret:%{public}d", ret);
        return ERR_INVALID_HANDLE;
    }
    return SUCCESS;
}

int32_t AudioDownMixStereo::Apply(const int32_t &frameLength, float *input, float *output)
{
    if (!mixer_) {
        AUDIO_ERR_LOG("mixer is nullptr.");
        return ERR_INVALID_HANDLE;
    }
    int32_t ret = mixer_->Apply(frameLength, input, output);
    if (ret != 0) {
        AUDIO_ERR_LOG("apply mixer failed.ret:%{public}d", ret);
        return ERR_INVALID_HANDLE;
    }
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS