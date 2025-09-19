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

#ifndef AUDIO_SUITE_INPUT_NODE_H
#define AUDIO_SUITE_INPUT_NODE_H

#include "audio_suite_channel.h"
#include "audio_suite_common.h"

class SuiteInputNodeWriteDataCallBack;
namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

static constexpr uint32_t SECONDS_TO_MS = 1000; // 1秒对应毫秒数

class AudioInputNode : public AudioNode {
public:
    explicit AudioInputNode(AudioFormat format);
    ~AudioInputNode();

    int32_t Connect(const std::shared_ptr<AudioNode>& preNode, AudioNodePortType type) override;
    int32_t DisConnect(const std::shared_ptr<AudioNode>& preNode) override;
    int32_t Init() override;
    int32_t DeInit() override;
    int32_t Flush() override;
    int32_t DoProcess() override;
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> GetOutputPort(AudioNodePortType type) override;
    int32_t SetOnWriteDataCallback(std::shared_ptr<SuiteInputNodeWriteDataCallBack> callback) override;
    bool IsSetReadDataCallback() override;
    void SetAudioNodeFormat(AudioFormat audioFormat) override;
    int32_t InstallTap(AudioNodePortType portType, std::shared_ptr<SuiteNodeReadTapDataCallback> callback) override;
    int32_t RemoveTap(AudioNodePortType portType) override;

private:
    int32_t GetDataFromUser();
    uint32_t GetFrameSize();
    uint32_t GetFrameSize(const AudioFormat& format);
    int32_t GeneratePushBuffer();
    int32_t HandleTapCallback();
    uint32_t GetCacheBufferCapacity(const AudioFormat& format);
    uint32_t GetFrameSizeAfterTransfer(const AudioFormat& format);
    int32_t SetFormatTransfer(AudioSamplingRate sampleRate);
    uint32_t GetUserDataSizeByCacheSize(uint32_t cacheSize);
    int32_t DoResample(uint8_t* inData, uint32_t inSize, AudioSamplingRate inSample,
        float* out, uint32_t outSize, AudioSamplingRate outSample);
    uint32_t GetCacheSizeByUserDataSize(uint32_t userSize);
    uint32_t GetNeedSizeFromUser();
    int32_t DoRequestData(uint8_t* rawData, uint32_t needSize, uint32_t& getSize, bool& isFinished);
    int32_t PushDataToCache(uint8_t* rawData, uint32_t dataSize);
    uint32_t GetNeedMinCacheSize();

    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> outputStream_ = nullptr;
    std::shared_ptr<SuiteInputNodeWriteDataCallBack> writeCallback_ = nullptr;
    AudioSuiteRingBuffer cachedBuffer_; // 数据缓存区
    AudioSuitePcmBuffer* inputNodeBuffer_ = nullptr; // 待返回的数据
    Tap tap_;
    bool needResample_ = false; // 从应用拿到数据后， 是否需要重采样
    bool needTransferBitWidth_ = false; // 往后面结点传数据时，是否需要位深转换
};
}
}
}
#endif