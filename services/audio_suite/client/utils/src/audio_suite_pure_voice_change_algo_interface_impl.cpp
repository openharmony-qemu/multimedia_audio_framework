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
#define LOG_TAG "AudioSuitePureVoiceChangeAlgoInterfaceImpl"
#endif

#include <dlfcn.h>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdint>
#include "audio_suite_log.h"
#include "audio_suite_pure_voice_change_algo_interface_impl.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
namespace {
constexpr int32_t DEFAULT_FRAME_LEN = 640;
constexpr int32_t NUMBER_OF_PARAMETER = 3;
constexpr int32_t NUMBER_OF_CHANNEL = 2;
const float PCM_SAMPLE_AVERAGE_FACTOR = 0.5f;
const float PCM_SAMPLE_SCALE_FACTOR = 1.0f / 32768.0f;
const float PCM_SAMPLE_CLIP_MAX = 32767.0f;
const float PCM_SAMPLE_CLIP_MIN = -32768.0f;
static const float AUDIO_VOICE_MORPHING_PITCH_MIN = 0.3f;
static const float AUDIO_VOICE_MORPHING_PITCH_MAX = 3.0f;
}

AudioSuitePureVoiceChangeAlgoInterfaceImpl::AudioSuitePureVoiceChangeAlgoInterfaceImpl(NodeCapability &nc)
{
    nodeCapability = nc;
}

AudioSuitePureVoiceChangeAlgoInterfaceImpl::~AudioSuitePureVoiceChangeAlgoInterfaceImpl()
{
    Deinit();
}

int32_t AudioSuitePureVoiceChangeAlgoInterfaceImpl::LoadAlgorithmFunction(void)
{
    vmAlgoApi_.getSize = reinterpret_cast<FunVoiceMphGetSize>(dlsym(libHandle_, "AudioVoiceMphGetsize"));
    CHECK_AND_RETURN_RET_LOG(vmAlgoApi_.getSize != nullptr, ERROR, "Failed to get symbol AudioVoiceMphGetsize");
    vmAlgoApi_.initAlgo = reinterpret_cast<FunVoiceMphInit>(dlsym(libHandle_, "AudioVoiceMphInit"));
    CHECK_AND_RETURN_RET_LOG(vmAlgoApi_.initAlgo != nullptr, ERROR, "Failed to get symbol AudioVoiceMphInit");
    vmAlgoApi_.applyAlgo = reinterpret_cast<FunVoiceMphApply>(dlsym(libHandle_, "AudioVoiceMphApply"));
    CHECK_AND_RETURN_RET_LOG(vmAlgoApi_.applyAlgo != nullptr, ERROR, "Failed to get symbol AudioVoiceMphApply");
    vmAlgoApi_.setPara = reinterpret_cast<FunVoiceMphSetPara>(dlsym(libHandle_, "AudioVoiceMphSetPara"));
    CHECK_AND_RETURN_RET_LOG(vmAlgoApi_.setPara != nullptr, ERROR, "Failed to get symbol AudioVoiceMphSetPara");

    bool loadAlgoApiFail = vmAlgoApi_.getSize == nullptr || vmAlgoApi_.initAlgo == nullptr ||
                           vmAlgoApi_.setPara == nullptr || vmAlgoApi_.applyAlgo == nullptr;

    return loadAlgoApiFail ? ERROR : SUCCESS;
}

int32_t AudioSuitePureVoiceChangeAlgoInterfaceImpl::ApplyAndWaitReady(void)
{
    AUDIO_INFO_LOG("start load vm algo so");
    std::string soPath = nodeCapability.soPath + nodeCapability.soName;
    libHandle_ = algoLibrary_.LoadLibrary(soPath);
    CHECK_AND_RETURN_RET_LOG(libHandle_ != nullptr, ERROR,
        "LoadLibrary failed with path: %{private}s", soPath.c_str());

    if (LoadAlgorithmFunction() != SUCCESS) {
        AUDIO_ERR_LOG("LoadAlgorithmFunction fail");
        UnApply();
        return ERROR;
    }

    AUDIO_INFO_LOG("end load vm algo so");
    return SUCCESS;
}

void AudioSuitePureVoiceChangeAlgoInterfaceImpl::UnApply(void)
{
    AUDIO_INFO_LOG("start unload pure algo so");

    if (libHandle_ != nullptr) {
        static_cast<void>(dlclose(libHandle_));
    }
    libHandle_ = nullptr;
    static_cast<void>(memset_s(&vmAlgoApi_, sizeof(vmAlgoApi_), 0, sizeof(vmAlgoApi_)));

    AUDIO_INFO_LOG("end unload pure algo so");
}

void AudioSuitePureVoiceChangeAlgoInterfaceImpl::Release()
{
    if (handle_) {
        delete[] handle_;
        handle_ = nullptr;
    }
    if (scratchBuf_) {
        delete[] scratchBuf_;
        scratchBuf_ = nullptr;
    }
}

int32_t AudioSuitePureVoiceChangeAlgoInterfaceImpl::Init()
{
    AUDIO_INFO_LOG("start init pure voice Morphing algorithm");

    int32_t ret = ApplyAndWaitReady();
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    AudioVoiceMphMemSize* memSize = new AudioVoiceMphMemSize();
    ret = vmAlgoApi_.getSize(memSize);
    if (ret != AUDIO_VOICEMPH_EOK) {
        AUDIO_ERR_LOG("AudioVoiceMphGetsize fail");
        delete memSize;
        memSize = nullptr;
        return ERROR;
    }
    handle_ = new char[memSize->stateSize];
    if (!handle_) {
        AUDIO_ERR_LOG("Init handle_ fail");
        delete memSize;
        memSize = nullptr;
        return ERROR;
    }
    scratchBuf_ = new char[memSize->scratchSize];
    if (!scratchBuf_) {
        AUDIO_ERR_LOG("Init scratchBuf_ fail");
        delete memSize;
        delete handle_;
        memSize = nullptr;
        handle_ = nullptr;
        return ERROR;
    }
    delete memSize;
    memSize = nullptr;

    inBuf_.resize(DEFAULT_FRAME_LEN * sizeof(float));
    outBuf_.resize(DEFAULT_FRAME_LEN * sizeof(float));

    ret = vmAlgoApi_.initAlgo(handle_, scratchBuf_);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_VOICEMPH_EOK, ERROR, "Init pure algo fail");
    AUDIO_INFO_LOG("init pure algoso success");
    return SUCCESS;
}

int32_t AudioSuitePureVoiceChangeAlgoInterfaceImpl::Deinit()
{
    AUDIO_INFO_LOG("start deinit pure algorithm");
    Release();
    UnApply();
    AUDIO_INFO_LOG("end deinit pure algorithm");
    return SUCCESS;
}

static std::vector<float> ParseStringToIntArray(const std::string &str, char delimiter)
{
    std::vector<float> result;
    std::string paramValue;
    std::istringstream iss(str);

    while (std::getline(iss, paramValue, delimiter)) {
        if (!paramValue.empty()) {
            float value;
            CHECK_AND_RETURN_RET_LOG(StringConverterFloat(paramValue, value), std::vector<float>(),
                "Pure voice change convert string to float value error, invalid data is %{public}s",
                paramValue.c_str());
            result.push_back(value);
        }
    }

    return result;
}

int32_t AudioSuitePureVoiceChangeAlgoInterfaceImpl::SetParameter(
    const std::string &paramType, const std::string &paramValue)
{
    std::vector<float> gainValue = ParseStringToIntArray(paramValue, ',');
    CHECK_AND_RETURN_RET_LOG(
        gainValue.size() == NUMBER_OF_PARAMETER, ERROR, "Wrong number of parameters %{public}zu", gainValue.size());
    auto typePtr = pureTypeMap.find(std::to_string(static_cast<int32_t>(gainValue[1])));
    CHECK_AND_RETURN_RET_LOG(
        typePtr != pureTypeMap.end(), ERROR, "Unknow type %{public}d", static_cast<int32_t>(gainValue[1]));
 
    auto typeSexPtr = pureSexTypeMap.find(std::to_string(static_cast<int32_t>(gainValue[0])));
    CHECK_AND_RETURN_RET_LOG(
        typeSexPtr != pureSexTypeMap.end(), ERROR, "Unknow Sex type %{public}d", static_cast<int32_t>(gainValue[0]));
 
    float pitch = static_cast<float>(gainValue[2]);
 
    CHECK_AND_RETURN_RET_LOG(
        (pitch >= AUDIO_VOICE_MORPHING_PITCH_MIN && pitch <= AUDIO_VOICE_MORPHING_PITCH_MAX) || pitch == 0.0f,
        ERROR,
        "Unknow Pitch value %{public}f",
        pitch);
 
    SpeakerSex valueSex = typeSexPtr->second;
    AudioVoiceMphTradType voiceType = typePtr->second;
    int32_t ret = vmAlgoApi_.setPara(handle_, valueSex, voiceType, pitch);

    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_VOICEMPH_EOK, ERROR, "Algo setParam failed with %{public}d", ret);
    return SUCCESS;
}

int32_t AudioSuitePureVoiceChangeAlgoInterfaceImpl::GetParameter(const std::string &paramType, std::string &paramValue)
{
    return SUCCESS;
}

int32_t AudioSuitePureVoiceChangeAlgoInterfaceImpl::Apply(
    std::vector<uint8_t *> &audioInputs, std::vector<uint8_t *> &audioOutputs)
{
    AUDIO_DEBUG_LOG("start apply pure algorithm");

    CHECK_AND_RETURN_RET_LOG(
        !audioInputs.empty() && !audioOutputs.empty(), ERROR, "Apply para check fail, input or output list is empty");

    CHECK_AND_RETURN_RET_LOG(audioInputs[0] != nullptr && audioOutputs[0] != nullptr,
        ERROR,
        "Apply para check fail, input or output list is empty");

    int16_t *inPcm = reinterpret_cast<int16_t *>(audioInputs[0]);
    int16_t *outPcm = reinterpret_cast<int16_t *>(audioOutputs[0]);

    int32_t src_index = 0;
    for (int32_t i = 0; i < DEFAULT_FRAME_LEN; i++, src_index += NUMBER_OF_CHANNEL) {
        inBuf_[i] = (static_cast<float>(inPcm[src_index]) + static_cast<float>(inPcm[src_index + 1])) *
                    PCM_SAMPLE_AVERAGE_FACTOR * PCM_SAMPLE_SCALE_FACTOR;
    }

    AudioVoiceMphData data = {
        .dataIn = reinterpret_cast<float *>(inBuf_.data()),
        .dataOut = reinterpret_cast<float *>(outBuf_.data()),
        .inCh = 1,
        .outCh = 1,
    };

    int32_t ret = vmAlgoApi_.applyAlgo(handle_, scratchBuf_, &data);

    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_VOICEMPH_EOK, ERROR, "apply vmalgo fail.");

    int32_t outIndex = 0;
    for (int32_t i = 0; i < DEFAULT_FRAME_LEN; i++) {
        float sample = outBuf_[i] * PCM_SAMPLE_CLIP_MAX;
        int16_t outSample = static_cast<int16_t>((sample > PCM_SAMPLE_CLIP_MAX)   ? INT16_MAX
                                                 : (sample < PCM_SAMPLE_CLIP_MIN) ? INT16_MIN
                                                                                  : sample);
        outPcm[outIndex++] = outSample;  // left channel
        outPcm[outIndex++] = outSample;  // right channel
    }

    return SUCCESS;
}
}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS