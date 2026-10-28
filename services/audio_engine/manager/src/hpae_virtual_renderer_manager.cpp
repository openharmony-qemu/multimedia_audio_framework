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

#ifndef LOG_TAG
#define LOG_TAG "HpaeVirtualRendererManager"
#endif

#include "hpae_virtual_renderer_manager.h"
#include "audio_errors.h"
#include "audio_engine_log.h"
#include "audio_log.h"
#include "audio_utils.h"
#include "hpae_node_common.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
static const std::string VIRTUAL_RENDER_SINK_NAME = "Virtual_Render";

HpaeVirtualRendererManager::HpaeVirtualRendererManager() : hpaeNoLockQueue_(CURRENT_REQUEST_COUNT)
{
    sinkInfo_.deviceName = VIRTUAL_RENDER_SINK_NAME;
    sinkInfo_.deviceClass = VIRTUAL_RENDER_SINK_NAME;
    sinkInfo_.adapterName = VIRTUAL_RENDER_SINK_NAME;
}

HpaeVirtualRendererManager::~HpaeVirtualRendererManager()
{
    if (isInit_.load()) {
        DeInit();
    }
}

void HpaeVirtualRendererManager::SendRequest(Request &&request, const std::string &funcName, bool isInit)
{
    (void)funcName;
    (void)isInit;
    CHECK_AND_RETURN_LOG(hpaeSignalProcessThread_ != nullptr, "hpaeSignalProcessThread_ is nullptr");
    hpaeNoLockQueue_.PushRequest(std::move(request));
    hpaeSignalProcessThread_->Notify();
}

void HpaeVirtualRendererManager::HandleMsg()
{
    hpaeNoLockQueue_.HandleRequests();
}

int32_t HpaeVirtualRendererManager::Init(bool isReload)
{
    (void)isReload;
    CHECK_AND_RETURN_RET_LOG(!IsInit(), SUCCESS, "already inited");
    hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    isInit_.store(true);
    return SUCCESS;
}

// Virtual sink is itself the move-fallback target, so isMoveDefault is ignored: just tear down.
int32_t HpaeVirtualRendererManager::DeInit(bool isMoveDefault)
{
    (void)isMoveDefault;
    if (hpaeSignalProcessThread_ != nullptr) {
        hpaeSignalProcessThread_->DeactivateThread();
        hpaeSignalProcessThread_ = nullptr;
    }
    hpaeNoLockQueue_.HandleRequests();
    sinkInputNodeMap_.clear();
    sessionNodeMap_.clear();
    isInit_.store(false);
    return SUCCESS;
}

bool HpaeVirtualRendererManager::IsInit()
{
    return isInit_.load();
}

bool HpaeVirtualRendererManager::IsRunning(void)
{
    return hpaeSignalProcessThread_ != nullptr && hpaeSignalProcessThread_->IsRunning() &&
        HasRunningSession();
}

bool HpaeVirtualRendererManager::IsMsgProcessing()
{
    return !hpaeNoLockQueue_.IsFinishProcess();
}

bool HpaeVirtualRendererManager::DeactivateThread()
{
    if (hpaeSignalProcessThread_ != nullptr) {
        hpaeSignalProcessThread_->DeactivateThread();
        hpaeSignalProcessThread_ = nullptr;
    }
    hpaeNoLockQueue_.HandleRequests();
    return true;
}

int32_t HpaeVirtualRendererManager::StopManager()
{
    return DeactivateThread() ? SUCCESS : ERROR;
}

std::string HpaeVirtualRendererManager::GetThreadName()
{
    return VIRTUAL_RENDER_SINK_NAME;
}

void HpaeVirtualRendererManager::Process()
{
    // replaced in Task 4
}

bool HpaeVirtualRendererManager::HasRunningSession()
{
    for (const auto &it : sessionNodeMap_) {
        if (it.second.state == HPAE_SESSION_RUNNING) {
            return true;
        }
    }
    return false;
}

HpaeSinkInfo HpaeVirtualRendererManager::GetSinkInfo()
{
    return sinkInfo_;
}

int32_t HpaeVirtualRendererManager::GetDeviceHDFDumpInfo()
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::SetMute(bool isMute)
{
    (void)isMute;
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::SuspendStreamManager(bool isSuspend)
{
    (void)isSuspend;
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::SetClientVolume(uint32_t sessionId, float volume)
{
    (void)sessionId;
    (void)volume;
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::SetLoudnessGain(uint32_t sessionId, float loudnessGain)
{
    (void)sessionId;
    (void)loudnessGain;
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::SetRate(uint32_t sessionId, int32_t rate)
{
    (void)sessionId;
    (void)rate;
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::SetAudioEffectMode(uint32_t sessionId, int32_t effectMode)
{
    (void)sessionId;
    (void)effectMode;
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::GetAudioEffectMode(uint32_t sessionId, int32_t &effectMode)
{
    (void)sessionId;
    (void)effectMode;
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::SetPrivacyType(uint32_t sessionId, int32_t privacyType)
{
    (void)sessionId;
    (void)privacyType;
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::GetPrivacyType(uint32_t sessionId, int32_t &privacyType)
{
    (void)sessionId;
    (void)privacyType;
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::RegisterReadCallback(uint32_t sessionId,
    const std::weak_ptr<ICapturerStreamCallback> &callback)
{
    (void)sessionId;
    (void)callback;
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::ReloadRenderManager(const HpaeSinkInfo &sinkInfo, bool isReload)
{
    (void)sinkInfo;
    (void)isReload;
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

size_t HpaeVirtualRendererManager::GetWritableSize(uint32_t sessionId)
{
    (void)sessionId;
    AUDIO_ERR_LOG("Unsupported operation");
    return 0;
}

int32_t HpaeVirtualRendererManager::UpdateSpatializationState(uint32_t sessionId,
    bool spatializationEnabled, bool headTrackingEnabled)
{
    (void)sessionId;
    (void)spatializationEnabled;
    (void)headTrackingEnabled;
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::UpdateMaxLength(uint32_t sessionId, uint32_t maxLength)
{
    (void)sessionId;
    (void)maxLength;
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::RefreshProcessClusterByDevice()
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

std::vector<SinkInput> HpaeVirtualRendererManager::GetAllSinkInputsInfo()
{
    AUDIO_ERR_LOG("Unsupported operation");
    return {};
}

// --- stubs replaced by later tasks ---
int32_t HpaeVirtualRendererManager::CreateStream(const HpaeStreamInfo &streamInfo)
{
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::DestroyStream(uint32_t sessionId)
{
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::Start(uint32_t sessionId)
{
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::Pause(uint32_t sessionId, bool isStandby)
{
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::Flush(uint32_t sessionId)
{
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::Drain(uint32_t sessionId)
{
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::Stop(uint32_t sessionId)
{
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::Release(uint32_t sessionId)
{
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::MoveStream(uint32_t sessionId, const std::string &sinkName)
{
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::MoveAllStream(const std::string &sinkName,
    const std::vector<uint32_t> &sessionIds, MoveSessionType moveType)
{
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::RegisterWriteCallback(uint32_t sessionId,
    const std::weak_ptr<IStreamCallback> &callback)
{
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::GetSinkInputInfo(uint32_t sessionId, HpaeSinkInputInfo &sinkInputInfo)
{
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::AddNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node)
{
    return SUCCESS;
}

int32_t HpaeVirtualRendererManager::AddAllNodesToSink(
    const std::vector<std::shared_ptr<HpaeSinkInputNode>> &sinkInputs, bool isConnect)
{
    return SUCCESS;
}

void HpaeVirtualRendererManager::OnNodeStatusUpdate(uint32_t sessionId, IOperation operation) {}

void HpaeVirtualRendererManager::OnNotifyQueue() {}

void HpaeVirtualRendererManager::CreateInputSession(const HpaeStreamInfo &streamInfo) {}

void HpaeVirtualRendererManager::DeleteInputSession(uint32_t sessionId) {}

void HpaeVirtualRendererManager::SetSessionState(uint32_t sessionId, HpaeSessionState state) {}

void HpaeVirtualRendererManager::MoveStreamSync(uint32_t sessionId, const std::string &sinkName) {}

void HpaeVirtualRendererManager::MoveAllStreamToNewSink(const std::string &sinkName,
    const std::vector<uint32_t> &moveIds, MoveSessionType moveType) {}

void HpaeVirtualRendererManager::AddSingleNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node,
    bool isConnect) {}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
