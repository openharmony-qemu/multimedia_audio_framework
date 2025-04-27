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

#ifndef ST_AUDIO_CAPTURER_DFX_COLLECTOR_H
#define ST_AUDIO_CAPTURER_DFX_COLLECTOR_H

#include <map>

#include "dfx_collector.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {

struct RecordStat {
    int64_t recordDuration_{};
};

class AudioCapturerDfxCollector : public DfxCollector<CapturerDfxInfo> {
public:
    void FlushDfxMsg(uint32_t index, int32_t appUid) override;

    AppInfo appInfo_{};
};

class CapturerDfxBuilder {
public:
    CapturerDfxBuilder &WriteActionMsg(uint32_t dfxIndex, CapturerStage stage);
    CapturerDfxBuilder &WriteInfoMsg(const AudioCapturerInfo &capturerInfo);
    CapturerDfxBuilder &WriteStatMsg(const AudioProcessConfig &processConfig, const RecordStat &stat);
    CapturerDfxInfo GetResult();
private:
    CapturerDfxInfo dfxInfo_{};
};

} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_CAPTURER_DFX_COLLECTOR_H