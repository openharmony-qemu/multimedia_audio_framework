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
#define LOG_TAG "HpaeOffloadSinkOutputNode"
#endif

#include "hpae_offload_sinkoutput_node.h"
#include "audio_errors.h"
#include <iostream>
#include <cinttypes>

#include "hpae_format_convert.h"
#include "hpae_node_common.h"
#include "audio_engine_log.h"
#include "audio_volume.h"
#include "audio_common_utils.h"
#ifdef ENABLE_HOOK_PCM
#include "hpae_pcm_dumper.h"
#endif
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
    constexpr uint32_t CACHE_FRAME_COUNT = 2;
    constexpr uint32_t TIME_US_PER_MS = 1000;
    constexpr uint32_t TIME_MS_PER_SEC = 1000;
    constexpr uint32_t ERR_RETRY_COUNT = 20;
    constexpr uint32_t FRAME_TIME_IN_MS = 20;
    constexpr int32_t OFFLOAD_FULL = -1;
    constexpr int32_t OFFLOAD_WRITE_FAILED = -2;
    constexpr uint32_t OFFLOAD_HDI_CACHE_BACKGROUND_IN_MS = 7000;
    constexpr uint32_t OFFLOAD_HDI_CACHE_FRONTGROUND_IN_MS = 200;
    // hdi fallback, modify when hdi change
    constexpr uint32_t OFFLOAD_FAD_INTERVAL_IN_US = 180000;
    constexpr uint32_t OFFLOAD_SET_BUFFER_SIZE_NUM = 5;
    constexpr uint32_t POLICY_STATE_DELAY_IN_SEC = 3;
    static constexpr float EPSILON = 1e-6f;

    const std::string DEVICE_CLASS_OFFLOAD = "offload";
}
HpaeOffloadSinkOutputNode::HpaeOffloadSinkOutputNode(HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo),
      renderFrameData_(nodeInfo.frameLen * nodeInfo.channels *
        GetSizeFromFormat(nodeInfo.format) * CACHE_FRAME_COUNT),
      interleveData_(nodeInfo.frameLen * nodeInfo.channels)
{
#ifdef ENABLE_HOOK_PCM
    outputPcmDumper_ = std::make_unique<HpaePcmDumper>(
        "HpaeOffloadSinkOutputNode_Out_bit_" + std::to_string(GetBitWidth()) + "_ch_" +
        std::to_string(GetChannelCount()) + "_rate_" + std::to_string(GetSampleRate()) + ".pcm");
#endif
    frameLenMs_ = nodeInfo.frameLen * TIME_MS_PER_SEC / nodeInfo.samplingRate;
}

bool HpaeOffloadSinkOutputNode::CheckIfSuspend()
{
    static uint32_t suspendCount = 0;
    if (!GetPreOutNum()) {
        suspendCount++;
        usleep(TIME_US_PER_MS * FRAME_TIME_IN_MS);
        if (suspendCount > timeoutThdFrames_) {
            RenderSinkStop();
        }
        return true;
    } else {
        suspendCount = 0;
        return false;
    }
}

void HpaeOffloadSinkOutputNode::DoProcess()
{
    CHECK_AND_RETURN_LOG(audioRendererSink_, "audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());
    if (CheckIfSuspend()) {
        return;
    }
    // if there are no enough frames in cache, read more data from pre-output
    size_t frameSize = GetSizeFromFormat(GetBitWidth()) * GetFrameLen() * GetChannelCount();
    while (renderFrameData_.size() < CACHE_FRAME_COUNT * frameSize) {
        std::vector<HpaePcmBuffer *> &outputVec = inputStream_.ReadPreOutputData();
        if (outputVec.front()->IsValid()) {
            renderFrameData_.resize(renderFrameData_.size() + frameSize);
            ConvertFromFloat(GetBitWidth(), GetChannelCount() * GetFrameLen(),
                outputVec.front()->GetPcmDataBuffer(), renderFrameData_.data() + renderFrameData_.size() - frameSize);
        } else {
            break;
        }
    }
    int32_t ret = ProcessRenderFrame();
    // if renderframe faild, sleep and return directly
    // if renderframe full, unlock the powerlock
    static uint32_t retryCount = 1;
    if (ret != SUCCESS) {
        if (ret == OFFLOAD_FULL) {
            RunningLock(false);
            isHdiFull_.store(true);
            return;
        }
        usleep(std::min(retryCount, FRAME_TIME_IN_MS) * TIME_US_PER_MS);
        if (retryCount < ERR_RETRY_COUNT) {
            retryCount++;
        }
        return;
    }
    retryCount = 1;
    return;
}

bool HpaeOffloadSinkOutputNode::Reset()
{
    const auto preOutputMap = inputStream_.GetPreOutputMap();
    for (const auto &preOutput : preOutputMap) {
        OutputPort<HpaePcmBuffer *> *output = preOutput.first;
        inputStream_.DisConnect(output);
    }
    return true;
}

bool HpaeOffloadSinkOutputNode::ResetAll()
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

void HpaeOffloadSinkOutputNode::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort());
}

void HpaeOffloadSinkOutputNode::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    inputStream_.DisConnect(preNode->GetOutputPort());
}

int32_t HpaeOffloadSinkOutputNode::GetRenderSinkInstance(const std::string &deviceClass,
    const std::string &deviceNetworkId)
{
    renderId_ = HdiAdapterManager::GetInstance().GetRenderIdByDeviceClass(
        deviceClass, HDI_ID_INFO_DEFAULT, true);
    audioRendererSink_ = HdiAdapterManager::GetInstance().GetRenderSink(renderId_, true);
    if (audioRendererSink_ == nullptr) {
        AUDIO_ERR_LOG("get offload sink fail, deviceClass: %{public}s, renderId_: %{public}u",
            deviceClass.c_str(), renderId_);
        HdiAdapterManager::GetInstance().ReleaseId(renderId_);
        return ERROR;
    }
    return SUCCESS;
}

void HpaeOffloadSinkOutputNode::OffloadReset()
{
    writePos_ = 0;
    hdiPos_ = std::make_pair(0, std::chrono::high_resolution_clock::now());
    firstWriteHdi_ = true;
    isHdiFull_.store(false);
    renderFrameData_.clear();
    setPolicyStateTask_.flag = false; // unset the task when reset
    RunningLock(true);
}

int32_t HpaeOffloadSinkOutputNode::RenderSinkInit(IAudioSinkAttr &attr)
{
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_, ERR_ILLEGAL_STATE,
        "audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());

    sinkOutAttr_ = attr;
    SetSinkState(STREAM_MANAGER_IDLE);
#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer timer;
    timer.Start();
#endif
    int32_t ret = audioRendererSink_->Init(attr);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "audioRendererSink_ init failed, errCode is %{public}d", ret);
#ifdef ENABLE_HOOK_PCM
    timer.Stop();
    uint64_t interval = timer.Elapsed();
    AUDIO_INFO_LOG("HpaeOffloadSinkOutputNode: name %{public}s, RenderSinkInit Elapsed: %{public}" PRIu64 " ms",
        sinkOutAttr_.adapterName.c_str(), interval);
#endif
    return ret;
}

int32_t HpaeOffloadSinkOutputNode::RenderSinkDeInit(void)
{
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_, ERR_ILLEGAL_STATE,
        "audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());
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
    uint64_t interval = timer.Elapsed();
    AUDIO_INFO_LOG("HpaeOffloadSinkOutputNode: name %{public}s, RenderSinkDeInit Elapsed: %{public}" PRIu64 " ms",
        sinkOutAttr_.adapterName.c_str(), interval);
#endif
    return SUCCESS;
}

int32_t HpaeOffloadSinkOutputNode::RenderSinkFlush(void)
{
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_, ERR_ILLEGAL_STATE,
        "audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());
    int32_t ret;
#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer timer;
    timer.Start();
#endif
    ret = audioRendererSink_->Flush();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "audioRendererSink_ flush failed, errCode is %{public}d", ret);
#ifdef ENABLE_HOOK_PCM
    timer.Stop();
    uint64_t interval = timer.Elapsed();
    AUDIO_INFO_LOG("HpaeOffloadSinkOutputNode: name %{public}s, RenderSinkFlush Elapsed: %{public}" PRIu64 " ms",
        sinkOutAttr_.adapterName.c_str(), interval);
#endif
    return ret;
}

int32_t HpaeOffloadSinkOutputNode::RenderSinkStart(void)
{
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_, ERR_ILLEGAL_STATE,
        "audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());

    int32_t ret;
#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer timer;
    timer.Start();
#endif
    ret = audioRendererSink_->Start();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "audioRendererSink_ start failed, errCode is %{public}d", ret);
    RegOffloadCallback();
    // start need lock
    RunningLock(true);
    OffloadSetHdiVolume();
#ifdef ENABLE_HOOK_PCM
    timer.Stop();
    uint64_t interval = timer.Elapsed();
    AUDIO_INFO_LOG("HpaeOffloadSinkOutputNode: name %{public}s, RenderSinkStart Elapsed: %{public}" PRIu64 " ms",
        sinkOutAttr_.adapterName.c_str(), interval);
#endif
    SetSinkState(STREAM_MANAGER_RUNNING);
    return SUCCESS;
}

int32_t HpaeOffloadSinkOutputNode::RenderSinkStop(void)
{
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_, ERR_ILLEGAL_STATE,
        "audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());
    int32_t ret;
#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer timer;
    timer.Start();
#endif
    ret = audioRendererSink_->Stop();
    OffloadReset();
    RunningLock(false);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "audioRendererSink_ stop failed, errCode is %{public}d", ret);
#ifdef ENABLE_HOOK_PCM
    timer.Stop();
    uint64_t interval = timer.Elapsed();
    AUDIO_INFO_LOG("HpaeOffloadSinkOutputNode: name %{public}s, RenderSinkStop Elapsed: %{public}" PRIu64 " ms",
        sinkOutAttr_.adapterName.c_str(), interval);
#endif
    SetSinkState(STREAM_MANAGER_SUSPENDED);
    return SUCCESS;
}

void HpaeOffloadSinkOutputNode::FlushStream()
{
    renderFrameData_.clear();
}

size_t HpaeOffloadSinkOutputNode::GetPreOutNum()
{
    return inputStream_.GetPreOutputNum();
}

StreamManagerState HpaeOffloadSinkOutputNode::GetSinkState(void)
{
    return isHdiFull_.load() ? STREAM_MANAGER_SUSPENDED : state_;
}

int32_t HpaeOffloadSinkOutputNode::SetSinkState(StreamManagerState sinkState)
{
    AUDIO_INFO_LOG("Sink[%{public}s] state change:[%{public}s]-->[%{public}s]",
        GetDeviceClass().c_str(), ConvertStreamManagerState2Str(state_).c_str(),
        ConvertStreamManagerState2Str(sinkState).c_str());
    state_ = sinkState;
    return SUCCESS;
}

const char *HpaeOffloadSinkOutputNode::GetRenderFrameData(void)
{
    return renderFrameData_.data();
}

void HpaeOffloadSinkOutputNode::StopStream()
{
    // flush hdi when disconnect
    RunningLock(true);
    auto ret = RenderSinkFlush();
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "RenderSinkFlush failed");
    uint64_t cacheLenInHdi = CalcOffloadCacheLenInHdi();
    cacheLenInHdi = cacheLenInHdi > OFFLOAD_FAD_INTERVAL_IN_US ?
        cacheLenInHdi - OFFLOAD_FAD_INTERVAL_IN_US : 0;
    uint64_t rewindTime = cacheLenInHdi + ConvertDatalenToUs(renderFrameData_.size(), GetNodeInfo());
    AUDIO_DEBUG_LOG("OffloadRewindAndFlush rewind time in us %{public}" PRIu64, rewindTime);
    auto callback = GetNodeInfo().statusCallback.lock();
    callback->OnRewindAndFlush(rewindTime);
    OffloadReset();
}

void HpaeOffloadSinkOutputNode::SetPolicyState(int32_t state)
{
    if (setPolicyStateTask_.flag) {
        if (state == hdiPolicyState_) {
            AUDIO_INFO_LOG("HpaeOffloadSinkOutputNode: unset policy state task");
            setPolicyStateTask_.flag = false;
        }
        return;
    }
    if (hdiPolicyState_ != state && state == OFFLOAD_INACTIVE_BACKGROUND) {
        AUDIO_INFO_LOG("HpaeOffloadSinkOutputNode: set policy state task");
        setPolicyStateTask_.flag = true;
        setPolicyStateTask_.time = std::chrono::high_resolution_clock::now();
        setPolicyStateTask_.state = OFFLOAD_INACTIVE_BACKGROUND;
        return;
    }
    hdiPolicyState_ = static_cast<AudioOffloadType>(state);
    int32_t bufferSize = hdiPolicyState_ == OFFLOAD_INACTIVE_BACKGROUND ?
        OFFLOAD_HDI_CACHE_BACKGROUND_IN_MS : OFFLOAD_HDI_CACHE_FRONTGROUND_IN_MS;
    AUDIO_INFO_LOG("HpaeOffloadSinkOutputNode: set hdi buffer size to %{public}d", bufferSize);
    audioRendererSink_->SetBufferSize(bufferSize);
}

uint64_t HpaeOffloadSinkOutputNode::GetLatency()
{
    return CalcOffloadCacheLenInHdi() + ConvertDatalenToUs(renderFrameData_.size(), GetNodeInfo());
}

int32_t HpaeOffloadSinkOutputNode::SetTimeoutStopThd(uint32_t timeoutThdMs)
{
    if (frameLenMs_ != 0) {
        timeoutThdFrames_ = timeoutThdMs / frameLenMs_;
    }
    AUDIO_INFO_LOG(
        "SetTimeoutStopThd: timeoutThdFrames_:%{public}u, timeoutThdMs :%{public}u", timeoutThdFrames_, timeoutThdMs);
    return SUCCESS;
}

void HpaeOffloadSinkOutputNode::RunningLock(bool islock)
{
    if (islock) {
        audioRendererSink_->LockOffloadRunningLock();
    } else if (!islock && hdiPolicyState_ == OFFLOAD_INACTIVE_BACKGROUND) {
        // only unlock when background
        audioRendererSink_->UnLockOffloadRunningLock();
    }
}

void HpaeOffloadSinkOutputNode::SetBufferSizeWhileRenderFrame()
{
    // 3s delay works, when change to BACKGROUND
    if (setPolicyStateTask_.flag) {
        auto now = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - setPolicyStateTask_.time).count() >=
            POLICY_STATE_DELAY_IN_SEC) {
            AUDIO_INFO_LOG("HpaeOffloadSinkOutputNode: excute set policy state task");
            setPolicyStateTask_.flag = false;
            hdiPolicyState_ = setPolicyStateTask_.state;
            audioRendererSink_->SetBufferSize(hdiPolicyState_ == OFFLOAD_INACTIVE_BACKGROUND ?
                OFFLOAD_HDI_CACHE_BACKGROUND_IN_MS : OFFLOAD_HDI_CACHE_FRONTGROUND_IN_MS);
        }
        return; // no need to set buffer size twice at one process
    }
    // first start need to set buffer size 5 times
    if (setHdiBufferSizeNum_ > 0) {
        setHdiBufferSizeNum_--;
        AUDIO_INFO_LOG("HpaeOffloadSinkOutputNode: set policy state cause first render");
        audioRendererSink_->SetBufferSize(hdiPolicyState_ == OFFLOAD_INACTIVE_BACKGROUND ?
            OFFLOAD_HDI_CACHE_BACKGROUND_IN_MS : OFFLOAD_HDI_CACHE_FRONTGROUND_IN_MS);
    }
}

int32_t HpaeOffloadSinkOutputNode::ProcessRenderFrame()
{
    if (renderFrameData_.empty()) {
        return OFFLOAD_WRITE_FAILED;
    }
    uint64_t writeLen = 0;
    char *renderFrameData = (char *)renderFrameData_.data();
#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer timer;
    timer.Start();
    intervalTimer_.Stop();
    uint64_t interval = intervalTimer_.Elapsed();
    AUDIO_DEBUG_LOG("HpaeOffloadSinkOutputNode: name %{public}s, RenderFrame interval: %{public}" PRIu64 " ms",
        sinkOutAttr_.adapterName.c_str(), interval);
#endif
    auto now = std::chrono::high_resolution_clock::now();
    auto ret = audioRendererSink_->RenderFrame(*renderFrameData, renderFrameData_.size(), writeLen);
    if (ret == SUCCESS && writeLen == 0 && !firstWriteHdi_) {
        AUDIO_INFO_LOG("HpaeOffloadSinkOutputNode: offload renderFrame full");
        return OFFLOAD_FULL;
    }
    if (!(ret == SUCCESS && writeLen == renderFrameData_.size())) {
        AUDIO_ERR_LOG("HpaeOffloadSinkOutputNode: offload renderFrame failed, errCode is %{public}d", ret);
        return OFFLOAD_WRITE_FAILED;
    }
    // calc written data length
    writePos_ += ConvertDatalenToUs(renderFrameData_.size(), GetNodeInfo());
    // now is the time to first write hdi
    if (firstWriteHdi_) {
        firstWriteHdi_ = false;
        hdiPos_ = std::make_pair(0, now);
        setHdiBufferSizeNum_ = OFFLOAD_SET_BUFFER_SIZE_NUM;
        // if the hdi is flushing, it will block the volume setting.
        // so the render frame judge it.
        OffloadSetHdiVolume();
        AUDIO_INFO_LOG("offload write pos: %{public}" PRIu64 " hdi pos: %{public}" PRIu64 " ",
            writePos_, hdiPos_.first);
    }
    // hdi fallback, dont modify
    SetBufferSizeWhileRenderFrame();
#ifdef ENABLE_HOOK_PCM
    AUDIO_DEBUG_LOG("HpaeOffloadSinkOutputNode: name %{public}s, RenderFrame interval: %{public}" PRIu64 " ms",
        sinkOutAttr_.adapterName.c_str(), interval);
    if (outputPcmDumper_) {
        outputPcmDumper_->Dump((int8_t *)renderFrameData, renderFrameData_.size());
    }
    timer.Stop();
    uint64_t elapsed = timer.Elapsed();
    AUDIO_DEBUG_LOG("HpaeOffloadSinkOutputNode :name %{public}s, RenderFrame elapsed time: %{public}" PRIu64 " ms",
        sinkOutAttr_.adapterName.c_str(), elapsed);
    intervalTimer_.Start();
#endif
    renderFrameData_.clear();
    return SUCCESS;
}

int32_t HpaeOffloadSinkOutputNode::UpdatePresentationPosition()
{
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_, ERR_ILLEGAL_STATE,
        "audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());
    uint64_t frames;
    int64_t timeSec;
    int64_t timeNanoSec;
    int ret = audioRendererSink_->GetPresentationPosition(frames, timeSec, timeNanoSec);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetPresentationPosition failed, errCode is %{public}d", ret);
    auto total_ns = std::chrono::seconds(timeSec) + std::chrono::nanoseconds(timeNanoSec);
    hdiPos_ = std::make_pair(frames, TimePoint(total_ns));
    return 0;
}

uint64_t HpaeOffloadSinkOutputNode::CalcOffloadCacheLenInHdi()
{
    auto now = std::chrono::high_resolution_clock::now();
    uint64_t time = now > hdiPos_.second ?
        std::chrono::duration_cast<std::chrono::microseconds>(now - hdiPos_.second).count() : 0;
    uint64_t hdiPos = hdiPos_.first + time;
    uint64_t cacheLenInHdi = writePos_ > hdiPos ? (writePos_ - hdiPos) : 0;
    AUDIO_DEBUG_LOG("offload latency: %{public}" PRIu64 " write pos: %{public}" PRIu64
                    " hdi pos: %{public}" PRIu64 " time: %{public}" PRIu64,
                    cacheLenInHdi, writePos_, hdiPos, time);
    return cacheLenInHdi;
}

void HpaeOffloadSinkOutputNode::OffloadSetHdiVolume()
{
    struct VolumeValues volumes;
    AudioStreamType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(GetStreamType());
    float volumeEnd = AudioVolume::GetInstance()->GetVolume(GetSessionId(), volumeType, DEVICE_CLASS_OFFLOAD, &volumes);
    float volumeBeg = AudioVolume::GetInstance()->GetHistoryVolume(GetSessionId());
    if (fabs(volumeBeg - volumeEnd) > EPSILON) {
        AUDIO_INFO_LOG("HpaeOffloadSinkOutputNode::sessionID:%{public}u, volumeBeg:%{public}f, volumeEnd:%{public}f",
            GetSessionId(), volumeBeg, volumeEnd);
        AudioVolume::GetInstance()->SetHistoryVolume(GetSessionId(), volumeEnd);
        AudioVolume::GetInstance()->Monitor(GetSessionId(), true);
    }
    CHECK_AND_RETURN_LOG(audioRendererSink_, "audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());
    audioRendererSink_->SetVolume(volumeEnd, volumeEnd);
}

void HpaeOffloadSinkOutputNode::OffloadCallback(const RenderCallbackType type)
{
    switch (type) {
        case CB_NONBLOCK_WRITE_COMPLETED: {
            if (isHdiFull_.load()) {
                RunningLock(true);
                UpdatePresentationPosition();
                auto callback = GetNodeInfo().statusCallback.lock();
                isHdiFull_.store(false);
                if (callback) {
                    callback->OnNotifyQueue();
                }
            }
            break;
        }
        default:
            break;
    }
}

void HpaeOffloadSinkOutputNode::RegOffloadCallback()
{
    CHECK_AND_RETURN_LOG(audioRendererSink_, "audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());
    audioRendererSink_->RegistOffloadHdiCallback([this](const RenderCallbackType type) { OffloadCallback(type); });
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
