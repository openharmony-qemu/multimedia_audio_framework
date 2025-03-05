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

#ifndef ST_RECORDER_DFX_WRITER_H
#define ST_RECORDER_DFX_WRITER_H

#include "audio_capturer_dfx_collector.h"

namespace OHOS {
namespace AudioStandard {

class RecorderDfxWriter {
public:
    RecorderDfxWriter(const AppInfo &appInfo, uint32_t index);
    RecorderDfxWriter() = delete;
    ~RecorderDfxWriter();

    void WriteDfxStartMsg(uint32_t index, CapturerStage stage, const AudioProcessConfig &processConfig);
    void WriteDfxStopMsg(uint32_t index, CapturerStage stage,
        int64_t duration, const AudioProcessConfig &processConfig);
    void WriteDfxActionMsg(uint32_t index, CapturerStage stage);
private:
    uint32_t index_{-1};
    AppInfo appInfo_{};
    std::unique_ptr<AudioCapturerDfxCollector> dfxCollector_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // ST_RECORDER_DFX_WRITER_H