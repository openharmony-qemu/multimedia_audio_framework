/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef HPAE_ADAPTER_MANAGER_H
#define HPAE_ADAPTER_MANAGER_H

#include <map>
#include <mutex>
#include <set>

#include "audio_timer.h"
#include "i_stream_manager.h"
#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {

class HpaeAdapterManager : public IStreamManager {
public:
    HpaeAdapterManager(ManagerType type);

    int32_t CreateRender(AudioProcessConfig processConfig, std::shared_ptr<IRendererStream> &stream,
        std::optional<std::string_view> originDeviceName = std::nullopt) override;
    int32_t ReleaseRender(uint32_t streamIndex_) override;
    int32_t StartRender(uint32_t streamIndex) override;
    int32_t StartRenderWithSyncId(uint32_t streamIndex, const int32_t &syncId) override;
    int32_t StopRender(uint32_t streamIndex) override;
    int32_t PauseRender(uint32_t streamIndex) override;
    int32_t GetStreamCount() const noexcept override;
    int32_t TriggerStartIfNecessary() override;
    int32_t CreateCapturer(AudioProcessConfig processConfig, std::shared_ptr<ICapturerStream> &stream) override;
    int32_t ReleaseCapturer(uint32_t streamIndex_) override;
    int32_t AddUnprocessStream(int32_t appUid) override;
    uint64_t GetLatency() noexcept override;
    void GetAllSinkInputs(std::vector<SinkInput> &sinkInputs) override;
private:
    int32_t GetDeviceNameForConnect(AudioProcessConfig processConfig, uint32_t sessionId, std::string &deviceName);
    // audio channel index
    std::shared_ptr<IRendererStream> CreateRendererStream(AudioProcessConfig processConfig,
        const std::string &deviceName = "");
    std::shared_ptr<ICapturerStream> CreateCapturerStream(AudioProcessConfig processConfig,
        const std::string &deviceName = "");
    void SetHighResolution(AudioProcessConfig &processConfig, uint32_t sessionId);
    bool CheckHighResolution(const AudioProcessConfig &processConfig) const;

    ManagerType managerType_ = PLAYBACK;
    std::mutex streamMapMutex_;
    std::mutex paElementsMutex_;
    std::map<int32_t, std::shared_ptr<IRendererStream>> rendererStreamMap_;
    std::map<int32_t, std::shared_ptr<ICapturerStream>> capturerStreamMap_;
    std::mutex sinkInputsMutex_;
    std::vector<SinkInput> sinkInputs_;
    std::set<int32_t> unprocessAppUidSet_;
    uint32_t highResolutionIndex_ = 0;
    bool isHighResolutionExist_ = false;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // HPAE_ADAPTER_MANAGER_H
