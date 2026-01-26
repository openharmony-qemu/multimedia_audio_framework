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
#ifndef HW_DECODING_STREAM_MANAGER_H
#define HW_DECODING_STREAM_MANAGER_H

#include <map>
#include <mutex>
#include "i_stream_manager.h"

namespace OHOS {
namespace AudioStandard {
class HWDecodingStreamManager : public IStreamManager {
public:
    explicit  HWDecodingStreamManager();
    int32_t CreateRender(AudioProcessConfig processConfig, std::shared_ptr<IRendererStream> &stream,
        std::optional<std::string_view> originDeviceName = std::nullopt) override;
    int32_t StartRender(uint32_t streamIndex) override;
    int32_t StopRender(uint32_t streamIndex) override;
    int32_t ReleaseRender(uint32_t streamIndex) override;
    int32_t PauseRender(uint32_t streamIndex, bool isStandby = false) override;
    int32_t GetStreamCount() const noexcept override;
    int32_t TriggerStartIfNecessary() override;

    int32_t CreateCapturer(AudioProcessConfig processConfig, std::shared_ptr<ICapturerStream> &stream) override;
    int32_t ReleaseCapturer(uint32_t streamIndex) override;

    int32_t AddUnprocessStream(int32_t appUid) override;

    uint64_t GetLatency() noexcept override;
    void GetAllSinkInputs(std::vector<SinkInput> &sinkInputs) override;
    int32_t GetSessionIdAndRemove(uint32_t paIndex, uint32_t &sessionId) override;

private:
    std::mutex streamMapMutex_;
    std::map<int32_t, std::shared_ptr<IRendererStream>> rendererStreamMap_; // should has only one stream
};
} // namespace AudioStandard
} // namespace OHOS
#endif // HW_DECODING_STREAM_MANAGER_H
