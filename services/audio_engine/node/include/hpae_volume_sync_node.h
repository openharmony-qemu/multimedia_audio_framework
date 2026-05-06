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

#ifndef HPAE_VOLUME_SYNC_NODE_H
#define HPAE_VOLUME_SYNC_NODE_H

#include <memory>
#include "hpae_plugin_node.h"
#include "sink/i_audio_render_sink.h"
#include "manager/hdi_adapter_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

/**
 * HpaeVolumeSyncNode is a gain node that does not perform actual gain calculation.
 * It only syncs volume changes to the render sink by calling SetVolume when volume changes.
 */
class HpaeVolumeSyncNode : public HpaePluginNode {
public:
    HpaeVolumeSyncNode(HpaeNodeInfo &nodeInfo);
    virtual ~HpaeVolumeSyncNode();

    /**
     * Reset volume to default
     */
    void ResetVolume();

    uint64_t GetLatency(uint32_t sessionId = 0) override;

protected:
    HpaePcmBuffer *SignalProcess(const std::vector<HpaePcmBuffer *> &inputs) override;

private:
    std::shared_ptr<IAudioRenderSink> GetRenderSink();
    void SetSinkVolume(float gainVolume);
    uint32_t renderId_ = HDI_INVALID_ID;
    bool isInnerCapturerOrInjector_ = false;
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif  // HPAE_VOLUME_SYNC_NODE_H
