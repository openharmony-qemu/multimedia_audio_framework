/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioSuiteNrAlgoInterfaceImpl"
#endif

#include <dlfcn.h>
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_nr_algo_interface_impl.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
namespace {
const std::string ALGO_PATH_BASE = "/system/lib64/";
const std::string ALGO_SO_NAME = "libimedia_vqe_ainr.z.so";
}  // namespace

AudioSuiteNrAlgoInterfaceImpl::AudioSuiteNrAlgoInterfaceImpl()
    : algoDefaultConfig_{AUDIO_AINR_PCM_SAMPLERATE_16K,
          AUDIO_AINR_PCM_CHANNEL_NUM,
          AUDIO_AINR_PCM_16K_FRAME_LEN,
          AUDIO_AINR_PCM_16_BIT}
{
    AUDIO_INFO_LOG("AudioSuiteNrAlgoInterfaceImpl::AudioSuiteNrAlgoInterfaceImpl()");
}

AudioSuiteNrAlgoInterfaceImpl::~AudioSuiteNrAlgoInterfaceImpl()
{
    AUDIO_INFO_LOG("AudioSuiteNrAlgoInterfaceImpl::~AudioSuiteNrAlgoInterfaceImpl()");
}

int32_t AudioSuiteNrAlgoInterfaceImpl::LoadAlgorithmFunction(void)
{
    algoApi_.getVersion = reinterpret_cast<FunAudioAinrGetVersion>(dlsym(libHandle_, "AudioAinrGetVersion"));
    algoApi_.getSize = reinterpret_cast<FunAudioAinrGetSize>(dlsym(libHandle_, "AudioAinrGetSize"));
    algoApi_.initAlgo = reinterpret_cast<FunAudioAinrInit>(dlsym(libHandle_, "AudioAinrInit"));
    algoApi_.applyAlgo = reinterpret_cast<FunAudioAinrApply>(dlsym(libHandle_, "AudioAinrApply"));

    bool loadAlgoApiFail = algoApi_.getVersion == nullptr || algoApi_.getSize == nullptr ||
                           algoApi_.initAlgo == nullptr || algoApi_.applyAlgo == nullptr;

    return loadAlgoApiFail ? ERROR : SUCCESS;
}

int32_t AudioSuiteNrAlgoInterfaceImpl::ApplyAndWaitReady(void)
{
    AUDIO_INFO_LOG("start load ainr algo so");

    std::string soPath = ALGO_PATH_BASE + ALGO_SO_NAME;
    libHandle_ = dlopen(soPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (libHandle_ == nullptr) {
        AUDIO_ERR_LOG("dlopen algo: %{private}s so fail, error: %{public}s", soPath.c_str(), dlerror());
        return ERROR;
    }

    if (LoadAlgorithmFunction() != SUCCESS) {
        AUDIO_ERR_LOG("LoadAlgorithmFunction fail");
        UnApply();
        return ERROR;
    }

    AUDIO_INFO_LOG("end load ainr algo so");
    return SUCCESS;
}


void AudioSuiteNrAlgoInterfaceImpl::UnApply(void)
{
    AUDIO_INFO_LOG("start unload ainr algo so");

    if (libHandle_ != nullptr) {
        static_cast<void>(dlclose(libHandle_));
    }
    libHandle_ = nullptr;
    static_cast<void>(memset_s(&algoApi_, sizeof(algoApi_), 0, sizeof(algoApi_)));

    AUDIO_INFO_LOG("end unload ainr algo so");
}

int32_t AudioSuiteNrAlgoInterfaceImpl::Init()
{
    AUDIO_INFO_LOG("start init ainr algorithm");

    int32_t ret = ApplyAndWaitReady();
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);

    int32_t chanSize = 0;
    ret = algoApi_.getSize(&chanSize);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_AINR_EOK, ret, "Get ainr algo chanSize fail, error: %{public}d", ret);
    AUDIO_INFO_LOG("Get ainr algo chanSize: %{public}d", chanSize);

    algoHandle_ = std::make_unique<signed char[]>(chanSize);
    ret = algoApi_.initAlgo(algoHandle_.get(), &algoDefaultConfig_, static_cast<uint32_t>(chanSize));
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_AINR_EOK, ret, "Init ainr algo fail, ret: %{public}d", ret);

    AUDIO_INFO_LOG("end init ainr algorithm");
    return SUCCESS;
}

int32_t AudioSuiteNrAlgoInterfaceImpl::Deinit()
{
    AUDIO_INFO_LOG("start deinit ainr algorithm");

    algoHandle_.reset();
    UnApply();
    
    AUDIO_INFO_LOG("end deinit ainr algorithm");
    return SUCCESS;
}

int32_t AudioSuiteNrAlgoInterfaceImpl::Apply(std::vector<uint8_t *> &audioInputs, std::vector<uint8_t *> &audioOutputs)
{
    AUDIO_INFO_LOG("start apply ainr algorithm");

    if (audioInputs.empty() ||  audioOutputs.empty()) {
        AUDIO_ERR_LOG("Apply para check fail, input or output list is empty");
        return ERROR;
    }

    if (audioInputs[0] == nullptr || audioOutputs[0] == nullptr) {
        AUDIO_ERR_LOG("Apply para check fail, input or output is nullptr");
        return ERROR;
    }

    AudioAinrDataTransferStruct audioData = AudioAinrDataTransferStruct();
    audioData.dataIn = reinterpret_cast<int16_t *>(audioInputs[0]);
    audioData.dataOut = reinterpret_cast<int16_t *>(audioOutputs[0]);

    int32_t ret = algoApi_.applyAlgo(algoHandle_.get(), &audioData);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_AINR_EOK, ret, "Apply ainr algorithm fail, ret: %{public}d", ret);

    return SUCCESS;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS