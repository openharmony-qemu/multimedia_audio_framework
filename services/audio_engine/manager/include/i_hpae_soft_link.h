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
#ifndef I_HPAE_SOFT_LINK_H
#define I_HPAE_SOFT_LINK_H
#include "hpae_info.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

enum class SoftLinkMode : int32_t {
    HEARING_AID = 0,
    OFFLOADINNERCAP_AID = 1,
};

class IHpaeSoftLink {
public:
    virtual ~IHpaeSoftLink() = default;
    static std::shared_ptr<IHpaeSoftLink> CreateSoftLink(uint32_t sinkIdx, uint32_t sourceIdx, SoftLinkMode mode);
    virtual int32_t Start() = 0;
    virtual int32_t Stop() = 0;
    virtual int32_t Release() = 0;
    virtual int32_t SetVolume(float volume) = 0;
    virtual int32_t SetVolumeMute(bool isMute) = 0;
    virtual int32_t SetVolumeDuckFactor(float duckFactor) = 0;
    virtual int32_t SetVolumeLowPowerFactor(float lowPowerFactor) = 0;
    virtual int32_t SetLoudnessGain(float loudnessGain) = 0;
};
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS
#endif // I_HPAE_SOFT_LINK_H
