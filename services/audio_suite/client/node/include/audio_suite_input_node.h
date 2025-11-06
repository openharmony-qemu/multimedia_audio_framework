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

#include "audio_suite_common.h"
#include "audio_suite_channel.h"
#include "audio_suite_format_conversion.h"

class InputNodeRequestDataCallBack;
namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class AudioInputNode : public AudioNode {
public:
    explicit AudioInputNode(AudioFormat format);
    ~AudioInputNode();

    int32_t Init() override;
    int32_t DeInit() override;
    int32_t Flush() override;

    int32_t Connect(const std::shared_ptr<AudioNode>& preNode) override;
    int32_t DisConnect(const std::shared_ptr<AudioNode>& preNode) override;
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> GetOutputPort() override;
    int32_t DoProcess() override;
    int32_t SetRequestDataCallback(std::shared_ptr<InputNodeRequestDataCallBack> callback) override;
    bool IsSetReadDataCallback() override;
    void SetAudioNodeFormat(AudioFormat audioFormat) override;

private:
    int32_t GetDataFromUser();
    int32_t GeneratePushBuffer();

    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> outputStream_ = nullptr;
    std::shared_ptr<InputNodeRequestDataCallBack> reqDataCallback_ = nullptr;

    AudioSuitePcmBuffer inPcmData_;
    AudioSuitePcmBuffer outPcmData_;
    AudioSuiteRingBuffer cachedBuffer_;
    uint32_t singleRequestSize_ = 0;
    uint32_t inPcmDataGetSize_ = 0;
    AudioSuiteFormatConversion convert_;
};
}
}
}
#endif