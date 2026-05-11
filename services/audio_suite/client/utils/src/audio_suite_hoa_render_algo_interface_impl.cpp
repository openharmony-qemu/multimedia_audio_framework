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
#define LOG_TAG "AudioSuiteHoaRenderAlgoInterface"
#endif

#include <cstring>
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_hoa_render_algo_interface_impl.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

namespace {
static constexpr int32_t USE_FORMAT_INPUT_FLAG = 0;
static constexpr int32_t INVALID_HOA_FORMAT = -1;
static constexpr int16_t MIN_AMB_ORDER = 0;
static constexpr int16_t MAX_AMB_ORDER = 3;
}  // namespace

AudioSuiteHoaRenderAlgoInterfaceImpl::AudioSuiteHoaRenderAlgoInterfaceImpl(NodeParameter &nc)
{
    nodeParameter_ = nc;
    AUDIO_INFO_LOG("AudioSuiteHoaRenderAlgoInterfaceImpl constructed");
}

AudioSuiteHoaRenderAlgoInterfaceImpl::~AudioSuiteHoaRenderAlgoInterfaceImpl()
{
    if (isHoaAlgoInit_) {
        Deinit();
    }
}

int32_t AudioSuiteHoaRenderAlgoInterfaceImpl::Init()
{
    std::string soPath = nodeParameter_.soPath + nodeParameter_.soName;
    libHandle_ = algoLibrary_.LoadLibrary(soPath);
    CHECK_AND_RETURN_RET_LOG(libHandle_ != nullptr, ERROR,
        "LoadLibrary failed with path: %{private}s", soPath.c_str());

    algoApi_.getSize = reinterpret_cast<FunAudioHvsGetSize>(dlsym(libHandle_, "AudioHvsGetSize"));
    CHECK_AND_RETURN_RET_LOG(algoApi_.getSize != nullptr, ERROR, "Failed to get symbol AudioHvsGetSize");

    algoApi_.initAlgo = reinterpret_cast<FunAudioHvsInit>(dlsym(libHandle_, "AudioHvsInit"));
    CHECK_AND_RETURN_RET_LOG(algoApi_.initAlgo != nullptr, ERROR, "Failed to get symbol AudioHvsInit");

    algoApi_.setParaAlgo = reinterpret_cast<FunAudioHvsSetPara>(dlsym(libHandle_, "AudioHvsSetPara"));
 	CHECK_AND_RETURN_RET_LOG(algoApi_.setParaAlgo != nullptr, ERROR, "Failed to get symbol AudioHvsSetPara");

    algoApi_.applyAlgo = reinterpret_cast<FunAudioHvsApply>(dlsym(libHandle_, "AudioHvsApply"));
    CHECK_AND_RETURN_RET_LOG(algoApi_.applyAlgo != nullptr, ERROR, "Failed to get symbol AudioHvsApply");

    AudioHvsMemSize memSize = {0, 0};
    int32_t ret = algoApi_.getSize(&memSize);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_HVS_OK, ret, "AudioHvsGetSize failed, ret = %{public}d", ret);

    if (memSize.handleSize > 0) {
        algoHandle_.resize(memSize.handleSize, 0);
    }
    if (memSize.scratchBufSize > 0) {
        scratchBuf_.resize(memSize.scratchBufSize, 0);
    }

    ret = algoApi_.initAlgo(algoHandle_.data(), scratchBuf_.data());
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_HVS_OK, ret, "AudioHvsInit failed, ret = %{public}d", ret);

    ret = algoApi_.setParaAlgo(algoHandle_.data(), scratchBuf_.data());
 	CHECK_AND_RETURN_RET_LOG(ret == AUDIO_HVS_OK, ret, "AudioHvsSetPara failed, ret = %{public}d", ret);

    isHoaAlgoInit_ = true;
    AUDIO_INFO_LOG("AudioSuiteHoaRenderAlgoInterfaceImpl::Init end");
    return SUCCESS;
}

int32_t AudioSuiteHoaRenderAlgoInterfaceImpl::Deinit()
{
    isHoaAlgoInit_ = false;
    if (libHandle_ != nullptr) {
        int32_t ret = dlclose(libHandle_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "dlclose failed: %{public}s", dlerror());
        libHandle_ = nullptr;
    }
    static_cast<void>(memset_s(&algoApi_, sizeof(algoApi_), 0, sizeof(algoApi_)));
    algoHandle_.clear();
    scratchBuf_.clear();
    AUDIO_INFO_LOG("AudioSuiteHoaRenderAlgoInterfaceImpl::Deinit end");
    return SUCCESS;
}

int32_t AudioSuiteHoaRenderAlgoInterfaceImpl::SetParameter(
    const std::string &paramType, const std::string &paramValue)
{
    AUDIO_INFO_LOG("Set hoa render param start");
    uint64_t layoutValue = 0;
    CHECK_AND_RETURN_RET_LOG(StringConverter(paramValue, layoutValue), ERROR,
        "Failed to convert layout value: %{public}s", paramValue.c_str());
    channelLayout_ = static_cast<AudioChannelLayout>(layoutValue);
    return SUCCESS;
}

int32_t AudioSuiteHoaRenderAlgoInterfaceImpl::GetParameter(
    const std::string &paramType, std::string &paramValue)
{
    AUDIO_INFO_LOG("HOA GetParameter: %{public}s (no-op for ambisonic mode)", paramType.c_str());
    return SUCCESS;
}

int32_t AudioSuiteHoaRenderAlgoInterfaceImpl::Apply(
    std::vector<uint8_t *> &pcmInBuf, std::vector<uint8_t *> &pcmOutBuf)
{
    CHECK_AND_RETURN_RET_LOG(isHoaAlgoInit_, ERROR, "HOA algo not initialized");
    CHECK_AND_RETURN_RET_LOG(!pcmInBuf.empty(), ERROR, "pcmInBuf is empty");
    CHECK_AND_RETURN_RET_LOG(pcmInBuf[0] != nullptr, ERROR, "pcmInBuf[0] is nullptr");
    CHECK_AND_RETURN_RET_LOG(!pcmOutBuf.empty(), ERROR, "pcmOutBuf is empty");
    CHECK_AND_RETURN_RET_LOG(pcmOutBuf[0] != nullptr, ERROR, "pcmOutBuf[0] is nullptr");

    int32_t *dataIn = reinterpret_cast<int32_t *>(pcmInBuf[0]);
    int32_t *dataOut = reinterpret_cast<int32_t *>(pcmOutBuf[0]);

    int16_t dataAmbOrder = AudioSuiteUtil::GetAmbOrder(channelLayout_);
    int32_t hoaInputFormat = AudioSuiteUtil::GetHoaInputFormat(channelLayout_);

    CHECK_AND_RETURN_RET_LOG(dataAmbOrder >= MIN_AMB_ORDER && dataAmbOrder <= MAX_AMB_ORDER, ERROR,
        "Invalid Ambisonics order: %{public}d", dataAmbOrder);
    CHECK_AND_RETURN_RET_LOG(hoaInputFormat != INVALID_HOA_FORMAT, ERROR, "Invalid HOA input format");

    AudioHvsDynamicData data;
    data.dataIn = dataIn;
    data.dataOut = dataOut;
    data.samplingRate = nodeParameter_.inSampleRate;
    data.stereoDownmix = 0;
    data.dataAmbOrder = dataAmbOrder;
    data.renderAmbOrder = dataAmbOrder;
    data.useAFormatInput = USE_FORMAT_INPUT_FLAG;
    data.hoaInputFormat = hoaInputFormat;
    data.dynUserData.quaterninos[0] = 1.0f;
    data.dynUserData.quaterninos[1] = 0.0f;
    data.dynUserData.quaterninos[2] = 0.0f;
    data.dynUserData.quaterninos[3] = 0.0f;

    int32_t ret = algoApi_.applyAlgo(algoHandle_.data(), scratchBuf_.data(), &data);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_HVS_OK, ret,
        "HOA algo apply failed, ret = %{public}d", ret);

    return SUCCESS;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS