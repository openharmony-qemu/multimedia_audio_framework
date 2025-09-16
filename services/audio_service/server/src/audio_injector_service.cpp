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
#include "audio_injector_service.h"

namespace OHOS {
namespace AudioStandard {
AudioInjectorService::AudioInjectorService()
{
}

int32_t AudioInjectorService::UpdateAudioInfo(AudioModuleInfo &info)
{
    return 0;
}

int32_t AudioInjectorService::PeekAudioData(const uint32_t sinkPortIndex, uint8_t *buffer, const size_t bufferSize,
    AudioStreamInfo &streamInfo)
{
    return 0;
}

void AudioInjectorService::SetSinkPortIdx(uint32_t sinkPortIdx)
{
    sinkPortIndex_ = sinkPortIdx;
}

uint32_t AudioInjectorService::GetSinkPortIdx()
{
    return sinkPortIndex_;
}

AudioModuleInfo &AudioInjectorService::GetModuleInfo()
{
    return moduleInfo_;
}
}  //  namespace AudioStandard
}  //  namespace OHOS