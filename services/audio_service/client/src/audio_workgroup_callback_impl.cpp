/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioWorkgroupCallbackStub"
#endif
 
#include "audio_workgroup_callback_impl.h"
#include "audio_common_log.h"
#include "audio_errors.h"
 
namespace OHOS {
namespace AudioStandard {
 
AudioWorkgroupCallbackImpl::AudioWorkgroupCallbackImpl()
{
    AUDIO_DEBUG_LOG("AudioWorkgroupCallbackImpl ctor");
}
 
AudioWorkgroupCallbackImpl::~AudioWorkgroupCallbackImpl()
{
    AUDIO_DEBUG_LOG("~AudioWorkgroupCallbackImpl dtor");
}

void AudioWorkgroupCallbackImpl::AddWorkgroupChangeCallback(
    std::shared_ptr<AudioWorkgroupChangeCallback> cb)
{
    workgroupCb_ = cb;
}

void AudioWorkgroupCallbackImpl::RemoveWorkgroupChangeCallback()
{
    workgroupCb_ = nullptr;
}

int32_t AudioWorkgroupCallbackImpl::OnWorkgroupChange(
    const AudioWorkgroupChangeInfoIpc &info)
{
    if (workgroupCb_ == nullptr) {
        return ERROR;
    }
    workgroupCb_->OnWorkgroupChange(info.changeInfo);
    return SUCCESS;
}

} // namespace AudioStandard
} // namespace OHOS