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
#define LOG_TAG "AudioSuiteVoiceBeautifierAlgoInterfaceImpl"
#endif

#include <dlfcn.h>
#include <cstring>
#include "securec.h"
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_voice_beautifier_algo_interface_impl.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

namespace {
constexpr int32_t DEFAULT_FRAME_LEN = 960; // single channel sample point number.
constexpr int32_t DEFAULT_CHANNEL_COUNT = 2;
}  // namespace

AudioSuiteVoiceBeautifierAlgoInterfaceImpl::AudioSuiteVoiceBeautifierAlgoInterfaceImpl(NodeCapability &nc)
{
    nodeCapability = nc;
}

AudioSuiteVoiceBeautifierAlgoInterfaceImpl::~AudioSuiteVoiceBeautifierAlgoInterfaceImpl()
{
    Deinit();
}

int32_t AudioSuiteVoiceBeautifierAlgoInterfaceImpl::LoadAlgorithmFunction(void)
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

int32_t AudioSuiteVoiceBeautifierAlgoInterfaceImpl::ApplyAndWaitReady(void)
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

void AudioSuiteVoiceBeautifierAlgoInterfaceImpl::UnApply(void)
{
    AUDIO_INFO_LOG("start unload vm algo so");

    if (libHandle_ != nullptr) {
        static_cast<void>(dlclose(libHandle_));
    }
    libHandle_ = nullptr;
    static_cast<void>(memset_s(&vmAlgoApi_, sizeof(vmAlgoApi_), 0, sizeof(vmAlgoApi_)));

    AUDIO_INFO_LOG("end unload vm algo so");
}

int32_t AudioSuiteVoiceBeautifierAlgoInterfaceImpl::Init()
{
    AUDIO_INFO_LOG("start init voice Morphing algorithm");

    int32_t ret = ApplyAndWaitReady();
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);

    AudioVoiceMorphingMemSize* memSize = new AudioVoiceMorphingMemSize();
    ret = vmAlgoApi_.getSize(memSize);
    if (ret != AUDIO_VMP_EOK) {
        AUDIO_ERR_LOG("AudioVoiceMorphingGetsize fail");
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
        memSize = nullptr;
        return ERROR;
    }
    delete memSize;
    memSize = nullptr;

    inBuf_ = new uint32_t[sizeof(uint32_t) * DEFAULT_FRAME_LEN * 2];
    if (!inBuf_) {
        AUDIO_ERR_LOG("Init inBuf_ fail");
        return ERROR;
    }
    outBuf_ = new uint32_t[sizeof(uint32_t) * DEFAULT_FRAME_LEN * 2];
    if (!outBuf_) {
        AUDIO_ERR_LOG("Init outBuf_ fail");
        return ERROR;
    }
    ret = vmAlgoApi_.init(handle_, scratchBuf_);
    if (ret != AUDIO_VMP_EOK) {
        AUDIO_ERR_LOG("Init vmalgo fail");
        return ERROR;
    }

    AUDIO_INFO_LOG("init vm algoso success");
    return SUCCESS;
}

int32_t AudioSuiteVoiceBeautifierAlgoInterfaceImpl::Deinit()
{
    AUDIO_INFO_LOG("start deinit vm algorithm");
    Release();
    UnApply();
    AUDIO_INFO_LOG("end deinit vm algorithm");
    return SUCCESS;
}

void AudioSuiteVoiceBeautifierAlgoInterfaceImpl::Release()
{
    if (inBuf_) {
        delete[] inBuf_;
        inBuf_ = nullptr;
    }
    if (outBuf_) {
        delete[] outBuf_;
        outBuf_ = nullptr;
    }
    if (handle_) {
        delete[] handle_;
        handle_ = nullptr;
    }
    if (scratchBuf_) {
        delete[] scratchBuf_;
        scratchBuf_ = nullptr;
    }
}

int32_t AudioSuiteVoiceBeautifierAlgoInterfaceImpl::SetParameter(
    const std::string &paramType_, const std::string &paramValue)
{
    auto type = OPTIONS_MAP.find(paramValue);
    if (type != OPTIONS_MAP.end()) {
        return vmAlgoApi_.setParam(handle_, type->second);
    } else {
        AUDIO_ERR_LOG("SetOptions UNKNOWN TYPE");
        return ERROR;
    }
}

int32_t AudioSuiteVoiceBeautifierAlgoInterfaceImpl::Apply(
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

    if (!inBuf_) {
        AUDIO_ERR_LOG("Init inBuf_ fail");
        return ERROR;
    }

    if (!outBuf_) {
        AUDIO_ERR_LOG("Init outBuf_ fail");
        return ERROR;
    }

    AudioVoiceMorphingData data = {
        .dataIn = reinterpret_cast<int *>(inBuf_),
        .dataOut = reinterpret_cast<int *>(outBuf_),
        .dataSize = DEFAULT_FRAME_LEN,
        .enableFlag = 1,
        .dataFormat = 1,
        .inCh = 2,
        .outCh = 2,
    };

    int16_t* inPcm = reinterpret_cast<int16_t *>(audioInputs[0]);
    int16_t* outPcm = reinterpret_cast<int16_t *>(audioOutputs[0]);
    int32_t offset = 16;
    for (int32_t i = 0; i < DEFAULT_FRAME_LEN * DEFAULT_CHANNEL_COUNT; i++) {
        inBuf_[i] = inPcm[i];
        inBuf_[i] <<= offset;
    }
    int32_t ret = vmAlgoApi_.apply(&data, handle_, scratchBuf_);
    if (ret != AUDIO_VMP_EOK) {
        AUDIO_ERR_LOG("apply vmalgo fail.");
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