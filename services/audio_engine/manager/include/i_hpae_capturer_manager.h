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
#include "hpae_dfx_map_tree.h"

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
    virtual int32_t SetStreamMute(uint32_t sessionId, bool isMute) = 0;
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
    virtual int32_t ReloadCaptureManager(const HpaeSourceInfo &sourceInfo, bool isReload = false) = 0;
    virtual int32_t DumpSourceInfo() { return 0; };
    virtual void UploadDumpSourceInfo(std::string &deviceName);
    virtual void OnNotifyDfxNodeAdmin(bool isAdd, const HpaeDfxNodeInfo &nodeInfo);
    virtual void OnNotifyDfxNodeInfo(bool isConnect, uint32_t parentId, uint32_t childId);
    virtual std::string GetDeviceHDFDumpInfo() = 0;
    virtual int32_t AddCaptureInjector(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &sinkOutputNode,
        const SourceType &sourceType) = 0;
    virtual int32_t RemoveCaptureInjector(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &sinkOutputNode,
        const SourceType &sourceType) = 0;
private:
#ifdef ENABLE_HIDUMP_DFX
    HpaeDfxMapTree dfxTree_;
#endif
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif