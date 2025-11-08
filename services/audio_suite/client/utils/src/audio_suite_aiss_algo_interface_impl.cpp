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
#define LOG_TAG "AudioSuiteAissAlgoInterfaceImpl"
#endif


#include <chrono>
#include <thread>
#include <dlfcn.h>
#include "audio_errors.h"
#include "audio_suite_aiss_algo_interface_impl.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

constexpr uint32_t DEFAULT_CHANNELS_IN = 2;
constexpr uint32_t DEFAULT_CHANNELS_OUT = 4;
constexpr uint32_t DEFAULT_CHANNEL_IN_LAYOUT = 3;
constexpr uint32_t DEFAULT_CHANNEL_OUT_LAYOUT = 51;
constexpr uint32_t DEFAULT_FRAME_LEN = 960;
constexpr uint32_t DEFAULT_SAMPLE_RATE = 48000;
constexpr uint8_t DEFAULT_SAMPLE_FORMAT = 4;
constexpr uint32_t MAX_UINT_VOLUME = 65535;
constexpr int32_t CHANNEL_1 = 1;
constexpr int32_t CHANNEL_2 = 2;
constexpr int32_t CHANNEL_3 = 3;
constexpr int32_t CHANNEL_4 = 4;
constexpr size_t NUM_THREE = 3;
const std::string AISS_NAME = "aiss";
const std::string AISS_PROPERTY = "AISSVX";
const std::string AISS_LIB = "AISSLIB";

static float UnifyFloatValue(float value)
{
    const float maxFloatValue = 1.f;
    const float minFloatValue = -1.f;
    if (value > maxFloatValue) {
        value = maxFloatValue;
    } else if (value < minFloatValue) {
        value = minFloatValue;
    }
    return value;
}

AudioSuiteAissAlgoInterfaceImpl::AudioSuiteAissAlgoInterfaceImpl(NodeCapability &nc)
{
    nodeCapability = nc;
}

int32_t AudioSuiteAissAlgoInterfaceImpl::Init()
{
    Deinit();
    std::string soPath = nodeCapability.soPath + nodeCapability.soName;
    if (CheckFilePath(soPath) != SUCCESS) {
        AUDIO_ERR_LOG("Check file path failed");
        return ERROR;
    }
    soHandle_ = dlopen(soPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (!soHandle_) {
        AUDIO_ERR_LOG("Error loading audio aiss so: %s", dlerror());
        return ERROR;
    }
    audioEffectLibHandle_ = static_cast<AudioEffectLibrary *>(dlsym(soHandle_, AISS_LIB.c_str()));
    if (!audioEffectLibHandle_) {
        AUDIO_ERR_LOG("Error loading symbol: %s", dlerror());
        Deinit();
        return ERROR;
    }
    AUDIO_DEBUG_LOG("Audio effect handle get success");
    AudioEffectDescriptor descriptor = {
        .libraryName = AISS_NAME,
        .effectName = AISS_NAME
    };
    int32_t ret = audioEffectLibHandle_->createEffect(descriptor, &algoHandle_);
    if (ret != SUCCESS || !algoHandle_) {
        AUDIO_ERR_LOG("Failed to create algo instance, return value is %d", ret);
        Deinit();
        return ERROR;
    }
    AUDIO_DEBUG_LOG("Create effect success");
    if (InitIOBufferConfig() != SUCCESS || InitAudioEffectParam() != SUCCESS || InitConfig() != SUCCESS ||
        InitAudioEffectProperty() != SUCCESS) {
        AUDIO_ERR_LOG("Failed to init aiss algo param");
        Deinit();
        return ERROR;
    }
    AUDIO_DEBUG_LOG("AudioSuiteAissAlgoInterfaceImpl Init success");
    inAudioBuffer_.frameLength = DEFAULT_FRAME_LEN;
    outAudioBuffer_.frameLength = DEFAULT_FRAME_LEN;
    return SUCCESS;
}
 
int32_t AudioSuiteAissAlgoInterfaceImpl::Deinit()
{
    if (algoHandle_ != nullptr && audioEffectLibHandle_ != nullptr) {
        audioEffectLibHandle_->releaseEffect(algoHandle_);
    }
    algoHandle_ = nullptr;
    audioEffectLibHandle_ = nullptr;
    if (soHandle_ != nullptr) {
        dlclose(soHandle_);
    }
    soHandle_ = nullptr;
    AUDIO_INFO_LOG("AudioSuiteAissAlgoInterfaceImpl Deinit success");
    return SUCCESS;
}
 
int32_t AudioSuiteAissAlgoInterfaceImpl::SetParameter(const std::string& paramType, const std::string& paramValue)
{
    return SUCCESS;
}
 
int32_t AudioSuiteAissAlgoInterfaceImpl::GetParameter(const std::string& paramType, std::string& paramValue)
{
    return SUCCESS;
}
 
int32_t AudioSuiteAissAlgoInterfaceImpl::Apply(std::vector<uint8_t*>& v1, std::vector<uint8_t*>& v2)
{
    CHECK_AND_RETURN_RET_LOG(!v1.empty(), ERROR, "Input parameter vector is empty");
    CHECK_AND_RETURN_RET_LOG(v2.size() == NUM_THREE, ERROR, "Output parameter vector size is not equal 3");
    for (uint8_t* ptr : v1) {
        if (ptr == nullptr) {
            AUDIO_ERR_LOG("Input parameter is nullptr");
            return ERROR;
        }
    }
    for (uint8_t* ptr : v2) {
        if (ptr == nullptr) {
            AUDIO_ERR_LOG("Output parameter is nullptr");
            return ERROR;
        }
    }
    inAudioBuffer_.raw = reinterpret_cast<void *>(v1[0]);
    outAudioBuffer_.raw = reinterpret_cast<void *>(v2[0]);
    if (algoHandle_ == nullptr) {
        AUDIO_ERR_LOG("algoHandle_ is nullptr");
        return ERROR;
    }
    int32_t ret = (*algoHandle_)->process(algoHandle_, &inAudioBuffer_, &outAudioBuffer_);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Apply failed, return value is %d", ret);
        return ERROR;
    }
    float* outAudioBuf = reinterpret_cast<float *>(outAudioBuffer_.raw);
    float* humanAudioBuf = reinterpret_cast<float *>(v2[1]);
    float* bkgAudioBuf = reinterpret_cast<float *>(v2[2]);
    SeparateChannels(outAudioBuffer_.frameLength, outAudioBuf, humanAudioBuf, bkgAudioBuf);
    AUDIO_DEBUG_LOG("AudioSuiteAissAlgoInterfaceImpl Apply success");
    return SUCCESS;
}

int32_t AudioSuiteAissAlgoInterfaceImpl::CheckFilePath(std::string &filePath)
{
    if (filePath.size() >= PATH_MAX) {
        AUDIO_ERR_LOG("File path size is too large: %d", (uint32_t)(filePath.size()));
        return ERROR;
    }
    char buffer[PATH_MAX] = {0};
    char *path = realpath(filePath.c_str(), buffer);
    if (path == nullptr) {
        AUDIO_ERR_LOG("Invalid file path: %s", filePath.c_str());
        return ERROR;
    }
    filePath = buffer;
    AUDIO_DEBUG_LOG("Check file path success, path: %s", filePath.c_str());
    return SUCCESS;
}

int32_t AudioSuiteAissAlgoInterfaceImpl::InitIOBufferConfig()
{
    uint32_t rate = DEFAULT_SAMPLE_RATE;
    uint8_t sampleFormat = DEFAULT_SAMPLE_FORMAT;
    AudioEncodingType encodingType = AudioEncodingType::ENCODING_PCM;
    AudioBufferConfig inBufferConfig = {rate, DEFAULT_CHANNELS_IN, sampleFormat,
        DEFAULT_CHANNEL_IN_LAYOUT, encodingType};
    AudioBufferConfig outBufferConfig = {rate, DEFAULT_CHANNELS_OUT, sampleFormat,
        DEFAULT_CHANNEL_OUT_LAYOUT, encodingType};
    AudioEffectConfig ioBufferConfig = {inBufferConfig, outBufferConfig};
    AudioEffectTransInfo cmdInfo = {sizeof(AudioEffectConfig), &ioBufferConfig};
    int32_t replyData = 0;
    AudioEffectTransInfo replyInfo = {sizeof(int32_t), &replyData};
    int32_t ret = (*algoHandle_)->command(algoHandle_, EFFECT_CMD_INIT, &cmdInfo, &replyInfo);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("EFFECT_CMD_INIT failed, return value is %d", ret);
        return ERROR;
    }
    ret = (*algoHandle_)->command(algoHandle_, EFFECT_CMD_ENABLE, &cmdInfo, &replyInfo);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("EFFECT_CMD_ENABLE failed, return value is %d", ret);
        return ERROR;
    }
    AUDIO_DEBUG_LOG("AudioSuiteAissAlgoInterfaceImpl::InitIOBufferConfig success");
    return SUCCESS;
}

int32_t AudioSuiteAissAlgoInterfaceImpl::InitConfig()
{
    uint32_t rate = DEFAULT_SAMPLE_RATE;
    uint8_t sampleFormat = DEFAULT_SAMPLE_FORMAT;
    AudioEncodingType encodingType = AudioEncodingType::ENCODING_PCM;
    AudioBufferConfig inBufferConfig = {rate, DEFAULT_CHANNELS_IN, sampleFormat,
        DEFAULT_CHANNEL_IN_LAYOUT, encodingType};
    AudioBufferConfig outBufferConfig = {rate, DEFAULT_CHANNELS_OUT, sampleFormat,
        DEFAULT_CHANNEL_OUT_LAYOUT, encodingType};
    AudioEffectConfig ioBufferConfig = {inBufferConfig, outBufferConfig};
    AudioEffectTransInfo cmdInfo = {sizeof(AudioEffectConfig), &ioBufferConfig};
    int32_t replyData = 0;
    AudioEffectTransInfo replyInfo = {sizeof(int32_t), &replyData};
    int32_t ret = (*algoHandle_)->command(algoHandle_, EFFECT_CMD_SET_CONFIG, &cmdInfo, &replyInfo);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("EFFECT_CMD_SET_CONFIG failed, return value %d", ret);
        return ERROR;
    }
    AUDIO_DEBUG_LOG("AudioSuiteAissAlgoInterfaceImpl::InitConfig success");
    return SUCCESS;
}

int32_t AudioSuiteAissAlgoInterfaceImpl::InitAudioEffectParam()
{
    int32_t replyData = 0;
    AudioEffectTransInfo replyInfo = {sizeof(int32_t), &replyData};
    std::vector<uint8_t> paramBuffer(sizeof(AudioEffectParam) + MAX_PARAM_INDEX * sizeof(int32_t));
    AudioEffectParam *effectParam = reinterpret_cast<AudioEffectParam*>(paramBuffer.data());
    if (!effectParam) {
        return ERROR;
    }
    effectParam->status = 0;
    effectParam->paramSize = sizeof(int32_t);
    effectParam->valueSize = 0;
    int32_t *data = &(effectParam->data[0]);
    data[COMMAND_CODE_INDEX] = EFFECT_SET_PARAM;
    data[SCENE_TYPE_INDEX] = static_cast<int32_t>(SCENE_MUSIC);
    data[EFFECT_MODE_INDEX] = EFFECT_DEFAULT;
    data[ROTATION_INDEX] = 0;
    data[VOLUME_INDEX] = static_cast<int32_t>(MAX_UINT_VOLUME);
    data[EXTRA_SCENE_TYPE_INDEX] = 0;
    data[SPATIAL_DEVICE_TYPE_INDEX] = EARPHONE_TYPE_NONE;
    data[SPATIALIZATION_SCENE_TYPE_INDEX] = SPATIALIZATION_SCENE_TYPE_MUSIC;
    data[SPATIALIZATION_ENABLED_INDEX] = false;
    data[STREAM_USAGE_INDEX] = STREAM_USAGE_MUSIC;
    data[FOLD_STATE_INDEX] = static_cast<int32_t>(0);
    data[LID_STATE_INDEX] = static_cast<int32_t>(0);
    data[ABS_VOLUME_STATE] = static_cast<int32_t>(1);
    AudioEffectTransInfo cmdInfo = {sizeof(AudioEffectParam) + sizeof(int32_t) * MAX_PARAM_INDEX, effectParam};
    int32_t ret = (*algoHandle_)->command(algoHandle_, EFFECT_CMD_SET_PARAM, &cmdInfo, &replyInfo);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("EFFECT_CMD_SET_PARAM failed, return value is %d", ret);
        return ERROR;
    }
    AUDIO_DEBUG_LOG("AudioSuiteAissAlgoInterfaceImpl::InitAudioEffectParam success");
    return SUCCESS;
}

int32_t AudioSuiteAissAlgoInterfaceImpl::InitAudioEffectProperty()
{
    std::string property = AISS_PROPERTY;
    const char *propCstr = property.c_str();
    AudioEffectTransInfo cmdInfo = {sizeof(const char *), reinterpret_cast<void*>(&propCstr)};
    int32_t replyData = 0;
    AudioEffectTransInfo replyInfo = {sizeof(int32_t), &replyData};
    int32_t ret = (*algoHandle_)->command(algoHandle_, EFFECT_CMD_SET_PROPERTY, &cmdInfo, &replyInfo);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("EFFECT_CMD_SET_PROPERTY failed, return value is %d", ret);
        return ERROR;
    }
    AUDIO_DEBUG_LOG("AudioSuiteAissAlgoInterfaceImpl::InitAudioEffectProperty success");
    return SUCCESS;
}

void AudioSuiteAissAlgoInterfaceImpl::SeparateChannels(const int32_t &frameLength, float *input,
    float *humanOutput, float *bkgOutput)
{
    if (!input || !humanOutput || !bkgOutput) {
        AUDIO_ERR_LOG("AudioSuiteAissAlgoInterfaceImpl SeparateChannels input param pointer is nullptr");
        return;
    }
    for (int32_t j = 0; j < frameLength; ++j) {
        humanOutput[j * CHANNEL_2] = UnifyFloatValue(input[CHANNEL_4 * j]);
        humanOutput[j * CHANNEL_2 + CHANNEL_1] =  UnifyFloatValue(input[CHANNEL_4 * j + CHANNEL_1]);
        bkgOutput[j * CHANNEL_2] = UnifyFloatValue(input[CHANNEL_4 * j + CHANNEL_2]);
        bkgOutput[j * CHANNEL_2 + CHANNEL_1] = UnifyFloatValue(input[CHANNEL_4 * j + CHANNEL_3]);
    }
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS