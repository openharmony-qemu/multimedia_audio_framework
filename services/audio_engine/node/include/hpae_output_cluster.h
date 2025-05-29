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

#ifndef HPAE_OUTPUT_CLUSTER_H
#define HPAE_OUTPUT_CLUSTER_H
#include "hpae_mixer_node.h"
#include "hpae_sink_output_node.h"
#include "hpae_audio_format_converter_node.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr uint32_t TIME_OUT_STOP_THD_DEFAULT_FRAME = 150;
constexpr uint32_t FRAME_LEN_MS_DEFAULT_MS = 20;
class HpaeOutputCluster : public InputNode<HpaePcmBuffer *> {
public:
    HpaeOutputCluster(HpaeNodeInfo &nodeInfo);
    virtual ~HpaeOutputCluster();
    virtual void DoProcess() override;
    virtual bool Reset() override;
    virtual bool ResetAll() override;
    void Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode) override;
    void DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode) override;
    virtual int32_t GetConverterNodeCount();
    virtual int32_t GetPreOutNum();
    virtual int32_t GetInstance(std::string deviceClass, std::string deviceNetId);
    virtual int32_t Init(IAudioSinkAttr &attr);
    virtual int32_t DeInit();
    virtual int32_t Flush(void);
    virtual int32_t Pause(void);
    virtual int32_t ResetRender(void);
    virtual int32_t Resume(void);
    virtual int32_t Start(void);
    virtual int32_t Stop(void);
    virtual int32_t SetTimeoutStopThd(uint32_t timeoutThdMs);
    virtual const char *GetFrameData(void);
    virtual StreamManagerState GetState(void);
    virtual bool IsProcessClusterConnected(HpaeProcessorType sceneType);
    virtual int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid);
    virtual int32_t SetPriPaPower(void);
private:
    std::shared_ptr<HpaeMixerNode> mixerNode_ = nullptr;
    std::shared_ptr<HpaeSinkOutputNode> hpaeSinkOutputNode_ = nullptr;
    std::unordered_map<HpaeProcessorType, std::shared_ptr<HpaeAudioFormatConverterNode>> sceneConverterMap_;
    uint32_t timeoutThdFrames_ = TIME_OUT_STOP_THD_DEFAULT_FRAME;
    uint32_t timeoutStopCount_ = 0;
    uint32_t frameLenMs_ = FRAME_LEN_MS_DEFAULT_MS;
    std::set<HpaeProcessorType> connectedProcessCluster_;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif