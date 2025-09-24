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
namespace {
const uint32_t BITLENGTH = 8;
const uint32_t MILLISECOND = 1000;
const uint32_t DEFAULT_FRAMELENGTH = 20;
const uint32_t DEFAULT_SAMPLE_RATE = 48000;
const uint32_t DEFAULT_FORMAT = 2;
const uint32_t DEFAULT_MICNUM = 2;
const uint32_t DEFAULT_ECOFF_CH = 0;
const uint32_t DEFAULT_MICREFOFF_CH = 0;
const uint32_t DEFAULT_ECON_CH = 2;
const uint32_t DEFAULT_MICREFON_CH = 4;
const uint32_t BYTE_SIZE_SAMPLE_U8 = 1;
const uint32_t BYTE_SIZE_SAMPLE_S16 = 2;
const uint32_t BYTE_SIZE_SAMPLE_S24 = 3;
const uint32_t BYTE_SIZE_SAMPLE_S32 = 4;
const uint32_t DEFAULT_DEVICE_TYPE_CH = 4;
const std::string DEFAULT_DEVICE_TYPE = "DEVICE_TYPE_MIC";

const std::vector<std::string> NEED_EC_SCENE = {
    "SCENE_VOIP_UP",
    "SCENE_PRE_ENHANCE",
    "SCENE_RECOGNITION",
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

AudioSampleFormat ConvertFormat(uint32_t format)
{
    auto item = FORMAT_CONVERT_MAP.find(format);
    if (item != FORMAT_CONVERT_MAP.end()) {
        return item->second;
    }
    return INVALID_WIDTH;
}
} // namespace

AudioEnhanceChain::AudioEnhanceChain(uint64_t chainId, const std::string &scene, ScenePriority scenePriority,
    const AudioEnhanceParamAdapter &algoParam, const AudioEnhanceDeviceAttr &deviceAttr)
    : chainId_(chainId), sceneType_(scene), scenePriority_(scenePriority), algoParam_(algoParam)
{
    deviceAttr_ = deviceAttr;
    if (deviceAttr_.micChannels == 1) {
        deviceAttr_.micChannels = DEFAULT_MICNUM;
    }

    InitAudioEnhanceChain();
}

void AudioEnhanceChain::InitAudioEnhanceChain()
{
    if ((algoParam_.preDevice == DEFAULT_DEVICE_TYPE) && (deviceAttr_.micChannels != DEFAULT_DEVICE_TYPE_CH)) {
        AUDIO_WARNING_LOG("mic channel[%{public}d] is set to 4", deviceAttr_.micChannels);
        deviceAttr_.micChannels = DEFAULT_DEVICE_TYPE_CH;
    }

    algoSupportedConfig_ = {DEFAULT_FRAMELENGTH, DEFAULT_SAMPLE_RATE, DEFAULT_FORMAT * BITLENGTH,
        deviceAttr_.micChannels, DEFAULT_ECOFF_CH, DEFAULT_MICREFOFF_CH, deviceAttr_.micChannels};

    uint32_t byteLenPerFrame = DEFAULT_FRAMELENGTH * (DEFAULT_SAMPLE_RATE / MILLISECOND) * DEFAULT_FORMAT;
    algoAttr_ = {DEFAULT_FORMAT, deviceAttr_.micChannels, byteLenPerFrame};

    if (count(NEED_EC_SCENE.begin(), NEED_EC_SCENE.end(), sceneType_)) {
        needEcFlag_ = true;
        algoSupportedConfig_.ecNum = DEFAULT_ECON_CH;
        algoAttr_.batchLen = deviceAttr_.micChannels + algoSupportedConfig_.ecNum;
        enhanceBuf_.ecBuffer.resize(deviceAttr_.ecChannels * byteLenPerFrame);
    }

    if (count(NEED_MICREF_SCENE.begin(), NEED_MICREF_SCENE.end(), sceneType_)) {
        needMicRefFlag_ = true;
        algoSupportedConfig_.micRefNum = DEFAULT_MICREFON_CH;
        algoAttr_.batchLen += algoSupportedConfig_.micRefNum;
        enhanceBuf_.micRefBuffer.resize(deviceAttr_.micRefChannels * byteLenPerFrame);
    }

    enhanceBuf_.micBuffer.resize(deviceAttr_.micChannels * byteLenPerFrame);
    outputCache_.resize(deviceAttr_.micChannels * byteLenPerFrame);
    algoCache_.input.resize(algoAttr_.byteLenPerFrame * algoAttr_.batchLen);
    algoCache_.output.resize(algoAttr_.byteLenPerFrame * deviceAttr_.micChannels);
    AUDIO_INFO_LOG("micNum: %{public}u ecNum: %{public}u micRefNum: %{public}u outNum: %{public}u"
        " byteLenPerFrame: %{public}u inputsize:%{public}zu outputsize:%{public}zu",
        algoSupportedConfig_.micNum, algoSupportedConfig_.ecNum, algoSupportedConfig_.micRefNum,
        algoSupportedConfig_.outNum, byteLenPerFrame, algoCache_.input.size(), algoCache_.output.size());

    traceTagIn_ = sceneType_ + "_IN";
    traceTagOut_ = sceneType_ + "_OUT";
    dfxStreamInfo_ = { static_cast<AudioSamplingRate>(DEFAULT_SAMPLE_RATE), AudioEncodingType::ENCODING_PCM,
        ConvertFormat(DEFAULT_FORMAT), static_cast<AudioChannel>(deviceAttr_.micChannels) };
    std::string dumpFileInName = "Enhance_" + sceneType_ + "_" + GetTime() + "_In_" +
        std::to_string(algoSupportedConfig_.ecNum) + "Ec_" + std::to_string(algoSupportedConfig_.micNum) + "Mic_" +
        std::to_string(algoSupportedConfig_.micRefNum) + "MicRef.pcm";
    std::string dumpFileOutName = "Enhance_" + sceneType_ + "_" + GetTime() + "_Out.pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileInName, &dumpFileIn_);
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileOutName, &dumpFileOut_);
}

AudioEnhanceChain::~AudioEnhanceChain()
{
    DumpFileUtil::CloseDumpFile(&dumpFileIn_);
    DumpFileUtil::CloseDumpFile(&dumpFileOut_);
}

int32_t AudioEnhanceChain::ProcessReleaseAllEnhanceModule(void)
{
    for (auto &module : enhanceModules_) {
        if (module.libHandle != nullptr) {
            module.libHandle->releaseEffect(module.enhanceHandle);
        }
    }
    enhanceModules_.clear();
    return SUCCESS;
}

void AudioEnhanceChain::ReleaseAllEnhanceModule(void)
{
    auto task = [self = weak_from_this()]() {
        if (auto chain = self.lock(); chain != nullptr) {
            chain->ProcessReleaseAllEnhanceModule();
        }
    };
    if (threadHandler_ != nullptr) {
        threadHandler_->EnsureTask(task);
    }
    threadHandler_ = nullptr;
}

int32_t AudioEnhanceChain::SetThreadHandler(const std::shared_ptr<ThreadHandler> &threadHandler)
{
    if (threadHandler == nullptr) {
        AUDIO_ERR_LOG("threadHandler is null");
        return ERROR;
    }

    threadHandler_ = threadHandler;
    return SUCCESS;
}

void AudioEnhanceChain::ScheduleAudioTask(const ThreadHandler::Task &task)
{
    if (threadHandler_ != nullptr) {
        threadHandler_->PostTask(task);
    } else {
        task();
    }
}

int32_t AudioEnhanceChain::ProcessSetInputDevice(const std::string &inputDevice, const std::string &deviceName)
{
    if (inputDevice == algoParam_.preDevice) {
        AUDIO_INFO_LOG("the current device does not need to be updated");
        return SUCCESS;
    }
    algoParam_.preDevice = inputDevice;
    algoParam_.preDeviceName = deviceName;
    AUDIO_INFO_LOG("update input device %{public}s name %{public}s", inputDevice.c_str(), deviceName.c_str());
    AudioEffectTransInfo cmdInfo = {};
    AudioEffectTransInfo replyInfo = {};
    for (const auto &module : enhanceModules_) {
        auto setParaCmdRet = SetEnhanceParamToHandle(module.enhanceHandle);
        CHECK_AND_RETURN_RET_LOG(setParaCmdRet == SUCCESS, ERROR,
            "[%{public}s] effect EFFECT_CMD_SET_PARAM fail", sceneType_.c_str());
        auto initCmdRet = (*module.enhanceHandle)->command(module.enhanceHandle, EFFECT_CMD_INIT, &cmdInfo, &replyInfo);
        CHECK_AND_RETURN_RET_LOG(initCmdRet == 0, ERROR,
            "[%{public}s] effect EFFECT_CMD_INIT fail", sceneType_.c_str());
    }

    return SUCCESS;
}

int32_t AudioEnhanceChain::SetInputDevice(const std::string &inputDevice, const std::string &deviceName)
{
    if (inputDevice.size() == 0) {
        return SUCCESS;
    }

    auto task = [self = weak_from_this(), inputDevice, deviceName]() {
        if (auto chain = self.lock(); chain != nullptr) {
            chain->ProcessSetInputDevice(inputDevice, deviceName);
        }
    };

    ScheduleAudioTask(task);

    return SUCCESS;
}

int32_t AudioEnhanceChain::ProcessSetFoldState(uint32_t foldState)
{
    if (foldState == algoParam_.foldState) {
        AUDIO_INFO_LOG("no need update fold state %{public}u", foldState);
        return SUCCESS;
    }
    algoParam_.foldState = foldState;
    AUDIO_INFO_LOG("update fold state %{public}u", foldState);

    AudioEffectTransInfo cmdInfo = {};
    AudioEffectTransInfo replyInfo = {};
    for (const auto &module : enhanceModules_) {
        auto setParaCmdRet = SetEnhanceParamToHandle(module.enhanceHandle);
        CHECK_AND_RETURN_RET_LOG(setParaCmdRet == SUCCESS, ERROR,
            "[%{public}s] effect EFFECT_CMD_SET_PARAM fail", sceneType_.c_str());
        auto initCmdRet = (*module.enhanceHandle)->command(module.enhanceHandle, EFFECT_CMD_INIT, &cmdInfo, &replyInfo);
        CHECK_AND_RETURN_RET_LOG(initCmdRet == 0, ERROR,
            "[%{public}s] effect EFFECT_CMD_INIT fail", sceneType_.c_str());
    }

    return SUCCESS;
}

int32_t AudioEnhanceChain::SetFoldState(uint32_t foldState)
{
    auto task = [self = weak_from_this(), foldState]() {
        if (auto chain = self.lock(); chain != nullptr) {
            chain->ProcessSetFoldState(foldState);
        }
    };

    ScheduleAudioTask(task);

    return SUCCESS;
}

int32_t AudioEnhanceChain::ProcessSetEnhanceParam(bool mute, uint32_t systemVol)
{
    algoParam_.muteInfo = mute;
    algoParam_.volumeInfo = systemVol;

    AudioEffectTransInfo cmdInfo = {};
    AudioEffectTransInfo replyInfo = {};
    for (const auto &module : enhanceModules_) {
        auto setParaCmdRet = SetEnhanceParamToHandle(module.enhanceHandle);
        CHECK_AND_RETURN_RET_LOG(setParaCmdRet == SUCCESS, ERROR,
            "[%{public}s] effect EFFECT_CMD_SET_PARAM fail", sceneType_.c_str());
        auto initCmdRet = (*module.enhanceHandle)->command(module.enhanceHandle, EFFECT_CMD_INIT, &cmdInfo, &replyInfo);
        CHECK_AND_RETURN_RET_LOG(initCmdRet == SUCCESS, ERROR,
            "[%{public}s] effect EFFECT_CMD_INIT fail", sceneType_.c_str());
    }

    return SUCCESS;
}

int32_t AudioEnhanceChain::SetEnhanceParam(bool mute, uint32_t systemVol)
{
    auto task = [self = weak_from_this(), mute, systemVol]() {
        if (auto chain = self.lock(); chain != nullptr) {
            chain->ProcessSetEnhanceParam(mute, systemVol);
        }
    };

    ScheduleAudioTask(task);

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

int32_t AudioEnhanceChain::InitSingleEnhanceModule(AudioEffectHandle enhanceHandle, const std::string &enhanceProp)
{
    int32_t ret = ERROR;
    AudioEffectTransInfo cmdInfo = {};
    AudioEffectTransInfo replyInfo = {};

    uint32_t maxSampleRate = DEFAULT_SAMPLE_RATE;
    replyInfo.data = &maxSampleRate;
    replyInfo.size = sizeof(maxSampleRate);
    ret = (*enhanceHandle)->command(enhanceHandle, EFFECT_CMD_GET_CONFIG, &cmdInfo, &replyInfo);
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
        AUDIO_INFO_LOG("algorate: %{public}u byteLenPerFrame: %{public}u inputsize:%{public}zu outputsize:%{public}zu",
            maxSampleRate, byteLenPerFrame, algoCache_.input.size(), algoCache_.output.size());
    }

    cmdInfo.data = static_cast<void *>(&algoSupportedConfig_);
    cmdInfo.size = sizeof(algoSupportedConfig_);

    ret = (*enhanceHandle)->command(enhanceHandle, EFFECT_CMD_SET_CONFIG, &cmdInfo, &replyInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "[%{public}s], either one of libs EFFECT_CMD_SET_CONFIG fail",
        sceneType_.c_str());

    CHECK_AND_RETURN_RET_LOG(SetEnhanceParamToHandle(enhanceHandle) == SUCCESS, ERROR,
        "[%{public}s] EFFECT_CMD_SET_PARAM fail", sceneType_.c_str());

    CHECK_AND_RETURN_RET_LOG(SetPropertyToHandle(enhanceHandle, enhanceProp) == SUCCESS, ERROR,
        "[%{public}s] EFFECT_CMD_SET_PROPERTY fail", sceneType_.c_str());

    ret = (*enhanceHandle)->command(enhanceHandle, EFFECT_CMD_INIT, &cmdInfo, &replyInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "[%{public}s], either one of libs EFFECT_CMD_INIT fail",
        sceneType_.c_str());

    return SUCCESS;
}

int32_t AudioEnhanceChain::ProcessCreateAllEnhanceModule(const std::vector<EnhanceModulePara> &moduleParas)
{
    for (const auto &para : moduleParas) {
        AudioEffectDescriptor descriptor = { para.libName, para.enhanceName };
        AUDIO_INFO_LOG("libName: %{public}s enhanceName:%{public}s", para.libName.c_str(), para.enhanceName.c_str());
        AudioEffectHandle enhanceHandle = nullptr;
        int32_t ret = para.libHandle->createEffect(descriptor, &enhanceHandle);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "create effect: %{public}s fail", para.enhanceName.c_str());
        CHECK_AND_RETURN_RET_LOG(enhanceHandle != nullptr, ERROR, "enhanceHandle is null");

        int32_t initSingleModuleRet = InitSingleEnhanceModule(enhanceHandle, para.enhanceProp);
        if (initSingleModuleRet != SUCCESS) {
            AUDIO_ERR_LOG("init enhance: %{public}s fail", para.enhanceName.c_str());
            para.libHandle->releaseEffect(enhanceHandle);
            return ERROR;
        }

        EnhanceModule module = { para.enhanceName, enhanceHandle, para.libHandle };
        enhanceModules_.emplace_back(module);
    }

    {
        std::lock_guard<std::mutex> lock(chainMutex_);
        chainIsReady_ = true;
    }

    return SUCCESS;
}

int32_t AudioEnhanceChain::CreateAllEnhanceModule(const std::vector<EnhanceModulePara> &moduleParas)
{
    for (const auto &para : moduleParas) {
        if (para.libHandle == nullptr || para.libHandle->createEffect == nullptr ||
            para.libHandle->releaseEffect == nullptr) {
            AUDIO_ERR_LOG("enhance: %{public}s interface is null", para.enhanceName.c_str());
            return ERROR;
        }
    }

    auto task = [self = weak_from_this(), moduleParas]() {
        if (auto chain = self.lock(); chain != nullptr) {
            chain->ProcessCreateAllEnhanceModule(moduleParas);
        }
    };

    ScheduleAudioTask(task);

    return SUCCESS;
}

bool AudioEnhanceChain::IsEmptyEnhanceHandles()
{
    return enhanceModules_.empty();
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

uint64_t AudioEnhanceChain::GetChainId(void) const
{
    return chainId_;
}

ScenePriority AudioEnhanceChain::GetScenePriority(void) const
{
    return scenePriority_;
}

int32_t AudioEnhanceChain::DeinterleaverData(uint8_t *src, uint32_t channel, uint8_t *dst, uint32_t dstLen)
{
    uint32_t srcIdx = 0;
    uint32_t frameCount = algoAttr_.byteLenPerFrame / algoAttr_.bitDepth;
    for (uint32_t i = 0; i < frameCount; ++i) {
        for (uint32_t j = 0; j < channel; ++j) {
            uint32_t dstIdx = j * algoAttr_.byteLenPerFrame + i * algoAttr_.bitDepth;
            auto memcpyRet = memcpy_s(dst + dstIdx, dstLen - srcIdx, &src[srcIdx], algoAttr_.bitDepth);
            CHECK_AND_RETURN_RET_LOG(memcpyRet == EOK, ERROR, "memcpy in deinterleaver error");
            srcIdx += algoAttr_.bitDepth;
        }
    }
    return SUCCESS;
}

int32_t AudioEnhanceChain::ProcessSetEnhanceProperty(const std::string &enhance, const std::string &property)
{
    AudioEffectTransInfo cmdInfo = {};
    AudioEffectTransInfo replyInfo = {};
    for (const auto &module : enhanceModules_) {
        if (module.enhanceName != enhance) {
            continue;
        }
        auto setPropCmdRet = SetPropertyToHandle(module.enhanceHandle, property);
        CHECK_AND_RETURN_RET_LOG(setPropCmdRet == SUCCESS, ERROR,
            "[%{public}s] %{public}s effect EFFECT_CMD_SET_PROPERTY fail", sceneType_.c_str(), enhance.c_str());
        auto initCmdRet = (*module.enhanceHandle)->command(module.enhanceHandle, EFFECT_CMD_INIT, &cmdInfo, &replyInfo);
        CHECK_AND_RETURN_RET_LOG(initCmdRet == SUCCESS, ERROR,
            "[%{public}s] %{public}s effect EFFECT_CMD_INIT fail", sceneType_.c_str(), enhance.c_str());
    }

    return SUCCESS;
}

int32_t AudioEnhanceChain::SetEnhanceProperty(const std::string &enhance, const std::string &property)
{
    auto task = [self = weak_from_this(), enhance, property]() {
        if (auto chain = self.lock(); chain != nullptr) {
            chain->ProcessSetEnhanceProperty(enhance, property);
        }
    };

    ScheduleAudioTask(task);

    return SUCCESS;
}

int32_t AudioEnhanceChain::SetPropertyToHandle(AudioEffectHandle handle, const std::string &property)
{
    int32_t replyData = 0;
    const char *propCstr = property.c_str();
    AudioEffectTransInfo cmdInfo = {sizeof(const char *), reinterpret_cast<void*>(&propCstr)};
    AudioEffectTransInfo replyInfo = {sizeof(int32_t), &replyData};
    return (*handle)->command(handle, EFFECT_CMD_SET_PROPERTY, &cmdInfo, &replyInfo);
}

int32_t AudioEnhanceChain::CacheChainInputData(const EnhanceTransBuffer &transBuf)
{
    if (transBuf.ecData != nullptr && transBuf.ecDataLen == enhanceBuf_.ecBuffer.size()) {
        auto memcpyEcRet = memcpy_s(enhanceBuf_.ecBuffer.data(), enhanceBuf_.ecBuffer.size(),
            transBuf.ecData, transBuf.ecDataLen);
        CHECK_AND_RETURN_RET_LOG(memcpyEcRet == EOK, ERROR, "cache ec data fail");
    }

    if (transBuf.micData != nullptr && transBuf.micDataLen == enhanceBuf_.micBuffer.size()) {
        auto memcpyMicRet = memcpy_s(enhanceBuf_.micBuffer.data(), enhanceBuf_.micBuffer.size(),
            transBuf.micData, transBuf.micDataLen);
        CHECK_AND_RETURN_RET_LOG(memcpyMicRet == EOK, ERROR, "cache mic data fail");
    }

    if (transBuf.micRefData != nullptr && transBuf.micRefDataLen == enhanceBuf_.micRefBuffer.size()) {
        auto memcpyMIcRefRet = memcpy_s(enhanceBuf_.micRefBuffer.data(), enhanceBuf_.micRefBuffer.size(),
            transBuf.micRefData, transBuf.micRefDataLen);
        CHECK_AND_RETURN_RET_LOG(memcpyMIcRefRet == EOK, ERROR, "cache micRef data fail");
    }

    hasTask_ = true;

    return SUCCESS;
}

int32_t AudioEnhanceChain::GetOutputDataFromChain(void *buf, size_t bufSize)
{
    CHECK_AND_RETURN_RET_LOG(buf != nullptr, ERROR, "buf is null");
    CHECK_AND_RETURN_RET_LOG(bufSize != 0, ERROR, "bufSize is 0");

    std::lock_guard<std::mutex> lock(chainMutex_);
    auto memcpyRet = memcpy_s(buf, bufSize, outputCache_.data(), outputCache_.size());
    CHECK_AND_RETURN_RET_LOG(memcpyRet == EOK, ERROR, "memcpy chain out data fail");

    return SUCCESS;
}

int32_t AudioEnhanceChain::WriteChainOutputData(void *buf, size_t bufSize)
{
    CHECK_AND_RETURN_RET_LOG(buf != nullptr, ERROR, "buf is null");
    CHECK_AND_RETURN_RET_LOG(bufSize != 0, ERROR, "bufSize is 0");

    auto memcpyRet = memcpy_s(outputCache_.data(), outputCache_.size(), buf, bufSize);
    CHECK_AND_RETURN_RET_LOG(memcpyRet == EOK, ERROR, "write chain out data fail");

    return SUCCESS;
}

int32_t AudioEnhanceChain::PrepareChainInputData(void)
{
    int32_t ret = 0;
    uint32_t ecIdx = 0;
    uint32_t micIdx = algoAttr_.byteLenPerFrame * algoSupportedConfig_.ecNum;
    uint32_t micRefIdx = micIdx + algoAttr_.byteLenPerFrame * algoSupportedConfig_.micNum;
    auto enhanceBufLen = enhanceBuf_.ecBuffer.size() + enhanceBuf_.micBuffer.size() + enhanceBuf_.micRefBuffer.size();
    CHECK_AND_RETURN_RET_LOG(enhanceBufLen <= algoCache_.input.size(), ERROR, "input cache insufficient");

    if (enhanceBuf_.ecBuffer.size() != 0) {
        ret = DeinterleaverData(enhanceBuf_.ecBuffer.data(), deviceAttr_.ecChannels,
            &algoCache_.input[ecIdx], enhanceBuf_.ecBuffer.size());
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "deinterleaver ec data fail");
    }

    if (enhanceBuf_.micBuffer.size() != 0) {
        BufferDesc bufferIn = { enhanceBuf_.micBuffer.data(), enhanceBuf_.micBuffer.size(),
            enhanceBuf_.micBuffer.size() };
        VolumeTools::DfxOperation(bufferIn, dfxStreamInfo_, traceTagIn_, volumeDataCountIn_);
        ret = DeinterleaverData(enhanceBuf_.micBuffer.data(), deviceAttr_.micChannels,
            &algoCache_.input[micIdx], enhanceBuf_.micBuffer.size());
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "deinterleaver mic data fail");
    }

    if (enhanceBuf_.micRefBuffer.size() != 0) {
        ret = DeinterleaverData(enhanceBuf_.micRefBuffer.data(), deviceAttr_.micRefChannels,
            &algoCache_.input[micRefIdx], enhanceBuf_.micRefBuffer.size());
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "deinterleaver micRef data fail");
    }

    return SUCCESS;
}

int32_t AudioEnhanceChain::ProcessApplyEnhanceChain(void)
{
    Trace trace("ProcessCapData");

    {
        std::lock_guard<std::mutex> lock(chainMutex_);
        CHECK_AND_RETURN_RET(hasTask_ == true, ERROR);
        if (PrepareChainInputData() != SUCCESS || enhanceModules_.empty()) {
            WriteChainOutputData(enhanceBuf_.micBuffer.data(), enhanceBuf_.micBuffer.size());
            return SUCCESS;
        }
        hasTask_ = false;
    }

    AudioBuffer audioBufIn = { .frameLength = algoCache_.input.size(), .raw = algoCache_.input.data() };
    AudioBuffer audioBufOut = { .frameLength = algoCache_.output.size(), .raw = algoCache_.output.data() };
    DumpFileUtil::WriteDumpFile(dumpFileIn_, algoCache_.input.data(), algoCache_.input.size());

    for (const auto &module : enhanceModules_) {
        int32_t ret = (*module.enhanceHandle)->process(module.enhanceHandle, &audioBufIn, &audioBufOut);
        CHECK_AND_CONTINUE_LOG(ret == 0, "module: [%{public}s] process fail", module.enhanceName.c_str());
    }

    DumpFileUtil::WriteDumpFile(dumpFileOut_, algoCache_.output.data(), algoCache_.output.size());
    BufferDesc bufferOut = { algoCache_.output.data(), algoCache_.output.size(), algoCache_.output.size() };
    VolumeTools::DfxOperation(bufferOut, dfxStreamInfo_, traceTagOut_, volumeDataCountOut_);

    {
        std::lock_guard<std::mutex> lock(chainMutex_);
        WriteChainOutputData(audioBufOut.raw, audioBufOut.frameLength);
    }

    return SUCCESS;
}

int32_t AudioEnhanceChain::ApplyEnhanceChain(const EnhanceTransBuffer &transBuf)
{
    {
        std::lock_guard<std::mutex> lock(chainMutex_);
        if (chainIsReady_) {
            auto cacheChainInputDataRet = CacheChainInputData(transBuf);
            CHECK_AND_RETURN_RET_LOG(cacheChainInputDataRet == SUCCESS, ERROR, "cache chain input data fail");
        } else {
            AUDIO_INFO_LOG("chain is not ready, passthrough data");
            WriteChainOutputData(transBuf.micData, transBuf.micDataLen);
            return SUCCESS;
        }
    }

    auto task = [self = weak_from_this()]() {
        if (auto chain = self.lock(); chain != nullptr) {
            chain->ProcessApplyEnhanceChain();
        }
    };

    ScheduleAudioTask(task);

    return SUCCESS;
}

int32_t AudioEnhanceChain::ProcessInitCommand(void)
{
    AudioEffectTransInfo cmdInfo = {};
    AudioEffectTransInfo replyInfo = {};
    for (const auto &module : enhanceModules_) {
        auto initCmdRet = (*module.enhanceHandle)->command(module.enhanceHandle, EFFECT_CMD_INIT, &cmdInfo, &replyInfo);
        CHECK_AND_RETURN_RET_LOG(initCmdRet == SUCCESS, ERROR,
            "[%{public}s] effect EFFECT_CMD_INIT fail", sceneType_.c_str());
    }

    return SUCCESS;
}

int32_t AudioEnhanceChain::InitCommand()
{
    auto task = [self = weak_from_this()]() {
        if (auto chain = self.lock(); chain != nullptr) {
            chain->ProcessInitCommand();
        }
    };

    ScheduleAudioTask(task);

    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS