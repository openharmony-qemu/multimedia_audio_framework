/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "VolumeTools"
#endif

#include <cmath>

#include "volume_tools.h"
#include "volume_tools_c.h"
#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_utils.h"

namespace {
static const int32_t UINT8_SHIFT = 0x80;
static const int32_t INT24_SHIFT = 8;
static const int32_t INT24_MAX_VALUE = 8388607;
static const uint32_t SHIFT_EIGHT = 8;
static const int32_t RIGHT_SHIFT_8 = 256;
static const uint32_t SHIFT_SIXTEEN = 16;
static const uint32_t ARRAY_INDEX_TWO = 2;
static const size_t MIN_FRAME_SIZE = 1;
static const size_t MAX_FRAME_SIZE = 100000; // max to about 2s for 48khz
static const uint32_t INT_32_MAX = 0x7fffffff;
static const int32_t HALF_FACTOR = 2;
static const int32_t INT32_VOLUME_MIN = 0; // 0, min volume
static const uint32_t VOLUME_SHIFT = 16;
static constexpr int32_t INT32_VOLUME_MAX = 1 << VOLUME_SHIFT; // 1 << 16 = 65536, max volume
}
namespace OHOS {
namespace AudioStandard {
bool VolumeTools::IsVolumeValid(float volFloat)
{
    return volFloat >= 0.0 && volFloat <= 1.0;
}

bool VolumeTools::IsVolumeValid(int32_t volInt)
{
    return volInt >= INT32_VOLUME_MIN && volInt <= INT32_VOLUME_MAX;
}
bool VolumeTools::IsVolumeValid(ChannelVolumes vols)
{
    if (vols.channel > CHANNEL_16 || vols.channel < MONO) {
        return false;
    }
    for (size_t i = 0; i < vols.channel; i++) {
        if (!IsVolumeValid(vols.volStart[i]) || !IsVolumeValid(vols.volEnd[i])) {
            return false;
        }
    }

    return true;
}

int32_t VolumeTools::GetInt32Vol(float volFloat)
{
    if (IsVolumeValid(volFloat)) {
        return volFloat * INT32_VOLUME_MAX;
    }
    if (volFloat < 0.0) {
        return INT32_VOLUME_MIN;
    }
    return INT32_VOLUME_MAX;
}

ChannelVolumes VolumeTools::GetChannelVolumes(AudioChannel channel, int32_t volStart, int32_t volEnd)
{
    ChannelVolumes vols = {};
    if (!IsVolumeValid(volStart) || !IsVolumeValid(volEnd) || channel > CHANNEL_16 || channel < MONO) {
        AUDIO_ERR_LOG("GetChannelVolumes failed with invalid vol:%{public}d %{public}d channel: %{public}d", volStart,
            volEnd, channel);
        return vols;
    }
    for (size_t i = 0; i < channel; i++) {
        vols.volStart[i] = volStart;
        vols.volEnd[i] = volEnd;
    }
    vols.channel = channel;
    return vols;
}

ChannelVolumes VolumeTools::GetChannelVolumes(AudioChannel channel, float volStart, float volEnd)
{
    ChannelVolumes vols = {};
    if (!IsVolumeValid(volStart) || !IsVolumeValid(volEnd) || channel > CHANNEL_16 || channel < MONO) {
        AUDIO_ERR_LOG("GetChannelVolumes failed with invalid vol:%{public}f %{public}f channel: %{public}d", volStart,
            volEnd, channel);
        return vols;
    }
    for (size_t i = 0; i < channel; i++) {
        vols.volStart[i] = GetInt32Vol(volStart);
        vols.volEnd[i] = GetInt32Vol(volEnd);
    }
    vols.channel = channel;
    return vols;
}

size_t VolumeTools::GetByteSize(AudioSampleFormat format)
{
    size_t bitWidthSize = 0;
    switch (format) {
        case SAMPLE_U8:
            bitWidthSize = 1; // size is 1
            break;
        case SAMPLE_S16LE:
            bitWidthSize = 2; // size is 2
            break;
        case SAMPLE_S24LE:
            bitWidthSize = 3; // size is 3
            break;
        case SAMPLE_S32LE:
            bitWidthSize = 4; // size is 4
            break;
        case SAMPLE_F32LE:
            bitWidthSize = 4; // size is 4
            break;
        default:
            bitWidthSize = 2; // default size is 2
            break;
    }
    return bitWidthSize;
}

static inline uint32_t ReadInt24LE(const uint8_t *p)
{
    return ((uint32_t) p[ARRAY_INDEX_TWO] << SHIFT_SIXTEEN) | ((uint32_t) p[1] << SHIFT_EIGHT) | ((uint32_t) p[0]);
}

static inline void WriteInt24LE(uint8_t *p, uint32_t u)
{
    p[ARRAY_INDEX_TWO] = (uint8_t) (u >> SHIFT_SIXTEEN);
    p[1] = (uint8_t) (u >> SHIFT_EIGHT);
    p[0] = (uint8_t) u;
}

inline int32_t VolumeFlatten(int32_t vol)
{
    return vol < INT32_VOLUME_MIN ? 0 : (vol > INT32_VOLUME_MAX ? INT32_VOLUME_MAX : vol);
}

void ProcessOneFrame(uint8_t *ptr, AudioSampleFormat format, int32_t vol)
{
    int64_t temp = 0;
    int16_t *raw16 = nullptr;
    int32_t *raw32 = nullptr;
    float *rawFloat = nullptr;
    switch (format) {
        case SAMPLE_U8:
            temp = *ptr - UINT8_SHIFT;
            temp = (temp * vol) >> VOLUME_SHIFT;
            temp = temp < INT8_MIN ? INT8_MIN : (temp > INT8_MAX ? INT8_MAX : temp);
            *ptr = static_cast<uint8_t>(temp + UINT8_SHIFT);
            break;
        case SAMPLE_S16LE:
            raw16 = reinterpret_cast<int16_t *>(ptr);
            temp = (*raw16 * static_cast<int64_t>(vol)) >> VOLUME_SHIFT;
            *raw16 = temp > INT16_MAX ? INT16_MAX : (temp < INT16_MIN ? INT16_MIN : temp);
            break;
        case SAMPLE_S24LE:
            temp = static_cast<int32_t>(ReadInt24LE(ptr) << INT24_SHIFT) * static_cast<int64_t>(vol) >> VOLUME_SHIFT;
            WriteInt24LE(ptr, (static_cast<uint32_t>(temp) >> INT24_SHIFT));
            break;
        case SAMPLE_S32LE:
            raw32 = reinterpret_cast<int32_t *>(ptr);
            // int32_t * int16_t, max result is int48_t
            temp = (*raw32 * static_cast<int64_t>(vol)) >> VOLUME_SHIFT;
            *raw32 = temp > INT32_MAX ? INT32_MAX : (temp < INT32_MIN ? INT32_MIN : temp);
            break;
        case SAMPLE_F32LE:
            rawFloat = reinterpret_cast<float *>(ptr);
            *rawFloat = *rawFloat * (static_cast<float>(vol) / INT32_VOLUME_MAX);
            break;
        default:
            AUDIO_ERR_LOG("ProcessOneFrame with invalid format");
            break;
    }
}

// |---------frame1--------|---------frame2--------|---------frame3--------|
// |ch1-ch2-ch3-ch4-ch5-ch6|ch1-ch2-ch3-ch4-ch5-ch6|ch1-ch2-ch3-ch4-ch5-ch6|
int32_t VolumeTools::Process(const RingBufferWrapper& ringBufferDesc, AudioSampleFormat format, ChannelVolumes vols)
{
    // parms check
    if (format > SAMPLE_F32LE || !IsVolumeValid(vols)) {
        AUDIO_ERR_LOG("Process failed with invalid params");
        return ERR_INVALID_PARAM;
    }
    size_t byteSizePerData = GetByteSize(format);
    size_t byteSizePerFrame = byteSizePerData * vols.channel;
    if (ringBufferDesc.dataLength == 0 || ((ringBufferDesc.dataLength % byteSizePerFrame) != 0) ||
            ((ringBufferDesc.basicBufferDescs[0].bufLength) % byteSizePerFrame != 0)) {
        AUDIO_ERR_LOG("Process failed with invalid buffer, size is %{public}zu", ringBufferDesc.dataLength);
        return ERR_INVALID_PARAM;
    }

    size_t frameSize = ringBufferDesc.dataLength / byteSizePerFrame;
    if (frameSize < MIN_FRAME_SIZE) {
        AUDIO_ERR_LOG("Process failed with invalid frameSize, size is %{public}zu", frameSize);
        return ERR_INVALID_PARAM;
    }

    float volStep[CHANNEL_MAX] = {};
    for (size_t channelIdx = 0; channelIdx < vols.channel; channelIdx++) {
        if (vols.volEnd[channelIdx] == vols.volStart[channelIdx] || frameSize == MIN_FRAME_SIZE) {
            volStep[channelIdx] = 0.0;
        } else {
            volStep[channelIdx] = (static_cast<float>(vols.volEnd[channelIdx] - vols.volStart[channelIdx])) /
                (frameSize - MIN_FRAME_SIZE);
        }
    }

    RingBufferWrapper srcBuffer = ringBufferDesc;
    for (size_t frameIndex = 0; frameIndex < frameSize; frameIndex++) {
        for (size_t channelIdx = 0; channelIdx < vols.channel; channelIdx++) {
            int32_t vol = volStep[channelIdx] * frameIndex + vols.volStart[channelIdx];
            vol = VolumeFlatten(vol);
            uint8_t *samplePtr = srcBuffer.basicBufferDescs[0].buffer + channelIdx * byteSizePerData;
            ProcessOneFrame(samplePtr, format, vol);
        }
        if (srcBuffer.SeekFromStart(byteSizePerFrame) != SUCCESS) {
            // This branch should never be executed under any valid conditions. Consider let it crash?
            AUDIO_ERR_LOG("Seek err");
            return ERR_INVALID_PARAM;
        }
    }

    return SUCCESS;
}

int32_t VolumeTools::Process(const BufferDesc &bufferDesc, AudioSampleFormat format, ChannelVolumes vols)
{
    if (bufferDesc.dataLength > bufferDesc.bufLength || bufferDesc.buffer == nullptr || bufferDesc.dataLength == 0) {
        AUDIO_ERR_LOG("invalid bufferDesc");
        return ERR_INVALID_PARAM;
    }
    RingBufferWrapper ringBufferDesc;
    ringBufferDesc.dataLength = bufferDesc.dataLength;
    ringBufferDesc.basicBufferDescs[0].bufLength = bufferDesc.bufLength;
    ringBufferDesc.basicBufferDescs[0].buffer = bufferDesc.buffer;

    return Process(ringBufferDesc, format, vols);
}

double VolumeTools::GetVolDb(AudioSampleFormat format, int32_t vol)
{
    double volume = static_cast<double>(vol);
    switch (format) {
        case SAMPLE_U8:
            volume = volume / INT8_MAX;
            break;
        case SAMPLE_S16LE:
            volume = volume / INT16_MAX;
            break;
        case SAMPLE_S24LE:
            volume = volume / INT24_MAX_VALUE;
            break;
        case SAMPLE_S32LE:
            volume = volume / INT32_MAX;
            break;
        case SAMPLE_F32LE:
            volume = volume / INT32_MAX;
            break;
        default:
            break;
    }
    return std::log10(volume);
}

static void CountU8Volume(const BufferDesc &buffer, AudioChannel channel, ChannelVolumes &volMaps, size_t split,
    AudioSampleFormat format)
{
    if (split == 0) {
        AUDIO_ERR_LOG("invalid split");
        return;
    }
    size_t byteSizePerData = VolumeTools::GetByteSize(format);
    size_t byteSizePerFrame = byteSizePerData * channel;
    if (buffer.buffer == nullptr || byteSizePerFrame == 0 || buffer.bufLength % byteSizePerFrame != 0) {
        AUDIO_ERR_LOG("invalid buffer, size is %{public}zu", buffer.bufLength);
        return;
    }
    size_t frameSize = buffer.bufLength / byteSizePerFrame;
    if (frameSize < MIN_FRAME_SIZE || frameSize >= MAX_FRAME_SIZE) {
        AUDIO_ERR_LOG("invalid frameSize, size is %{public}zu", frameSize);
        return;
    }

    // Calculate the average value
    size_t size = frameSize / split;
    if (size == 0) {
        AUDIO_ERR_LOG("invalid size");
        return;
    }

    // reset maps
    for (size_t index = 0; index < channel; index++) {
        volMaps.volStart[index] = 0;
        volMaps.volEnd[index] = 0;
    }
    uint8_t *raw8 = buffer.buffer;
    for (size_t frameIndex = 0; frameIndex < frameSize - (split - 1); frameIndex += split) {
        for (size_t channelIdx = 0; channelIdx < channel; channelIdx++) {
            volMaps.volStart[channelIdx] += (*raw8 >= UINT8_SHIFT ? *raw8 - UINT8_SHIFT : UINT8_SHIFT - *raw8);
            raw8++;
        }
        raw8 += (split - 1) * channel;
    }

    for (size_t index = 0; index < channel; index++) {
        volMaps.volStart[index] /= static_cast<int32_t>(size);
    }
    return;
}

static void CountS16Volume(const BufferDesc &buffer, AudioChannel channel, ChannelVolumes &volMaps, size_t split,
    AudioSampleFormat format)
{
    if (split == 0) {
        AUDIO_ERR_LOG("invalid split");
        return;
    }
    size_t byteSizePerData = VolumeTools::GetByteSize(format);
    size_t byteSizePerFrame = byteSizePerData * channel;
    if (buffer.buffer == nullptr || byteSizePerFrame == 0 || buffer.bufLength % byteSizePerFrame != 0) {
        AUDIO_ERR_LOG("invalid buffer, size is %{public}zu", buffer.bufLength);
        return;
    }
    size_t frameSize = buffer.bufLength / byteSizePerFrame;
    if (frameSize < MIN_FRAME_SIZE || frameSize >= MAX_FRAME_SIZE) {
        AUDIO_ERR_LOG("invalid frameSize, size is %{public}zu", frameSize);
        return;
    }

    // Calculate the average value
    size_t size = frameSize / split;
    if (size == 0) {
        AUDIO_ERR_LOG("invalid size");
        return;
    }

    // reset maps
    for (size_t index = 0; index < channel; index++) {
        volMaps.volStart[index] = 0;
        volMaps.volEnd[index] = 0;
    }
    int16_t *raw16 = reinterpret_cast<int16_t *>(buffer.buffer);
    for (size_t frameIndex = 0; frameIndex < frameSize - (split - 1); frameIndex += split) {
        for (size_t channelIdx = 0; channelIdx < channel; channelIdx++) {
            volMaps.volStart[channelIdx] += (*raw16 >= 0 ? *raw16 : (-*raw16));
            raw16++;
        }
        raw16 += (split - 1) * channel;
    }

    for (size_t index = 0; index < channel; index++) {
        volMaps.volStart[index] /= static_cast<int32_t>(size);
    }
    return;
}

static void CountS24Volume(const BufferDesc &buffer, AudioChannel channel, ChannelVolumes &volMaps, size_t split,
    AudioSampleFormat format)
{
    if (split == 0) {
        AUDIO_ERR_LOG("invalid split");
        return;
    }
    const size_t byteSizePerData = VolumeTools::GetByteSize(format);
    size_t byteSizePerFrame = byteSizePerData * channel;
    if (buffer.buffer == nullptr || byteSizePerFrame == 0 || buffer.bufLength % byteSizePerFrame != 0) {
        AUDIO_ERR_LOG("invalid buffer, size is %{public}zu", buffer.bufLength);
        return;
    }
    size_t frameSize = buffer.bufLength / byteSizePerFrame;
    if (frameSize < MIN_FRAME_SIZE || frameSize >= MAX_FRAME_SIZE) {
        AUDIO_ERR_LOG("invalid frameSize, size is %{public}zu", frameSize);
        return;
    }

    // Calculate the average value
    size_t size = frameSize / split;
    if (size == 0) {
        AUDIO_ERR_LOG("invalid size");
        return;
    }

    // reset maps
    for (size_t index = 0; index < channel; index++) {
        volMaps.volStart[index] = 0;
        volMaps.volEnd[index] = 0;
    }
    int64_t volSums[CHANNEL_MAX] = {0};
    uint8_t *raw8 = buffer.buffer;
    for (size_t frameIndex = 0; frameIndex < frameSize - (split - 1); frameIndex += split) {
        for (size_t channelIdx = 0; channelIdx < channel; channelIdx++) {
            int32_t sample = static_cast<int32_t>(ReadInt24LE(raw8) << INT24_SHIFT);
            volSums[channelIdx] += std::abs(sample);
            raw8 += byteSizePerData;
        }
        raw8 += (split - 1) * channel * byteSizePerData;
    }

    for (size_t index = 0; index < channel; index++) {
        volSums[index] /= RIGHT_SHIFT_8; // equal to " >> INT24_SHIFT"
        volMaps.volStart[index] = static_cast<int32_t>(volSums[index] / size);
    }
    return;
}

static void CountS32Volume(const BufferDesc &buffer, AudioChannel channel, ChannelVolumes &volMaps, size_t split,
    AudioSampleFormat format)
{
    if (split == 0) {
        AUDIO_ERR_LOG("invalid split");
        return;
    }
    const size_t byteSizePerData = VolumeTools::GetByteSize(format);
    size_t byteSizePerFrame = byteSizePerData * channel;
    if (buffer.buffer == nullptr || byteSizePerFrame == 0 || buffer.bufLength % byteSizePerFrame != 0) {
        AUDIO_ERR_LOG("invalid buffer, size is %{public}zu", buffer.bufLength);
        return;
    }
    size_t frameSize = buffer.bufLength / byteSizePerFrame;
    if (frameSize < MIN_FRAME_SIZE || frameSize >= MAX_FRAME_SIZE) {
        AUDIO_ERR_LOG("invalid frameSize, size is %{public}zu", frameSize);
        return;
    }

    // Calculate the average value
    size_t size = frameSize / split;
    if (size == 0) {
        AUDIO_ERR_LOG("invalid size");
        return;
    }

    // reset maps
    int64_t volSums[CHANNEL_MAX] = {0};
    for (size_t index = 0; index < CHANNEL_MAX; index++) {
        volSums[index] = 0;
    }
    int32_t *raw32 = reinterpret_cast<int32_t *>(buffer.buffer);
    for (size_t frameIndex = 0; frameIndex < frameSize - (split - 1); frameIndex += split) {
        for (size_t channelIdx = 0; channelIdx < channel; channelIdx++) {
            if (*raw32 >= 0) {
                volSums[channelIdx] += *raw32;
            } else {
                volSums[channelIdx] -= *raw32;
            }
            raw32++;
        }
        raw32 += (split - 1) * channel;
    }

    for (size_t index = 0; index < channel; index++) {
        volSums[index] /= static_cast<int64_t>(size);
        volMaps.volStart[index] = static_cast<int32_t>(volSums[index]);
    }
    return;
}

static void CountF32Volume(const BufferDesc &buffer, AudioChannel channel, ChannelVolumes &volMaps, size_t split,
    AudioSampleFormat format)
{
    if (split == 0) {
        AUDIO_ERR_LOG("invalid split");
        return;
    }
    size_t byteSizePerData = VolumeTools::GetByteSize(format);
    size_t byteSizePerFrame = byteSizePerData * channel;
    if (buffer.buffer == nullptr || byteSizePerFrame == 0 || buffer.bufLength % byteSizePerFrame != 0) {
        AUDIO_ERR_LOG("invalid buffer, size is %{public}zu", buffer.bufLength);
        return;
    }
    size_t frameSize = buffer.bufLength / byteSizePerFrame;
    if (frameSize < MIN_FRAME_SIZE || frameSize >= MAX_FRAME_SIZE) {
        AUDIO_ERR_LOG("invalid frameSize, size is %{public}zu", frameSize);
        return;
    }

    // Calculate the average value
    size_t size = frameSize / split;
    if (size == 0) {
        AUDIO_ERR_LOG("invalid size");
        return;
    }

    // reset maps
    double volSums[CHANNEL_MAX] = {0};
    for (size_t index = 0; index < CHANNEL_MAX; index++) {
        volSums[index] = 0.0;
    }
    float *raw32 = reinterpret_cast<float *>(buffer.buffer);
    for (size_t frameIndex = 0; frameIndex < frameSize - (split - 1); frameIndex += split) {
        for (size_t channelIdx = 0; channelIdx < channel; channelIdx++) {
            volSums[channelIdx] += (*raw32 >= 0 ? *raw32 : (-*raw32));
            raw32++;
        }
        raw32 += (split - 1) * channel;
    }

    for (size_t index = 0; index < channel; index++) {
        volSums[index] = (volSums[index] * INT32_MAX) / static_cast<double>(size);
        volMaps.volStart[index] = static_cast<int32_t>(volSums[index]);
    }
    return;
}

ChannelVolumes VolumeTools::CountVolumeLevel(const BufferDesc &buffer, AudioSampleFormat format, AudioChannel channel,
    size_t split)
{
    ChannelVolumes channelVols = {};
    channelVols.channel = channel;
    if (format > SAMPLE_F32LE || channel > CHANNEL_16) {
        AUDIO_DEBUG_LOG("failed with invalid params");
        return channelVols;
    }
    switch (format) {
        case SAMPLE_U8:
            CountU8Volume(buffer, channel, channelVols, split, format);
            break;
        case SAMPLE_S16LE:
            CountS16Volume(buffer, channel, channelVols, split, format);
            break;
        case SAMPLE_S24LE:
            CountS24Volume(buffer, channel, channelVols, split, format);
            break;
        case SAMPLE_S32LE:
            CountS32Volume(buffer, channel, channelVols, split, format);
            break;
        case SAMPLE_F32LE:
            CountF32Volume(buffer, channel, channelVols, split, format);
            break;
        default:
            break;
    }

    return channelVols;
}

void VolumeTools::DfxOperation(const BufferDesc &buffer, AudioStreamInfo streamInfo, std::string logTag,
    int64_t &volumeDataCount, size_t split)
{
    size_t byteSizePerData = GetByteSize(streamInfo.format);
    size_t frameLen = byteSizePerData * static_cast<size_t>(streamInfo.channels) *
        static_cast<size_t>(streamInfo.samplingRate) * 0.02; // 0.02s
    CHECK_AND_RETURN_LOG(frameLen > 0, "frameLen is invalid");
    int64_t minVolume = INT_32_MAX;
    for (size_t index = 0; index < (buffer.bufLength + frameLen - 1) / frameLen; index++) {
        BufferDesc temp = {buffer.buffer + frameLen * index, std::min(buffer.bufLength - frameLen * index, frameLen),
            std::min(buffer.dataLength - frameLen * index, frameLen)};
        ChannelVolumes vols = CountVolumeLevel(temp, streamInfo.format, streamInfo.channels, split);
        if (streamInfo.channels == MONO) {
            minVolume = std::min(minVolume, static_cast<int64_t>(vols.volStart[0]));
        } else {
            minVolume = std::min(minVolume, static_cast<int64_t>(vols.volStart[0]) / HALF_FACTOR
            + static_cast<int64_t>(vols.volStart[1]) / HALF_FACTOR);
        }
        AudioLogUtils::ProcessVolumeData(logTag, vols, volumeDataCount);
    }
    Trace::Count(logTag, minVolume);
}

void VolumeTools::CalcMuteFrame(BufferDesc &buffer, AudioStreamInfo streamInfo, std::string logTag,
    int64_t &volumeDataCount, int64_t &muteFrameCnt, size_t split)
{
    size_t byteSizePerData = VolumeTools::GetByteSize(streamInfo.format);
    size_t byteSizePerFrame = byteSizePerData * streamInfo.channels;
    size_t frameLen = byteSizePerData * static_cast<size_t>(streamInfo.channels) *
        static_cast<size_t>(streamInfo.samplingRate) * 0.02; // 0.02s

    if (frameLen == 0) {
        AUDIO_ERR_LOG("invalid size");
        return;
    }
    int64_t minVolume = INT_32_MAX;
    for (size_t index = 0; index < (buffer.bufLength + frameLen - 1) / frameLen; index++) {
        BufferDesc temp = {buffer.buffer + frameLen * index, std::min(buffer.bufLength - frameLen * index, frameLen),
            std::min(buffer.dataLength - frameLen * index, frameLen)};

        size_t frameSize = temp.bufLength / byteSizePerFrame;
        ChannelVolumes vols = VolumeTools::CountVolumeLevel(temp, streamInfo.format, streamInfo.channels, split);
        if (streamInfo.channels == MONO) {
            minVolume = std::min(minVolume, static_cast<int64_t>(vols.volStart[0]));
        } else {
            minVolume = std::min(minVolume, static_cast<int64_t>(vols.volStart[0]) / HALF_FACTOR
                + static_cast<int64_t>(vols.volStart[1]) / HALF_FACTOR);
        }
        AudioLogUtils::ProcessVolumeData(logTag, vols, volumeDataCount);
        if (volumeDataCount < 0) {
            muteFrameCnt += static_cast<int64_t>(frameSize);
        }
    }
    Trace::Count(logTag, minVolume);
}

bool VolumeTools::IsZeroVolume(float volume)
{
    float d = volume - 0.0f;
    if ((d >= 0 && d <= FLOAT_EPS) || (d <= 0 && d >= -FLOAT_EPS)) {
        return true;
    }
    return false;
}
} // namespace AudioStandard
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
#endif
using namespace OHOS::AudioStandard;

int32_t ProcessVol(uint8_t *buffer, size_t length, AudioRawFormat rawformat, float volStart, float volEnd)
{
    BufferDesc desc = {0};
    desc.buffer = buffer;
    desc.bufLength = length;
    desc.dataLength = length;
    ChannelVolumes mapVols = VolumeTools::GetChannelVolumes(static_cast<AudioChannel>(rawformat.channels), volStart,
        volEnd);
    return VolumeTools::Process(desc, static_cast<AudioSampleFormat>(rawformat.format), mapVols);
}

#ifdef __cplusplus
}
#endif