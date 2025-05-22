/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioSpatialChannelConverter"
#endif

#include "audio_spatial_channel_converter.h"

#include <cstdint>
#include <string>
#include <iostream>
#include "media_monitor_manager.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
static constexpr int32_t AUDIO_VIVID_SAMPLES = 1024;
static constexpr int32_t AVS3METADATA_SIZE = 19824;
static constexpr int32_t INVALID_FORMAT = -1;
static constexpr uint64_t DEFAULT_LAYOUT = CH_LAYOUT_UNKNOWN;

#if (defined(__aarch64__) || defined(__x86_64__))
constexpr const char *LD_EFFECT_LIBRARY_PATH[] = {"/system/lib64/"};
#else
constexpr const char *LD_EFFECT_LIBRARY_PATH[] = {"/system/lib/"};
#endif

static std::map<uint8_t, int8_t> format2bps = {
    {SAMPLE_U8, sizeof(uint8_t)},
    {SAMPLE_S16LE, sizeof(int16_t)},
    {SAMPLE_S24LE, sizeof(int16_t) + sizeof(int8_t)},
    {SAMPLE_S32LE, sizeof(int32_t)},
    {SAMPLE_F32LE, sizeof(float)}};

static int8_t GetBps(uint8_t format)
{
    return format2bps.count(format) > 0 ? format2bps[format] : INVALID_FORMAT;
}

size_t AudioSpatialChannelConverter::GetPcmLength(int32_t channels, int8_t bps)
{
    if (encoding_ == ENCODING_AUDIOVIVID) {
        return channels * AUDIO_VIVID_SAMPLES * bps;
    }
    AUDIO_INFO_LOG("encodingType is not supported."); // never run
    return 0;
}

size_t AudioSpatialChannelConverter::GetMetaSize()
{
    if (encoding_ == ENCODING_AUDIOVIVID) {
        return AVS3METADATA_SIZE;
    }
    AUDIO_INFO_LOG("encodingType is not supported."); // never run
    return 0;
}

bool AudioSpatialChannelConverter::Init(const AudioStreamParams info, const ConverterConfig cfg)
{
    inChannel_ = info.channels;
    outChannel_ = info.channels;

    encoding_ = info.encoding;
    sampleRate_ = info.samplingRate;

    bps_ = GetBps(info.format);
    CHECK_AND_RETURN_RET_LOG(bps_ > 0, false, "channel converter: Unsupported sample format");

    Library library = cfg.library;
    outChannelLayout_ = cfg.outChannelLayout;

    loadSuccess_ = false;
    if (externalLoader_.AddAlgoHandle(library)) {
        outChannel_ = __builtin_popcountll(outChannelLayout_);
        externalLoader_.SetIOBufferConfig(true, sampleRate_, info.format, inChannel_, info.channelLayout);
        externalLoader_.SetIOBufferConfig(false, sampleRate_, info.format, outChannel_, outChannelLayout_);
        if (externalLoader_.Init()) {
            loadSuccess_ = true;
        }
    }
    if (!loadSuccess_) {
        outChannel_ = info.channels;
        outChannelLayout_ = DEFAULT_LAYOUT; // can not convert
    }
    return true;
}

void AudioSpatialChannelConverter::ConverterChannels(uint8_t &channels, uint64_t &channelLayout)
{
    channels = outChannel_;
    channelLayout = outChannelLayout_;
}

bool AudioSpatialChannelConverter::GetInputBufferSize(size_t &bufferSize)
{
    bufferSize = GetPcmLength(inChannel_, bps_);
    return bufferSize > 0;
}

bool AudioSpatialChannelConverter::CheckInputValid(const BufferDesc bufDesc)
{
    if (bufDesc.buffer == nullptr || bufDesc.metaBuffer == nullptr) {
        AUDIO_ERR_LOG("pcm or metadata buffer is nullptr");
        return false;
    }
    if (bufDesc.bufLength != GetPcmLength(inChannel_, bps_)) {
        AUDIO_ERR_LOG("pcm bufLength invalid, pcmBufferSize = %{public}zu, excepted %{public}zu", bufDesc.bufLength,
            GetPcmLength(inChannel_, bps_));
        return false;
    }
    if (bufDesc.metaLength != GetMetaSize()) {
        AUDIO_ERR_LOG("metadata bufLength invalid, metadataBufferSize = %{public}zu, excepted %{public}zu",
            bufDesc.metaLength, GetMetaSize());
        return false;
    }
    return true;
}

bool AudioSpatialChannelConverter::AllocateMem()
{
    outPcmBuf_ = std::make_unique<uint8_t[]>(GetPcmLength(outChannel_, bps_));
    return outPcmBuf_ != nullptr;
}

void AudioSpatialChannelConverter::GetOutputBufferStream(uint8_t *&buffer, uint32_t &bufferLen)
{
    buffer = outPcmBuf_.get();
    bufferLen = static_cast<uint32_t>(GetPcmLength(outChannel_, bps_));
}

void AudioSpatialChannelConverter::Process(const BufferDesc bufDesc)
{
    size_t n = GetPcmLength(outChannel_, bps_);
    if (!loadSuccess_) {
        std::copy(bufDesc.buffer, bufDesc.buffer + n, outPcmBuf_.get());
    } else {
        AudioBuffer inBuffer = {.frameLength = AUDIO_VIVID_SAMPLES,
            .raw = bufDesc.buffer,
            .metaData = bufDesc.metaBuffer};
        AudioBuffer outBuffer = {.frameLength = AUDIO_VIVID_SAMPLES,
            .raw = outPcmBuf_.get(),
            .metaData = bufDesc.metaBuffer};
        if (externalLoader_.ApplyAlgo(inBuffer, outBuffer) != 0) {
            std::fill(outPcmBuf_.get(), outPcmBuf_.get() + n, 0);
        }
    }
}

bool AudioSpatialChannelConverter::Flush()
{
    return loadSuccess_ ? externalLoader_.FlushAlgo() : true;
}

uint32_t AudioSpatialChannelConverter::GetLatency()
{
    return loadSuccess_ ? externalLoader_.GetLatency() : 0;
}

static bool ResolveLibrary(const std::string &path, std::string &resovledPath)
{
    for (auto *libDir : LD_EFFECT_LIBRARY_PATH) {
        std::string candidatePath = std::string(libDir) + "/" + path;
        if (access(candidatePath.c_str(), R_OK) == 0) {
            resovledPath = std::move(candidatePath);
            return true;
        }
    }
    return false;
}

LibLoader::~LibLoader()
{
    if (libEntry_ != nullptr && libEntry_->audioEffectLibHandle != nullptr) {
        libEntry_->audioEffectLibHandle->releaseEffect(handle_);
    }
    if (libHandle_ != nullptr) {
#ifndef TEST_COVERAGE
        dlclose(libHandle_);
#endif
        libHandle_ = nullptr;
    }
}

bool LibLoader::LoadLibrary(const std::string &relativePath) noexcept
{
    std::string absolutePath;
    // find library in adsolutePath
    bool ret = ResolveLibrary(relativePath, absolutePath);
    CHECK_AND_RETURN_RET_LOG(ret, false, "<log error> find library falied in effect directories: %{public}s",
        relativePath.c_str());

    libHandle_ = dlopen(absolutePath.c_str(), 1);
    CHECK_AND_RETURN_RET_LOG(libHandle_, false, "<log error> dlopen lib %{public}s Fail", relativePath.c_str());
    AUDIO_INFO_LOG("<log info> dlopen lib %{public}s successful", relativePath.c_str());
    dlerror(); // clear error, only need to check libHandle_ is not nullptr

    AudioEffectLibrary *audioEffectLibHandle = static_cast<AudioEffectLibrary *>(dlsym(libHandle_,
        AUDIO_EFFECT_LIBRARY_INFO_SYM_AS_STR));
    if (!audioEffectLibHandle) {
        AUDIO_ERR_LOG("<log error> dlsym failed: error: %{public}s", dlerror());
#ifndef TEST_COVERAGE
        dlclose(libHandle_);
#endif
        return false;
    }
    AUDIO_INFO_LOG("<log info> dlsym lib %{public}s successful", relativePath.c_str());

    libEntry_->audioEffectLibHandle = audioEffectLibHandle;

    return true;
}

void LibLoader::SetIOBufferConfig(bool isInput, uint32_t sampleRate, uint8_t format, uint32_t channels,
    uint64_t channelLayout)
{
    AudioBufferConfig &target = isInput ? ioBufferConfig_.inputCfg : ioBufferConfig_.outputCfg;
    target = {sampleRate, channels, format, channelLayout, ENCODING_AUDIOVIVID};
}

bool LibLoader::AddAlgoHandle(Library library)
{
    AudioEffectDescriptor descriptor = {.libraryName = library.name, .effectName = library.name};
    libEntry_ = std::make_unique<AudioEffectLibEntry>();
    libEntry_->libraryName = library.name;
    bool loadLibrarySuccess = LoadLibrary(library.path);
    if (!loadLibrarySuccess) {
        Trace trace("SYSEVENT FAULT EVENT LOAD_EFFECT_ENGINE_ERROR, ENGINE_TYPE: "
            + std::to_string(Media::MediaMonitor::AUDIO_CONVERTER_ENGINE));
        // hisysevent for load engine error
        std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
            Media::MediaMonitor::AUDIO, Media::MediaMonitor::LOAD_EFFECT_ENGINE_ERROR,
            Media::MediaMonitor::FAULT_EVENT);
        bean->Add("ENGINE_TYPE", Media::MediaMonitor::AUDIO_CONVERTER_ENGINE);
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);

        AUDIO_ERR_LOG("loadLibrary fail, please check logs!");
        return false;
    }

    int32_t ret = libEntry_->audioEffectLibHandle->createEffect(descriptor, &handle_);
    CHECK_AND_RETURN_RET_LOG(ret == 0, false, "%{public}s create fail", library.name.c_str());
    return true;
}

bool LibLoader::Init()
{
    int32_t ret = 0;
    uint32_t replyData = 0;
    AudioEffectTransInfo cmdInfo = {sizeof(AudioEffectConfig), &ioBufferConfig_};
    AudioEffectTransInfo replyInfo = {sizeof(int32_t), &replyData};
    ret = (*handle_)->command(handle_, EFFECT_CMD_INIT, &cmdInfo, &replyInfo);
    CHECK_AND_RETURN_RET_LOG(ret == 0, false, "[%{public}s] lib EFFECT_CMD_INIT fail", libEntry_->libraryName.c_str());
    ret = (*handle_)->command(handle_, EFFECT_CMD_ENABLE, &cmdInfo, &replyInfo);
    CHECK_AND_RETURN_RET_LOG(ret == 0, false, "[%{public}s] lib EFFECT_CMD_ENABLE fail",
        libEntry_->libraryName.c_str());
    ret = (*handle_)->command(handle_, EFFECT_CMD_SET_CONFIG, &cmdInfo, &replyInfo);
    CHECK_AND_RETURN_RET_LOG(ret == 0, false, "[%{public}s] lib EFFECT_CMD_SET_CONFIG fail",
        libEntry_->libraryName.c_str());
    latency_ = replyData;
    AUDIO_INFO_LOG("The delay of [%{public}s] lib is %{public}u", libEntry_->libraryName.c_str(), latency_);
    return true;
}

uint32_t LibLoader::GetLatency()
{
    return latency_;
}

int32_t LibLoader::ApplyAlgo(AudioBuffer &inputBuffer, AudioBuffer &outputBuffer)
{
    int32_t ret = (*handle_)->process(handle_, &inputBuffer, &outputBuffer);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "converter algo lib process fail");
    return ret;
}

bool LibLoader::FlushAlgo()
{
    int32_t ret = 0;
    int32_t replyData = 0;
    AudioEffectTransInfo cmdInfo = {sizeof(AudioEffectConfig), &ioBufferConfig_};
    AudioEffectTransInfo replyInfo = {sizeof(int32_t), &replyData};
    ret = (*handle_)->command(handle_, EFFECT_CMD_ENABLE, &cmdInfo, &replyInfo);
    CHECK_AND_RETURN_RET_LOG(ret == 0, false, "[%{public}s] lib EFFECT_CMD_ENABLE fail",
        libEntry_->libraryName.c_str());
    return true;
}
} // namespace AudioStandard
} // namespace OHOS