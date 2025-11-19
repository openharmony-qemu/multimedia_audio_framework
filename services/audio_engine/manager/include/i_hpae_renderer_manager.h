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
#ifndef HPAE_I_RENDER_MANAGER_H
#define HPAE_I_RENDER_MANAGER_H
#include "audio_info.h"
#include "audio_errors.h"
#include "i_renderer_stream.h"
#include "i_capturer_stream.h"
#include "hpae_sink_input_node.h"
#include "hpae_stream_manager.h"
#include "hpae_dfx_map_tree.h"
#include "hpae_co_buffer_node.h"
#include "hpae_sink_virtual_output_node.h"
namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class IHpaeRendererManager : public HpaeStreamManager {
public:
    static std::shared_ptr<IHpaeRendererManager> CreateRendererManager(HpaeSinkInfo &sinkInfo);

    virtual ~IHpaeRendererManager()
    {}
    virtual int32_t CreateStream(const HpaeStreamInfo &streamInfo) = 0;
    virtual int32_t DestroyStream(uint32_t sessionId) = 0;
    virtual int32_t Start(uint32_t sessionId) = 0;
    virtual int32_t StartWithSyncId(uint32_t sessionId, int32_t syncId)
    {
        return Start(sessionId);
    }
    virtual int32_t Pause(uint32_t sessionId) = 0;
    virtual int32_t Flush(uint32_t sessionId) = 0;
    virtual int32_t Drain(uint32_t sessionId) = 0;
    virtual int32_t Stop(uint32_t sessionId) = 0;
    virtual int32_t Release(uint32_t sessionId) = 0;
    virtual int32_t SuspendStreamManager(bool isSuspend) = 0;
    virtual void Process() = 0;
    virtual void HandleMsg() = 0;
    virtual int32_t Init(bool isReload = false) = 0;
    virtual int32_t DeInit(bool isMoveDefault = false) = 0;
    virtual bool IsInit() = 0;
    virtual bool IsRunning(void) = 0;
    virtual bool IsMsgProcessing() = 0;
    virtual bool DeactivateThread() = 0;
    virtual int32_t SetClientVolume(uint32_t sessionId, float volume) = 0;
    virtual int32_t SetLoudnessGain(uint32_t sessionId, float loudnessGain) = 0;
    virtual int32_t SetRate(uint32_t sessionId, int32_t rate) = 0;
    virtual int32_t SetAudioEffectMode(uint32_t sessionId, int32_t effectMode) = 0;
    virtual int32_t GetAudioEffectMode(uint32_t sessionId, int32_t &effectMode) = 0;
    virtual int32_t SetPrivacyType(uint32_t sessionId, int32_t privacyType) = 0;
    virtual int32_t GetPrivacyType(uint32_t sessionId, int32_t &privacyType) = 0;
    virtual int32_t RegisterWriteCallback(uint32_t sessionId, const std::weak_ptr<IStreamCallback> &callback) = 0;
    virtual int32_t ReloadRenderManager(const HpaeSinkInfo &sinkInfo, bool isReload = false) = 0;

    virtual int32_t SetOffloadPolicy(uint32_t sessionId, int32_t state)
    {
        return ERR_NOT_SUPPORTED;
    };
    virtual size_t GetWritableSize(uint32_t sessionId) = 0;
    virtual int32_t UpdateSpatializationState(
        uint32_t sessionId, bool spatializationEnabled, bool headTrackingEnabled) = 0;
    virtual int32_t UpdateMaxLength(uint32_t sessionId, uint32_t maxLength) = 0;
    virtual int32_t SetOffloadRenderCallbackType(uint32_t sessionId, int32_t type) { return ERR_NOT_SUPPORTED; };
    virtual void SetSpeed(uint32_t sessionId, float speed) {}
    virtual std::vector<SinkInput> GetAllSinkInputsInfo() = 0;
    virtual int32_t GetSinkInputInfo(uint32_t sessionId, HpaeSinkInputInfo &sinkInputInfo) = 0;
    virtual int32_t RefreshProcessClusterByDevice() = 0;
    virtual HpaeSinkInfo GetSinkInfo() = 0;
    virtual int32_t AddNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node) = 0;
    virtual int32_t AddAllNodesToSink(
        const std::vector<std::shared_ptr<HpaeSinkInputNode>> &sinkInputs, bool isConnect) = 0;
    virtual int32_t RegisterReadCallback(uint32_t sessionId,
        const std::weak_ptr<ICapturerStreamCallback> &callback) = 0;
    virtual int32_t GetSourceOutputInfo(uint32_t sessionId, HpaeSourceOutputInfo &sourceOutputInfo)
    {
        return 0;
    };
    virtual std::vector<SourceOutput> GetAllSourceOutputsInfo()
    {
        return {};
    };
    virtual std::string GetThreadName() = 0;

    virtual int32_t DumpSinkInfo() { return 0; };

    virtual void UploadDumpSinkInfo(std::string& deviceName);

    virtual void OnNotifyDfxNodeAdmin(bool isAdd, const HpaeDfxNodeInfo &nodeInfo);

    virtual void OnNotifyDfxNodeInfo(bool isConnect, uint32_t parentId, uint32_t childId);

    virtual void OnNotifyDfxNodeInfoChanged(uint32_t nodeId, const HpaeDfxNodeInfo &nodeInfo)
    {
#ifdef ENABLE_HIDUMP_DFX
        dfxTree_.UpdateNodeInfo(nodeId, nodeInfo);
#endif
    }
    virtual int32_t UpdateCollaborativeState(bool isCollaborationEnabled) {return 0;};
    virtual int32_t ConnectCoBufferNode(const std::shared_ptr<HpaeCoBufferNode> &coBufferNode) {return 0;};
    virtual int32_t DisConnectCoBufferNode(const std::shared_ptr<HpaeCoBufferNode> &coBufferNode) {return 0;};
    virtual std::string GetDeviceHDFDumpInfo() = 0;
    virtual int32_t SetSinkVirtualOutputNode(const std::shared_ptr<HpaeSinkVirtualOutputNode> &sinkVirtualOutputNode);

private:
#ifdef ENABLE_HIDUMP_DFX
    HpaeDfxMapTree dfxTree_;
#endif
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif