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

#ifndef HPAE_I_CAPTURER_MANAGER_H
#define HPAE_I_CAPTURER_MANAGER_H
#include "audio_info.h"
#include "i_capturer_stream.h"
#include "hpae_stream_manager.h"
#include "hpae_capture_move_info.h"
#include "audio_engine_log.h"
#include "hpae_dfx_tree.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class IHpaeCapturerManager : public HpaeStreamManager {
public:
    virtual ~IHpaeCapturerManager() {}
    virtual int32_t CreateStream(const HpaeStreamInfo& streamInfo) = 0;
    virtual int32_t DestroyStream(uint32_t sessionId) = 0;

    virtual int32_t Start(uint32_t sessionId) = 0;
    virtual int32_t Pause(uint32_t sessionId) = 0;
    virtual int32_t Flush(uint32_t sessionId) = 0;
    virtual int32_t Drain(uint32_t sessionId) = 0;
    virtual int32_t Stop(uint32_t sessionId) = 0;
    virtual int32_t Release(uint32_t sessionId) = 0;
    virtual void Process() = 0;
    virtual void HandleMsg() = 0;
    virtual int32_t Init(bool isReload = false) = 0;
    virtual int32_t DeInit(bool isMoveDefault = false) = 0;
    virtual bool IsInit() = 0;
    virtual bool IsRunning(void) = 0;
    virtual bool IsMsgProcessing() = 0;
    virtual bool DeactivateThread() = 0;
    
    virtual int32_t RegisterReadCallback(uint32_t sessionId,
        const std::weak_ptr<ICapturerStreamCallback> &callback) = 0;
    virtual int32_t GetSourceOutputInfo(uint32_t sessionId, HpaeSourceOutputInfo &sourceOutputInfo) = 0;
    virtual HpaeSourceInfo GetSourceInfo() = 0;
    virtual std::vector<SourceOutput> GetAllSourceOutputsInfo() = 0;
    virtual int32_t AddNodeToSource(const HpaeCaptureMoveInfo &moveInfo) = 0;
    virtual int32_t AddAllNodesToSource(const std::vector<HpaeCaptureMoveInfo> &moveInfos, bool isConnect) = 0;
    virtual std::string GetThreadName() = 0;
    virtual int32_t ReloadCaptureManager(const HpaeSourceInfo &sourceInfo) = 0;
    virtual int32_t DumpSourceInfo() { return 0; };
    virtual void UploadDumpSourceInfo(std::string &deviceName)
    {
#ifdef ENABLE_HIDUMP_DFX
        std::string dumpStr;
        dfxTree_.PrintTree(dumpStr);
        TriggerCallback(DUMP_SOURCE_INFO, deviceName, dumpStr);
#endif
    };
    virtual void OnNotifyDfxNodeInfo(bool isConnect, uint32_t preNodeId, HpaeDfxNodeInfo &nodeInfo)
    {
#ifdef ENABLE_HIDUMP_DFX
        AUDIO_INFO_LOG("%{public}s preNodeId %{public}u nodeName:%{public}s, NodeId: %{public}u",
            isConnect ? "connect" : "disconnect",
            preNodeId,
            nodeInfo.nodeName.c_str(),
            nodeInfo.nodeId);
        if (isConnect) {
            dfxTree_.Insert(preNodeId, nodeInfo);
        } else {
            dfxTree_.Remove(nodeInfo.nodeId);
        }
#endif
    };

    virtual uint32_t OnGetNodeId()
    {
        if (nodeIdCounter_.load() == std::numeric_limits<uint32_t>::max()) {
            nodeIdCounter_.store(MIN_START_NODE_ID);
        } else {
            nodeIdCounter_.fetch_add(1);
        }
        return nodeIdCounter_.load();
    };
    virtual std::string GetDeviceHDFDumpInfo() = 0;
private:
    std::atomic<uint32_t> nodeIdCounter_ = 0;
#ifdef ENABLE_HIDUMP_DFX
    HpaeDfxTree dfxTree_;
#endif
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif