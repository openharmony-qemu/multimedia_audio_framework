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

#ifndef HPAE_CAPTURE_EFFECT_NODE_H
#define HPAE_CAPTURE_EFFECT_NODE_H
#include <string>
#include "hpae_plugin_node.h"
#include "hpae_node.h"
#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

constexpr uint32_t SCENE_TYPE_OFFSET = 32;
constexpr uint32_t CAPTURER_ID_OFFSET = 16;
constexpr uint32_t BITLENGTH = 8;
constexpr uint32_t FRAME_LEN = 20;

struct CaptureEffectAttr {
    uint32_t micChannels;
    uint32_t ecChannels;
    uint32_t micRefChannels;
};

class HpaeCaptureEffectNode : public HpaePluginNode {
public:
    HpaeCaptureEffectNode(HpaeNodeInfo &nodeInfo);
    virtual ~HpaeCaptureEffectNode();
    virtual bool Reset() override;
    void ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode, HpaeNodeInfo &nodeInfo) override;
    void DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode,
        HpaeNodeInfo &nodeInfo) override;
    bool GetCapturerEffectConfig(HpaeNodeInfo& nodeInfo, HpaeSourceBufferType type = HPAE_SOURCE_BUFFER_TYPE_MIC);
    int32_t CaptureEffectCreate(uint64_t sceneKeyCode, CaptureEffectAttr attr);
    int32_t CaptureEffectRelease(uint64_t sceneKeyCode);
protected:
    HpaePcmBuffer *SignalProcess(const std::vector<HpaePcmBuffer*> &inputs) override;
private:
    void SetCapturerEffectConfig(AudioBufferConfig micConfig, AudioBufferConfig ecConfig,
        AudioBufferConfig micrefConfig);

    uint64_t sceneKeyCode_ = 0;
    std::string sceneType_ = "";
    uint32_t micBufferLength_ = 0;
    uint32_t ecBufferLength_ = 0;
    uint32_t micrefBufferLength_ = 0;
    std::vector<uint8_t> ecCache_;
    std::vector<uint8_t> micCache_;
    std::vector<uint8_t> micRefCache_;
    std::vector<uint8_t> cacheDataOut_;
    std::unique_ptr<HpaePcmBuffer> outPcmBuffer_ { nullptr };
    std::unordered_map<HpaeSourceBufferType, HpaeNodeInfo> capturerEffectConfigMap_;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif // HPAE_CAPTURE_EFFECT_NODE_H