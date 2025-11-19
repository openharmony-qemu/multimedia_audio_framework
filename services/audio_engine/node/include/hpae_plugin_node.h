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
#ifndef HPAE_PLUGIN_NODE_H
#define HPAE_PLUGIN_NODE_H
#include <memory>
#include "hpae_node.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class HpaePluginNode : public OutputNode<HpaePcmBuffer *>, public InputNode<HpaePcmBuffer *> {
public:
    HpaePluginNode(HpaeNodeInfo& nodeInfo);
    virtual ~HpaePluginNode() {};
    virtual void DoProcess() override;
    virtual bool Reset() override;
    virtual bool ResetAll() override;
    
    std::shared_ptr<HpaeNode> GetSharedInstance() override;
    OutputPort<HpaePcmBuffer*>* GetOutputPort() override;
    std::shared_ptr<HpaeNode> GetSharedInstance(HpaeNodeInfo &nodeInfo) override;
    OutputPort<HpaePcmBuffer *> *GetOutputPort(HpaeNodeInfo &nodeInfo, bool isDisConnect = false) override;
    void Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode) override;
    void DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode) override;
    virtual size_t GetPreOutNum();
    virtual size_t GetOutputPortNum();
    virtual int32_t EnableProcess(bool enable);
    virtual bool IsEnableProcess();
    HpaePluginNode(const HpaePluginNode& others) = delete;
    void SetSourceNode(bool isSourceNode);
    virtual uint64_t GetLatency(uint32_t sessionId = 0) = 0;
private:
    PcmBufferInfo pcmBufferInfo_;
protected:
    virtual HpaePcmBuffer* SignalProcess(const std::vector<HpaePcmBuffer*>& inputs) = 0;
    OutputPort<HpaePcmBuffer *> outputStream_;
    InputPort<HpaePcmBuffer*> inputStream_;
    bool enableProcess_;
    HpaePcmBuffer silenceData_;
    bool isSourceNode_ = false;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif