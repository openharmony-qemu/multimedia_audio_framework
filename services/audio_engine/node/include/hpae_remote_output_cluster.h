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

#ifndef HPAE_REMOTE_OUTPUT_CLUSTER_H
#define HPAE_REMOTE_OUTPUT_CLUSTER_H
#include "i_hpae_output_cluster.h"
#include "hpae_remote_sink_output_node.h"
#include "hpae_audio_format_converter_node.h"
#include "hpae_mixer_node.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

HpaeProcessorType TransStreamUsageToSplitSceneType(StreamUsage streamUsage, const std::string &splitMode);

class HpaeRemoteOutputCluster : public IHpaeOutputCluster {
public:
    HpaeRemoteOutputCluster(HpaeNodeInfo &nodeInfo, HpaeSinkInfo &sinkInfo);
    virtual ~HpaeRemoteOutputCluster();
    void DoProcess() override;
    bool Reset() override;
    bool ResetAll() override;
    void Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode) override;
    void DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode) override;
    int32_t GetConverterNodeCount() override;
    int32_t GetPreOutNum() override;
    int32_t GetInstance(const std::string &deviceClass, const std::string &deviceNetId) override;
    int32_t Init(IAudioSinkAttr &attr) override;
    int32_t DeInit() override;
    int32_t Flush(void) override;
    int32_t Pause(void) override;
    int32_t ResetRender(void) override;
    int32_t Resume(void) override;
    int32_t Start(void) override;
    int32_t Stop(void) override;
    int32_t SetTimeoutStopThd(uint32_t timeoutThdMs) override;
    const char *GetFrameData(void) override;
    StreamManagerState GetState(void) override;
    bool IsProcessClusterConnected(HpaeProcessorType sceneType) override;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) override;
    void UpdateStreamInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> preNode) override;
    uint64_t GetLatency(HpaeProcessorType sceneType) override;
private:
    std::shared_ptr<HpaeRemoteSinkOutputNode> hpaeSinkOutputNode_ = nullptr;
    std::unordered_map<HpaeProcessorType, std::shared_ptr<HpaeAudioFormatConverterNode>> sceneConverterMap_;
    std::unordered_map<HpaeProcessorType, std::shared_ptr<HpaeMixerNode>> sceneMixerMap_;
    uint32_t stopCount_ = 0;
    uint32_t timeoutThdFrames_ = TIME_OUT_STOP_THD_DEFAULT_FRAME;
    uint32_t frameLenMs_ = FRAME_LEN_MS_DEFAULT_MS;
    std::set<HpaeProcessorType> connectedProcessCluster_;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif
