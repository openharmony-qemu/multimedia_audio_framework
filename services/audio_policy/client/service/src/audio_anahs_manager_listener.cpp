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
#define LOG_TAG "AudioAnahsManagerListener"
#endif

#include "audio_anahs_manager_listener.h"

#include "audio_errors.h"
#include "audio_policy_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

AudioAnahsManagerListener::AudioAnahsManagerListener()
{
}

AudioAnahsManagerListener::~AudioAnahsManagerListener()
{
}

void AudioAnahsManagerListener::SetAudioDeviceAnahsCallback(const std::weak_ptr<AudioDeviceAnahs> &callback)
{
    audioDeviceAnahsCallback_ = callback;
    std::shared_ptr<AudioDeviceAnahs> audioDeviceAnahsCallback = audioDeviceAnahsCallback_.lock();
    CHECK_AND_RETURN_LOG(audioDeviceAnahsCallback != nullptr, "audioDeviceAnahsCallback_ is nullptr");
}

int32_t AudioAnahsManagerListener::OnExtPnpDeviceStatusChanged(const std::string &anahsStatus,
    const std::string &anahsShowType)
{
    std::shared_ptr<AudioDeviceAnahs> audioDeviceAnahsCallback = audioDeviceAnahsCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(audioDeviceAnahsCallback != nullptr, ERR_CALLBACK_NOT_REGISTERED,
        "audioDeviceAnahsCallback_ is nullptr");
    return audioDeviceAnahsCallback->OnExtPnpDeviceStatusChanged(anahsStatus, anahsShowType);
}

} // namespace AudioStandard
} // namespace OHOS
