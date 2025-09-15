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
#define LOG_TAG "HpaeSinkOutputNode"
#endif

#include <hpae_sink_output_node.h>
#include "audio_errors.h"
#include <iostream>
#include "hpae_format_convert.h"
#include "audio_utils.h"
#include "hpae_node_common.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
constexpr uint32_t SLEEP_TIME_IN_US = 20000;
static constexpr int64_t WAIT_CLOSE_PA_TIME = 4; // 4s
static constexpr int64_t MONITOR_CLOSE_PA_TIME = 5 * 60; // 5m
static constexpr int64_t TIME_IN_US = 1000000;
static constexpr uint64_t MS_PER_FRAME = 20; // 20ms
}

HpaeSinkOutputNode::HpaeSinkOutputNode(HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo),
      renderFrameData_(nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format)),
      interleveData_(nodeInfo.frameLen * nodeInfo.channels)
{
    AUDIO_INFO_LOG("name is %{public}s", sinkOutAttr_.adapterName.c_str());
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("hpaeSinkOutputNode");
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeInfo(true, 0, GetNodeInfo());
    }
#endif
}

HpaeSinkOutputNode::~HpaeSinkOutputNode()
{
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
#endif
}

void HpaeSinkOutputNode::HandleRemoteTiming()
{
    CHECK_AND_RETURN(GetDeviceClass() == "remote");
    auto now = std::chrono::high_resolution_clock::now();
    remoteTimePoint_ += std::chrono::milliseconds(20);  // 20ms frameLen, need optimize
    if (remoteTimePoint_ > now) {
        remoteSleepTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(remoteTimePoint_ - now);
    } else {
        remoteSleepTime_ = std::chrono::milliseconds(0);
    }
    std::this_thread::sleep_for(remoteSleepTime_);
    AUDIO_DEBUG_LOG("remoteSleepTime_ %{public}lld", remoteSleepTime_.count());
}

void HpaeSinkOutputNode::DoProcess()
{
    Trace trace("HpaeSinkOutputNode::DoProcess " + GetTraceInfo());
    if (audioRendererSink_ == nullptr) {
        AUDIO_WARNING_LOG("audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());
        return;
    }
    
    std::vector<HpaePcmBuffer *> &outputVec = inputStream_.ReadPreOutputData();
    CHECK_AND_RETURN(!outputVec.empty());
    HpaePcmBuffer *outputData = outputVec.front();
    HandlePaPower(outputData);
    ConvertFromFloat(
        GetBitWidth(), GetChannelCount() * GetFrameLen(), outputData->GetPcmDataBuffer(), renderFrameData_.data());
    uint64_t writeLen = 0;
    char *renderFrameData = (char *)renderFrameData_.data();

#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer timer;
    timer.Start();
    intervalTimer_.Stop();
#endif
    HandleHapticParam(renderFrameTimes_);
    renderFrameTimes_ += MS_PER_FRAME;
    auto ret = audioRendererSink_->RenderFrame(*renderFrameData, renderFrameData_.size(), writeLen);
    if (ret != SUCCESS || writeLen != renderFrameData_.size()) {
        AUDIO_ERR_LOG("RenderFrame failed");
        if (GetDeviceClass() != "remote") {
            periodTimer_.Stop();
            uint64_t usedTimeUs = periodTimer_.Elapsed<std::chrono::microseconds>();
            usleep(SLEEP_TIME_IN_US > usedTimeUs ? SLEEP_TIME_IN_US - usedTimeUs : 0);
        }
    }
    periodTimer_.Start();
    HandleRemoteTiming(); // used to control remote RenderFrame tempo.
#ifdef ENABLE_HOOK_PCM
    timer.Stop();
    int64_t elapsed = timer.Elapsed();
    AUDIO_DEBUG_LOG("name %{public}s, RenderFrame elapsed time: %{public}" PRId64 " ms",
        sinkOutAttr_.adapterName.c_str(),
        elapsed);
    intervalTimer_.Start();
#endif
    return;
}

const char *HpaeSinkOutputNode::GetRenderFrameData(void)
{
    return renderFrameData_.data();
}

bool HpaeSinkOutputNode::Reset()
{
    const auto preOutputMap = inputStream_.GetPreOutputMap();
    for (const auto &preOutput : preOutputMap) {
        OutputPort<HpaePcmBuffer *> *output = preOutput.first;
        inputStream_.DisConnect(output);
    }
    return true;
}

bool HpaeSinkOutputNode::ResetAll()
{
    const auto preOutputMap = inputStream_.GetPreOutputMap();
    for (const auto &preOutput : preOutputMap) {
        OutputPort<HpaePcmBuffer *> *output = preOutput.first;
        std::shared_ptr<HpaeNode> hpaeNode = preOutput.second;
        if (hpaeNode->ResetAll()) {
            inputStream_.DisConnect(output);
        }
    }
    return true;
}

void HpaeSinkOutputNode::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort());
#ifdef ENABLE_HIDUMP_DFX
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeInfo(true, GetNodeId(), preNode->GetSharedInstance()->GetNodeInfo());
    }
#endif
}

void HpaeSinkOutputNode::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    inputStream_.DisConnect(preNode->GetOutputPort());
#ifdef ENABLE_HIDUMP_DFX
    if (auto callback = GetNodeStatusCallback().lock()) {
        auto preNodeReal = preNode->GetSharedInstance();
        callback->OnNotifyDfxNodeInfo(false, preNodeReal->GetNodeId(), preNodeReal->GetNodeInfo());
    }
#endif
}

int32_t HpaeSinkOutputNode::GetRenderSinkInstance(const std::string &deviceClass, const std::string &deviceNetId)
{
    if (deviceNetId.empty()) {
        renderId_ = HdiAdapterManager::GetInstance().GetRenderIdByDeviceClass(deviceClass, HDI_ID_INFO_DEFAULT, true);
    } else {
        renderId_ = HdiAdapterManager::GetInstance().GetRenderIdByDeviceClass(deviceClass, deviceNetId, true);
    }
    audioRendererSink_ = HdiAdapterManager::GetInstance().GetRenderSink(renderId_, true);
    if (audioRendererSink_ == nullptr) {
        AUDIO_ERR_LOG("get sink fail, deviceClass: %{public}s, deviceNetId: %{public}s, renderId_: %{public}u",
            deviceClass.c_str(),
            deviceNetId.c_str(),
            renderId_);
        HdiAdapterManager::GetInstance().ReleaseId(renderId_);
        return ERROR;
    }
    return SUCCESS;
}

int32_t HpaeSinkOutputNode::RenderSinkInit(IAudioSinkAttr &attr)
{
    CHECK_AND_RETURN_RET(audioRendererSink_ != nullptr, ERROR);

    sinkOutAttr_ = attr;
    if (audioRendererSink_->IsInited()) {
        AUDIO_WARNING_LOG("audioRenderSink already inited");
        SetSinkState(STREAM_MANAGER_IDLE);
        return SUCCESS;
    }
#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer timer;
    timer.Start();
#endif
    int32_t ret = audioRendererSink_->Init(attr);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "audioRendererSink_ init failed, errCode is %{public}d", ret);
    SetSinkState(STREAM_MANAGER_IDLE);
#ifdef ENABLE_HOOK_PCM
    timer.Stop();
    int64_t interval = timer.Elapsed();
    AUDIO_INFO_LOG("name %{public}s, RenderSinkInit Elapsed: %{public}" PRId64 " ms ret: %{public}d",
        sinkOutAttr_.adapterName.c_str(), interval, ret);
    std::string adapterName = sinkOutAttr_.adapterName;
#endif
    return ret;
}

int32_t HpaeSinkOutputNode::RenderSinkDeInit(void)
{
    CHECK_AND_RETURN_RET(audioRendererSink_ != nullptr, ERROR);
    SetSinkState(STREAM_MANAGER_RELEASED);
#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer timer;
    timer.Start();
#endif
    audioRendererSink_->DeInit();
    audioRendererSink_ = nullptr;
    HdiAdapterManager::GetInstance().ReleaseId(renderId_);
#ifdef ENABLE_HOOK_PCM
    timer.Stop();
    int64_t interval = timer.Elapsed();
    AUDIO_INFO_LOG("name %{public}s, RenderSinkDeInit Elapsed: %{public}" PRId64 " ms",
        sinkOutAttr_.adapterName.c_str(),
        interval);
#endif
    return SUCCESS;
}

int32_t HpaeSinkOutputNode::RenderSinkFlush(void)
{
    CHECK_AND_RETURN_RET(audioRendererSink_ != nullptr, ERROR);
    return audioRendererSink_->Flush();
}

int32_t HpaeSinkOutputNode::RenderSinkPause(void)
{
    CHECK_AND_RETURN_RET(audioRendererSink_ != nullptr, ERROR);
    audioRendererSink_->Pause();
    SetSinkState(STREAM_MANAGER_SUSPENDED);
    return SUCCESS;
}

int32_t HpaeSinkOutputNode::RenderSinkReset(void)
{
    CHECK_AND_RETURN_RET(audioRendererSink_ != nullptr, ERROR);
    return audioRendererSink_->Reset();
}

int32_t HpaeSinkOutputNode::RenderSinkResume(void)
{
    CHECK_AND_RETURN_RET(audioRendererSink_ != nullptr, ERROR);
    int32_t ret = audioRendererSink_->Resume();
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    SetSinkState(STREAM_MANAGER_RUNNING);
    return SUCCESS;
}

int32_t HpaeSinkOutputNode::RenderSinkStart(void)
{
    CHECK_AND_RETURN_RET(audioRendererSink_ != nullptr, ERROR);
    renderFrameTimes_ = 0;
    int32_t ret;
#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer timer;
    timer.Start();
#endif
    ret = audioRendererSink_->Start();
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
#ifdef ENABLE_HOOK_PCM
    timer.Stop();
    int64_t interval = timer.Elapsed();
    AUDIO_INFO_LOG("name %{public}s, RenderSinkStart Elapsed: %{public}" PRId64 " ms",
        sinkOutAttr_.adapterName.c_str(),
        interval);
#endif
    SetSinkState(STREAM_MANAGER_RUNNING);
    if (GetDeviceClass() == "remote") {
        remoteTimePoint_ = std::chrono::high_resolution_clock::now();
    }
    if (GetDeviceClass() == "primary") {
        ret = audioRendererSink_->SetPaPower(true);
        isOpenPaPower_ = true;
        isDisplayPaPowerState_ = false;
        silenceDataUs_ = 0;
        AUDIO_INFO_LOG("Speaker sink started, open pa:[%{public}s] -- [%{public}s], ret:%{public}d",
            GetDeviceClass().c_str(), (ret == 0 ? "success" : "failed"), ret);
    }
    periodTimer_.Start();
    return SUCCESS;
}

int32_t HpaeSinkOutputNode::RenderSinkStop(void)
{
    CHECK_AND_RETURN_RET(audioRendererSink_ != nullptr, ERROR);
    SetSinkState(STREAM_MANAGER_SUSPENDED);
    int32_t ret;
#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer timer;
    timer.Start();
#endif
    ret = audioRendererSink_->Stop();
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
#ifdef ENABLE_HOOK_PCM
    timer.Stop();
    int64_t interval = timer.Elapsed();
    AUDIO_INFO_LOG("name %{public}s, RenderSinkStop Elapsed: %{public}" PRId64 " ms",
        sinkOutAttr_.adapterName.c_str(), interval);
#endif
    return SUCCESS;
}

StreamManagerState HpaeSinkOutputNode::GetSinkState(void)
{
    return state_;
}

int32_t HpaeSinkOutputNode::SetSinkState(StreamManagerState sinkState)
{
    HILOG_COMM_INFO("Sink[%{public}s] state change:[%{public}s]-->[%{public}s]",
        GetDeviceClass().c_str(), ConvertStreamManagerState2Str(state_).c_str(),
        ConvertStreamManagerState2Str(sinkState).c_str());
        state_ = sinkState;
        return SUCCESS;
}

size_t HpaeSinkOutputNode::GetPreOutNum()
{
    return inputStream_.GetPreOutputNum();
}

int32_t HpaeSinkOutputNode::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_ != nullptr, ERROR, "audioRendererSink_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_->IsInited(), ERR_ILLEGAL_STATE, "audioRendererSink_ not init");
    return audioRendererSink_->UpdateAppsUid(appsUid);
}

void HpaeSinkOutputNode::HandlePaPower(HpaePcmBuffer *pcmBuffer)
{
    if (GetDeviceClass() != "primary" || !pcmBuffer->IsValid()) {
        return;
    }
    if (pcmBuffer->IsSilence()) {
        if (!isDisplayPaPowerState_) {
            AUDIO_INFO_LOG("Timing begins, will close speaker after [%{public}" PRId64 "]s", WAIT_CLOSE_PA_TIME);
            isDisplayPaPowerState_ = true;
        }
        silenceDataUs_ += static_cast<int64_t>(pcmBuffer->GetFrameLen()) * TIME_IN_US /
            static_cast<int64_t>(pcmBuffer->GetSampleRate());
        if (isOpenPaPower_ && silenceDataUs_ >= WAIT_CLOSE_PA_TIME * TIME_IN_US &&
            audioRendererSink_->GetAudioScene() == 0) {
            int32_t ret = audioRendererSink_->SetPaPower(false);
            isOpenPaPower_ = false;
            silenceDataUs_ = 0;
            AUDIO_INFO_LOG("Speaker pa volume change to zero over [%{public}" PRId64
                "]s, close %{public}s pa [%{public}s], ret:%{public}d",
                WAIT_CLOSE_PA_TIME, GetDeviceClass().c_str(), (ret == 0 ? "success" : "failed"), ret);
        } else if (!isOpenPaPower_ && silenceDataUs_ >= MONITOR_CLOSE_PA_TIME * TIME_IN_US) {
            silenceDataUs_ = 0;
            AUDIO_INFO_LOG("Speaker pa have closed [%{public}" PRId64 "]s.", MONITOR_CLOSE_PA_TIME);
        }
    } else {
        if (isDisplayPaPowerState_) {
            isDisplayPaPowerState_ = false;
            AUDIO_INFO_LOG("Volume change to non zero, break the speaker closing.");
        }
        silenceDataUs_ = 0;
        if (!isOpenPaPower_) {
            int32_t ret = audioRendererSink_->SetPaPower(true);
            isOpenPaPower_ = true;
            AUDIO_INFO_LOG("Volume change to non zero, open closed pa:[%{public}s] -- [%{public}s], ret:%{public}d",
                GetDeviceClass().c_str(), (ret == 0 ? "success" : "failed"), ret);
        }
    }
}

int32_t HpaeSinkOutputNode::RenderSinkSetPriPaPower()
{
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_ != nullptr, ERROR, "audioRendererSink_ is nullptr");
    int32_t ret = audioRendererSink_->SetPriPaPower();
    AUDIO_INFO_LOG("Open pri pa:[%{public}s] -- [%{public}s], ret:%{public}d",
        GetDeviceClass().c_str(), (ret == 0 ? "success" : "failed"), ret);
    return ret;
}

uint32_t HpaeSinkOutputNode::GetLatency()
{
    if (audioRendererSink_ == nullptr) {
        return ERROR;
    }
    audioRendererSink_->GetLatency(latency_);
    return latency_;
}

int32_t HpaeSinkOutputNode::RenderSinkSetSyncId(int32_t syncId)
{
    isSyncIdSet_ = true;
    syncId_ = syncId;
    return SUCCESS;
}

void HpaeSinkOutputNode::HandleHapticParam(uint64_t syncTime)
{
    if (isSyncIdSet_) {
        isSyncIdSet_ = false;
        AudioParamKey key = NONE;
        std::string condition = "haptic";
        std::string param = "haptic_sessionid=" + std::to_string(syncId_) +
            ";haptic_offset=" + std::to_string(syncTime);
        audioRendererSink_->SetAudioParameter(key, condition, param);
    }
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
