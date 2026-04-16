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

#ifndef AUDIO_FORMAT_CONVERTER_IMPL_H
#define AUDIO_FORMAT_CONVERTER_IMPL_H

#include "audio_suite_pcm_buffer.h"
#include "audio_suite_base.h"
#include "audio_suite_format_conversion.h"
#include "audio_format_converter.h"
#include <functional>
#include <memory>
#include <vector>
#include <mutex>
#include <cstdint>
#include <unordered_set>
#include <unordered_map>

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

// 格式转换器配置
struct FormatConverterConfig {
    PcmBufferFormat inputFormat;
    PcmBufferFormat outputFormat;
};

// 格式校验支持的采样率集合
static const std::unordered_set<AudioSamplingRate> SUPPORTED_SAMPLE_RATES = {
    AudioSamplingRate::SAMPLE_RATE_8000,
    AudioSamplingRate::SAMPLE_RATE_11025,
    AudioSamplingRate::SAMPLE_RATE_12000,
    AudioSamplingRate::SAMPLE_RATE_16000,
    AudioSamplingRate::SAMPLE_RATE_22050,
    AudioSamplingRate::SAMPLE_RATE_24000,
    AudioSamplingRate::SAMPLE_RATE_32000,
    AudioSamplingRate::SAMPLE_RATE_44100,
    AudioSamplingRate::SAMPLE_RATE_48000,
    AudioSamplingRate::SAMPLE_RATE_64000,
    AudioSamplingRate::SAMPLE_RATE_88200,
    AudioSamplingRate::SAMPLE_RATE_96000,
    AudioSamplingRate::SAMPLE_RATE_176400,
    AudioSamplingRate::SAMPLE_RATE_192000
};

// 格式校验支持的采样格式集合
static const std::unordered_set<AudioSampleFormat> SUPPORTED_SAMPLE_FORMATS = {
    AudioSampleFormat::SAMPLE_U8,
    AudioSampleFormat::SAMPLE_S16LE,
    AudioSampleFormat::SAMPLE_S24LE,
    AudioSampleFormat::SAMPLE_S32LE,
    AudioSampleFormat::SAMPLE_F32LE
};

// LAYOUT_TO_CHANNEL 已移至 audio_suite_base.h

// InputDataStatus 和 FormatConverterDataCallback 已移至 audio_format_converter.h

// 可处理字节计算参数
struct ProcessableBytesCalculationParams {
    uint32_t inputBytesPerFrame;
    uint32_t maxOutputFrames;
    uint32_t outputBytesPerFrame;
    uint32_t minBytesForIntMs;
    uint32_t alignedCachedSize;
};

// 输入处理参数
struct InputProcessParams {
    uint32_t inputBytesPerFrame;
    uint32_t maxOutputFrames;
    uint32_t outputBytesPerFrame;
    uint32_t outputCapacity;
};

// 处理上下文
struct ProcessingContext {
    uint32_t totalOutputSize;
    uint8_t* outputBufferPtr;
    uint32_t remainingCapacity;
    std::vector<uint8_t> backupInputData;
};

// 格式转换器 InnerAPI 类（实现接口）
class AudioFormatConverterImpl : public IAudioFormatConverter {
public:
    AudioFormatConverterImpl();
    ~AudioFormatConverterImpl();

    // 接口1：Create - 初始化转换器（带格式参数和格式校验）
    int32_t Create(const PcmBufferFormat& inputFormat, const PcmBufferFormat& outputFormat) override;

    // 接口2：SetInputCallback - 设置数据请求回调
    int32_t SetInputCallback(std::shared_ptr<FormatConverterDataCallback> callback) override;

    // 接口3：Process - 处理音频数据
    int32_t Process(void* outputData, uint32_t outputCapacity, uint32_t* outputSize) override;

    // 接口4：Destroy - 销毁转换器
    void Destroy() override;

private:
    // 内部实现方法
    int32_t WrapInputData(const void* inputData, uint32_t inputSize, bool finished,
                          AudioSuitePcmBuffer& buffer);
    AudioSuitePcmBuffer* DoConvert(AudioSuitePcmBuffer* inputBuffer);
    void CopyOutputData(AudioSuitePcmBuffer* buffer, void* outputBuffer,
                        uint32_t maxFrameSize, uint32_t* actualFrameSize);
    
    // 计算相关方法
    uint32_t CalculateMinBytesForIntMs(uint32_t inputBytesPerFrame) const;
    int32_t CalculateMinBytesForIntMsInternal(uint32_t inputBytesPerFrame,
        uint32_t& minBytesForIntMs, uint32_t& alignedCachedSize, bool& shouldStopProcessing, uint32_t outputCapacity);
    int32_t CalculateProcessableBytesInternal(const ProcessableBytesCalculationParams& params,
        uint32_t& alignedProcessableBytes, bool& shouldStopProcessing);
    
    // 数据读取和缓存
    int32_t ReadAndCacheInputData(bool* shouldStopProcessing);
    
    // 处理流程方法
    int32_t WrapAndConvertInputData(uint32_t processableSize, AudioSuitePcmBuffer* outConvertedBuffer);
    int32_t CheckAndProcessInputData(const InputProcessParams& params, uint32_t& minBytesForIntMs,
        uint32_t& alignedCachedSize, uint32_t& alignedProcessableBytes, bool& shouldStopProcessing);
    int32_t InitializeProcessingContext(void* audioData, uint32_t outputCapacity,
        uint32_t inputBytesPerFrame, ProcessingContext& context, bool& shouldStopProcessing);
    int32_t ValidateOutputCapacity(AudioSuitePcmBuffer& convertedBuffer,
        uint32_t remainingCapacity, uint32_t* framesToWrite);
    void UpdateProcessingState(AudioSuitePcmBuffer& convertedBuffer,
        uint32_t processableSize, ProcessingContext& context, uint32_t actualFramesWritten);
    int32_t ProcessSingleBlock(const InputProcessParams& params,
        ProcessingContext& context, uint32_t inputBytesPerFrame,
        uint32_t outputBytesPerFrame, bool& shouldStopProcessing);
    int32_t ProcessCachedOutputData(void* audioData, uint32_t outputCapacity,
        uint32_t outputBytesPerFrame, ProcessingContext& context);
    int32_t ProcessDataBlocks(ProcessingContext& context, uint32_t inputBytesPerFrame,
        uint32_t outputBytesPerFrame);
    int32_t ProcessConversion(void* audioData, uint32_t outputCapacity,
        uint32_t* outputSize, uint32_t outputBytesPerFrame, uint32_t maxOutputFrames);
    
    // 成员变量
    PcmBufferFormat inputFormat_;
    PcmBufferFormat outputFormat_;
    std::unique_ptr<AudioSuiteFormatConversion> formatConversion_;
    std::shared_ptr<FormatConverterDataCallback> dataCallback_;
    
    std::vector<uint8_t> cachedInputData_;
    std::vector<uint8_t> cachedOutputData_;
    bool inputFinished_ = false;
    std::mutex mutex_;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_FORMAT_CONVERTER_IMPL_H