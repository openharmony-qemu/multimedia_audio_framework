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

#ifndef HPAE_PCM_PROCESS_H
#define HPAE_PCM_PROCESS_H
#include <vector>
#include "securec.h"
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class HpaePcmProcess {
public:
    HpaePcmProcess(float *begin, size_t size) : pcmDataPtr_(begin), size_(size) {}
    
    float &operator[](size_t index)
    {
        return *(pcmDataPtr_ + index);
    }

    const float &operator[](size_t index) const
    {
        return *(pcmDataPtr_ + index);
    }

    size_t Size() const
    {
        return size_;
    }

    float *Begin()
    {
        return pcmDataPtr_;
    }

    float *End()
    {
        return pcmDataPtr_ + size_;
    }

    const float *Begin() const
    {
        return pcmDataPtr_;
    }

    const float *End() const
    {
        return pcmDataPtr_ + size_;
    }

    HpaePcmProcess(const HpaePcmProcess &other) = default;

    HpaePcmProcess &operator = (const HpaePcmProcess &other);

    HpaePcmProcess &operator = (const std::vector<float>& other);

    HpaePcmProcess &operator += (const HpaePcmProcess &other);

    HpaePcmProcess &operator -= (const HpaePcmProcess &other);

    HpaePcmProcess &operator *= (const HpaePcmProcess &other);

    void Reset() ;

    int32_t GetErrNo()
    {
        int32_t tmpErrNo = errNo_;
        errNo_ = 0;
        return tmpErrNo;
    }

private:
    int32_t errNo_ = 0;
    float* const pcmDataPtr_;
    const size_t size_;
};
}}}
#endif // HPAE_PCM_PROCESS_H