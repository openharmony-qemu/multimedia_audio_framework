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
#define LOG_TAG "AudioSuiteVoiceMorphingAlgoInterfaceImpl"
#endif

#include <dlfcn.h>
#include <cstring>
#include "securec.h"
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_voice_morphing_algo_interface_impl.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

namespace {
constexpr int32_t DEFAULT_FRAME_LEN = 960;  // single channel sample point number.
constexpr int32_t DEFAULT_CHANNEL_COUNT = 2;
const std::string voiceMorphingMode = "VoiceBeautifierType";
const std::string generalVoiceChangeMode = "AudioGeneralVoiceChangeType";
}  // namespace

AudioSuiteVoiceMorphingAlgoInterfaceImpl::AudioSuiteVoiceMorphingAlgoInterfaceImpl(NodeCapability &nc)
{
    nodeCapability = nc;
}

AudioSuiteVoiceMorphingAlgoInterfaceImpl::~AudioSuiteVoiceMorphingAlgoInterfaceImpl()
{
    Deinit();
}

int32_t AudioSuiteVoiceMorphingAlgoInterfaceImpl::LoadAlgorithmFunction(void)
{
    vmAlgoApi_.getSize = reinterpret_cast<FunAudioVoiceMorphingGetsize>(dlsym(libHandle_, "AudioVoiceMorphingGetsize"));
    vmAlgoApi_.init = reinterpret_cast<FunAudioVoiceMorphingInit>(dlsym(libHandle_, "AudioVoiceMorphingInit"));
    vmAlgoApi_.setParam =
        reinterpret_cast<FunAudioVoiceMorphingSetParam>(dlsym(libHandle_, "AudioVoiceMorphingSetParam"));
    vmAlgoApi_.apply = reinterpret_cast<FunAudioVoiceMorphingApply>(dlsym(libHandle_, "AudioVoiceMorphingApply"));

    bool loadAlgoApiFail = vmAlgoApi_.getSize == nullptr || vmAlgoApi_.init == nullptr ||
                           vmAlgoApi_.setParam == nullptr || vmAlgoApi_.apply == nullptr;

    return loadAlgoApiFail ? ERROR : SUCCESS;
}

int32_t AudioSuiteVoiceMorphingAlgoInterfaceImpl::ApplyAndWaitReady(void)
{
    AUDIO_INFO_LOG("start load vm algo so");

    std::string soPath = nodeCapability.soPath + nodeCapability.soName;
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

    AUDIO_INFO_LOG("end load vm algo so");
    return SUCCESS;
}

void AudioSuiteVoiceMorphingAlgoInterfaceImpl::UnApply(void)
{
    AUDIO_INFO_LOG("start unload vm algo so");

    if (libHandle_ != nullptr) {
        static_cast<void>(dlclose(libHandle_));
    }
    libHandle_ = nullptr;
    static_cast<void>(memset_s(&vmAlgoApi_, sizeof(vmAlgoApi_), 0, sizeof(vmAlgoApi_)));

    AUDIO_INFO_LOG("end unload vm algo so");
}

int32_t AudioSuiteVoiceMorphingAlgoInterfaceImpl::Init()
{
    AUDIO_INFO_LOG("start init voice Morphing algorithm");

    int32_t ret = ApplyAndWaitReady();
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);

    AudioVoiceMorphingMemSize memSize;
    ret = vmAlgoApi_.getSize(&memSize);
    if (ret != AUDIO_VMP_EOK) {
        AUDIO_ERR_LOG("AudioVoiceMorphingGetsize fail");
        return ERROR;
    }
    handle_.resize(memSize.stateSize);
    scratchBuf_.resize(memSize.scratchSize);

    inBuf_.resize(DEFAULT_FRAME_LEN * DEFAULT_CHANNEL_COUNT * sizeof(uint32_t));
    outBuf_.resize(DEFAULT_FRAME_LEN * DEFAULT_CHANNEL_COUNT * sizeof(uint32_t));

    ret = vmAlgoApi_.init(handle_.data(), scratchBuf_.data());
    if (ret != AUDIO_VMP_EOK) {
        AUDIO_ERR_LOG("Init vmalgo fail");
        return ERROR;
    }

    AUDIO_INFO_LOG("init vm algoso success");
    return SUCCESS;
}

int32_t AudioSuiteVoiceMorphingAlgoInterfaceImpl::Deinit()
{
    AUDIO_INFO_LOG("start deinit vm algorithm");
    UnApply();
    AUDIO_INFO_LOG("end deinit vm algorithm");
    return SUCCESS;
}

int32_t AudioSuiteVoiceMorphingAlgoInterfaceImpl::SetParameter(
    const std::string &paramType_, const std::string &paramValue)
{
    using typeIterator = std::unordered_map<std::string, AudioVoiceMorphingType>::const_iterator;
    typeIterator beautyType = voiceBeautifierTypeMap.end();
    typeIterator generalType = generalVoiceChangeTypeMap.end();
    if (paramType_ == voiceMorphingMode) {
        beautyType = voiceBeautifierTypeMap.find(paramValue);
    } else if (paramType_ == generalVoiceChangeMode) {
        generalType = generalVoiceChangeTypeMap.find(paramValue);
    }
 
    if (beautyType == voiceBeautifierTypeMap.end() && generalType == generalVoiceChangeTypeMap.end()) {
        AUDIO_ERR_LOG("SetOptions UNKNOWN TYPE");
        return ERROR;
    } else if (beautyType != voiceBeautifierTypeMap.end()) {
        int32_t ret = vmAlgoApi_.setParam(handle_.data(), beautyType->second);
        CHECK_AND_RETURN_RET_LOG(ret == AUDIO_VMP_EOK, ERROR, "Algo setParam failed with %{public}d", ret);
        return SUCCESS;
    } else {
        int32_t ret = vmAlgoApi_.setParam(handle_.data(), generalType->second);
        CHECK_AND_RETURN_RET_LOG(ret == AUDIO_VMP_EOK, ERROR, "Algo setParam failed with %{public}d", ret);
        return SUCCESS;
    }
}

int32_t AudioSuiteVoiceMorphingAlgoInterfaceImpl::Apply(
    std::vector<uint8_t *> &audioInputs, std::vector<uint8_t *> &audioOutputs)
{
    AUDIO_INFO_LOG("start apply vm algorithm");

    if (audioInputs.empty() || audioOutputs.empty()) {
        AUDIO_ERR_LOG("Apply para check fail, input or output list is empty");
        return ERROR;
    }

    if (audioInputs[0] == nullptr || audioOutputs[0] == nullptr) {
        AUDIO_ERR_LOG("Apply para check fail, input or output is nullptr");
        return ERROR;
    }

    AudioVoiceMorphingData data = {
        .dataIn = reinterpret_cast<int *>(inBuf_.data()),
        .dataOut = reinterpret_cast<int *>(outBuf_.data()),
        .dataSize = DEFAULT_FRAME_LEN,
        .enableFlag = 1,
        .dataFormat = 1,
        .inCh = 2,
        .outCh = 2,
    };

    int16_t *inPcm = reinterpret_cast<int16_t *>(audioInputs[0]);
    int16_t *outPcm = reinterpret_cast<int16_t *>(audioOutputs[0]);
    int32_t offset = 16;
    int32_t requiredSize = DEFAULT_FRAME_LEN * DEFAULT_CHANNEL_COUNT;
    for (int32_t i = 0; i < requiredSize; i++) {
        inBuf_[i] = inPcm[i];
        inBuf_[i] <<= offset;
    }
    int32_t ret = vmAlgoApi_.apply(&data, handle_.data(), scratchBuf_.data());
    if (ret != AUDIO_VMP_EOK) {
        AUDIO_ERR_LOG("apply vmalgo fail, error code: %{public}d.", ret);
        return ERROR;
    }

    for (int32_t i = 0; i < DEFAULT_FRAME_LEN * DEFAULT_CHANNEL_COUNT; i++) {
        outPcm[i] = outBuf_[i] >> offset;
    }

    return SUCCESS;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS