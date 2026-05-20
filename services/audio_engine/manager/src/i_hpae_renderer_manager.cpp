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
#define LOG_TAG "IHpaeRendererManager"
#endif
#include "i_hpae_renderer_manager.h"
#include "hpae_renderer_manager.h"
#include "hpae_offload_renderer_manager.h"
#include "hpae_fast_renderer_manager.h"
#include "hpae_inner_capturer_manager.h"
#include "hpae_injector_renderer_manager.h"
#include "hpae_direct_renderer_manager.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
static const std::string DEVICE_CLASS_OFFLOAD = "offload";
static const std::string DEVICE_CLASS_PRIMARY_MMAP = "primary_mmap";
static const std::string DEVICE_CLASS_PRIMARY_MMAP_VOIP = "primary_mmap_voip";
static const std::string DEVICE_CLASS_A2DP_VOIP = "a2dp_fast";
static const std::string DEVICE_CLASS_REMOTE_OFFLOAD = "remote_offload";
static const std::string DEVICE_CLASS_ULTRA_FAST = "ultra_fast";
static const std::string DEVICE_NAME_INNER_CAP = "InnerCapturerSink";
static const std::string DEVICE_NAME_CAST_INNER_CAP = "RemoteCastInnerCapturer";
static const std::string DEVICE_CLASS_VOIP_DIRECT = "voip";
static const std::string DEVICE_CLASS_DIRECT = "direct";
static const std::string DEVICE_CLASS_USB_FAST = "usb_arm_fast";
static const std::string DEVICE_CLASS_USB_ULTRA_FAST = "usb_arm_ultra_fast";
std::shared_ptr<IHpaeRendererManager> IHpaeRendererManager::CreateRendererManager(HpaeSinkInfo &sinkInfo)
{
    if (sinkInfo.deviceClass == DEVICE_CLASS_OFFLOAD || sinkInfo.deviceClass == DEVICE_CLASS_REMOTE_OFFLOAD) {
        AUDIO_INFO_LOG("IHpaeRendererManager::CreateRendererManager HpaeOffloadRendererManager");
        return std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    } else if (sinkInfo.deviceClass ==  DEVICE_CLASS_PRIMARY_MMAP ||
        sinkInfo.deviceClass == DEVICE_CLASS_PRIMARY_MMAP_VOIP ||
        sinkInfo.deviceClass == DEVICE_CLASS_A2DP_VOIP ||
        sinkInfo.deviceClass == DEVICE_CLASS_ULTRA_FAST ||
        sinkInfo.deviceClass == DEVICE_CLASS_USB_FAST ||
        sinkInfo.deviceClass == DEVICE_CLASS_USB_ULTRA_FAST) {
        AUDIO_INFO_LOG("IHpaeRendererManager::CreateRendererManager HpaeFastRendererManager");
        return std::make_shared<HpaeFastRendererManager>(sinkInfo);
    } else if ((sinkInfo.deviceName.compare(0, DEVICE_NAME_INNER_CAP.length(), DEVICE_NAME_INNER_CAP) == 0)
        || sinkInfo.deviceName == DEVICE_NAME_CAST_INNER_CAP) {
            AUDIO_INFO_LOG("IHpaeRendererManager::CreateRendererManager HpaeInnerCapturerManager");
        return std::make_shared<HpaeInnerCapturerManager>(sinkInfo);
    } else if (sinkInfo.deviceName == VIRTUAL_INJECTOR) {
        AUDIO_INFO_LOG("IHpaeRendererManager::CreateRendererManager HpaeInjectorRendererManager");
        return std::make_shared<HpaeInjectorRendererManager>(sinkInfo);
    } else if (sinkInfo.deviceName == DEVICE_CLASS_VOIP_DIRECT || sinkInfo.deviceName == DEVICE_CLASS_DIRECT) {
        AUDIO_INFO_LOG("IHpaeRendererManager::CreateRendererManager HpaeDirectRendererManager");
        return std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    }
    AUDIO_INFO_LOG("IHpaeRendererManager::CreateRendererManager HpaeRendererManager");
    return std::make_shared<HpaeRendererManager>(sinkInfo);
}

void IHpaeRendererManager::UploadDumpSinkInfo(std::string& deviceName)
{
#ifdef ENABLE_HIDUMP_DFX
        std::string dumpStr;
        dfxTree_.PrintTree(dumpStr);
        TriggerCallback(DUMP_SINK_INFO, deviceName, dumpStr);
#endif
};

void IHpaeRendererManager::OnNotifyDfxNodeAdmin(bool isAdd, const HpaeDfxNodeInfo &nodeInfo)
{
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("%{public}s node, nodeName:%{public}s nodeId:%{public}u",
        isAdd ? "Add" : "Remove", nodeInfo.nodeName.c_str(), nodeInfo.nodeId);
    if (isAdd) {
        dfxTree_.AddNode(nodeInfo);
    } else {
        dfxTree_.RemoveNode(nodeInfo.nodeId);
    }
#endif
};

void IHpaeRendererManager::OnNotifyDfxNodeInfo(bool isConnect, uint32_t parentId, uint32_t childId)
{
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("%{public}s preNodeId:%{public}u, NodeId:%{public}u",
        isConnect ? "connect" : "disconnect", parentId, childId);
    if (isConnect) {
        dfxTree_.ConnectNodes(parentId, childId);
    } else {
        dfxTree_.DisConnectNodes(parentId, childId);
    }
#endif
};

int32_t IHpaeRendererManager::SetSinkVirtualOutputNode(
    const std::shared_ptr<HpaeSinkVirtualOutputNode> &sinkVirtualOutputNode)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return ERROR;
};

bool IHpaeRendererManager::AdjustFrameLen(HpaeNodeInfo &inNodeInfo, const HpaeNodeInfo &outNodeInfo)
{
    // Check if input node's frameLen needs to modify.
    uint32_t inRate = inNodeInfo.GetEffectiveSampleRate();
    uint32_t outRate = outNodeInfo.GetEffectiveSampleRate();
    if (inRate == 0 || outRate == 0) {
        AUDIO_ERR_LOG("AdjustFrameLen invalid rate: inRate=%{public}u outRate=%{public}u", inRate, outRate);
        return false;
    }
    inNodeInfo.frameLen = static_cast<uint32_t>(outNodeInfo.frameLen) * inRate / outRate;
    return true;
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
