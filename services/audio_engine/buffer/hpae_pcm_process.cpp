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
#define LOG_TAG "HpaePcmProcess"
#endif

#include "hpae_pcm_process.h"
#include "simd_utils.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
HpaePcmProcess &HpaePcmProcess::operator = (const std::vector<float>& other)
{
    errNo_ = memcpy_s(pcmDataPtr_, sizeof(float) * size_, other.data(), sizeof(float) * other.size());
    CHECK_AND_RETURN_RET_LOG(errNo_ == 0, *this, "memcpy_s failed, errNo: %{public}d", errNo_);
    return *this;
}

HpaePcmProcess &HpaePcmProcess::operator = (const HpaePcmProcess& other)
{
    if (this != &other) {
        errNo_ = memcpy_s(pcmDataPtr_, sizeof(float) * size_, other.Begin(), sizeof(float) * other.Size());
        CHECK_AND_RETURN_RET_LOG(errNo_ == 0, *this, "memcpy_s failed, errNo: %{public}d", errNo_);
    }
    return *this;
}

HpaePcmProcess &HpaePcmProcess::operator+=(const HpaePcmProcess &other)
{
    float *curData = Begin();
    const float *otherData = other.Begin();
    SimdPointByPointAdd(size_, otherData, curData, curData);
    return *this;
}

HpaePcmProcess &HpaePcmProcess::operator-=(const HpaePcmProcess &other)
{
    float *curData = Begin();
    const float *otherData = other.Begin();
    SimdPointByPointSub(size_, curData, otherData, curData);
    return *this;
}

HpaePcmProcess &HpaePcmProcess::operator*=(const HpaePcmProcess &other)
{
    float *curData = Begin();
    const float *otherData = other.Begin();
    SimdPointByPointMul(size_, otherData, curData, curData);
    return *this;
}

void HpaePcmProcess::Reset()
{
    errNo_ = memset_s(Begin(), sizeof(float) * size_, 0, sizeof(float) * size_);
    CHECK_AND_RETURN_LOG(errNo_ == 0, "memcpy_s failed, errNo: %{public}d", errNo_);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS