/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioGroupHandle"
#endif

#include "audio_policy_log.h"
#include "audio_group_handle.h"


namespace OHOS {
namespace AudioStandard {
AudioGroupHandle::~AudioGroupHandle()
{
    AUDIO_DEBUG_LOG("~AudioGroupHandle()");
}

int32_t AudioGroupHandle::GetNextId(GroupType type)
{
    CheckId(type);
    if (type == GroupType::VOLUME_TYPE) {
        return ++currentVolumeId_;
    } else {
        return ++currentInterruptId_;
    }
}

void AudioGroupHandle::CheckId(GroupType type)
{
    if (type == GroupType::VOLUME_TYPE && currentVolumeId_ == MAX_ID) {
        currentVolumeId_ = 0;
    } else if (type == GroupType::INTERRUPT_TYPE && currentInterruptId_ == MAX_ID) {
        currentInterruptId_ = 0;
    }
}
} // namespace AudioStandard
} // namespace OHOS
