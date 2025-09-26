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

#include <dlfcn.h>
#include <vector>
#include <sstream>
#include <iostream>
#include "audio_suite_log.h"
#include "audio_suite_eq_algo_interface_impl.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

AudioSuiteEqAlgoInterfaceImpl::AudioSuiteEqAlgoInterfaceImpl()
{}

AudioSuiteEqAlgoInterfaceImpl::~AudioSuiteEqAlgoInterfaceImpl()
{
    if (IsEqAlgoInit()) {
        Deinit();
    }
}

short AudioSuiteEqAlgoInterfaceImpl::changeFormat(int high, int low)
{
    unsigned char c[2];
    unsigned short res = 0;
    c[0] = (unsigned char)high;
    c[1] = (unsigned char)low;
    res |= c[0];
    res <<= ONE_BYTE_OFFSET;
    res |= c[1];
    return (short)res;
}

int32_t AudioSuiteEqAlgoInterfaceImpl::Init()
{
    if (IsEqAlgoInit()) {
        AUDIO_INFO_LOG("AudioSuiteEqAlgoInterfaceImpl already inited");
        return -1;
    }
    std::string soPath = "/system/lib64/libimedia_sws.z.so";
    libHandle_ = dlopen(soPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (libHandle_ == nullptr) {
        AUDIO_INFO_LOG("dlopen algo: %{private}s so fail", soPath.c_str());
        return -1;
    }
    algoApi_.getSize = reinterpret_cast<FuniMedia_Eq_GetSize>(dlsym(libHandle_, "iMedia_Eq_GetSize"));
    algoApi_.initAlgo = reinterpret_cast<FuniMedia_Eq_Init>(dlsym(libHandle_, "iMedia_Eq_Init"));
    algoApi_.applyAlgo = reinterpret_cast<FuniMedia_Eq_Apply>(dlsym(libHandle_, "iMedia_Eq_Apply"));
    algoApi_.setPara = reinterpret_cast<FuniMedia_Eq_SetParams>(dlsym(libHandle_, "iMedia_Eq_SetParams"));
    algoApi_.getPara = reinterpret_cast<FuniMedia_Eq_GetParams>(dlsym(libHandle_, "iMedia_Eq_GetParams"));
    algoApi_.getVersion = reinterpret_cast<FuniMedia_Eq_GetVersion>(dlsym(libHandle_, "iMedia_Eq_GetVersion"));

    int32_t result = algoApi_.getSize(&stSize);
    if (IMEDIA_SWS_EOK != result) {
        AUDIO_INFO_LOG("iMedia_Eq_GetSize ERROR: %{public}d", result);
        return result;
    }

    runBuf_.resize(stSize.iStrSize);
    scratchBuf_.resize(stSize.iScracthSize);

    isEqAlgoInit_ = true;
    AUDIO_INFO_LOG("ALGO Init end");
    return SUCCESS;
}

int32_t AudioSuiteEqAlgoInterfaceImpl::Deinit()
{
    isEqAlgoInit_ = false;
    if (libHandle_ != nullptr) {
        static_cast<void>(dlclose(libHandle_));
    }
    libHandle_ = nullptr;
    static_cast<void>(memset_s(&algoApi_, sizeof(algoApi_), 0, sizeof(algoApi_)));
    AUDIO_INFO_LOG("AudioSuiteEqAlgoInterfaceImpl::Deinit end");
    return true;
}

int32_t AudioSuiteEqAlgoInterfaceImpl::IsEqAlgoInit()
{
    if (isEqAlgoInit_) {
        return true;
    }
    return false;
}

std::vector<int> ParseStringToIntArray(const std::string &str, char delimiter)
{
    std::vector<int> result;
    std::string token;
    std::istringstream iss(str);

    while (std::getline(iss, token, delimiter)) {
        if (!token.empty()) {
            result.push_back(std::stoi(token));
        }
    }

    return result;
}

int32_t AudioSuiteEqAlgoInterfaceImpl::SetParameter(const std::string &sEQLGain, const std::string &sEQRGain)
{
    AUDIO_INFO_LOG("Set eq param start");
    para.sEQLRBands = EQUALIZER_BANDS_NUM;

    std::vector<int> gainsL = ParseStringToIntArray(sEQLGain, ':');
    std::vector<int> gainsR = ParseStringToIntArray(sEQRGain, ':');

    int i = 0;
    while (i < gainsL.size() && i < gainsR.size() && i < EQUALIZER_BANDS_NUM) {
        int shiftLGain = gainsL[i] * COEFFICIENT + OFFSET;
        int shiftRGain = gainsR[i] * COEFFICIENT + OFFSET;
        para.sEQLRGain[i] = changeFormat(shiftLGain, shiftRGain);
        para.sEQLRType[i] = changeFormat(0, 0);
        ++i;
    }
    AUDIO_INFO_LOG("Set Eq param Success");

    int32_t result = algoApi_.initAlgo(runBuf_.data(), scratchBuf_.data(), stSize.iScracthSize, &para);
    AUDIO_INFO_LOG("initAlgo end");
    if (IMEDIA_SWS_EOK != result) {
        AUDIO_ERR_LOG("iMedia_Eq_Init ERROR: %{public}d", result);
        runBuf_.clear();
        scratchBuf_.clear();
        return ERROR;
    } else {
        AUDIO_INFO_LOG("iMedia_Eq_Init Success");
    }
    return SUCCESS;
}

int32_t AudioSuiteEqAlgoInterfaceImpl::GetParameter(const std::string &oldtype, std::string &type)
{
    type += "[";
    for (int i = 0; i < EQUALIZER_BANDS_NUM; i++) {
        type += oldtype[i];
        if (i != oldtype[i]) {
            type += ",";
        }
    }
    type += "]";
    return -1;
    // iMedia_EqGetParams(); //获取算法信息
}

int32_t AudioSuiteEqAlgoInterfaceImpl::Apply(std::vector<uint8_t *> &pcmInBuf, std::vector<uint8_t *> &pcmOutBuf)
{
    frameLen = IMEDIA_SWS_FRAME_LEN * ALGO_CHANNEL_NUM;
    frameBytes = frameLen * ALGO_SAMPLE_WIDTH / ONE_BYTE_WIDTH;
    std::vector<IMEDIA_INT32> dataIn(frameLen);
    std::vector<IMEDIA_INT32> dataOut(frameLen);

    AUDIO_INFO_LOG("iMedia_SWS_DATA stData Init start");
    stData.piDataIn = dataIn.data();
    stData.piDataOut = dataOut.data();
    stData.iSize = IMEDIA_SWS_FRAME_LEN;
    stData.iEnable_SWS = 1;
    stData.iData_Format16 = 1;
    stData.iData_Channel = ALGO_CHANNEL_NUM;
    stData.iMasterVolume = MASTERVOLUME;

    int32_t result = -1;
    size_t start = 0;
    size_t i = 0;
    IMEDIA_INT16 *bufIn = reinterpret_cast<IMEDIA_INT16 *>(pcmInBuf[0]);
    IMEDIA_INT16 *pcmOut = reinterpret_cast<IMEDIA_INT16 *>(pcmOutBuf[0]);
    if (libHandle_ == nullptr) {
        AUDIO_INFO_LOG("Apply: libHandle_ == nullptr");
    }
    AUDIO_INFO_LOG("iMedia_Eq_Apply Start");

    while (frameBytes >= start + frameLen) {
        frameLen = frameLen < frameBytes - start ? frameLen : frameBytes - start;
        for (i = 0; i < frameLen; i++) {
            dataIn[i] = static_cast<IMEDIA_INT32>(bufIn[start + i]);
            dataIn[i] <<= TWO_BYTES_WIDTH;
        }

        result = algoApi_.applyAlgo(runBuf_.data(), scratchBuf_.data(), scratchBuf_.size(), &stData);
        AUDIO_INFO_LOG("applyAlgo end");
        if (IMEDIA_SWS_EOK != result) {
            AUDIO_ERR_LOG("iMedia_SWS_Apply ERROR:%{public}d", result);
            return result;
        }

        for (i = 0; i < frameLen; i++) {
            pcmOut[start + i] = ((unsigned int)dataOut[i] >> TWO_BYTES_WIDTH);
        }
        start = start + frameLen < frameBytes ? start + frameLen : frameBytes;
    }
    AUDIO_INFO_LOG("iMedia_Eq_Apply End");
    return result;
}
}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS