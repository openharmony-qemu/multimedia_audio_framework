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
#ifndef LOG_TAG
#define LOG_TAG "AudioEffectHdiParam"
#endif

#include "audio_effect.h"
#include "audio_effect_hdi_param.h"
#include "audio_errors.h"
#include "audio_effect_log.h"
#include "securec.h"

namespace OHOS {
namespace AudioStandard {
AudioEffectHdiParam::AudioEffectHdiParam()
{
    AUDIO_DEBUG_LOG("constructor.");
    DeviceTypeToHdiControlMap_.clear();
    int32_t ret = memset_s(static_cast<void *>(input_), sizeof(input_), 0, sizeof(input_));
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("hdi constructor memset input failed");
    }
    ret = memset_s(static_cast<void *>(output_), sizeof(output_), 0, sizeof(output_));
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("hdi constructor memset output failed");
    }
    replyLen_ = GET_HDI_BUFFER_LEN;
    hdiModel_ = nullptr;
}

AudioEffectHdiParam::~AudioEffectHdiParam()
{
    AUDIO_DEBUG_LOG("destructor!");
}

void AudioEffectHdiParam::CreateHdiControl()
{
    for (const auto &item : HDI_EFFECT_LIB_MAP) {
        libName_ = item.second[0];
        effectId_ = item.second[1];
        EffectInfo info = {
            .libName = &libName_[0],
            .effectId = &effectId_[0],
            .ioDirection = 1,
        };
        ControllerId controllerId;
        IEffectControl *hdiControl = nullptr;
        int32_t ret = hdiModel_->CreateEffectController(hdiModel_, &info, &hdiControl, &controllerId);
        if ((ret != SUCCESS) || (hdiControl == nullptr)) {
            AUDIO_WARNING_LOG("hdi init failed");
        } else {
            DeviceTypeToHdiControlMap_.emplace(item.first, hdiControl);
        }
    }
    return;
}

void AudioEffectHdiParam::InitHdi()
{
    hdiModel_ = IEffectModelGet(false);
    if (hdiModel_ == nullptr) {
        AUDIO_ERR_LOG("IEffectModelGet failed");
        return;
    }

    CreateHdiControl();
}

int32_t AudioEffectHdiParam::SetHdiCommand(IEffectControl *hdiControl, int8_t *effectHdiInput)
{
    int32_t ret = memcpy_s(static_cast<void *>(input_), sizeof(input_),
        static_cast<void *>(effectHdiInput), sizeof(input_));
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("hdi memcpy failed");
    }
    uint32_t replyLen = GET_HDI_BUFFER_LEN;
    ret = hdiControl->SendCommand(hdiControl, HDI_SET_PATAM, input_, SEND_HDI_COMMAND_LEN,
        output_, &replyLen);
    return ret;
}

int32_t AudioEffectHdiParam::UpdateHdiState(int8_t *effectHdiInput)
{
    if (hdiModel_ == nullptr) {
        return ERROR;
    }
    int32_t ret = ERROR;
    for (const auto &item : DeviceTypeToHdiControlMap_) {
        IEffectControl *hdiControl = item.second;
        if (hdiControl == nullptr) {
            AUDIO_WARNING_LOG("hdiControl is nullptr.");
            continue;
        }
        ret = SetHdiCommand(hdiControl, effectHdiInput);
        CHECK_AND_CONTINUE_LOG(ret == 0, "hdi send command failed");
    }
    return ret;
}

int32_t AudioEffectHdiParam::UpdateHdiState(int8_t *effectHdiInput, DeviceType deviceType)
{
    if (hdiModel_ == nullptr) {
        return ERROR;
    }
    IEffectControl *hdiControl = DeviceTypeToHdiControlMap_[deviceType];
    if (hdiControl == nullptr) {
        AUDIO_WARNING_LOG("hdiControl is nullptr.");
        return ERROR;
    }
    int32_t ret = SetHdiCommand(hdiControl, effectHdiInput);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "hdi send command failed");
    return ret;
}
}  // namespace AudioStandard
}  // namespace OHOS