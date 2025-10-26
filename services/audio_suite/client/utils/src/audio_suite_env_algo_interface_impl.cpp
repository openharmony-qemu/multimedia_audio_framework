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
#define LOG_TAG "AudioSuiteEnvAlgoInterface"
#endif

#include <dlfcn.h>
#include <vector>
#include <sstream>
#include <cstring>
#include "securec.h"
#include <iostream>
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_env_algo_interface_impl.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

AudioSuiteEnvAlgoInterfaceImpl::AudioSuiteEnvAlgoInterfaceImpl(NodeCapability &nc)
{
    nodeCapability = nc;
}

AudioSuiteEnvAlgoInterfaceImpl::~AudioSuiteEnvAlgoInterfaceImpl()
{
    if (isEnvAlgoInit_) {
        Deinit();
    }
}

int32_t AudioSuiteEnvAlgoInterfaceImpl::Init()
{
    if (isEnvAlgoInit_) {
        AUDIO_ERR_LOG("AudioSuiteEnvAlgoInterfaceImpl already inited");
        return ERROR;
    }
    std::string soPath = nodeCapability.soPath + nodeCapability.soName;
    libHandle_ = dlopen(soPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    CHECK_AND_RETURN_RET_LOG(libHandle_ != nullptr, ERROR, "dlopen algo: %{private}s so fail", soPath.c_str());

    algoApi_.getSize = reinterpret_cast<FuniMedia_Env_GetSize>(dlsym(libHandle_, "iMedia_Env_GetSize"));
    CHECK_AND_RETURN_RET_LOG(algoApi_.getSize != nullptr, ERROR, "Failed to get symbol iMedia_Env_GetSize");
    algoApi_.initAlgo = reinterpret_cast<FuniMedia_Env_Init>(dlsym(libHandle_, "iMedia_Env_Init"));
    CHECK_AND_RETURN_RET_LOG(algoApi_.initAlgo != nullptr, ERROR, "Failed to get symbol iMedia_Env_Init");
    algoApi_.applyAlgo = reinterpret_cast<FuniMedia_Env_Apply>(dlsym(libHandle_, "iMedia_Env_Apply"));
    CHECK_AND_RETURN_RET_LOG(algoApi_.applyAlgo != nullptr, ERROR, "Failed to get symbol iMedia_Env_Apply");
    algoApi_.setPara = reinterpret_cast<FuniMedia_Env_SetParams>(dlsym(libHandle_, "iMedia_Env_SetParams"));
    CHECK_AND_RETURN_RET_LOG(algoApi_.setPara != nullptr, ERROR, "Failed to get symbol iMedia_Env_SetParams");
    algoApi_.getPara = reinterpret_cast<FuniMedia_Env_GetParams>(dlsym(libHandle_, "iMedia_Env_GetParams"));
    CHECK_AND_RETURN_RET_LOG(algoApi_.getPara != nullptr, ERROR, "Failed to get symbol iMedia_Env_GetParams");

    int32_t ret = algoApi_.getSize(&stSize_);
    CHECK_AND_RETURN_RET_LOG(ret == IMEDIA_SWS_EOK, ret, "iMedia_Env_GetSize ERROR: %{public}d", ret);

    runBuf_.resize(stSize_.iStrSize);
    scratchBuf_.resize(stSize_.iScracthSize);
    isEnvAlgoInit_ = true;
    AUDIO_INFO_LOG("ALGO Init End, size of runBuf: %{public}d, size of scratchBuf: %{public}d",
        stSize_.iStrSize,
        stSize_.iScracthSize);
    para_ = IMEDIA_SWS_ENV_BROADCAST;
    return SUCCESS;
}

int32_t AudioSuiteEnvAlgoInterfaceImpl::Deinit()
{
    isEnvAlgoInit_ = false;
    if (libHandle_ != nullptr) {
        int32_t ret = dlclose(libHandle_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "dlclose failed: %{public}s", dlerror());
        libHandle_ = nullptr;
    }
    static_cast<void>(memset_s(&algoApi_, sizeof(algoApi_), 0, sizeof(algoApi_)));
    AUDIO_INFO_LOG("AudioSuiteEnvAlgoInterfaceImpl::Deinit end");
    return SUCCESS;
}

int32_t StringToEnvMode(const std::string &modStr, iMedia_Env_PARA &para)
{
    if (modStr == "0") {
        AUDIO_INFO_LOG("EnvMode is Close");
        para = IMEDIA_SWS_ENV_BROADCAST;
    } else if (modStr == "1") {
        AUDIO_INFO_LOG("Set EnvMode to BROADCAST");
        para = IMEDIA_SWS_ENV_BROADCAST;
    } else if (modStr == "2") {
        AUDIO_INFO_LOG("Set EnvMode to TELEPHONE_RECEIVER");
        para = IMEDIA_SWS_ENV_TELEPHONE_RECEIVER;
    } else if (modStr == "3") {
        AUDIO_INFO_LOG("Set EnvMode to UNDER_WATER");
        para = IMEDIA_SWS_ENV_UNDER_WATER;
    } else if (modStr == "4") {
        AUDIO_INFO_LOG("Set EnvMode to PHONOGRAPH");
        para = IMEDIA_SWS_ENV_PHONOGRAPH;
    } else {
        AUDIO_ERR_LOG("Unknow EnvMode %{public}s, Set EnvMode to BROADCAST", modStr.c_str());
        return ERROR;
    }
    return SUCCESS;
}

int32_t AudioSuiteEnvAlgoInterfaceImpl::SetParameter(const std::string &paramType, const std::string &paramValue)
{
    int32_t ret = StringToEnvMode(paramValue, para_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Set EnvMOde Failed");
    ret = algoApi_.initAlgo(runBuf_.data(), scratchBuf_.data(), stSize_.iScracthSize, para_);
    if (IMEDIA_SWS_EOK != ret) {
        AUDIO_ERR_LOG("iMedia_Env_Init ERROR: %{public}d", ret);
        runBuf_.clear();
        scratchBuf_.clear();
        return ERROR;
    } else {
        AUDIO_INFO_LOG("iMedia_Env_Init Success");
    }

    ret = algoApi_.setPara(runBuf_.data(), scratchBuf_.data(), stSize_.iScracthSize, para_);
    if (IMEDIA_SWS_EOK != ret) {
        AUDIO_ERR_LOG("iMedia_Env_SetParams ERROR: %{public}d", ret);
        runBuf_.clear();
        scratchBuf_.clear();
        return ERROR;
    }

    frameLen_ = IMEDIA_SWS_FRAME_LEN * ALGO_CHANNEL_NUM;
    inputSamples_ = frameLen_ * AUDIO_DURATION;
    dataIn_.resize(frameLen_);
    dataOut_.resize(frameLen_);

    stData_.piDataIn = reinterpret_cast<IMEDIA_INT32 *>(dataIn_.data());
    stData_.piDataOut = reinterpret_cast<IMEDIA_INT32 *>(dataOut_.data());
    stData_.iSize = IMEDIA_SWS_FRAME_LEN;
    stData_.iEnable_SWS = 1;
    stData_.iData_Format16 = 1;
    stData_.iData_Channel = ALGO_CHANNEL_NUM;
    stData_.iMasterVolume = MASTERVOLUME;

    AUDIO_INFO_LOG("Set Env Parameter Success");
    return SUCCESS;
}

int32_t AudioSuiteEnvAlgoInterfaceImpl::GetParameter(const std::string &paramType, std::string &paramValue)
{
    iMedia_Env_PARA param = {};
    algoApi_.getPara(runBuf_.data(), &param);
    if (param != para_) {
        AUDIO_ERR_LOG("Set or get wrong param, set = %{public}d, get = %{public}d",
            static_cast<int32_t>(para_),
            static_cast<int32_t>(param));
        return ERROR;
    }
    paramValue = std::to_string(static_cast<int32_t>(param) + static_cast<int32_t>(1));
    return SUCCESS;
}

int32_t AudioSuiteEnvAlgoInterfaceImpl::Apply(std::vector<uint8_t *> &pcmInBuf, std::vector<uint8_t *> &pcmOutBuf)
{
    CHECK_AND_RETURN_RET_LOG(!pcmInBuf.empty(), ERROR, "pcmInBuf is empty");
    CHECK_AND_RETURN_RET_LOG(pcmInBuf[0] != nullptr, ERROR, "pcmInBuf[0] is empty");
    CHECK_AND_RETURN_RET_LOG(!pcmOutBuf.empty(), ERROR, "pcmOutBuf is empty");
    CHECK_AND_RETURN_RET_LOG(pcmOutBuf[0] != nullptr, ERROR, "pcmOutBuf[0] is empty");

    int32_t ret = -1;
    size_t start = 0;
    size_t i = 0;

    IMEDIA_INT16 *bufIn = reinterpret_cast<IMEDIA_INT16 *>(pcmInBuf[0]);
    IMEDIA_INT16 *pcmOut = reinterpret_cast<IMEDIA_INT16 *>(pcmOutBuf[0]);
    if (libHandle_ == nullptr) {
        AUDIO_INFO_LOG("Apply: libHandle_ == nullptr");
    }
    AUDIO_DEBUG_LOG("iMedia_Env_Apply Start");

    while (inputSamples_ >= start + frameLen_) {
        frameLen_ = frameLen_ < inputSamples_ - start ? frameLen_ : inputSamples_ - start;
        for (i = 0; i < frameLen_; i++) {
            dataIn_[i] = static_cast<uint32_t>(bufIn[start + i]);
            dataIn_[i] <<= TWO_BYTES_WIDTH;
        }

        ret = algoApi_.applyAlgo(runBuf_.data(), scratchBuf_.data(), scratchBuf_.size(), &stData_);
        AUDIO_DEBUG_LOG("applyAlgo end");
        CHECK_AND_RETURN_RET_LOG(ret == IMEDIA_SWS_EOK, ret, "iMedia_SWS_Apply ERROR:%{public}d", ret);
        for (i = 0; i < frameLen_; i++) {
            pcmOut[start + i] = ((unsigned int)dataOut_[i] >> TWO_BYTES_WIDTH);
        }
        start = start + frameLen_ < inputSamples_ ? start + frameLen_ : inputSamples_;
    }
    AUDIO_DEBUG_LOG("iMedia_Env_Apply End");
    return ret;
}
}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS