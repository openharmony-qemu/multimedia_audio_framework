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
#include <cstring>
#include "securec.h"
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_nr_algo_interface_impl.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

AudioSuiteNrAlgoInterfaceImpl::AudioSuiteNrAlgoInterfaceImpl(NodeCapability &nc)
    : algoDefaultConfig_{AUDIO_AINR_PCM_SAMPLERATE_16K,
          AUDIO_AINR_PCM_CHANNEL_NUM,
          AUDIO_AINR_PCM_16K_FRAME_LEN,
          AUDIO_AINR_PCM_16_BIT}
{
    nodeCapability = nc;
    AUDIO_INFO_LOG("AudioSuiteNrAlgoInterfaceImpl::AudioSuiteNrAlgoInterfaceImpl()");
}

AudioSuiteNrAlgoInterfaceImpl::~AudioSuiteNrAlgoInterfaceImpl()
{
    AUDIO_INFO_LOG("AudioSuiteNrAlgoInterfaceImpl::~AudioSuiteNrAlgoInterfaceImpl()");
    Deinit();
}

int32_t AudioSuiteNrAlgoInterfaceImpl::Init()
{
    AUDIO_INFO_LOG("start init ainr algorithm");

    // load algorithm so
    std::string soPath = nodeCapability.soPath + nodeCapability.soName;
    libHandle_ = dlopen(soPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    CHECK_AND_RETURN_RET_LOG(libHandle_ != nullptr, ERROR, "dlopen algo: %{private}s so fail, error: %{public}s",
        soPath.c_str(), dlerror());

    // load functions in ainr algorithm so
    algoApi_.getVersion = reinterpret_cast<FunAudioAinrGetVersion>(dlsym(libHandle_, "AudioAinrGetVersion"));
    algoApi_.getSize = reinterpret_cast<FunAudioAinrGetSize>(dlsym(libHandle_, "AudioAinrGetSize"));
    algoApi_.initAlgo = reinterpret_cast<FunAudioAinrInit>(dlsym(libHandle_, "AudioAinrInit"));
    algoApi_.applyAlgo = reinterpret_cast<FunAudioAinrApply>(dlsym(libHandle_, "AudioAinrApply"));

    bool loadAlgoApiFail = algoApi_.getVersion == nullptr || algoApi_.getSize == nullptr ||
                           algoApi_.initAlgo == nullptr || algoApi_.applyAlgo == nullptr;
    if (loadAlgoApiFail) {
        AUDIO_ERR_LOG("load ainr algorithm function fail");
        Deinit();
        return ERROR;
    }

    // allocate memory for ainr algorithm
    int32_t chanSize = 0;
    int32_t ret = algoApi_.getSize(&chanSize);
    if (ret != AUDIO_AINR_EOK || chanSize <= 0) {
        AUDIO_ERR_LOG("Get ainr algorithm chanSize error, ret: %{public}d, chanSize: %{public}d", ret, chanSize);
        Deinit();
        return ERROR;
    }
    algoHandle_ = std::make_unique<signed char[]>(chanSize);

    // init ainr algorithm
    ret = algoApi_.initAlgo(algoHandle_.get(), &algoDefaultConfig_, static_cast<uint32_t>(chanSize));
    if (ret != AUDIO_AINR_EOK) {
        AUDIO_ERR_LOG("Init ainr algorithm fail, ret: %{public}d", ret);
        Deinit();
        return ERROR;
    }

    AUDIO_INFO_LOG("end init ainr algorithm");
    return SUCCESS;
}

int32_t AudioSuiteNrAlgoInterfaceImpl::Deinit()
{
    AUDIO_INFO_LOG("start deinit ainr algorithm");

    if (libHandle_ != nullptr) {
        static_cast<void>(dlclose(libHandle_));
        libHandle_ = nullptr;
    }
    static_cast<void>(memset_s(&algoApi_, sizeof(algoApi_), 0, sizeof(algoApi_)));
    algoHandle_.reset();
    
    AUDIO_INFO_LOG("end deinit ainr algorithm");
    return SUCCESS;
}

int32_t AudioSuiteNrAlgoInterfaceImpl::Apply(std::vector<uint8_t *> &audioInputs, std::vector<uint8_t *> &audioOutputs)
{
    AUDIO_DEBUG_LOG("Apply ainr algorithm");
    
    CHECK_AND_RETURN_RET_LOG(
        !audioInputs.empty() && !audioOutputs.empty(), ERROR, "Invalid audioInputs or audioOutputs");

    CHECK_AND_RETURN_RET_LOG(
        audioInputs[0] != nullptr && audioOutputs[0] != nullptr, ERROR, "Apply input para is nullptr");

    CHECK_AND_RETURN_RET_LOG(algoHandle_ != nullptr, ERROR, "Apply para algoHandle_ is nullptr, need init first");

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