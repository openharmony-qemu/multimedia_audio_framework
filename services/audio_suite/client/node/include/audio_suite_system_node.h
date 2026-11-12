/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#ifndef AUDIO_SUITE_SYSTEM_NODE_H
#define AUDIO_SUITE_SYSTEM_NODE_H

#include <memory>
#include <mutex>
#include "audio_suite_process_node.h"
#include "audio_info.h"
#include "offline_audio_effect_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class AudioSuiteSystemNode : public AudioSuiteProcessNode {
public:
    AudioSuiteSystemNode(AudioNodeType nodeType);
    ~AudioSuiteSystemNode() override;
    int32_t Init() override;
    int32_t DeInit() override;
    int32_t SetSystemParam(std::vector<uint8_t> &param) override;
    int32_t GetSystemParam(std::vector<uint8_t> &param) override;
    int32_t GetInOutSize(uint32_t &inSize, uint32_t &outSize) override;
    int32_t PrintNodeInfo(std::string &nodeInfo) override;
    int32_t Connect(const std::shared_ptr<AudioNode>& preNode) override;
    using AudioNode::DoProcess;
    int32_t DoProcess(AudioMetaFrame &metaFrame,
        int32_t *responseAudioSize, int32_t *responseMetaSize, bool *finishedFlag) override;
    void UpdateOutputFormat(AudioFormat audioFormat) override;
    void UpdateInputFormat(AudioFormat audioFormat) override;
    void Prepare() override;

protected:
    std::vector<AudioSuitePcmBuffer *> SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs) override;
    void SetInputFormat(AudioFormat format);
    void SetOutputFormat(AudioFormat format);
    void TryConfigure();

    virtual std::string GetEffectChainName() const = 0;
    int32_t ConfigureEffectChain();
    int32_t PrepareEffectChain();

    std::unique_ptr<OfflineAudioEffectChain> effectChain_;
    uint32_t inBufferSize_ = 0;
    uint32_t outBufferSize_ = 0;
    uint32_t outAudioDataSize_ = 0;
    bool hasInputFormat_ = false;
    bool hasOutputFormat_ = false;
    bool isConnected_ = false;
    AudioFormat inputFormat_;
    AudioFormat outputFormat_;

private:
    int32_t ValidateDoProcess(AudioMetaFrame &metaFrame,
        int32_t *responseAudioSize, int32_t *responseMetaSize, bool *finishedFlag);
    int32_t PullUpstreamData(int32_t &responseInAudioSize, int32_t &responseInMetaSize, bool *finishedFlag);
    int32_t PackInputBuffer(int32_t audioSize, int32_t metaSize, int32_t &inputDataSize);
    int32_t UnpackOutputBuffer(AudioMetaFrame &metaFrame, int32_t *responseAudioSize, int32_t *responseMetaSize);

    // for effect process
    std::vector<uint8_t> inMetaData_;
    std::vector<uint8_t> inAudioData_;
    std::vector<uint8_t> inData_;
    std::vector<uint8_t> outData_;
    AudioMetaFrame metaFrame_;
};

}
}
}
#endif
