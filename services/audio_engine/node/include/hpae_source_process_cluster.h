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

#ifndef HPAE_SOURCE_PROCESS_CLUSTER_H
#define HPAE_SOURCE_PROCESS_CLUSTER_H
#include "hpae_capture_effect_node.h"
#include "hpae_audio_format_converter_node.h"
#include "hpae_mixer_node.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class HpaeSourceProcessCluster : public OutputNode<HpaePcmBuffer *>, public InputNode<HpaePcmBuffer *> {
public:
    HpaeSourceProcessCluster(HpaeNodeInfo &nodeInfo);
    virtual ~HpaeSourceProcessCluster();
    virtual void DoProcess() override;
    virtual bool Reset() override;
    virtual bool ResetAll() override;
    std::shared_ptr<HpaeNode> GetSharedInstance() override;
    std::shared_ptr<HpaeNode> GetSharedInstance(HpaeNodeInfo &nodeInfo) override;
    OutputPort<HpaePcmBuffer *> *GetOutputPort() override;
    OutputPort<HpaePcmBuffer *> *GetOutputPort(HpaeNodeInfo &nodeInfo, bool isDisConnect = false) override;

    // HpaeNodeInfo& GetOutputNodeInfo() override;
    void Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode) override;
    void DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode) override;
    void ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode, HpaeNodeInfo &nodeInfo) override;
    void DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode,
        HpaeNodeInfo &nodeInfo) override;
    // connect injector
    void ConnectInjector(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode);
    void DisConnectInjector(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode);

    bool GetCapturerEffectConfig(HpaeNodeInfo& nodeInfo, HpaeSourceBufferType type = HPAE_SOURCE_BUFFER_TYPE_MIC);

    size_t GetOutputPortNum();
    int32_t CaptureEffectCreate(uint64_t sceneKeyCode, CaptureEffectAttr attr);
    int32_t CaptureEffectRelease(uint64_t sceneKeyCode);
    bool IsEffectNodeValid();

    // for ut test
    uint32_t GetMixerNodeUseCount();
    uint32_t GetCapturerEffectNodeUseCount();
    uint32_t GetConverterNodeCount();
    size_t GetPreOutNum();
private:
    std::shared_ptr<HpaeCaptureEffectNode> captureEffectNode_;
    std::shared_ptr<HpaeMixerNode> mixerNode_;
    std::unordered_map<std::string, std::shared_ptr<HpaeAudioFormatConverterNode>> fmtConverterNodeMap_;
    std::unordered_map<std::shared_ptr<OutputNode<HpaePcmBuffer*>>,
        std::shared_ptr<HpaeAudioFormatConverterNode>> injectorFmtConverterNodeMap_;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif