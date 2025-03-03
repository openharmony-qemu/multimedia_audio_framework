/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#undef LOG_TAG
#define LOG_TAG "AudioEnhanceChain"

#include "audio_enhance_chain.h"

#include <chrono>

#include "securec.h"
#include "audio_effect_log.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "volume_tools.h"

namespace OHOS {
namespace AudioStandard {

const uint32_t BITLENGTH = 8;
const uint32_t MILLISECOND = 1000;
const uint32_t DEFAULT_FRAMELENGTH = 20;
const uint32_t DEFAULT_SAMPLE_RATE = 48000;
const uint32_t DEFAULT_FORMAT = 2;
const uint32_t DEFAULT_MICNUM = 2;
const uint32_t DEFAULT_ECNUM = 0;
const uint32_t DEFAULT_MICREFNUM = 0;
const uint32_t BYTE_SIZE_SAMPLE_U8 = 1;
const uint32_t BYTE_SIZE_SAMPLE_S16 = 2;
const uint32_t BYTE_SIZE_SAMPLE_S24 = 3;
const uint32_t BYTE_SIZE_SAMPLE_S32 = 4;

const std::vector<std::string> NEED_EC_SCENE = {
    "SCENE_VOIP_UP",
    "SCENE_PRE_ENHANCE",
};

const std::vector<std::string> NEED_MICREF_SCENE = {
    "SCENE_VOIP_UP",
    "SCENE_RECORD",
};

const std::map<uint32_t, AudioSampleFormat> FORMAT_CONVERT_MAP = {
    {BYTE_SIZE_SAMPLE_U8, SAMPLE_U8},
    {BYTE_SIZE_SAMPLE_S16, SAMPLE_S16LE},
    {BYTE_SIZE_SAMPLE_S24, SAMPLE_S24LE},
    {BYTE_SIZE_SAMPLE_S32, SAMPLE_S32LE},
};

AudioEnhanceChain::AudioEnhanceChain(const std::string &scene, const AudioEnhanceParamAdapter &algoParam,
    const AudioEnhanceDeviceAttr &deviceAttr, const bool defaultFlag)
{
    sceneType_ = scene;
    algoParam_ = algoParam;
    defaultFlag_ = defaultFlag;
    deviceAttr_ = deviceAttr;
    if (deviceAttr_.micChannels == 1) {
        deviceAttr_.micChannels = DEFAULT_MICNUM;
    }
    
    InitAudioEnhanceChain();
    InitDump();
}

void AudioEnhanceChain::InitAudioEnhanceChain()
{
    setConfigFlag_ = false;
    needEcFlag_ = false;
    needMicRefFlag_ = false;
    enhanceLibHandles_.clear();
    standByEnhanceHandles_.clear();

    algoSupportedConfig_ = {DEFAULT_FRAMELENGTH, DEFAULT_SAMPLE_RATE, DEFAULT_FORMAT * BITLENGTH,
        deviceAttr_.micChannels, DEFAULT_ECNUM, DEFAULT_MICREFNUM, deviceAttr_.micChannels};
    
    uint32_t byteLenPerFrame = DEFAULT_FRAMELENGTH * (DEFAULT_SAMPLE_RATE / MILLISECOND) * DEFAULT_FORMAT;
    algoAttr_ = {DEFAULT_FORMAT, deviceAttr_.micChannels, byteLenPerFrame};

    if (count(NEED_EC_SCENE.begin(), NEED_EC_SCENE.end(), sceneType_)) {
        needEcFlag_ = true;
        algoSupportedConfig_.ecNum = deviceAttr_.ecChannels;
        algoAttr_.batchLen = deviceAttr_.micChannels + deviceAttr_.ecChannels;
    }

    if (count(NEED_MICREF_SCENE.begin(), NEED_MICREF_SCENE.end(), sceneType_)) {
        needMicRefFlag_ = true;
        algoSupportedConfig_.micRefNum = deviceAttr_.micRefChannels;
        algoAttr_.batchLen += deviceAttr_.micRefChannels;
    }

    algoCache_.input.resize(algoAttr_.byteLenPerFrame * algoAttr_.batchLen);
    algoCache_.output.resize(algoAttr_.byteLenPerFrame * deviceAttr_.micChannels);
    AUDIO_INFO_LOG("micNum: %{public}u ecNum: %{public}u micRefNum: %{public}u outNum: %{public}u \
        byteLenPerFrame: %{public}u inputsize:%{public}zu outputsize:%{public}zu",
        algoSupportedConfig_.micNum, algoSupportedConfig_.ecNum, algoSupportedConfig_.micRefNum,
        algoSupportedConfig_.outNum, byteLenPerFrame, algoCache_.input.size(), algoCache_.output.size());
}

void AudioEnhanceChain::InitDump()
{
    std::string dumpFileName = "Enhance_";
    std::string dumpFileInName = dumpFileName + sceneType_ + "_" + GetTime() + "_In";
    if (needEcFlag_) {
        dumpFileInName += "_EC_" + std::to_string(algoSupportedConfig_.ecNum);
    }
    dumpFileInName += "_Mic_" + std::to_string(algoSupportedConfig_.micNum);
    if (needMicRefFlag_) {
        dumpFileInName += "_MicRef_" + std::to_string(algoSupportedConfig_.micRefNum);
    }
    dumpFileInName += ".pcm";
    std::string dumpFileOutName = dumpFileName + sceneType_ + "_" + GetTime() + "_Out.pcm";
    std::string dumpFileDeInterleaverName = dumpFileName + sceneType_ + "_" + GetTime() + "_DeInterLeaver.pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileInName, &dumpFileIn_);
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileOutName, &dumpFileOut_);
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileDeInterleaverName, &dumpFileDeinterLeaver_);
}

AudioEnhanceChain::~AudioEnhanceChain()
{
    ReleaseEnhanceChain();
    DumpFileUtil::CloseDumpFile(&dumpFileIn_);
    DumpFileUtil::CloseDumpFile(&dumpFileOut_);
    DumpFileUtil::CloseDumpFile(&dumpFileDeinterLeaver_);
}

void AudioEnhanceChain::ReleaseEnhanceChain()
{
    for (uint32_t i = 0; i < standByEnhanceHandles_.size() && i < enhanceLibHandles_.size(); i++) {
        if (!enhanceLibHandles_[i]) {
            continue;
        }
        if (!standByEnhanceHandles_[i]) {
            continue;
        }
        if (!enhanceLibHandles_[i]->releaseEffect) {
            continue;
        }
        enhanceLibHandles_[i]->releaseEffect(standByEnhanceHandles_[i]);
    }
    standByEnhanceHandles_.clear();
    enhanceLibHandles_.clear();
}

int32_t AudioEnhanceChain::SetInputDevice(const std::string &inputDevice, const std::string &deviceName)
{
    if (inputDevice.size() == 0) {
        return SUCCESS;
    }
    algoParam_.preDevice = inputDevice;
    algoParam_.preDeviceName = deviceName;
    AUDIO_INFO_LOG("update input device %{public}s name %{public}s", inputDevice.c_str(), deviceName.c_str());
    std::lock_guard<std::mutex> lock(chainMutex_);
    uint32_t size = standByEnhanceHandles_.size();
    AudioEffectTransInfo cmdInfo = {};
    AudioEffectTransInfo replyInfo = {};
    for (uint32_t index = 0; index < size; index++) {
        auto &handle = standByEnhanceHandles_[index];
        CHECK_AND_RETURN_RET_LOG(SetEnhanceParamToHandle(handle) == SUCCESS, ERROR,
            "[%{public}s] effect EFFECT_CMD_SET_PARAM fail", sceneType_.c_str());
        CHECK_AND_RETURN_RET_LOG((*handle)->command(handle, EFFECT_CMD_INIT, &cmdInfo, &replyInfo) == 0, ERROR,
            "[%{public}s] effect EFFECT_CMD_INIT fail", sceneType_.c_str());
    }
    return SUCCESS;
}

int32_t AudioEnhanceChain::SetFoldState(uint32_t foldState)
{
    if (algoParam_.foldState == foldState) {
        AUDIO_INFO_LOG("no need update fold state %{public}u", foldState);
        return SUCCESS;
    }
    algoParam_.foldState = foldState;
    AUDIO_INFO_LOG("update fold state %{public}u", foldState);
    std::lock_guard<std::mutex> lock(chainMutex_);
    AudioEffectTransInfo cmdInfo = {};
    AudioEffectTransInfo replyInfo = {};
    for (const auto &handle : standByEnhanceHandles_) {
        CHECK_AND_RETURN_RET_LOG(SetEnhanceParamToHandle(handle) == SUCCESS, ERROR,
            "[%{public}s] effect EFFECT_CMD_SET_PARAM fail", sceneType_.c_str());
        CHECK_AND_RETURN_RET_LOG((*handle)->command(handle, EFFECT_CMD_INIT, &cmdInfo, &replyInfo) == 0, ERROR,
            "[%{public}s] effect EFFECT_CMD_INIT fail", sceneType_.c_str());
    }
    return SUCCESS;
}

int32_t AudioEnhanceChain::SetEnhanceParam(bool mute, uint32_t systemVol)
{
    algoParam_.muteInfo = mute;
    algoParam_.volumeInfo = systemVol;

    std::lock_guard<std::mutex> lock(chainMutex_);
    uint32_t size = standByEnhanceHandles_.size();
    AudioEffectTransInfo cmdInfo{};
    AudioEffectTransInfo replyInfo{};
    for (uint32_t index = 0; index < size; index++) {
        auto &handle = standByEnhanceHandles_[index];
        CHECK_AND_RETURN_RET_LOG(SetEnhanceParamToHandle(handle) == SUCCESS, ERROR,
            "[%{public}s] effect EFFECT_CMD_SET_PARAM fail", sceneType_.c_str());
        CHECK_AND_RETURN_RET_LOG(
            (*handle)->command(handle, EFFECT_CMD_INIT, &cmdInfo, &replyInfo) == SUCCESS, ERROR,
            "[%{public}s] effect EFFECT_CMD_INIT fail", sceneType_.c_str());
    }
    return SUCCESS;
}

int32_t AudioEnhanceChain::SetEnhanceParamToHandle(AudioEffectHandle handle)
{
    AudioEffectTransInfo cmdInfo = {};
    AudioEffectTransInfo replyInfo = {};
    AudioEnhanceParam setParam = {algoParam_.muteInfo, algoParam_.volumeInfo, algoParam_.foldState,
        algoParam_.preDevice.c_str(), algoParam_.postDevice.c_str(), algoParam_.sceneType.c_str(),
        algoParam_.preDeviceName.c_str()};
    cmdInfo.data = static_cast<void *>(&setParam);
    cmdInfo.size = sizeof(setParam);
    return (*handle)->command(handle, EFFECT_CMD_SET_PARAM, &cmdInfo, &replyInfo);
}

int32_t AudioEnhanceChain::AddEnhanceHandle(AudioEffectHandle handle, AudioEffectLibrary *libHandle,
    const std::string &enhance, const std::string &property)
{
    std::lock_guard<std::mutex> lock(chainMutex_);
    int32_t ret = 0;
    AudioEffectTransInfo cmdInfo = {};
    AudioEffectTransInfo replyInfo = {};

    uint32_t maxSampleRate = DEFAULT_SAMPLE_RATE;
    replyInfo.data = &maxSampleRate;
    replyInfo.size = sizeof(maxSampleRate);
    ret = (*handle)->command(handle, EFFECT_CMD_GET_CONFIG, &cmdInfo, &replyInfo);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("get algo maxSampleRate failed!");
    }
    if (algoSupportedConfig_.sampleRate != maxSampleRate) {
        algoSupportedConfig_.sampleRate = maxSampleRate;
        uint32_t byteLenPerFrame = DEFAULT_FRAMELENGTH * (maxSampleRate / MILLISECOND) *
            DEFAULT_FORMAT;
        algoAttr_.byteLenPerFrame = byteLenPerFrame;

        algoCache_.input.resize(algoAttr_.byteLenPerFrame * algoAttr_.batchLen);
        algoCache_.output.resize(algoAttr_.byteLenPerFrame * deviceAttr_.micChannels);
        AUDIO_INFO_LOG("algo rate: %{public}u byteLenPerFrame: %{public}u inputsize:%{public}zu outputsize:%{public}zu",
            maxSampleRate, byteLenPerFrame, algoCache_.input.size(), algoCache_.output.size());
    }

    cmdInfo.data = static_cast<void *>(&algoSupportedConfig_);
    cmdInfo.size = sizeof(algoSupportedConfig_);
    
    ret = (*handle)->command(handle, EFFECT_CMD_SET_CONFIG, &cmdInfo, &replyInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "[%{public}s], either one of libs EFFECT_CMD_SET_CONFIG fail",
        sceneType_.c_str());

    CHECK_AND_RETURN_RET_LOG(SetEnhanceParamToHandle(handle) == SUCCESS, ERROR,
        "[%{public}s] %{public}s lib EFFECT_CMD_SET_PARAM fail", sceneType_.c_str(), libHandle->name);

    CHECK_AND_RETURN_RET_LOG(SetPropertyToHandle(handle, property) == SUCCESS, ERROR,
        "[%{public}s] %{public}s effect EFFECT_CMD_SET_PROPERTY fail", sceneType_.c_str(), enhance.c_str());

    ret = (*handle)->command(handle, EFFECT_CMD_INIT, &cmdInfo, &replyInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "[%{public}s], either one of libs EFFECT_CMD_INIT fail",
        sceneType_.c_str());
    
    setConfigFlag_ = true;
    enhanceNames_.emplace_back(enhance);
    standByEnhanceHandles_.emplace_back(handle);
    enhanceLibHandles_.emplace_back(libHandle);
    return SUCCESS;
}

bool AudioEnhanceChain::IsEmptyEnhanceHandles()
{
    std::lock_guard<std::mutex> lock(chainMutex_);
    return standByEnhanceHandles_.size() == 0;
}

void AudioEnhanceChain::GetAlgoConfig(AudioBufferConfig &micConfig, AudioBufferConfig &ecConfig,
    AudioBufferConfig &micRefConfig)
{
    uint8_t configDataformat = static_cast<uint8_t>(algoSupportedConfig_.dataFormat);
    micConfig.samplingRate = algoSupportedConfig_.sampleRate;
    micConfig.channels = algoSupportedConfig_.micNum;
    micConfig.format = configDataformat;

    if (needEcFlag_) {
        ecConfig.samplingRate = algoSupportedConfig_.sampleRate;
        ecConfig.channels = algoSupportedConfig_.ecNum;
        ecConfig.format = configDataformat;
    }

    if (needMicRefFlag_) {
        micRefConfig.samplingRate = algoSupportedConfig_.sampleRate;
        micRefConfig.channels = algoSupportedConfig_.micRefNum;
        micRefConfig.format = configDataformat;
    }
    return;
}

uint32_t AudioEnhanceChain::GetAlgoBufferSize()
{
    uint32_t byteLenPerFrame = DEFAULT_FRAMELENGTH * (algoSupportedConfig_.sampleRate / MILLISECOND) *
        DEFAULT_FORMAT;
    return byteLenPerFrame * deviceAttr_.micChannels;
}

uint32_t AudioEnhanceChain::GetAlgoBufferSizeEc()
{
    uint32_t byteLenPerFrame = DEFAULT_FRAMELENGTH * (algoSupportedConfig_.sampleRate / MILLISECOND) *
        DEFAULT_FORMAT;
    return byteLenPerFrame * deviceAttr_.ecChannels;
}

uint32_t AudioEnhanceChain::GetAlgoBufferSizeMicRef()
{
    uint32_t byteLenPerFrame = DEFAULT_FRAMELENGTH * (algoSupportedConfig_.sampleRate / MILLISECOND) *
        DEFAULT_FORMAT;
    return byteLenPerFrame * deviceAttr_.micRefChannels;
}

int32_t AudioEnhanceChain::DeinterleaverData(uint8_t *src, uint32_t channel, uint8_t *dst, uint32_t offset)
{
    CHECK_AND_RETURN_RET_LOG(src != nullptr, ERROR, "src is nullptr");
    CHECK_AND_RETURN_RET_LOG(dst != nullptr, ERROR, "dst is nullptr");
    int32_t ret = 0;
    uint32_t idx = 0;
    for (uint32_t i = 0; i < algoAttr_.byteLenPerFrame / algoAttr_.bitDepth; ++i) {
        for (uint32_t j = 0; j < channel; ++j) {
            ret = memcpy_s(dst + j * algoAttr_.byteLenPerFrame + i * algoAttr_.bitDepth,
                algoCache_.input.size() - (j * algoAttr_.byteLenPerFrame + i * algoAttr_.bitDepth + offset),
                src + idx, algoAttr_.bitDepth);
            CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "memcpy in deinterleaver error");
            idx += algoAttr_.bitDepth;
        }
    }
    return SUCCESS;
}

int32_t AudioEnhanceChain::GetOneFrameInputData(std::unique_ptr<EnhanceBuffer> &enhanceBuffer)
{
    CHECK_AND_RETURN_RET_LOG(enhanceBuffer != nullptr, ERROR, "enhance buffer is null");

    uint32_t offset = 0;
    int32_t ret = 0;
    if ((enhanceBuffer->ecBuffer.size() != 0) && needEcFlag_) {
        ret = DeinterleaverData(enhanceBuffer->ecBuffer.data(), deviceAttr_.ecChannels,
            &algoCache_.input[offset], offset);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "memcpy error in ec channel memcpy");
        offset += algoAttr_.byteLenPerFrame * deviceAttr_.ecChannels;
    }

    if (enhanceBuffer->micBufferIn.size() != 0) {
        ret = DeinterleaverData(enhanceBuffer->micBufferIn.data(), deviceAttr_.micChannels,
            &algoCache_.input[offset], offset);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "memcpy error in mic channel memcpy");
        offset += algoAttr_.byteLenPerFrame * deviceAttr_.micChannels;
    }

    if ((enhanceBuffer->micRefBuffer.size() != 0) && needMicRefFlag_) {
        ret = DeinterleaverData(enhanceBuffer->micRefBuffer.data(), deviceAttr_.micRefChannels,
            &algoCache_.input[offset], offset);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "memcpy error in mic ref channel memcpy");
    }
    return SUCCESS;
}

void AudioEnhanceChain::WriteDumpFile(std::unique_ptr<EnhanceBuffer> &enhanceBuffer, uint32_t length)
{
    if (dumpFileIn_ == nullptr) {
        return;
    }
    std::vector<uint8_t> buffer;
    size_t ecLen = algoAttr_.bitDepth * algoSupportedConfig_.ecNum;
    size_t micLen = algoAttr_.bitDepth * algoSupportedConfig_.micNum;
    size_t micRefLen = algoAttr_.bitDepth * algoSupportedConfig_.micRefNum;
    size_t offset = 0;
    buffer.reserve(length);
    for (size_t i = 0; i < algoAttr_.byteLenPerFrame / algoAttr_.bitDepth; i++) {
        if (needEcFlag_) {
        offset = i * ecLen;
        buffer.insert(buffer.end(), enhanceBuffer->ecBuffer.begin() + offset,
            enhanceBuffer->ecBuffer.begin() + offset + ecLen);
        }
        offset= i * micLen;
        buffer.insert(buffer.end(), enhanceBuffer->micBufferIn.begin() + offset,
            enhanceBuffer->micBufferIn.begin() + offset + micLen);
        if (needMicRefFlag_) {
            offset = i * micRefLen;
            buffer.insert(buffer.end(), enhanceBuffer->micRefBuffer.begin() + offset,
                enhanceBuffer->micRefBuffer.begin() + offset + micRefLen);
        }
    }
    DumpFileUtil::WriteDumpFile(dumpFileIn_, buffer.data(), buffer.size());
}

int32_t AudioEnhanceChain::ApplyEnhanceChain(std::unique_ptr<EnhanceBuffer> &enhanceBuffer, uint32_t length)
{
    std::lock_guard<std::mutex> lock(chainMutex_);
    CHECK_AND_RETURN_RET_LOG(enhanceBuffer != nullptr, ERROR, "enhance buffer is null");

    uint32_t inputLen = algoAttr_.byteLenPerFrame * algoAttr_.batchLen;
    uint32_t outputLen = algoAttr_.byteLenPerFrame * algoSupportedConfig_.outNum;

    BufferDesc bufferIn = {enhanceBuffer->micBufferIn.data(), length, length};
    AudioStreamInfo streamInfo(static_cast<AudioSamplingRate>(deviceAttr_.micRate),
        AudioEncodingType::ENCODING_PCM, ConvertFormat(deviceAttr_.micFormat),
        static_cast<AudioChannel>(deviceAttr_.micChannels));

    CHECK_AND_RETURN_RET_LOG(algoCache_.input.size() == inputLen, ERROR,
        "algo cache input size:%{public}zu != inputLen:%{public}u", algoCache_.input.size(), inputLen);
    CHECK_AND_RETURN_RET_LOG(algoCache_.output.size() == outputLen, ERROR,
        "algo cache output size:%{public}zu != outputLen:%{public}u", algoCache_.output.size(), outputLen);
    VolumeTools::DfxOperation(bufferIn, streamInfo, sceneType_, volumeDataCount_);
    WriteDumpFile(enhanceBuffer, inputLen);
    if (standByEnhanceHandles_.size() == 0) {
        AUDIO_DEBUG_LOG("audioEnhanceChain->standByEnhanceHandles is empty");
        CHECK_AND_RETURN_RET_LOG(memcpy_s(enhanceBuffer->micBufferOut.data(), enhanceBuffer->micBufferOut.size(),
            enhanceBuffer->micBufferIn.data(), length) == 0, ERROR, "memcpy error in IsEmptyEnhanceHandles");
        return ERROR;
    }
    if (GetOneFrameInputData(enhanceBuffer) != SUCCESS) {
        AUDIO_ERR_LOG("GetOneFrameInputData failed");
        CHECK_AND_RETURN_RET_LOG(memcpy_s(enhanceBuffer->micBufferOut.data(), enhanceBuffer->micBufferOut.size(),
            enhanceBuffer->micBufferIn.data(), length) == 0, ERROR, "memcpy error in GetOneFrameInputData");
        return ERROR;
    }
    DumpFileUtil::WriteDumpFile(dumpFileDeinterLeaver_, algoCache_.input.data(), algoCache_.input.size());
    AudioBuffer audioBufIn_ = {};
    AudioBuffer audioBufOut_ = {};
    audioBufIn_.frameLength = algoCache_.input.size();
    audioBufOut_.frameLength = algoCache_.output.size();
    audioBufIn_.raw = static_cast<void *>(algoCache_.input.data());
    audioBufOut_.raw = static_cast<void *>(algoCache_.output.data());

    for (AudioEffectHandle handle : standByEnhanceHandles_) {
        int32_t ret = (*handle)->process(handle, &audioBufIn_, &audioBufOut_);
        CHECK_AND_CONTINUE_LOG(ret == 0, "[%{public}s] either one of libs process fail", sceneType_.c_str());
    }
    CHECK_AND_RETURN_RET_LOG(memcpy_s(enhanceBuffer->micBufferOut.data(), enhanceBuffer->micBufferOut.size(),
        audioBufOut_.raw, audioBufOut_.frameLength) == 0,
        ERROR, "memcpy error in audioBufOut_ to enhanceBuffer->output");
    DumpFileUtil::WriteDumpFile(dumpFileOut_, enhanceBuffer->micBufferOut.data(), static_cast<uint64_t>(length));
    return SUCCESS;
}

int32_t AudioEnhanceChain::SetEnhanceProperty(const std::string &enhance, const std::string &property)
{
    if (property.empty()) { return SUCCESS; }
    std::lock_guard<std::mutex> lock(chainMutex_);
    uint32_t size = standByEnhanceHandles_.size();
    AudioEffectTransInfo cmdInfo{};
    AudioEffectTransInfo replyInfo{};
    for (uint32_t index = 0; index < size; index++) {
        auto &handle = standByEnhanceHandles_[index];
        auto const &enhanceName = enhanceNames_[index];
        if (enhance == enhanceName) {
            CHECK_AND_RETURN_RET_LOG(SetPropertyToHandle(handle, property) == SUCCESS, ERROR,
                "[%{public}s] %{public}s effect EFFECT_CMD_SET_PROPERTY fail",
                sceneType_.c_str(), enhance.c_str());
            CHECK_AND_RETURN_RET_LOG(
                (*handle)->command(handle, EFFECT_CMD_INIT, &cmdInfo, &replyInfo) == SUCCESS, ERROR,
                "[%{public}s] %{public}s effect EFFECT_CMD_INIT fail",
                sceneType_.c_str(), enhance.c_str());
        }
    }
    return SUCCESS;
}

int32_t AudioEnhanceChain::SetPropertyToHandle(AudioEffectHandle handle, const std::string &property)
{
    if (property.empty()) { return SUCCESS; }
    int32_t replyData = 0;
    const char *propCstr = property.c_str();
    AudioEffectTransInfo cmdInfo = {sizeof(const char *), reinterpret_cast<void*>(&propCstr)};
    AudioEffectTransInfo replyInfo = {sizeof(int32_t), &replyData};
    return (*handle)->command(handle, EFFECT_CMD_SET_PROPERTY, &cmdInfo, &replyInfo);
}

bool AudioEnhanceChain::IsDefaultChain()
{
    return defaultFlag_;
}

int32_t AudioEnhanceChain::InitCommand()
{
    std::lock_guard<std::mutex> lock(chainMutex_);
    uint32_t size = standByEnhanceHandles_.size();
    AudioEffectTransInfo cmdInfo{};
    AudioEffectTransInfo replyInfo{};
    for (uint32_t index = 0; index < size; index++) {
        auto &handle = standByEnhanceHandles_[index];
        CHECK_AND_RETURN_RET_LOG(
            (*handle)->command(handle, EFFECT_CMD_INIT, &cmdInfo, &replyInfo) == SUCCESS, ERROR,
            "[%{public}s] effect EFFECT_CMD_INIT fail", sceneType_.c_str());
    }
    return SUCCESS;
}

AudioSampleFormat AudioEnhanceChain::ConvertFormat(uint32_t format)
{
    auto item = FORMAT_CONVERT_MAP.find(format);
    if (item != FORMAT_CONVERT_MAP.end()) {
        return item->second;
    }
    return INVALID_WIDTH;
}
} // namespace AudioStandard
} // namespace OHOS