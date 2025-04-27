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

#ifndef LINEAR_POS_TIME_MODEL_H
#define LINEAR_POS_TIME_MODEL_H

#include "stdint.h"
#include <vector>
#include <audio_info.h>

namespace OHOS {
namespace AudioStandard {
class LinearPosTimeModel {
public:
    LinearPosTimeModel();

    bool ConfigSampleRate(int32_t sampleRate);

    void ResetFrameStamp(uint64_t frame, int64_t nanoTime);

    CheckPosTimeRes UpdataFrameStamp(uint64_t frame, int64_t nanoTime);

    bool GetFrameStamp(uint64_t &frame, int64_t &nanoTime);

    void SetSpanCount(uint64_t spanCountInFrame);

    int64_t GetTimeOfPos(uint64_t posInFrame);

    virtual ~LinearPosTimeModel() = default;
private:
    bool IsReasonable(uint64_t frame, int64_t nanoTime);
    CheckPosTimeRes CheckReasonable(uint64_t frame, int64_t nanoTime);
    bool CheckPosTimeReasonable(std::pair<uint64_t, int64_t> &pre, std::pair<uint64_t, int64_t> &next);

private:
    bool isConfiged = false;
    int32_t sampleRate_ = 0;
    int64_t nanoTimePerFrame_ = 0;
    uint64_t spanCountInFrame_ = 0;

    uint64_t stampFrame_ = 0;
    int64_t stampNanoTime_ = 0;
    std::vector<std::pair<uint64_t, int64_t>> posTimeVec_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // LINEAR_POS_TIME_MODEL_H
