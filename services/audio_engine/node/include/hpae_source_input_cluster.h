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

#ifndef HPAE_SOURCE_INPUT_CLUSTER_H
#define HPAE_SOURCE_INPUT_CLUSTER_H

#include "hpae_source_input_node.h"
#include "hpae_audio_format_converter_node.h"
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class HpaeSourceInputCluster : public OutputNode<HpaePcmBuffer *> {
public:
    HpaeSourceInputCluster(HpaeNodeInfo &nodeInfo);
    HpaeSourceInputCluster(std::vector<HpaeNodeInfo> &nodeInfos);
    virtual ~HpaeSourceInputCluster();
    virtual void DoProcess() final;
    virtual bool Reset() final;
    virtual bool ResetAll() final;
    std::shared_ptr<HpaeNode> GetSharedInstance() final;
    std::shared_ptr<HpaeNode> GetSharedInstance(HpaeNodeInfo &nodeInfo) final;
    OutputPort<HpaePcmBuffer*> *GetOutputPort() final;
    OutputPort<HpaePcmBuffer*> *GetOutputPort(HpaeNodeInfo &nodeInfo, bool isDisConnect = false) final;
    int32_t GetCapturerSourceInstance(const std::string &deviceClass, const std::string &deviceNetId,
        const SourceType &sourceType, const std::string &sourceName);
    int32_t CapturerSourceInit(IAudioSourceAttr &attr);
    int32_t CapturerSourceDeInit();
    int32_t CapturerSourceFlush(void);
    int32_t CapturerSourcePause(void);
    int32_t CapturerSourceReset(void);
    int32_t CapturerSourceResume(void);
    int32_t CapturerSourceStart(void);
    int32_t CapturerSourceStop(void);
    StreamManagerState GetSourceState(void);
    size_t GetOutputPortNum();
    size_t GetOutputPortNum(HpaeNodeInfo &nodeInfo);
    HpaeSourceInputNodeType GetSourceInputNodeType();
    void SetSourceInputNodeType(HpaeSourceInputNodeType type);
    void UpdateAppsUidAndSessionId(std::vector<int32_t> &appsUid, std::vector<int32_t> &sessionsId);
    uint32_t GetCaptureId();

    // for test
    uint32_t GetConverterNodeCount();
    uint32_t GetSourceInputNodeUseCount();
    int32_t WriteCapturerData(char *data, int32_t dataSize);
private:
    HpaeNodeInfo &GetNodeInfoWithInfo(HpaeSourceBufferType &type);

    std::shared_ptr<HpaeSourceInputNode> sourceInputNode_;
    std::unordered_map<std::string, std::shared_ptr<HpaeAudioFormatConverterNode>> fmtConverterNodeMap_;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif