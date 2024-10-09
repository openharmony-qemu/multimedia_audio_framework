/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef AUDIO_EFFECT_HDI_PARAM_H
#define AUDIO_EFFECT_HDI_PARAM_H

#include <mutex>
#include "v1_0/ieffect_model.h"

const uint32_t SEND_HDI_COMMAND_LEN = 20;

namespace OHOS {
namespace AudioStandard {
class AudioEffectHdiParam {
public:
    AudioEffectHdiParam();
    ~AudioEffectHdiParam();
    void InitHdi();
    int32_t UpdateHdiState(int8_t *effectHdiInput);
    int32_t UpdateHdiState(int8_t *effectHdiInput, DeviceType deviceType);
private:
    static const uint32_t GET_HDI_BUFFER_LEN = 10;
    void CreateHdiControl();
    int8_t input_[SEND_HDI_COMMAND_LEN];
    int8_t output_[GET_HDI_BUFFER_LEN];
    uint32_t replyLen_;
    std::string libName_;
    std::string effectId_;
    IEffectModel *hdiModel_;
    std::map<DeviceType, IEffectControl *> DeviceTypeToHdiControlMap_;
    int32_t SetHdiCommand(IEffectControl *hdiControl, int8_t *effectHdiInput);
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_EFFECT_HDI_PARAM_H