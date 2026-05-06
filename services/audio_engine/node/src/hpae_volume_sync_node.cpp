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
#define LOG_TAG "HpaeVolumeSyncNode"
#endif

#include "hpae_volume_sync_node.h"
#include "audio_volume.h"
#include "audio_utils.h"
#include "audio_engine_log.h"
#include "audio_errors.h"
#include "hpae_info.h"
#include "manager/hdi_adapter_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

static constexpr float EPSILON = 1e-6f;
const std::string VOIP_SINK_NAME = "voip";

HpaeVolumeSyncNode::HpaeVolumeSyncNode(HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo), HpaePluginNode(nodeInfo)
{
    AUDIO_INFO_LOG("HpaeVolumeSyncNode created, SessionId:%{public}u deviceClass:%{public}s",
        GetSessionId(), GetDeviceClass().c_str());
    renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, VOIP_SINK_NAME, true);
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("hpaeVolumeSyncNode");
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(true, GetNodeInfo());
    }
#endif
}

HpaeVolumeSyncNode::~HpaeVolumeSyncNode()
{
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(false, GetNodeInfo());
    }
#endif
}

std::shared_ptr<IAudioRenderSink> HpaeVolumeSyncNode::GetRenderSink()
{
    if (renderId_ == HDI_INVALID_ID) {
        renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY,
            VOIP_SINK_NAME, true);
    }
    return HdiAdapterManager::GetInstance().GetRenderSink(renderId_, true);
}

void HpaeVolumeSyncNode::ResetVolume()
{
    auto audioVolume = AudioVolume::GetInstance();
    struct VolumeValues volumes;
    float curSystemGain = audioVolume->GetVolume(GetSessionId(), GetStreamType(), renderId_, &volumes);
    SetSinkVolume(curSystemGain);
    audioVolume->SetHistoryVolume(GetSessionId(), curSystemGain);
    audioVolume->Monitor(GetSessionId(), true);
    AUDIO_INFO_LOG("ResetVolume, SessionId:%{public}u, curSystemGain:%{public}f", GetSessionId(), curSystemGain);
}

HpaePcmBuffer *HpaeVolumeSyncNode::SignalProcess(const std::vector<HpaePcmBuffer *> &inputs)
{
    // This node does not perform actual gain calculation
    // It only syncs volume to render sink when volume changes
    if (inputs.empty()) {
        AUDIO_WARNING_LOG("inputs size is empty, SessionId:%{public}u", GetSessionId());
        return nullptr;
    }

    // Get current and history volume, reference from HpaeGainNode::DoGain
    struct VolumeValues volumes;
    AudioVolume *audioVolume = AudioVolume::GetInstance();

    float curSystemGain = audioVolume->GetVolume(GetSessionId(), GetStreamType(), renderId_, &volumes);
    float preSystemGain = volumes.volumeHistory;

    AUDIO_DEBUG_LOG("curSystemGain:%{public}f, preSystemGain:%{public}f deviceClass:%{public}s",
        curSystemGain, preSystemGain, GetDeviceClass().c_str());

    // Check if volume has changed
    if (fabs(curSystemGain - preSystemGain) > EPSILON) {
        AUDIO_INFO_LOG("Volume changed, SessionId:%{public}u, preSystemGain:%{public}f, curSystemGain:%{public}f",
            GetSessionId(), preSystemGain, curSystemGain);
        audioVolume->SetHistoryVolume(GetSessionId(), curSystemGain);
        audioVolume->Monitor(GetSessionId(), true);

        SetSinkVolume(curSystemGain);
    }
    return inputs[0];
}

void HpaeVolumeSyncNode::SetSinkVolume(float gainVolume)
{
    // Set volume to render sink
    auto audioRendererSink = GetRenderSink();
    if (audioRendererSink != nullptr) {
        int32_t ret = audioRendererSink->SetVolume(gainVolume, gainVolume);
        if (ret != SUCCESS) {
            AUDIO_WARNING_LOG("SetVolume failed, SessionId:%{public}u, ret:%{public}d", GetSessionId(), ret);
        }
    } else {
        AUDIO_WARNING_LOG("audioRendererSink is nullptr, SessionId:%{public}u", GetSessionId());
    }
}

uint64_t HpaeVolumeSyncNode::GetLatency(uint32_t sessionId)
{
    return 0;
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
