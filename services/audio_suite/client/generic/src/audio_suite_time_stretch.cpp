/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioTimeStretchProcess"
#endif

#include "audio_suite_time_stretch.h"

#include <algorithm>
#include <climits>
#include <dlfcn.h>
#include <filesystem>

#include "securec.h"
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_time_stretch_impl.h"
#ifdef FEATURE_HITRACE_METER
#include "hitrace_meter.h"
#endif

namespace OHOS {
namespace AudioStandard {

namespace {
constexpr float FLOAT_EPS = 1e-6f;
constexpr int OFFSET_BIT_24 = 3;
constexpr int BIT_DEPTH_TWO = 2;
constexpr int BIT_8 = 8;
constexpr int BIT_16 = 16;
constexpr int BIT_32 = 32;

const std::string ALGO_SO_PATH = "/system/lib64/libaudio_time_stretch.z.so";
constexpr const char *PREFIX_SYSTEM_LIB64 = "/system/lib64/";

constexpr uint32_t MIN_SAMPLE_RATE = 8000;
constexpr uint32_t MAX_SAMPLE_RATE = 384000;
constexpr uint32_t MIN_CHANNELS = 1;
constexpr uint32_t MAX_CHANNELS = 16;
constexpr uint32_t MAX_PROCESS_SECONDS = 2;

constexpr uint32_t U8_SIZE  = 1;
constexpr uint32_t S16_SIZE = 2;
constexpr uint32_t S24_SIZE = 3;
constexpr uint32_t S32_SIZE = 4;
constexpr uint32_t F32_SIZE = 4;

static uint32_t Read24Bit(const uint8_t *p)
{
    return ((uint32_t)p[BIT_DEPTH_TWO] << BIT_16) | ((uint32_t)p[1] << BIT_8) | ((uint32_t)p[0]);
}

static void Write24Bit(uint8_t *p, uint32_t u)
{
    p[BIT_DEPTH_TWO] = (uint8_t)(u >> BIT_16);
    p[1] = (uint8_t)(u >> BIT_8);
    p[0] = (uint8_t)u;
}

static float CapMax(float v)
{
    return std::clamp(v, -1.0f + FLOAT_EPS, 1.0f - FLOAT_EPS);
}

static void ConvertFromU8ToFloat(unsigned n, const uint8_t *a, float *b)
{
    for (; n > 0; n--, a++, b++) {
        *b = static_cast<float>(*a - static_cast<uint8_t>(0x80U)) * (1.0 / 0x80U);
    }
}

static void ConvertFrom16BitToFloat(unsigned n, const int16_t *a, float *b)
{
    for (; n > 0; n--) {
        *(b++) = *(a++) * (1.0f / (1 << (BIT_16 - 1)));
    }
}

static void ConvertFrom24BitToFloat(unsigned n, const uint8_t *a, float *b)
{
    for (; n > 0; n--) {
        int32_t s = static_cast<int32_t>(Read24Bit(a)) << BIT_8;
        *b = s * (1.0f / (1U << (BIT_32 - 1)));
        a += OFFSET_BIT_24;
        b++;
    }
}

static void ConvertFrom32BitToFloat(unsigned n, const int32_t *a, float *b)
{
    for (; n > 0; n--) {
        *(b++) = *(a++) * (1.0f / (1U << (BIT_32 - 1)));
    }
}

static void ConvertFromFloatToU8(unsigned n, const float *a, uint8_t *b)
{
    for (; n > 0; n--) {
        float v = *(a++);
        *(b++) = static_cast<uint8_t>(CapMax(v) * 127.0f + 128.0f);
    }
}

static void ConvertFromFloatTo16Bit(unsigned n, const float *a, int16_t *b)
{
    for (; n > 0; n--) {
        float v = CapMax(*(a++)) * (1 << (BIT_16 - 1));
        *(b++) = static_cast<int16_t>(v);
    }
}

static void ConvertFromFloatTo24Bit(unsigned n, const float *a, uint8_t *b)
{
    for (; n > 0; n--) {
        float v = CapMax(*(a++)) * (1U << (BIT_32 - 1));
        Write24Bit(b, static_cast<uint32_t>(static_cast<int32_t>(v)) >> BIT_8);
        b += OFFSET_BIT_24;
    }
}

static void ConvertFromFloatTo32Bit(unsigned n, const float *a, int32_t *b)
{
    for (; n > 0; n--) {
        float v = CapMax(*(a++)) * (1U << (BIT_32 - 1));
        *(b++) = static_cast<int32_t>(v);
    }
}

static void ConvertToFloat(AudioSampleFormat format, unsigned n, void *src, float *dst)
{
    CHECK_AND_RETURN_LOG(src != nullptr, "ConvertToFloat failed: src is null");
    CHECK_AND_RETURN_LOG(dst != nullptr, "ConvertToFloat failed: dst is null");

    switch (format) {
        case SAMPLE_U8:
            ConvertFromU8ToFloat(n, (const uint8_t *)src, dst);
            break;
        case SAMPLE_S16LE:
            ConvertFrom16BitToFloat(n, (const int16_t *)src, dst);
            break;
        case SAMPLE_S24LE:
            ConvertFrom24BitToFloat(n, (const uint8_t *)src, dst);
            break;
        case SAMPLE_S32LE:
            ConvertFrom32BitToFloat(n, (const int32_t *)src, dst);
            break;
        default:
            break;
    }
}

static void ConvertFromFloat(AudioSampleFormat format, unsigned n, float *src, void *dst)
{
    CHECK_AND_RETURN_LOG(src != nullptr, "ConvertFromFloat failed: src is null");
    CHECK_AND_RETURN_LOG(dst != nullptr, "ConvertFromFloat failed: dst is null");

    switch (format) {
        case SAMPLE_U8:
            ConvertFromFloatToU8(n, src, (uint8_t *)dst);
            break;
        case SAMPLE_S16LE:
            ConvertFromFloatTo16Bit(n, src, (int16_t *)dst);
            break;
        case SAMPLE_S24LE:
            ConvertFromFloatTo24Bit(n, src, (uint8_t *)dst);
            break;
        case SAMPLE_S32LE:
            ConvertFromFloatTo32Bit(n, src, (int32_t *)dst);
            break;
        default:
            break;
    }
}
} // namespace

std::shared_ptr<AudioSuiteTimeStretch> AudioSuiteTimeStretch::Create(
    uint32_t sampleRate, uint32_t channels, AudioSampleFormat format)
{
    auto impl = std::make_shared<AudioSuiteTimeStretchImpl>(sampleRate, channels, format);
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, nullptr,
        "AudioSuiteTimeStretch::Create failed: out of memory, sr=%{public}u, ch=%{public}u, format=%{public}d",
        sampleRate, channels, static_cast<int32_t>(format));

    int32_t ret = impl->Initialize();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr,
        "AudioSuiteTimeStretch::Create failed: Initialize failed, ret=%{public}d, "
        "sr=%{public}u, ch=%{public}u, format=%{public}d", ret, sampleRate, channels,
        static_cast<int32_t>(format));

    AUDIO_INFO_LOG("AudioSuiteTimeStretch::Create end, sr=%{public}u, ch=%{public}u, format=%{public}d",
        sampleRate, channels, static_cast<int32_t>(format));
    return impl;
}

bool AudioSuiteTimeStretch::IsTimeStretchSupported()
{
    bool exists = std::filesystem::exists(ALGO_SO_PATH);
    AUDIO_INFO_LOG("AudioSuiteTimeStretch::IsTimeStretchSupported exists=%{public}d", exists);
    return exists;
}

AudioSuiteTimeStretchImpl::AudioSuiteTimeStretchImpl(uint32_t sampleRate, uint32_t channels,
    AudioSampleFormat format)
{
    sampleRate_ = sampleRate;
    channels_ = channels;
    format_ = format;
    maxFramesLimit_ = static_cast<int32_t>(sampleRate_) * static_cast<int32_t>(MAX_PROCESS_SECONDS);
    MapFormat(format);
}

AudioSuiteTimeStretchImpl::~AudioSuiteTimeStretchImpl()
{
    if (algoHandle_ != nullptr && algoApi_.destroy != nullptr) {
        algoApi_.destroy(algoHandle_);
        algoHandle_ = nullptr;
    }

    if (soHandle_ != nullptr) {
        dlclose(soHandle_);
        soHandle_ = nullptr;
    }
}

int32_t AudioSuiteTimeStretchImpl::Initialize()
{
    std::lock_guard<std::mutex> lock(mutex_);

    CHECK_AND_RETURN_RET_LOG(algoHandle_ == nullptr, ERR_ILLEGAL_STATE,
        "Initialize failed: already initialized");

    int32_t ret = ValidateConfig();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Initialize failed: ValidateConfig failed, ret=%{public}d", ret);

    ret = LoadAlgoSymbols();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Initialize failed: LoadAlgoSymbols failed, ret=%{public}d", ret);

    algoHandle_ = algoApi_.create(sampleRate_, channels_);
    CHECK_AND_RETURN_RET_LOG(algoHandle_ != nullptr, ERR_OPERATION_FAILED,
        "TimeStretchCreate failed, sr=%{public}u, ch=%{public}u", sampleRate_, channels_);

    return SUCCESS;
}

int32_t AudioSuiteTimeStretchImpl::MapFormat(AudioSampleFormat fmt)
{
    switch (fmt) {
        case SAMPLE_U8:
            formatBytes_ = U8_SIZE;
            needFormatConvert_ = true;
            break;
        case SAMPLE_S16LE:
            formatBytes_ = S16_SIZE;
            needFormatConvert_ = true;
            break;
        case SAMPLE_S24LE:
            formatBytes_ = S24_SIZE;
            needFormatConvert_ = true;
            break;
        case SAMPLE_S32LE:
            formatBytes_ = S32_SIZE;
            needFormatConvert_ = true;
            break;
        case SAMPLE_F32LE:
            formatBytes_ = F32_SIZE;
            needFormatConvert_ = false;
            break;
        default:
            formatBytes_ = 0;
            needFormatConvert_ = false;
            return ERR_NOT_SUPPORTED;
    }
    return SUCCESS;
}

int32_t AudioSuiteTimeStretchImpl::LoadAlgorithm()
{
    char buffer[PATH_MAX] = {0};
    char *canonicalPath = realpath(ALGO_SO_PATH.c_str(), buffer);
    CHECK_AND_RETURN_RET_LOG(canonicalPath != nullptr, ERR_OPERATION_FAILED, "realpath failed");
    CHECK_AND_RETURN_RET_LOG(strncmp(canonicalPath, PREFIX_SYSTEM_LIB64, strlen(PREFIX_SYSTEM_LIB64)) == 0,
        ERR_OPERATION_FAILED, "invalid so path, prefix check failed");

    soHandle_ = dlopen(canonicalPath, RTLD_NOW);
    CHECK_AND_RETURN_RET_LOG(soHandle_ != nullptr, ERR_OPERATION_FAILED,
        "dlopen failed");

    return SUCCESS;
}

int32_t AudioSuiteTimeStretchImpl::ValidateConfig() const
{
    CHECK_AND_RETURN_RET_LOG(formatBytes_ != 0, ERR_NOT_SUPPORTED,
        "unsupported format, formatBytes=0");
    CHECK_AND_RETURN_RET_LOG(sampleRate_ >= MIN_SAMPLE_RATE && sampleRate_ <= MAX_SAMPLE_RATE,
        ERR_INVALID_PARAM, "sampleRate %{public}u out of range [%{public}u, %{public}u]",
        sampleRate_, MIN_SAMPLE_RATE, MAX_SAMPLE_RATE);
    CHECK_AND_RETURN_RET_LOG(channels_ >= MIN_CHANNELS && channels_ <= MAX_CHANNELS,
        ERR_INVALID_PARAM, "channels %{public}u out of range [%{public}u, %{public}u]",
        channels_, MIN_CHANNELS, MAX_CHANNELS);
    return SUCCESS;
}

int32_t AudioSuiteTimeStretchImpl::LoadAlgoSymbols()
{
    int32_t ret = LoadAlgorithm();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "LoadAlgorithm failed, ret=%{public}d", ret);

    struct SymEntry { const char *name; void **target; };
    SymEntry entries[] = {
        {"TimeStretchCreate",           reinterpret_cast<void**>(&algoApi_.create)},
        {"TimeStretchDestroy",          reinterpret_cast<void**>(&algoApi_.destroy)},
        {"TimeStretchSetSpeed",         reinterpret_cast<void**>(&algoApi_.setSpeed)},
        {"TimeStretchGetSpeed",         reinterpret_cast<void**>(&algoApi_.getSpeed)},
        {"TimeStretchSetPitch",         reinterpret_cast<void**>(&algoApi_.setPitch)},
        {"TimeStretchGetPitch",         reinterpret_cast<void**>(&algoApi_.getPitch)},
        {"TimeStretchChangeSpeedFloat", reinterpret_cast<void**>(&algoApi_.changeSpeedFloat)},
        {"TimeStretchFlush",            reinterpret_cast<void**>(&algoApi_.flush)},
        {"TimeStretchGetSpec",          reinterpret_cast<void**>(&algoApi_.getSpec)},
        {"TimeStretchSamplesAvailable", reinterpret_cast<void**>(&algoApi_.samplesAvailable)},
    };
    for (const auto &e : entries) {
        *e.target = dlsym(soHandle_, e.name);
        CHECK_AND_RETURN_RET_LOG(*e.target != nullptr, ERR_OPERATION_FAILED,
            "Failed to get symbol %{public}s", e.name);
    }
    return SUCCESS;
}

uint32_t AudioSuiteTimeStretchImpl::GetBytesPerFrame() const
{
    return formatBytes_ * channels_;
}

void AudioSuiteTimeStretchImpl::EnsureBuffers(int32_t inFrames, int32_t maxOutFrames)
{
    int64_t totalInSamples = static_cast<int64_t>(inFrames) * static_cast<int64_t>(channels_);
    if (totalInSamples > 0 && static_cast<size_t>(totalInSamples) > floatInputBuffer_.size()) {
        floatInputBuffer_.resize(static_cast<size_t>(totalInSamples));
    }
    int64_t totalOutSamples = static_cast<int64_t>(maxOutFrames) * static_cast<int64_t>(channels_);
    if (totalOutSamples > 0 && static_cast<size_t>(totalOutSamples) > floatOutputBuffer_.size()) {
        floatOutputBuffer_.resize(static_cast<size_t>(totalOutSamples));
    }
}

int32_t AudioSuiteTimeStretchImpl::RunChangeSpeed(const uint8_t *inputData,
    int32_t inFrames, int32_t maxOutFrames, uint8_t *outputData)
{
#ifdef FEATURE_HITRACE_METER
    StartTrace(HITRACE_TAG_ZAUDIO, "AudioTimeStretch::ChangeSpeedFloat");
#endif
    if (needFormatConvert_) {
        EnsureBuffers(inFrames, maxOutFrames);
        if (inFrames > 0) {
            int64_t inSamples = static_cast<int64_t>(inFrames) * static_cast<int64_t>(channels_);
            ConvertToFloat(format_, static_cast<unsigned>(inSamples),
                           const_cast<uint8_t *>(inputData), floatInputBuffer_.data());
        }
        int32_t outFrames = algoApi_.changeSpeedFloat(algoHandle_,
            inFrames > 0 ? floatInputBuffer_.data() : nullptr,
            floatOutputBuffer_.data(), inFrames, maxOutFrames);
        if (outFrames > 0) {
            int64_t outSamples = static_cast<int64_t>(outFrames) * static_cast<int64_t>(channels_);
            ConvertFromFloat(format_, static_cast<unsigned>(outSamples),
                             floatOutputBuffer_.data(), outputData);
        }
#ifdef FEATURE_HITRACE_METER
        FinishTrace(HITRACE_TAG_ZAUDIO);
#endif
        return outFrames;
    }
    int32_t outFrames = algoApi_.changeSpeedFloat(algoHandle_,
        reinterpret_cast<const float *>(inputData),
        reinterpret_cast<float *>(outputData), inFrames, maxOutFrames);
#ifdef FEATURE_HITRACE_METER
    FinishTrace(HITRACE_TAG_ZAUDIO);
#endif
    return outFrames;
}

int32_t AudioSuiteTimeStretchImpl::SetSpeed(float speed)
{
    std::lock_guard<std::mutex> lock(mutex_);

    CHECK_AND_RETURN_RET_LOG(algoHandle_ != nullptr && algoApi_.setSpeed != nullptr,
        ERR_INVALID_HANDLE, "SetSpeed failed: algoHandle or setSpeed is null");
    CHECK_AND_RETURN_RET_LOG(speed >= TIME_STRETCH_MIN_SPEED && speed <= TIME_STRETCH_MAX_SPEED,
        ERR_INVALID_PARAM, "SetSpeed failed: speed %{public}f out of range [%{public}f, %{public}f]",
        speed, TIME_STRETCH_MIN_SPEED, TIME_STRETCH_MAX_SPEED);

    int32_t ret = algoApi_.setSpeed(algoHandle_, speed);
    CHECK_AND_RETURN_RET_LOG(ret == TIME_STRETCH_SUCCESS, ERR_OPERATION_FAILED,
        "TimeStretchSetSpeed failed: %{public}d, speed=%{public}f", ret, speed);
    speed_ = speed;
    AUDIO_INFO_LOG("AudioSuiteTimeStretchImpl::SetSpeed end, speed=%{public}f", speed);
    return SUCCESS;
}

float AudioSuiteTimeStretchImpl::GetSpeed() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (algoHandle_ == nullptr || algoApi_.getSpeed == nullptr) {
        AUDIO_WARNING_LOG("GetSpeed called with invalid state, algoHandle=%{public}p, getSpeed=%{public}p, "
                          "returning default 1.0f", algoHandle_, algoApi_.getSpeed);
        return 1.0f;
    }
    float speed = algoApi_.getSpeed(algoHandle_);
    AUDIO_INFO_LOG("AudioSuiteTimeStretchImpl::GetSpeed end, speed=%{public}f", speed);
    return speed;
}

int32_t AudioSuiteTimeStretchImpl::SetPitch(float pitch)
{
    std::lock_guard<std::mutex> lock(mutex_);

    CHECK_AND_RETURN_RET_LOG(algoHandle_ != nullptr && algoApi_.setPitch != nullptr,
        ERR_INVALID_HANDLE, "SetPitch failed: algoHandle or setPitch is null");
    CHECK_AND_RETURN_RET_LOG(pitch >= TIME_STRETCH_MIN_PITCH && pitch <= TIME_STRETCH_MAX_PITCH,
        ERR_INVALID_PARAM, "SetPitch failed: pitch %{public}f out of range [%{public}f, %{public}f]",
        pitch, TIME_STRETCH_MIN_PITCH, TIME_STRETCH_MAX_PITCH);

    int32_t ret = algoApi_.setPitch(algoHandle_, pitch);
    CHECK_AND_RETURN_RET_LOG(ret == TIME_STRETCH_SUCCESS, ERR_OPERATION_FAILED,
        "TimeStretchSetPitch failed: %{public}d, pitch=%{public}f", ret, pitch);
    AUDIO_INFO_LOG("AudioSuiteTimeStretchImpl::SetPitch end, pitch=%{public}f", pitch);
    return SUCCESS;
}

float AudioSuiteTimeStretchImpl::GetPitch() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (algoHandle_ == nullptr || algoApi_.getPitch == nullptr) {
        AUDIO_WARNING_LOG("GetPitch called with invalid state, algoHandle=%{public}p, getPitch=%{public}p, "
                          "returning default 1.0f", algoHandle_, algoApi_.getPitch);
        return 1.0f;
    }
    float pitch = algoApi_.getPitch(algoHandle_);
    AUDIO_INFO_LOG("AudioSuiteTimeStretchImpl::GetPitch end, pitch=%{public}f", pitch);
    return pitch;
}

int32_t AudioSuiteTimeStretchImpl::GetDelayFrames() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (algoHandle_ == nullptr || algoApi_.getSpec == nullptr) {
        AUDIO_WARNING_LOG("GetDelayFrames called with invalid state, algoHandle=%{public}p, getSpec=%{public}p, "
                          "returning default 0", algoHandle_, algoApi_.getSpec);
        return 0;
    }
    AudioTimeStretchSpec spec = algoApi_.getSpec(algoHandle_);
    return static_cast<int32_t>(spec.algorithmDelayFrames);
}

int32_t AudioSuiteTimeStretchImpl::Process(const uint8_t *inputData, int32_t inputSize,
    uint8_t *outputData, int32_t outputCapacity, int32_t &outSize)
{
    outSize = 0;

    CHECK_AND_RETURN_RET_LOG(inputData != nullptr && inputSize > 0 && outputData != nullptr &&
        outputCapacity > 0, ERR_INVALID_PARAM, "Process failed: invalid input/output parameters");

    std::lock_guard<std::mutex> lock(mutex_);

    CHECK_AND_RETURN_RET_LOG(algoHandle_ != nullptr && algoApi_.changeSpeedFloat != nullptr,
        ERR_INVALID_HANDLE, "Process failed: algoHandle or changeSpeedFloat is null");

    uint32_t bytesPerFrame = GetBytesPerFrame();
    CHECK_AND_RETURN_RET_LOG(bytesPerFrame != 0, ERR_ILLEGAL_STATE,
        "Process failed: bytesPerFrame is 0");
    CHECK_AND_RETURN_RET_LOG(inputSize % static_cast<int32_t>(bytesPerFrame) == 0,
        ERR_INVALID_PARAM, "Process failed: inputSize %{public}d not aligned to bytesPerFrame %{public}u",
        inputSize, bytesPerFrame);
    int32_t inFrames = inputSize / static_cast<int32_t>(bytesPerFrame);
    int32_t maxOutFrames = outputCapacity / static_cast<int32_t>(bytesPerFrame);
    CHECK_AND_RETURN_RET_LOG(maxOutFrames > 0, ERR_INVALID_PARAM,
        "Process failed: outputCapacity %{public}d smaller than one frame bytesPerFrame %{public}u",
        outputCapacity, bytesPerFrame);
    CHECK_AND_RETURN_RET_LOG(inFrames <= maxFramesLimit_, ERR_INVALID_PARAM,
        "Process failed: inFrames %{public}d exceeds limit %{public}d (sampleRate=%{public}u, maxSeconds=%{public}u)",
        inFrames, maxFramesLimit_, sampleRate_, MAX_PROCESS_SECONDS);

    int32_t expectedOutFrames = static_cast<int32_t>(static_cast<float>(inFrames) / speed_);
    if (maxOutFrames < expectedOutFrames) {
        AUDIO_WARNING_LOG("outputCapacity too small: maxOutFrames=%{public}d, expectedOutFrames=%{public}d "
            "(inFrames=%{public}d, speed=%{public}f)", maxOutFrames, expectedOutFrames, inFrames, speed_);
    }

    AUDIO_DEBUG_LOG("AudioTimeStretch::Process start, inFrames=%{public}d", inFrames);
    int32_t outFrames = RunChangeSpeed(inputData, inFrames, maxOutFrames, outputData);
    AUDIO_DEBUG_LOG("AudioTimeStretch::Process end, outFrames=%{public}d", outFrames);
    CHECK_AND_RETURN_RET_LOG(outFrames >= 0, ERR_OPERATION_FAILED,
        "TimeStretchChangeSpeedFloat failed: %{public}d", outFrames);

    AUDIO_DEBUG_LOG("AudioSuiteTimeStretchImpl::Process end, inFrames=%{public}d, outFrames=%{public}d, "
        "outputCapacity=%{public}d", inFrames, outFrames, outputCapacity);
    outSize = outFrames * static_cast<int32_t>(bytesPerFrame);
    return SUCCESS;
}

int32_t AudioSuiteTimeStretchImpl::Flush(uint8_t *outputData, int32_t outputCapacity, int32_t &outSize,
    int32_t &residualFrames)
{
    outSize = 0;
    residualFrames = 0;

    CHECK_AND_RETURN_RET_LOG(outputData != nullptr && outputCapacity > 0, ERR_INVALID_PARAM,
        "Flush failed: invalid output parameters");

    std::lock_guard<std::mutex> lock(mutex_);

    CHECK_AND_RETURN_RET_LOG(algoHandle_ != nullptr && algoApi_.flush != nullptr &&
        algoApi_.changeSpeedFloat != nullptr && algoApi_.samplesAvailable != nullptr,
        ERR_INVALID_HANDLE, "Flush failed: algoHandle or flush/changeSpeedFloat/samplesAvailable is null");

    uint32_t bytesPerFrame = GetBytesPerFrame();
    CHECK_AND_RETURN_RET_LOG(bytesPerFrame != 0, ERR_ILLEGAL_STATE,
        "Flush failed: bytesPerFrame is 0");
    int32_t maxOutFrames = outputCapacity / static_cast<int32_t>(bytesPerFrame);

    CHECK_AND_RETURN_RET_LOG(maxOutFrames > 0, ERR_INVALID_PARAM,
        "Flush failed: outputCapacity %{public}d smaller than one frame bytesPerFrame %{public}u",
        outputCapacity, bytesPerFrame);
    // Step 1: flush fills internal silence to drain residual frames
    AUDIO_DEBUG_LOG("AudioTimeStretch::Flush start");
    int32_t ret = algoApi_.flush(algoHandle_, nullptr);
    CHECK_AND_RETURN_RET_LOG(ret == TIME_STRETCH_SUCCESS, ERR_OPERATION_FAILED,
        "TimeStretchFlush failed: %{public}d", ret);

    // Step 2: drain remaining output with ChangeSpeedFloat(inFrames=0)
    int32_t outFrames = RunChangeSpeed(nullptr, 0, maxOutFrames, outputData);
    AUDIO_DEBUG_LOG("AudioTimeStretch::Flush end, outFrames=%{public}d", outFrames);
    CHECK_AND_RETURN_RET_LOG(outFrames >= 0, ERR_OPERATION_FAILED,
        "Flush changeSpeedFloat failed: %{public}d", outFrames);

    // Step 3: query residual frames still in algorithm buffer
    residualFrames = algoApi_.samplesAvailable(algoHandle_);

    outSize = outFrames * static_cast<int32_t>(bytesPerFrame);
    AUDIO_INFO_LOG("AudioSuiteTimeStretchImpl::Flush end, outSize=%{public}d, outputCapacity=%{public}d, "
        "residualFrames=%{public}d", outSize, outputCapacity, residualFrames);
    return SUCCESS;
}

} // namespace AudioStandard
} // namespace OHOS
