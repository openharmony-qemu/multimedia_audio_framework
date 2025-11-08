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
#define LOG_TAG "AudioSuiteSoundFieldAlgoInterfaceImpl"
#endif

#include <dlfcn.h>
#include <cstring>
#include "securec.h"
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_soundfield_algo_interface_impl.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
namespace {
static constexpr uint32_t SAMPLE_SHIFT_AMOUNT = 16;
}  // namespace

AudioSuiteSoundFieldAlgoInterfaceImpl::AudioSuiteSoundFieldAlgoInterfaceImpl(NodeCapability &nc)
{
    AUDIO_INFO_LOG("AudioSuiteSoundFieldAlgoInterfaceImpl::AudioSuiteSoundFieldAlgoInterfaceImpl()");
    stData_.piDataIn = dataIn_.data();
    stData_.piDataOut = dataOut_.data();
    stData_.iSize = AUDIO_SURROUND_PCM_48K_FRAME_LEN;
    stData_.iEnable_SWS = AUDIO_SURROUND_ENABLE_SWS;
    stData_.iData_Format16 = AUDIO_SURROUND_PCM_16_BIT;
    stData_.iData_Channel = AUDIO_SURROUND_PCM_CHANNEL_NUM;
    stData_.iMasterVolume = AUDIO_SURROUND_MASTER_VOLUME;
    nodeCapability = nc;
}

AudioSuiteSoundFieldAlgoInterfaceImpl::~AudioSuiteSoundFieldAlgoInterfaceImpl()
{
    AUDIO_INFO_LOG("AudioSuiteSoundFieldAlgoInterfaceImpl::~AudioSuiteSoundFieldAlgoInterfaceImpl()");
    Deinit();
}

int32_t AudioSuiteSoundFieldAlgoInterfaceImpl::Init()
{
    AUDIO_INFO_LOG("start init SoundField algorithm");

    // load algorithm so
    std::string soPath = nodeCapability.soPath + nodeCapability.soName;
    libHandle_ = dlopen(soPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    CHECK_AND_RETURN_RET_LOG(libHandle_ != nullptr, ERROR, "dlopen algo: %{private}s so fail, error: %{public}s",
        soPath.c_str(), dlerror());

    // load functions in SoundField algorithm so
    algoApi_.getSize = reinterpret_cast<Fun_iMedia_Surround_GetSize>(dlsym(libHandle_, "iMedia_Surround_GetSize"));
    algoApi_.initAlgo = reinterpret_cast<Fun_iMedia_Surround_Init>(dlsym(libHandle_, "iMedia_Surround_Init"));
    algoApi_.applyAlgo = reinterpret_cast<Fun_iMedia_Surround_Apply>(dlsym(libHandle_, "iMedia_Surround_Apply"));
    algoApi_.setPara = reinterpret_cast<Fun_iMedia_Surround_SetParams>(dlsym(libHandle_, "iMedia_Surround_SetParams"));
    algoApi_.getPara = reinterpret_cast<Fun_iMedia_Surround_GetParams>(dlsym(libHandle_, "iMedia_Surround_GetParams"));

    bool loadAlgoApiFail =
        algoApi_.getSize == nullptr || algoApi_.initAlgo == nullptr || algoApi_.applyAlgo == nullptr ||
        algoApi_.setPara == nullptr || algoApi_.getPara == nullptr;
    CHECK_AND_RETURN_RET_LOG(!loadAlgoApiFail, ERROR, "load SoundField algorithm function fail");

    // allocate memory for SoundField algorithm
    int32_t ret = algoApi_.getSize(&stSize_);
    CHECK_AND_RETURN_RET_LOG(ret == IMEDIA_SWS_EOK, ERROR, "SoundField algo GetSize , ret: %{public}d", ret);

    algoRunBuf_ = std::make_unique<uint8_t[]>(stSize_.iStrSize);
    algoScratchBuf_ = std::make_unique<uint8_t[]>(stSize_.iScracthSize);
    AUDIO_INFO_LOG("Init SoundField algorithm, size of runBuf: %{public}d, size of scratchBuf: %{public}d",
        stSize_.iStrSize, stSize_.iScracthSize);
    
    // init SoundField algorithm - use broad mode by default
    ret = algoApi_.initAlgo(algoRunBuf_.get(), algoScratchBuf_.get(), stSize_.iScracthSize, IMEDIA_SWS_SOUROUND_BROAD);
    if (ret != IMEDIA_SWS_EOK) {
        AUDIO_ERR_LOG("Init SoundField algorithm fail, ret: %{public}d", ret);
        Deinit();
        return ERROR;
    }

    AUDIO_INFO_LOG("end init SoundField algorithm");
    return SUCCESS;
}

int32_t AudioSuiteSoundFieldAlgoInterfaceImpl::Deinit()
{
    AUDIO_INFO_LOG("start deinit SoundField algorithm");

    if (libHandle_ != nullptr) {
        static_cast<void>(dlclose(libHandle_));
        libHandle_ = nullptr;
    }

    static_cast<void>(memset_s(&algoApi_, sizeof(algoApi_), 0, sizeof(algoApi_)));

    algoRunBuf_.reset();
    algoScratchBuf_.reset();
    
    AUDIO_INFO_LOG("end deinit SoundField algorithm");
    return SUCCESS;
}

int32_t AudioSuiteSoundFieldAlgoInterfaceImpl::SetParameter(const std::string &paramType, const std::string &paramValue)
{
    AUDIO_INFO_LOG("SoundField algo set [iMedia_Surround_PARA]: %{public}s", paramValue.c_str());

    CHECK_AND_RETURN_RET_LOG(
        algoRunBuf_ != nullptr && algoScratchBuf_ != nullptr, ERROR, "Invalid run buffer, need init first");

    // set SoundField mode
    iMedia_Surround_PARA surroundType = static_cast<iMedia_Surround_PARA>(std::stoi(paramValue));
    int32_t ret = algoApi_.setPara(algoRunBuf_.get(), algoScratchBuf_.get(), stSize_.iScracthSize, surroundType);
    CHECK_AND_RETURN_RET_LOG(ret == IMEDIA_SWS_EOK, ERROR, "set parameter fail, ret: %{public}d", ret);

    return SUCCESS;
}

int32_t AudioSuiteSoundFieldAlgoInterfaceImpl::GetParameter(const std::string &paramType, std::string &paramValue)
{
    CHECK_AND_RETURN_RET_LOG(algoRunBuf_ != nullptr, ERROR, "Invalid run buffer, need init first");

    iMedia_Surround_PARA param;
    int32_t ret = algoApi_.getPara(algoRunBuf_.get(), &param);
    CHECK_AND_RETURN_RET_LOG(ret == IMEDIA_SWS_EOK, ERROR, "get parameter fail, ret: %{public}d", ret);
    paramValue = std::to_string(static_cast<int32_t>(param));
    
    AUDIO_INFO_LOG("SoundField get parameter success, [iMedia_Surround_PARA]: %{public}s", paramValue.c_str());
    return SUCCESS;
}

int32_t AudioSuiteSoundFieldAlgoInterfaceImpl::Apply(
    std::vector<uint8_t *> &pcmInputs, std::vector<uint8_t *> &pcmOutputs)
{
    AUDIO_DEBUG_LOG("Apply SoundField algorithm");
    
    CHECK_AND_RETURN_RET_LOG(
        !pcmInputs.empty() && !pcmOutputs.empty(), ERROR, "Invalid para, pcmInputs or pcmOutputs is null");

    CHECK_AND_RETURN_RET_LOG(
        pcmInputs[0] != nullptr && pcmOutputs[0] != nullptr, ERROR, "Invalid para, input or output data is null");

    CHECK_AND_RETURN_RET_LOG(
        algoRunBuf_ != nullptr && algoScratchBuf_ != nullptr, ERROR, "Invalid run buffer, need init first");

    int16_t *bufIn = reinterpret_cast<int16_t *>(pcmInputs[0]);
    int16_t *bufOut = reinterpret_cast<int16_t *>(pcmOutputs[0]);

    // sample data convert from int16_t to IMEDIA_INT32
    for (size_t i = 0; i < SOUNDFIELD_ALGO_FRAME_LEN; i++) {
        dataIn_[i] = static_cast<IMEDIA_INT32>(static_cast<uint32_t>(bufIn[i]) << SAMPLE_SHIFT_AMOUNT);
    }

    // apply SoundField algorithm
    int32_t ret = algoApi_.applyAlgo(algoRunBuf_.get(), algoScratchBuf_.get(), stSize_.iScracthSize, &stData_);
    CHECK_AND_RETURN_RET_LOG(ret == IMEDIA_SWS_EOK, ret, "Apply SoundField algorithm fail, ret: %{public}d", ret);

    // sample data convert from IMEDIA_INT32 to int16_t
    for (size_t i = 0; i < SOUNDFIELD_ALGO_FRAME_LEN; i++) {
        bufOut[i] = static_cast<uint32_t>(dataOut_[i]) >> SAMPLE_SHIFT_AMOUNT;
    }

    return SUCCESS;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS