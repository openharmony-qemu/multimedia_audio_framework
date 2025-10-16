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

#ifndef I_HPAE_OUTPUT_CLUSTER_H
#define I_HPAE_OUTPUT_CLUSTER_H
#include "hpae_node.h"
#include "sink/i_audio_render_sink.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr uint32_t TIME_OUT_STOP_THD_DEFAULT_FRAME = 150;
constexpr uint32_t FRAME_LEN_MS_DEFAULT_MS = 20;
class IHpaeOutputCluster : public InputNode<HpaePcmBuffer *> {
public:
    virtual ~IHpaeOutputCluster() = default;
    virtual int32_t GetConverterNodeCount() = 0;
    virtual int32_t GetPreOutNum() = 0;
    virtual int32_t GetInstance(const std::string &deviceClass, const std::string &deviceNetId) = 0;
    virtual int32_t Init(IAudioSinkAttr &attr) = 0;
    virtual int32_t DeInit() = 0;
    virtual int32_t Flush(void) = 0;
    virtual int32_t Pause(void) = 0;
    virtual int32_t ResetRender(void) = 0;
    virtual int32_t Resume(void) = 0;
    virtual int32_t Start(void) = 0;
    virtual int32_t Stop(void) = 0;
    virtual int32_t SetTimeoutStopThd(uint32_t timeoutThdMs) = 0;
    virtual const char *GetFrameData(void) = 0;
    virtual StreamManagerState GetState(void) = 0;
    virtual bool IsProcessClusterConnected(HpaeProcessorType sceneType) = 0;
    virtual int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) = 0;
    virtual int32_t SetPriPaPower(void) { return 0; };
    virtual int32_t SetSyncId(int32_t syncId) { return 0; };
    virtual uint32_t GetHdiLatency() { return 0; };
    virtual uint64_t GetLatency(HpaeProcessorType sceneType) { return 0; };
    virtual void UpdateStreamInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> preNode) {};
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif // I_HPAE_OUTPUT_CLUSTER_H