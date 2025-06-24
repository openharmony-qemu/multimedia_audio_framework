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
 
#ifndef AUDIO_WORKGROUP_CALLBACK_IMPL_H
#define AUDIO_WORKGROUP_CALLBACK_IMPL_H
 
#include "audio_workgroup_callback_stub.h"
#include "audio_system_manager.h"
 
namespace OHOS {
namespace AudioStandard {
class AudioWorkgroupCallbackImpl : public AudioWorkgroupCallbackStub {
public:
    AudioWorkgroupCallbackImpl();
    virtual ~AudioWorkgroupCallbackImpl();
    int32_t OnWorkgroupChange(const AudioWorkgroupChangeInfoIpc &info) override;
    void AddWorkgroupChangeCallback(std::shared_ptr<AudioWorkgroupChangeCallback> cb);
    void RemoveWorkgroupChangeCallback();
private:
    std::shared_ptr<AudioWorkgroupChangeCallback> workgroupCb_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
 
#endif // AUDIO_WORKGROUP_CALLBACK_IMPL_H
