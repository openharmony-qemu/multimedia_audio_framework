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
#define LOG_TAG "AudioSuiteInputNode"
#endif

#include <cinttypes>
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_info.h"
#include "audio_suite_input_node.h"
#include "audio_suite_pcm_buffer.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

static constexpr uint32_t REQUEST_DATA_TRY_COUNTS = 3; // 找应用要数据最大次数
static constexpr uint32_t CACHE_FRAME_LEN = 3; // 缓存帧数
static constexpr uint32_t CACHE_FRAME_SAMPLE_RATE_11025_LEN = 4; // 采样率11025缓存帧数
static constexpr uint32_t RESAMPLE_QUALITY = 5; // 重采样品质
static constexpr uint32_t MAX_CACHE = AudioSamplingRate::SAMPLE_RATE_192000 / SECONDS_TO_MS *
    CACHE_FRAME_LEN * SINGLE_FRAME_DURATION * 16 * sizeof(float); // 最大分配容量

AudioInputNode::AudioInputNode(AudioFormat format) : AudioNode(AudioNodeType::NODE_TYPE_INPUT, format),
    cachedBuffer_(GetCacheBufferCapacity(format))
{
    AUDIO_INFO_LOG("numChannels:%{public}u, channelLayout:%{public}" PRIu64 "sampleFormat:%{public}u,"
        "sampleRate:%{public}d, encodingType:%{public}d", format.audioChannelInfo.numChannels,
        format.audioChannelInfo.channelLayout, format.format, format.rate, format.encodingType);
    AUDIO_INFO_LOG("AudioInputNode::AudioInputNode finish.");
}

AudioInputNode::~AudioInputNode()
{
    if (inputNodeBuffer_ != nullptr) {
        delete inputNodeBuffer_;
        inputNodeBuffer_ = nullptr;
    }
    AUDIO_INFO_LOG("AudioInputNode::~AudioInputNode");
}


int32_t AudioInputNode::Init()
{
    AUDIO_INFO_LOG("AudioInputNode::Init");
    AudioFormat format = GetAudioNodeFormat();
    AudioSamplingRate bufferRate = format.rate == AudioSamplingRate::SAMPLE_RATE_11025 ?
        AudioSamplingRate::SAMPLE_RATE_16000 : format.rate;
    inputNodeBuffer_ = new(std::nothrow) AudioSuitePcmBuffer(bufferRate, format.audioChannelInfo.numChannels,
        format.audioChannelInfo.channelLayout);
    CHECK_AND_RETURN_RET_LOG(inputNodeBuffer_ != nullptr, ERR_INVALID_OPERATION,
        "AudioInputNode::Init inputNodeBuffer_ is null");
    outputStream_ = std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(GetSharedInstance());
    SetFormatTransfer(format.rate);
    SetUpResample(AudioSamplingRate::SAMPLE_RATE_11025, AudioSamplingRate::SAMPLE_RATE_16000,
        format.audioChannelInfo.numChannels, RESAMPLE_QUALITY);
    return SUCCESS;
}

int32_t AudioInputNode::DeInit()
{
    Flush();
    return SUCCESS;
}

int32_t AudioInputNode::Flush()
{
    AUDIO_INFO_LOG("AudioInputNode::Flush");
    cachedBuffer_.ClearBuffer();
    SetAudioNodeDataFinishedFlag(false);
    return SUCCESS;
}

int32_t AudioInputNode::Connect(const std::shared_ptr<AudioNode>& preNode, AudioNodePortType type)
{
    AUDIO_ERR_LOG("AudioInputNode::Connect not support opt");
    return ERROR;
}

int32_t AudioInputNode::DisConnect(const std::shared_ptr<AudioNode>& preNode)
{
    AUDIO_ERR_LOG("AudioInputNode::DisConnect not support opt");
    return ERROR;
}

std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> AudioInputNode::GetOutputPort(AudioNodePortType type)
{
    return outputStream_;
}

int32_t AudioInputNode::DoProcess()
{
    AUDIO_INFO_LOG("AudioInputNode::DoProcess");
    CHECK_AND_RETURN_RET(outputStream_ != nullptr, ERR_INVALID_PARAM,
        "AudioInputNode::DoProcess outputStream_ is null");
    CHECK_AND_RETURN_RET(GetDataFromUser() == SUCCESS, ERR_WRITE_FAILED,
        "AudioInputNode::DoProcess GetDataFromUser fail");
    CHECK_AND_RETURN_RET(GeneratePushBuffer() == SUCCESS, ERR_WRITE_FAILED,
        "AudioInputNode::DoProcess GeneratePushBuffer fail");
    outputStream_->WriteDataToOutput(inputNodeBuffer_);
    HandleTapCallback();
    return SUCCESS;
}

int32_t AudioInputNode::SetOnWriteDataCallback(std::shared_ptr<SuiteInputNodeWriteDataCallBack> callback)
{
    CHECK_AND_RETURN_RET(callback != nullptr, ERR_INVALID_PARAM,
        "AudioInputNode::SetOnWriteDataCallback callback is null");
    writeCallback_ = callback;
    return SUCCESS;
}

bool AudioInputNode::IsSetReadDataCallback()
{
    return writeCallback_ != nullptr;
}

void AudioInputNode::SetAudioNodeFormat(AudioFormat audioFormat)
{
    AUDIO_INFO_LOG("AudioInputNode::SetAudioNodeFormat, numChannels:%{public}u,"
        "channelLayout:%{public}" PRIu64 "sampleFormat:%{public}u, sampleRate:%{public}d, encodingType:%{public}d",
        audioFormat.audioChannelInfo.numChannels, audioFormat.audioChannelInfo.channelLayout,
        audioFormat.format, audioFormat.rate, audioFormat.encodingType);
    AudioNode::SetAudioNodeFormat(audioFormat);
    AudioSamplingRate bufferRate = audioFormat.rate == AudioSamplingRate::SAMPLE_RATE_11025 ?
        AudioSamplingRate::SAMPLE_RATE_16000 : audioFormat.rate;
    CHECK_AND_RETURN_LOG(inputNodeBuffer_ != nullptr, "AudioInputNode::SetAudioNodeFormat inputNodeBuffer_ is null");
    inputNodeBuffer_->ResizePcmBuffer(bufferRate, audioFormat.audioChannelInfo.numChannels);
    cachedBuffer_.ResizeBuffer(GetCacheBufferCapacity(audioFormat));
    SetFormatTransfer(audioFormat.rate);
}

int32_t AudioInputNode::InstallTap(AudioNodePortType portType, std::shared_ptr<SuiteNodeReadTapDataCallback> callback)
{
    CHECK_AND_RETURN_RET(outputStream_ != nullptr, ERR_INVALID_PARAM,
        "AudioInputNode::InstallTap outputStream_ is null");
    if (portType != outputStream_->GetPortType() || callback == nullptr) {
        AUDIO_ERR_LOG("AudioInputNode::InstallTap param error");
        return ERR_INVALID_PARAM;
    }
    tap_.SetAudioNodePortType(portType);
    tap_.SetOnReadTapDataCallback(callback);
    return SUCCESS;
}

int32_t AudioInputNode::RemoveTap(AudioNodePortType portType)
{
    tap_.SetOnReadTapDataCallback(nullptr);
    return SUCCESS;
}

int32_t AudioInputNode::GetDataFromUser()
{
    CHECK_AND_RETURN_RET_LOG(writeCallback_ != nullptr, ERR_INVALID_PARAM,
        "AudioInputNode::GetDataFromUser writeCallback_ is null");
    bool isFinished = GetAudioNodeDataFinishedFlag();
    if (isFinished) {
        AUDIO_INFO_LOG("AudioInputNode::GetDataFromUser already finish");
        return SUCCESS;
    }
    uint32_t needSize = GetNeedSizeFromUser();
    if (needSize == 0) {
        // 从缓存拿数据
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG((needSize > 0 && needSize < MAX_CACHE), ERR_INVALID_PARAM,
        "AudioInputNode::GetDataFromUser needSize error");
    uint8_t* rawData = new(std::nothrow) uint8_t[needSize];
    CHECK_AND_RETURN_RET_LOG(rawData != nullptr, ERR_MEMORY_ALLOC_FAILED,
        "AudioInputNode::GetDataFromUser rawData is null");
    uint32_t getSize = 0;
    if (DoRequestData(rawData, needSize, getSize, isFinished) != SUCCESS) {
        AUDIO_ERR_LOG("AudioInputNode::GetDataFromUser DoRequestData fail");
        delete[] rawData;
        return ERR_INVALID_OPERATION;
    }
    if (PushDataToCache(rawData, getSize) != SUCCESS) {
        AUDIO_ERR_LOG("AudioInputNode::GetDataFromUser PushDataToCache fail");
        delete[] rawData;
        return ERR_INVALID_OPERATION;
    }
    SetAudioNodeDataFinishedFlag(isFinished);
    delete[] rawData;
    return SUCCESS;
}

// 获取本次需要找应用拉取的数据大小
uint32_t AudioInputNode::GetNeedSizeFromUser()
{
    uint32_t needMinCacheSize = GetNeedMinCacheSize(); // 单帧转化后，占用的缓存大小
    uint32_t restSize = cachedBuffer_.GetRestSpace();
    if (needMinCacheSize > restSize) {
        // 找用户拿一帧数据后， 缓存放不下，本次不找用户获取数据
        AUDIO_INFO_LOG("AudioInputNode::GetNeedSizeFromUser get from cache");
        return 0;
    }
    uint32_t singleFrameSize = GetFrameSize(); // 单帧应用数据大小（11025采样率 40ms一帧）
    AUDIO_INFO_LOG("AudioInputNode::GetNeedSizeFromUser singleFrameSize:%{public}u,"
        "needMinCacheSize:%{public}u, restSize:%{public}u", singleFrameSize, needMinCacheSize, restSize);
    return std::max(singleFrameSize, GetUserDataSizeByCacheSize(restSize));
}

uint32_t AudioInputNode::GetNeedMinCacheSize()
{
    AudioFormat format = GetAudioNodeFormat();
    if (format.rate == AudioSamplingRate::SAMPLE_RATE_11025) {
        // 如果采样率为11025， 则缓存大小按 采样率16k， 位深f32，40ms
        return AudioSamplingRate::SAMPLE_RATE_16000 * SINGLE_FRAME_DURATION_SAMPLE_RATE_11025 *
            format.audioChannelInfo.numChannels * sizeof(float) / SECONDS_TO_MS;
    } else {
        return GetFrameSize(format);
    }
}

/*
    从应用处获取数据，若获取到的数据小于1帧，则卡在此处，一直获取，
    若数据足够一帧，则在满足重试次数小于REQUEST_DATA_TRY_COUNTS
    的情况下，重复找应用要数据，直到缓存区被填满
    若最小帧大于剩余缓存，不找应用要数据
*/
int32_t AudioInputNode::DoRequestData(uint8_t* rawData, uint32_t needSize, uint32_t& getSize, bool& isFinished)
{
    uint32_t curTryCounts = 0;
    uint32_t singleFrameSize = GetFrameSize();
    // 没有finish时，若数据小于1帧时或者（缓存没满且还有重试次数时）找用户拿数据
    while (!isFinished && (getSize < singleFrameSize ||
        ((getSize < needSize) && (curTryCounts < REQUEST_DATA_TRY_COUNTS)))) {
        ++curTryCounts;
        int32_t singleGetSize = writeCallback_->OnWriteDataCallBack(rawData + getSize, needSize, &isFinished);
        CHECK_AND_RETURN_RET_LOG(singleGetSize > 0, ERROR, "AudioInputNode::DoRequestData OnWriteDataCallBack error");
        getSize += static_cast<uint32_t>(singleGetSize);
        needSize -= static_cast<uint32_t>(singleGetSize);
        AUDIO_INFO_LOG("AudioInputNode Get data from user, getSize=%{public}d, singleGetSize=%{public}d,"
            "isFinished=%{public}d.", getSize, singleGetSize, isFinished);
    }
    return SUCCESS;
}

// 从应用获取的数据，放入缓存中
int32_t AudioInputNode::PushDataToCache(uint8_t* rawData, uint32_t dataSize)
{
    if (needResample_) { // 11025采样率数据，需要先重采样，再存入缓存
        uint32_t outSize = GetCacheSizeByUserDataSize(std::max(dataSize, GetFrameSize()));
        CHECK_AND_RETURN_RET_LOG((outSize > 0 && outSize < MAX_CACHE), ERR_INVALID_PARAM,
            "AudioInputNode::PushDataToCache outSize error");
        uint8_t* out = new(std::nothrow) uint8_t[outSize]{0};
        CHECK_AND_RETURN_RET_LOG(out != nullptr, ERR_MEMORY_ALLOC_FAILED,
            "AudioInputNode::PushDataToCache new out fail");
        AUDIO_INFO_LOG("AudioInputNode::PushDataToCache DoResample dataSize:%{public}u, outSize:%{public}u",
            dataSize, outSize);
        if (DoResample(rawData, dataSize, AudioSamplingRate::SAMPLE_RATE_11025, (float*)out,
            outSize, AudioSamplingRate::SAMPLE_RATE_16000) != SUCCESS) {
            AUDIO_ERR_LOG("AudioInputNode::PushDataToCache DoResample fail");
            delete[] out;
            return ERR_OPERATION_FAILED;
        }
        cachedBuffer_.PushData(out, outSize);
        delete[] out;
    } else {
        cachedBuffer_.PushData(rawData, dataSize);
    }
    return SUCCESS;
}

int32_t AudioInputNode::DoResample(uint8_t* inData, uint32_t inSize, AudioSamplingRate inSample,
    float* out, uint32_t outSize, AudioSamplingRate outSample)
{
    float* inFloatData = nullptr;
    AudioFormat format = GetAudioNodeFormat();
    uint32_t singleFrameSize = GetFrameSize(format);
    CHECK_AND_RETURN_RET_LOG(singleFrameSize != 0, ERR_INVALID_PARAM,
        "AudioInputNode::DoResample singleFrameSize is 0");
    uint32_t frameNum = inSize / singleFrameSize;
    // 数据不够一帧，按一帧算，算法会自动补静音数据
    frameNum = frameNum == 0 ? frameNum + 1 : frameNum;
    if (format.format != AudioSampleFormat::SAMPLE_F32LE) {
        uint32_t sampleSize = AudioSuiteUtil::GetSampleSize(format.format);
        CHECK_AND_RETURN_RET_LOG(sampleSize != 0, ERR_INVALID_PARAM,
            "AudioInputNode::DoResample sampleSize is 0");
        uint32_t sampleNum = inSize / sampleSize;
        CHECK_AND_RETURN_RET_LOG((sampleNum > 0 && sampleNum < MAX_CACHE), ERR_INVALID_PARAM,
            "AudioInputNode::PushDataToCache sampleNum error");
        inFloatData = new(std::nothrow) float[sampleNum];
        CHECK_AND_RETURN_RET_LOG(inFloatData != nullptr, ERR_MEMORY_ALLOC_FAILED,
            "AudioInputNode::DoResample new inFloatData fail");
        ConvertToFloat(format.format, sampleNum, inData, inFloatData);
        inSize = inSize * sizeof(float) / sampleSize;
    } else {
        inFloatData = (float*)inData;
    }
    uint32_t channelNum = format.audioChannelInfo.numChannels;
    uint32_t singleInSize = AudioSamplingRate::SAMPLE_RATE_11025 *
        SINGLE_FRAME_DURATION_SAMPLE_RATE_11025 / SECONDS_TO_MS;
    uint32_t singleHandleOutSize = AudioSamplingRate::SAMPLE_RATE_16000 * SINGLE_FRAME_DURATION / SECONDS_TO_MS;
    uint32_t singleFrameHandleOutSize = AudioSamplingRate::SAMPLE_RATE_16000 *
        SINGLE_FRAME_DURATION_SAMPLE_RATE_11025 / SECONDS_TO_MS;
    int32_t ret = SUCCESS;
    for (uint32_t i = 0; i < frameNum; i++) {
        ret = DoResampleProcess(inFloatData + singleInSize * channelNum * i, singleInSize,
            out + singleFrameHandleOutSize * channelNum * i, singleHandleOutSize);
        CHECK_AND_BREAK_LOG(ret == SUCCESS, "AudioInputNode::DoResample fail");
        ret = DoResampleProcess(inFloatData + singleInSize * channelNum * i, 0,
            out + singleFrameHandleOutSize * channelNum * i + singleHandleOutSize * channelNum, singleHandleOutSize);
        CHECK_AND_BREAK_LOG(ret == SUCCESS, "AudioInputNode::DoResample fail");
    }
    if (format.format != AudioSampleFormat::SAMPLE_F32LE) {
        delete[] inFloatData;
    }
    return ret;
}

int32_t AudioInputNode::GeneratePushBuffer()
{
    AUDIO_INFO_LOG("AudioInputNode::GeneratePushBuffer");
    CHECK_AND_RETURN_RET_LOG(inputNodeBuffer_ != nullptr, ERR_INVALID_OPERATION,
        "AudioInputNode::SetAudioNodeFormat inputNodeBuffer_ is null");
    uint32_t needMinSize = GetFrameSizeAfterTransfer(GetAudioNodeFormat());
    bool pcmIsFinished = false;
    pcmIsFinished = (cachedBuffer_.GetSize() <= needMinSize) ? GetAudioNodeDataFinishedFlag() : pcmIsFinished;
    inputNodeBuffer_->SetIsFinished(pcmIsFinished);
    AUDIO_INFO_LOG("AudioInputNode::GeneratePushBuffer, needMinSize:%{public}u, cachedBuffersize:%{public}u",
        needMinSize, cachedBuffer_.GetSize());
    uint32_t getSize = std::min(needMinSize, cachedBuffer_.GetSize());
    CHECK_AND_RETURN_RET_LOG((getSize > 0 && getSize < MAX_CACHE), ERR_INVALID_PARAM,
        "AudioInputNode::GeneratePushBuffer getSize error");
    AudioFormat format = GetAudioNodeFormat();
    uint32_t sampleSize = AudioSuiteUtil::GetSampleSize(format.format);
    CHECK_AND_RETURN_RET_LOG(sampleSize != 0, ERR_OPERATION_FAILED, "GetSampleSize sampleSize is zero");
    uint8_t* rawData = new(std::nothrow) uint8_t[getSize];
    CHECK_AND_RETURN_RET_LOG(rawData != nullptr, ERR_MEMORY_ALLOC_FAILED,
        "AudioInputNode::GeneratePushBuffer new rawData fail");
    inputNodeBuffer_->Reset();
    cachedBuffer_.GetData(rawData, getSize);
    if (needTransferBitWidth_) {
        AUDIO_INFO_LOG("AudioInputNode::GeneratePushBuffer, transferbit"
            "getSize:%{public}u, pcmIsFinished:%{public}d", getSize, pcmIsFinished);
        ConvertToFloat(format.format, getSize / sampleSize, rawData, inputNodeBuffer_->GetPcmDataBuffer());
    } else {
        if (memcpy_s(inputNodeBuffer_->GetPcmDataBuffer(), inputNodeBuffer_->GetFrameLen() * sizeof(float),
            rawData, getSize) != 0) {
            AUDIO_ERR_LOG("AudioInputNode::GeneratePushBuffer memcpy_s fail");
            delete[] rawData;
            return ERR_OPERATION_FAILED;
        }
        AUDIO_INFO_LOG("AudioInputNode::GeneratePushBuffer, "
            "getSize:%{public}u, pcmIsFinished:%{public}d", getSize, pcmIsFinished);
    }
    delete[] rawData;
    return SUCCESS;
}

// 将用户数据大小转为缓存大小
uint32_t AudioInputNode::GetCacheSizeByUserDataSize(uint32_t userSize)
{
    AudioFormat format = GetAudioNodeFormat();
    if (format.rate == AudioSamplingRate::SAMPLE_RATE_11025) {
        uint32_t sampleSize = AudioSuiteUtil::GetSampleSize(format.format);
        CHECK_AND_RETURN_RET_LOG(sampleSize != 0, ERR_OPERATION_FAILED,
            "AudioInputNode::GetCacheSizeByUserDataSize sampleSize is 0");
        return userSize * AudioSamplingRate::SAMPLE_RATE_16000 / AudioSamplingRate::SAMPLE_RATE_11025 *
            sizeof(float) / sampleSize;
    } else {
        return userSize;
    }
}

// 将缓存大小转换为用户数据大小
uint32_t AudioInputNode::GetUserDataSizeByCacheSize(uint32_t cacheSize)
{
    AudioFormat format = GetAudioNodeFormat();
    if (format.rate == AudioSamplingRate::SAMPLE_RATE_11025) {
        uint32_t sampleSize = AudioSuiteUtil::GetSampleSize(format.format);
        return cacheSize * AudioSamplingRate::SAMPLE_RATE_11025 * sampleSize /
            AudioSamplingRate::SAMPLE_RATE_16000 / sizeof(float);
    } else {
        return cacheSize;
    }
}

// 计算缓存大小
uint32_t AudioInputNode::GetCacheBufferCapacity(const AudioFormat& format)
{
    if (format.rate == AudioSamplingRate::SAMPLE_RATE_11025) {
        return GetFrameSizeAfterTransfer(format) * CACHE_FRAME_SAMPLE_RATE_11025_LEN;
    } else {
        return GetFrameSizeAfterTransfer(format) * CACHE_FRAME_LEN;
    }
}

// 缓存数据20ms的大小
uint32_t AudioInputNode::GetFrameSizeAfterTransfer(const AudioFormat& format)
{
    if (format.rate == AudioSamplingRate::SAMPLE_RATE_11025) {
        // 如果采样率为11025， 则缓存大小按 采样率16k， 位深f32，存4帧数据
        return AudioSamplingRate::SAMPLE_RATE_16000 * SINGLE_FRAME_DURATION *
            format.audioChannelInfo.numChannels * sizeof(float) / SECONDS_TO_MS;
    } else {
        return GetFrameSize(format);
    }
}

// 找应用单次获取的最小字节数
uint32_t AudioInputNode::GetFrameSize()
{
    return GetFrameSize(GetAudioNodeFormat());
}

uint32_t AudioInputNode::GetFrameSize(const AudioFormat& format)
{
    uint32_t sampleSize = AudioSuiteUtil::GetSampleSize(format.format);
    uint32_t frameTime = format.rate == AudioSamplingRate::SAMPLE_RATE_11025 ?
        SINGLE_FRAME_DURATION_SAMPLE_RATE_11025 : SINGLE_FRAME_DURATION;
    return format.rate * frameTime * format.audioChannelInfo.numChannels * sampleSize / SECONDS_TO_MS;
}

int32_t AudioInputNode::HandleTapCallback()
{
    CHECK_AND_RETURN_RET(outputStream_ != nullptr, ERR_INVALID_PARAM,
        "AudioInputNode::HandleTapCallback outputStream_ is null");
    CHECK_AND_RETURN_RET_LOG(inputNodeBuffer_ != nullptr, ERR_INVALID_OPERATION,
        "AudioInputNode::HandleTapCallback inputNodeBuffer_ is null");
    std::shared_ptr<SuiteNodeReadTapDataCallback> callback = tap_.GetOnReadTapDataCallback();
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "tap callback is nullptr");
    AudioNodePortType portType = outputStream_->GetPortType();
    AudioNodePortType tapType = tap_.GetAudioNodePortType();
    CHECK_AND_RETURN_RET_LOG(portType == tapType, ERR_INVALID_PARAM, "tap error");
    callback->OnReadTapDataCallback(static_cast<void*>(inputNodeBuffer_->GetPcmDataBuffer()),
        inputNodeBuffer_->GetFrameLen() * sizeof(float));
    return SUCCESS;
}

int32_t AudioInputNode::SetFormatTransfer(AudioSamplingRate sampleRate)
{
    needResample_ = sampleRate == AudioSamplingRate::SAMPLE_RATE_11025;
    needTransferBitWidth_ = !needResample_;
    return SUCCESS;
}
}
}
}