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
#define LOG_TAG "IHpaeCapturerManager"
#endif
#include "i_hpae_capturer_manager.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
void IHpaeCapturerManager::UploadDumpSourceInfo(std::string &deviceName)
{
#ifdef ENABLE_HIDUMP_DFX
    std::string dumpStr;
    dfxTree_.PrintTree(dumpStr);
    TriggerCallback(DUMP_SOURCE_INFO, deviceName, dumpStr);
#endif
}

void IHpaeCapturerManager::OnNotifyDfxNodeInfo(bool isConnect, uint32_t preNodeId, HpaeDfxNodeInfo &nodeInfo)
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
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS