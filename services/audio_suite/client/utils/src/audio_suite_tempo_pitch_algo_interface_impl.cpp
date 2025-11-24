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
#define LOG_TAG "AudioSuiteTempoPitchAlgoInterfaceImpl"
#endif

#include <dlfcn.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_tempo_pitch_algo_interface_impl.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

AudioSuiteTempoPitchAlgoInterfaceImpl::AudioSuiteTempoPitchAlgoInterfaceImpl(NodeCapability &nc)
{
    nodeCapability = nc;
}

AudioSuiteTempoPitchAlgoInterfaceImpl::~AudioSuiteTempoPitchAlgoInterfaceImpl()
{
    Deinit();
}

int32_t AudioSuiteTempoPitchAlgoInterfaceImpl::TempoInit(std::string soName)
{
    std::string tempoSoPath = nodeCapability.soPath + soName;
    tempoSoHandle_ = dlopen(tempoSoPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    CHECK_AND_RETURN_RET_LOG(tempoSoHandle_ != nullptr, ERROR, "dlopen algo: %{private}s so fail, error: %{public}s",
        tempoSoPath.c_str(), dlerror());

    tempoAlgoApi_.create = reinterpret_cast<TEMPO_CREATE_FUNC>(dlsym(tempoSoHandle_, "PVCreate"));
    tempoAlgoApi_.destroy = reinterpret_cast<TEMPO_DESTROY_FUNC>(dlsym(tempoSoHandle_, "PVDestroypvHandle"));
    tempoAlgoApi_.setParam = reinterpret_cast<TEMPO_SET_FUNC>(dlsym(tempoSoHandle_, "PVSetSpeed"));
    tempoAlgoApi_.apply = reinterpret_cast<TEMPO_APPLY_FUNC>(dlsym(tempoSoHandle_, "PVChangeSpeed"));
    bool loadAlgoApiFail = tempoAlgoApi_.create == nullptr || tempoAlgoApi_.destroy == nullptr ||
                           tempoAlgoApi_.setParam == nullptr || tempoAlgoApi_.apply == nullptr;
    CHECK_AND_RETURN_RET_LOG(!loadAlgoApiFail, ERROR, "load tempo algorithm function fail");
    tempoAlgoHandle_ = tempoAlgoApi_.create(INIT_ALGO_SAMPLE_RATE);
    CHECK_AND_RETURN_RET_LOG(tempoAlgoHandle_, ERROR, "create algoHandle fail");
    return SUCCESS;
}

int32_t AudioSuiteTempoPitchAlgoInterfaceImpl::PitchInit(std::string soName)
{
    std::string pitchSoPath = nodeCapability.soPath + soName;
    pitchSoHandle_ = dlopen(pitchSoPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    CHECK_AND_RETURN_RET_LOG(pitchSoHandle_ != nullptr, ERROR, "dlopen algo: %{private}s so fail, error: %{public}s",
        pitchSoPath.c_str(), dlerror());
    pitchLibHandle_ = static_cast<AudioEffectLibrary *>(dlsym(pitchSoHandle_, PITCH_LIB.c_str()));
    CHECK_AND_RETURN_RET_LOG(pitchLibHandle_ != nullptr, ERROR, "load pitch lib symbol fail");

    AudioEffectDescriptor descriptor = {
        .libraryName = "audio_pitch_change",
        .effectName = "audio_pitch_change"
    };
    int32_t ret = pitchLibHandle_->createEffect(descriptor, &pitchAlgoHandle_);
    CHECK_AND_RETURN_RET_LOG(ret == 0 && pitchAlgoHandle_, ERROR, "load pitch algo handle fail");

    uint32_t replyData = 0;
    int32_t sampleRate = INIT_ALGO_SAMPLE_RATE;
    AudioEffectTransInfo replyInfo = {sizeof(int32_t), &replyData};
    AudioEffectTransInfo cmdInfo = {sizeof(int32_t), &sampleRate};
    ret = (*pitchAlgoHandle_)->command(pitchAlgoHandle_, EFFECT_CMD_SET_CONFIG, &cmdInfo, &replyInfo);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "config sample rate fail");
    return SUCCESS;
}

int32_t AudioSuiteTempoPitchAlgoInterfaceImpl::Init()
{
    AUDIO_INFO_LOG("start init tempo and pitch algorithm");
    std::istringstream iss(nodeCapability.soName);
    std::string tempoSoName = "";
    std::string pitchSoName = "";
    std::getline(iss, tempoSoName, ',');
    std::getline(iss, pitchSoName);
    CHECK_AND_RETURN_RET_LOG(!tempoSoName.empty() && !pitchSoName.empty(), ERROR,
        "Init error, parse so name fail");

    if (TempoInit(tempoSoName) != SUCCESS) {
        AUDIO_ERR_LOG("Tempo init error");
        Deinit();
        return ERROR;
    }

    if(PitchInit(pitchSoName) != SUCCESS) {
        AUDIO_ERR_LOG("Pitch init error");
        Deinit();
        return ERROR;
    }
    AUDIO_INFO_LOG("end init tempo and pitch algorithm");
    return SUCCESS;
}

int32_t AudioSuiteTempoPitchAlgoInterfaceImpl::Deinit()
{
    AUDIO_INFO_LOG("start deinit tempo and pitch algorithm");
    int32_t ret = 0;
    if (tempoAlgoHandle_) {
        tempoAlgoApi_.destroy(tempoAlgoHandle_);
        tempoAlgoHandle_ = nullptr;
    }
    if (tempoSoHandle_) {
        ret = dlclose(tempoSoHandle_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "dlclose tempoSo failed: %{public}s", dlerror());
        tempoSoHandle_ = nullptr;
    }
    if (pitchAlgoHandle_ && pitchLibHandle_) {
        pitchLibHandle_->releaseEffect(pitchAlgoHandle_);
        pitchAlgoHandle_ = nullptr;
    }
    if (pitchSoHandle_) {
        ret = dlclose(pitchSoHandle_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "dlclose pitchSo failed: %{public}s", dlerror());
        pitchSoHandle_ = nullptr;
        pitchLibHandle_ = nullptr;
    }
    static_cast<void>(memset_s(&tempoAlgoApi_, sizeof(tempoAlgoApi_), 0, sizeof(tempoAlgoApi_)));
    tempDataOut_.resize(0);
    AUDIO_INFO_LOG("end deinit tempo and pitch algorithm");
    return SUCCESS;
}

int32_t AudioSuiteTempoPitchAlgoInterfaceImpl::SetParameter(const std::string &paramType, const std::string &paramValue)
{
    CHECK_AND_RETURN_RET_LOG(tempoAlgoHandle_, ERROR, "Invalid tempoAlgoHandle_");
    CHECK_AND_RETURN_RET_LOG(pitchAlgoHandle_, ERROR, "Invalid pitchAlgoHandle_");
    std::vector<float> params = ParseStringToFloatArray(paramValue, ',');
    CHECK_AND_RETURN_RET_LOG(params.size() == 2, ERROR, "ParseStringToFloatArray error");

    speedRate_ = params[0];
    pitchRate_ = params[1];
    int32_t ret = tempoAlgoApi_.setParam(tempoAlgoHandle_, speedRate_);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "Set tempo param error %{public}d", ret);

    uint32_t replyData = 0;
    AudioEffectTransInfo replyInfo = {sizeof(int32_t), &replyData};
    AudioEffectTransInfo cmdInfo = {sizeof(float), &pitchRate_};
    ret = (*pitchAlgoHandle_)->command(pitchAlgoHandle_, EFFECT_CMD_SET_PARAM, &cmdInfo, &replyInfo);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "Set pitch param error %{public}d", ret);

    // Add 1024 frames of expansion
    expendSize_ = std::ceil(TEMPO_PITCH_ALGO_FRAME_LEN / speedRate_) + RESIZE_EXPAND_SIZE;
    tempDataOut_.resize(expendSize_);
    AUDIO_INFO_LOG("Set tempo:%{public}f, pitch:%{public}f successful", speedRate_, pitchRate_);

    return SUCCESS;
}

int32_t AudioSuiteTempoPitchAlgoInterfaceImpl::GetParameter(const std::string &paramType, std::string &paramValue)
{
    return SUCCESS;
}

int32_t AudioSuiteTempoPitchAlgoInterfaceImpl::Apply(
    std::vector<uint8_t *> &pcmInBuf, std::vector<uint8_t *> &pcmOutBuf)
{
    AUDIO_DEBUG_LOG("Apply tempo and pitch algorithm");
    CHECK_AND_RETURN_RET_LOG(
        !pcmInBuf.empty() && !pcmOutBuf.empty(), ERROR, "Invalid audioInputs or audioOutputs");
    CHECK_AND_RETURN_RET_LOG(
        pcmInBuf[0] != nullptr && pcmOutBuf[0] != nullptr, ERROR, "Apply input para is nullptr");
    CHECK_AND_RETURN_RET_LOG(tempoAlgoHandle_ != nullptr, ERROR,
        "Apply para tempoAlgoHandle_ is nullptr, need init first");
    CHECK_AND_RETURN_RET_LOG(pitchAlgoHandle_ != nullptr, ERROR,
        "Apply para pitchAlgoHandle_ is nullptr, need init first");

    int32_t frameLen = TEMPO_PITCH_ALGO_FRAME_LEN;
    int32_t outFrameLen = -1;
    int32_t copyRet = -1;
    int16_t *pcmIn = reinterpret_cast<int16_t *>(pcmInBuf[0]);
    int16_t *pcmOut = reinterpret_cast<int16_t *>(pcmOutBuf[0]);

    // tempo
    if (speedRate_ != 1.0) {
        outFrameLen = tempoAlgoApi_.apply(
            tempoAlgoHandle_, pcmIn, tempDataOut_.data(), frameLen, tempDataOut_.capacity());
        CHECK_AND_RETURN_RET_LOG(outFrameLen >= 0, outFrameLen, "tempo algo apply error:%{public}d", outFrameLen);
    } else {
        outFrameLen = frameLen;
        copyRet = memcpy_s(tempDataOut_.data(), expendSize_ * sizeof(int16_t), pcmIn, outFrameLen * sizeof(int16_t));
        CHECK_AND_RETURN_RET_LOG(copyRet == 0, ERROR, "output buffer not enough");
    }

    // pitch
    if (pitchRate_ != 1.0 && outFrameLen > 0) {
        AudioBuffer inBuffer = {
            .frameLength = outFrameLen,
            .raw = tempDataOut_.data(),
            .metaData = nullptr
        };
        AudioBuffer outBuffer = {
            .frameLength = outFrameLen,
            .raw = pcmOut,
            .metaData = nullptr
        };
        int32_t ret = (*pitchAlgoHandle_)->process(pitchAlgoHandle_, &inBuffer, &outBuffer);
        outFrameLen = outBuffer.frameLength;
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "apply pitch algo fail:%{public}d", ret);
    } else {
        copyRet = memcpy_s(pcmOut, expendSize_ * sizeof(int16_t), tempDataOut_.data(), outFrameLen * sizeof(int16_t));
        CHECK_AND_RETURN_RET_LOG(copyRet == 0, ERROR, "output buffer not enough");
    }
    // return outFrameLen >= 0
    return outFrameLen;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS