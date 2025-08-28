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
#ifndef LOG_TAG
#define LOG_TAG "AudioUsrSelectManager"
#endif

#include "audio_usr_select_manager.h"
#include "audio_policy_log.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {

void AudioUsrSelectManager::SelectInputDeviceByUid(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor,
    int32_t uid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    audioUsrSelectMap_[uid].push_front(deviceDescriptor);
}

std::shared_ptr<AudioDeviceDescriptor> AudioUsrSelectManager::GetSelectedInputDeviceByUid(int32_t uid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AudioDeviceList descriptors = audioUsrSelectMap_[uid];
    std::shared_ptr<AudioDeviceDescriptor> descriptor;
    if (descriptors.size() > 0) {
        descriptor = descriptors.front();
    } else {
        descriptor = std::make_shared<AudioDeviceDescriptor>(AudioDeviceDescriptor::DEVICE_INFO);
    }
    return descriptor;
}

void AudioUsrSelectManager::ClearSelectedInputDeviceByUid(int32_t uid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    audioUsrSelectMap_[uid].clear();
}
} // namespace AudioStandard
} // namespace OHOS
