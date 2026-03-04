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

#ifndef HPAE_SOURCE_OUTPUT_NODE_H
#define HPAE_SOURCE_OUTPUT_NODE_H
#include <memory>
#include "hpae_node.h"
#include "hpae_pcm_buffer.h"
#include "audio_info.h"
#include "i_capturer_stream.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class HpaeSourceOutputNode : public InputNode<HpaePcmBuffer *> {
public:
    HpaeSourceOutputNode(HpaeNodeInfo &nodeInfo);
    virtual ~HpaeSourceOutputNode();
    virtual void DoProcess() final;
    virtual bool Reset() final;
    bool ResetAll() final;
    void Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode) override;
    void ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode, HpaeNodeInfo &nodeInfo) override;
    void DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode) override;
    void DisConnectWithInfo(
        const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode, HpaeNodeInfo &nodeInfo) override;
    bool RegisterReadCallback(const std::weak_ptr<ICapturerStreamCallback> &callback);
    int32_t SetState(HpaeSessionState captureState);
    HpaeSessionState GetState();
    void SetAppUid(int32_t appUid);
    int32_t GetAppUid();
    void SetMute(bool isMute);
private:
    uint64_t GetTimestamp();
private:
    InputPort<HpaePcmBuffer *> inputStream_;
    std::weak_ptr<ICapturerStreamCallback> readCallback_;
    AudioCallBackCapturerStreamInfo streamInfo_;
    std::vector<char> sourceOutputData_;
    std::vector<float> interleveData_;
    std::atomic<uint64_t> framesRead_;
    HpaeSessionState state_ = HPAE_SESSION_NEW;
    uint64_t totalFrames_;
    int32_t appUid_ = -1;
    bool isMute_;
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif