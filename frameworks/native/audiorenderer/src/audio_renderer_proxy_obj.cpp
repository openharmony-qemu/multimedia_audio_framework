/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "audio_renderer_proxy_obj.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
void AudioRendererProxyObj::SaveRendererObj(std::weak_ptr<AudioRenderer> rendererObj)
{
    std::lock_guard<std::mutex> lock(mutex_);
    renderer_ = rendererObj;
}

void AudioRendererProxyObj::UnsetRendererObj()
{
    std::lock_guard<std::mutex> lock(mutex_);
    renderer_.reset();
}

void AudioRendererProxyObj::MuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal)
{
    std::unique_lock<std::mutex> lock(mutex_);
    std::shared_ptr<AudioRenderer> renderer = renderer_.lock();
    lock.unlock();
    if (renderer != nullptr) {
        renderer->Mute(CMD_FROM_SYSTEM);
    }
}

void AudioRendererProxyObj::UnmuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal)
{
    std::unique_lock<std::mutex> lock(mutex_);
    std::shared_ptr<AudioRenderer> renderer = renderer_.lock();
    lock.unlock();
    if (renderer != nullptr) {
        renderer->Unmute(CMD_FROM_SYSTEM);
    }
}

void AudioRendererProxyObj::PausedStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal)
{
    std::unique_lock<std::mutex> lock(mutex_);
    std::shared_ptr<AudioRenderer> renderer = renderer_.lock();
    lock.unlock();
    if (renderer != nullptr) {
        renderer->Pause(CMD_FROM_SYSTEM);
    }
}

void AudioRendererProxyObj::ResumeStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal)
{
    std::unique_lock<std::mutex> lock(mutex_);
    std::shared_ptr<AudioRenderer> renderer = renderer_.lock();
    lock.unlock();
    if (renderer != nullptr) {
        renderer->Start(CMD_FROM_SYSTEM);
    }
}

void AudioRendererProxyObj::SetLowPowerVolumeImpl(float volume)
{
    std::unique_lock<std::mutex> lock(mutex_);
    std::shared_ptr<AudioRenderer> renderer = renderer_.lock();
    lock.unlock();
    if (renderer != nullptr) {
        renderer->SetLowPowerVolume(volume);
    }
}

void AudioRendererProxyObj::GetLowPowerVolumeImpl(float &volume)
{
    std::unique_lock<std::mutex> lock(mutex_);
    std::shared_ptr<AudioRenderer> renderer = renderer_.lock();
    lock.unlock();
    if (renderer != nullptr) {
        volume = renderer->GetLowPowerVolume();
    }
}

void AudioRendererProxyObj::SetOffloadModeImpl(int32_t state, bool isAppBack)
{
    std::unique_lock<std::mutex> lock(mutex_);
    std::shared_ptr<AudioRenderer> renderer = renderer_.lock();
    lock.unlock();
    if (renderer != nullptr) {
        renderer->SetOffloadMode(state, isAppBack);
    }
}

void AudioRendererProxyObj::UnsetOffloadModeImpl()
{
    std::unique_lock<std::mutex> lock(mutex_);
    std::shared_ptr<AudioRenderer> renderer = renderer_.lock();
    lock.unlock();
    if (renderer != nullptr) {
        renderer->UnsetOffloadMode();
    }
}

void AudioRendererProxyObj::GetSingleStreamVolumeImpl(float &volume)
{
    std::unique_lock<std::mutex> lock(mutex_);
    std::shared_ptr<AudioRenderer> renderer = renderer_.lock();
    lock.unlock();
    if (renderer != nullptr) {
        volume = renderer->GetSingleStreamVolume();
    }
}
} // namespace AudioStandard
} // namespace OHOS
