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

#include "format_converter.h"
#include "audio_stream_info.h"
#include "audio_log.h"
#include <string>
#include <vector>

namespace OHOS {
namespace AudioStandard {
#define PCM_FLOAT_EPS 1e-6f
#define BIT_16 16
#define INT32_FORMAT_SHIFT 31
static constexpr int32_t VOLUME_SHIFT_NUMBER = 16; // 1 >> 16 = 65536, max volume

static float CapMax(float v)
{
    float value = v;
    if (v >= 1.0f) {
        value = 1.0f - PCM_FLOAT_EPS;
    } else if (v <= -1.0f) {
        value = -1.0f + PCM_FLOAT_EPS;
    }
    return value;
}

static int16_t ConvertFromFloatTo16Bit(const float *a)
{
    float tmp = *a;
    float v = CapMax(tmp) * (1 << (BIT_16 - 1));
    return static_cast<int16_t>(v);
}

void MixS16Volume(const std::vector<AudioStreamData> &srcDataList, const AudioStreamData &dstData)
{
    size_t srcListSize = srcDataList.size();
    size_t loopCount = dstData.bufferDesc.dataLength / sizeof(int16_t);

    int16_t *dstPtr = reinterpret_cast<int16_t *>(dstData.bufferDesc.buffer);
    for (size_t offset = 0; loopCount > 0; loopCount--) {
        int32_t sum = 0;
        for (size_t i = 0; i < srcListSize; i++) {
            int32_t vol = srcDataList[i].volumeStart;
            int16_t *srcPtr = reinterpret_cast<int16_t *>(srcDataList[i].bufferDesc.buffer) + offset;
            sum += (*srcPtr * static_cast<int64_t>(vol)) >> VOLUME_SHIFT_NUMBER;
        }
        offset++;
        *dstPtr++ = sum > INT16_MAX ? INT16_MAX : (sum < INT16_MIN ? INT16_MIN : sum);
    }
}

void MixS32Volume(const std::vector<AudioStreamData> &srcDataList, const AudioStreamData &dstData)
{
    size_t srcListSize = srcDataList.size();
    size_t loopCount = dstData.bufferDesc.dataLength / sizeof(int32_t);

    int32_t *dstPtr = reinterpret_cast<int32_t *>(dstData.bufferDesc.buffer);
    for (size_t offset = 0; loopCount > 0; loopCount--) {
        int64_t sum = 0;
        for (size_t i = 0; i < srcListSize; i++) {
            int32_t vol = srcDataList[i].volumeStart;
            int32_t *srcPtr = reinterpret_cast<int32_t *>(srcDataList[i].bufferDesc.buffer) + offset;
            sum += (*srcPtr * static_cast<int64_t>(vol)) >> VOLUME_SHIFT_NUMBER;
        }
        offset++;
        sum = sum > INT32_MAX ? INT32_MAX : (sum < INT32_MIN ? INT32_MIN : sum);
        *dstPtr++ = static_cast<int32_t>(sum);
    }
}

// only use volumeStart, not smooth from volumeStart to volumeEnd
bool FormatConverter::DataAccumulationFromVolume(const std::vector<AudioStreamData> &srcDataList,
    const AudioStreamData &dstData)
{
    size_t srcListSize = srcDataList.size();
    for (size_t i = 0; i < srcListSize; i++) {
        if (srcDataList[i].streamInfo.format != dstData.streamInfo.format ||
            srcDataList[i].streamInfo.channels != STEREO ||
            srcDataList[i].bufferDesc.bufLength != dstData.bufferDesc.bufLength ||
            srcDataList[i].bufferDesc.dataLength != dstData.bufferDesc.dataLength) {
            AUDIO_ERR_LOG("ProcessData failed, streamInfo are different: format %{public}d channels %{public}d "
                "bufLength %{public}zu dataLength %{public}zu", srcDataList[i].streamInfo.format,
                srcDataList[i].streamInfo.channels, srcDataList[i].bufferDesc.bufLength,
                srcDataList[i].bufferDesc.dataLength);
            return false;
        }
    }
    // Assum using the same format and same size
    CHECK_AND_RETURN_RET_LOG((dstData.streamInfo.format == SAMPLE_S16LE || dstData.streamInfo.format == SAMPLE_S32LE) &&
        dstData.streamInfo.channels == STEREO, false, "ProcessData failed, streamInfo are not support");

    if (dstData.streamInfo.format == SAMPLE_S16LE) {
        MixS16Volume(srcDataList, dstData);
    } else if (dstData.streamInfo.format == SAMPLE_S32LE) {
        MixS32Volume(srcDataList, dstData);
    }
    return true;
}

int32_t FormatConverter::S32MonoToS16Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc)
{
    size_t quarter = sizeof(int32_t);
    if (srcDesc.bufLength != dstDesc.bufLength || srcDesc.buffer == nullptr || dstDesc.buffer == nullptr ||
        srcDesc.bufLength % quarter != 0) {
        return -1;
    }
    int32_t *stcPtr = reinterpret_cast<int32_t *>(srcDesc.buffer);
    int16_t *dstPtr = reinterpret_cast<int16_t *>(dstDesc.buffer);
    size_t count = srcDesc.bufLength / quarter;

    double maxInt32 = INT32_MAX;
    double maxInt16 = INT16_MAX;
    for (size_t idx = 0; idx < count; idx++) {
        int16_t temp = static_cast<int16_t>((static_cast<double>(*stcPtr) / maxInt32) * maxInt16);
        stcPtr++;
        *(dstPtr++) = temp;
        *(dstPtr++) = temp;
    }
    return 0;
}

int32_t FormatConverter::S32StereoToS16Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc)
{
    size_t half = 2;
    if (srcDesc.bufLength / half != dstDesc.bufLength || srcDesc.buffer == nullptr || dstDesc.buffer == nullptr ||
        dstDesc.bufLength % half != 0) {
        return -1;
    }
    int32_t *stcPtr = reinterpret_cast<int32_t *>(srcDesc.buffer);
    int16_t *dstPtr = reinterpret_cast<int16_t *>(dstDesc.buffer);
    size_t count = srcDesc.bufLength / half / half;
    double maxInt32 = INT32_MAX;
    double maxInt16 = INT16_MAX;
    for (size_t idx = 0; idx < count; idx++) {
        int16_t temp = static_cast<int16_t>((static_cast<double>(*stcPtr) / maxInt32) * maxInt16);
        stcPtr++;
        *(dstPtr++) = temp;
    }
    return 0;
}

int32_t FormatConverter::S16MonoToS16Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc)
{
    size_t half = 2; // mono(1) -> stereo(2)
    if (srcDesc.bufLength != dstDesc.bufLength / half || srcDesc.buffer == nullptr || dstDesc.buffer == nullptr) {
        return -1;
    }
    int16_t *stcPtr = reinterpret_cast<int16_t *>(srcDesc.buffer);
    int16_t *dstPtr = reinterpret_cast<int16_t *>(dstDesc.buffer);
    size_t count = srcDesc.bufLength / sizeof(int16_t);
    for (size_t idx = 0; idx < count; idx++) {
        *(dstPtr++) = *stcPtr;
        *(dstPtr++) = *stcPtr++;
    }
    return 0;
}

int32_t FormatConverter::S16StereoToS16Mono(const BufferDesc &srcDesc, const BufferDesc &dstDesc)
{
    size_t half = 2; // stereo(2) -> mono(1)
    if (dstDesc.bufLength != srcDesc.bufLength / half || srcDesc.buffer == nullptr || dstDesc.buffer == nullptr) {
        return -1;
    }
    int16_t *stcPtr = reinterpret_cast<int16_t *>(srcDesc.buffer);
    int16_t *dstPtr = reinterpret_cast<int16_t *>(dstDesc.buffer);
    size_t count = srcDesc.bufLength / half / sizeof(int16_t);
    for (size_t idx = 0; idx < count; idx++) {
        *(dstPtr++) = (*stcPtr + *(stcPtr + 1)) / 2; // To obtain mono channel, add left to right, then divide by 2
        stcPtr += 2; // ptr++ on mono is equivalent to ptr+=2 on stereo
    }
    return 0;
}

int32_t FormatConverter::S16StereoToS32Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc)
{
    const size_t half = sizeof(int16_t);
    if (srcDesc.bufLength != dstDesc.bufLength / half || srcDesc.buffer == nullptr || dstDesc.buffer == nullptr) {
        return -1;
    }
    int16_t *srcPtr = reinterpret_cast<int16_t *>(srcDesc.buffer);
    int32_t *dstPtr = reinterpret_cast<int32_t *>(dstDesc.buffer);
    size_t count = srcDesc.bufLength / sizeof(int16_t);
    double maxInt16 = INT16_MAX;
    double maxInt32 = INT32_MAX;
    for (size_t idx = 0; idx < count; idx++) {
        int32_t temp = static_cast<int32_t>((static_cast<double>(*srcPtr) / maxInt16) * maxInt32);
        srcPtr++;
        *(dstPtr++) = temp;
    }
    return 0;
}

int32_t FormatConverter::S16MonoToS32Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc)
{
    const size_t quarter = sizeof(int32_t);
    if (srcDesc.bufLength != dstDesc.bufLength / quarter || srcDesc.buffer == nullptr || dstDesc.buffer == nullptr) {
        return -1;
    }
    int16_t *srcPtr = reinterpret_cast<int16_t *>(srcDesc.buffer);
    int32_t *dstPtr = reinterpret_cast<int32_t *>(dstDesc.buffer);
    size_t count = srcDesc.bufLength / sizeof(int16_t);
    double maxInt16 = INT16_MAX;
    double maxInt32 = INT32_MAX;
    for (size_t idx = 0; idx < count; idx++) {
        int32_t temp = static_cast<int32_t>((static_cast<double>(*srcPtr) / maxInt16) * maxInt32);
        srcPtr++;
        *(dstPtr++) = temp; // left
        *(dstPtr++) = temp; // right
    }
    return 0;
}

int32_t FormatConverter::S32MonoToS32Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc)
{
    const size_t half = sizeof(int16_t);
    if (srcDesc.bufLength != dstDesc.bufLength / half || srcDesc.buffer == nullptr || dstDesc.buffer == nullptr) {
        return -1;
    }
    int32_t *srcPtr = reinterpret_cast<int32_t *>(srcDesc.buffer);
    int32_t *dstPtr = reinterpret_cast<int32_t *>(dstDesc.buffer);
    size_t count = srcDesc.bufLength / sizeof(int32_t);

    for (size_t idx = 0; idx < count; idx++) {
        *(dstPtr++) = *srcPtr; // left
        *(dstPtr++) = *srcPtr++; // right
    }
    return 0;
}

int32_t FormatConverter::F32MonoToS32Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc)
{
    const size_t half = 2;
    if (srcDesc.bufLength != dstDesc.bufLength / half || srcDesc.buffer == nullptr || dstDesc.buffer == nullptr) {
        return -1;
    }
    float *srcPtr = reinterpret_cast<float *>(srcDesc.buffer);
    int32_t *dstPtr = reinterpret_cast<int32_t *>(dstDesc.buffer);
    size_t count = srcDesc.bufLength / sizeof(float);
    for (size_t idx = 0; idx < count; idx++) {
        int32_t temp = static_cast<int32_t>(CapMax(*(srcPtr++)) * (1 << INT32_FORMAT_SHIFT));
        *(dstPtr++) = temp; // left
        *(dstPtr++) = temp; // right
    }
    return 0;
}

int32_t FormatConverter::F32StereoToS32Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc)
{
    if (srcDesc.bufLength != dstDesc.bufLength || srcDesc.buffer == nullptr || dstDesc.buffer == nullptr) {
        return -1;
    }
    float *srcPtr = reinterpret_cast<float *>(srcDesc.buffer);
    int32_t *dstPtr = reinterpret_cast<int32_t *>(dstDesc.buffer);
    size_t count = srcDesc.bufLength / sizeof(float);
    for (size_t idx = 0; idx < count; idx++) {
        int32_t temp = static_cast<int32_t>(CapMax(*(srcPtr++)) * (1 << INT32_FORMAT_SHIFT));
        *(dstPtr++) = temp;
    }
    return 0;
}

int32_t FormatConverter::S16StereoToF32Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc)
{
    size_t half = 2;
    if (srcDesc.bufLength != dstDesc.bufLength / half || srcDesc.buffer == nullptr || dstDesc.buffer == nullptr) {
        return -1;
    }
    int16_t *srcPtr = reinterpret_cast<int16_t *>(srcDesc.buffer);
    float *dstPtr = reinterpret_cast<float *>(dstDesc.buffer);
    size_t count = srcDesc.bufLength / sizeof(int16_t);
    const float FLOAT_SCALE = 1.0f / (1 << (BIT_16 - 1));
    for (size_t idx = 0; idx < count; idx++) {
        *dstPtr = (*srcPtr) * FLOAT_SCALE;
        dstPtr++;
        srcPtr++;
    }
    return 0;
}

int32_t FormatConverter::S16StereoToF32Mono(const BufferDesc &srcDesc, const BufferDesc &dstDesc)
{
    size_t half = 2;
    if (srcDesc.bufLength != dstDesc.bufLength || srcDesc.buffer == nullptr || dstDesc.buffer == nullptr) {
        return -1;
    }
    int16_t *srcPtr = reinterpret_cast<int16_t *>(srcDesc.buffer);
    float *dstPtr = reinterpret_cast<float *>(dstDesc.buffer);
    size_t count = srcDesc.bufLength / half / sizeof(int16_t);
    const float FLOAT_SCALE = 1.0f / (1 << (BIT_16 - 1));
    const size_t SRC_INCREMENT = 2;
    for (size_t idx = 0; idx < count; idx++) {
        *dstPtr = (static_cast<float>(*srcPtr + *(srcPtr + 1)) / half) * FLOAT_SCALE;
        dstPtr++;
        srcPtr += SRC_INCREMENT;
    }
    return 0;
}

int32_t FormatConverter::F32MonoToS16Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc)
{
    size_t quarter = 4;
    if (srcDesc.bufLength != dstDesc.bufLength || srcDesc.buffer == nullptr || dstDesc.buffer == nullptr ||
        srcDesc.bufLength % quarter != 0) {
        return -1;
    }
    float *stcPtr = reinterpret_cast<float *>(srcDesc.buffer);
    int16_t *dstPtr = reinterpret_cast<int16_t *>(dstDesc.buffer);
    size_t count = srcDesc.bufLength / quarter;

    for (size_t idx = 0; idx < count; idx++) {
        int16_t temp = ConvertFromFloatTo16Bit(stcPtr);
        stcPtr++;
        *(dstPtr++) = temp;
        *(dstPtr++) = temp;
    }
    return 0;
}

int32_t FormatConverter::F32StereoToS16Stereo(const BufferDesc &srcDesc, const BufferDesc &dstDesc)
{
    size_t half = 2;
    if (srcDesc.bufLength / half != dstDesc.bufLength || srcDesc.buffer == nullptr || dstDesc.buffer == nullptr ||
        dstDesc.bufLength % half != 0) {
        return -1;
    }
    float *stcPtr = reinterpret_cast<float *>(srcDesc.buffer);
    int16_t *dstPtr = reinterpret_cast<int16_t *>(dstDesc.buffer);
    size_t count = srcDesc.bufLength / half / half;

    for (size_t idx = 0; idx < count; idx++) {
        int16_t temp = ConvertFromFloatTo16Bit(stcPtr);
        stcPtr++;
        *(dstPtr++) = temp;
    }
    return 0;
}

int32_t FormatConverter::S32MonoToS16Mono(std::vector<char> &audioBuffer, std::vector<char> &audioBufferConverted)
{
    size_t half = 2;
    int32_t size = audioBuffer.size();
    if (size == 0) {
        return -1;
    }

    audioBufferConverted.resize(size / half);
    int32_t *stcPtr = reinterpret_cast<int32_t *>(audioBuffer.data());
    int16_t *dstPtr = reinterpret_cast<int16_t *>(audioBufferConverted.data());
    size_t count = size / sizeof(int32_t);

    double maxInt32 = INT32_MAX;
    double maxInt16 = INT16_MAX;
    for (size_t idx = 0; idx < count; idx++) {
        int16_t temp = static_cast<int16_t>((static_cast<double>(*stcPtr) / maxInt32) * maxInt16);
        *(dstPtr++) = temp;
        stcPtr++;
    }
    return 0;
}

int32_t FormatConverter::S32StereoToS16Stereo(std::vector<char> &audioBuffer, std::vector<char> &audioBufferConverted)
{
    size_t half = 2;
    int32_t size = audioBuffer.size();
    if (size == 0) {
        return -1;
    }

    audioBufferConverted.resize(size / half);
    int32_t *stcPtr = reinterpret_cast<int32_t *>(audioBuffer.data());
    int16_t *dstPtr = reinterpret_cast<int16_t *>(audioBufferConverted.data());
    size_t count = size / sizeof(int32_t);

    double maxInt32 = INT32_MAX;
    double maxInt16 = INT16_MAX;
    for (size_t idx = 0; idx < count; idx++) {
        int16_t temp = static_cast<int16_t>((static_cast<double>(*stcPtr) / maxInt32) * maxInt16);
        *(dstPtr++) = temp;
        stcPtr++;
    }
    return 0;
}

} // namespace AudioStandard
} // namespace OHOS
