/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "audio_policy_interface.h"
#include "audio_service_log.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
AudioRendererFilter::AudioRendererFilter()
{}

AudioRendererFilter::~AudioRendererFilter()
{}

bool AudioRendererFilter::Marshalling(Parcel &parcel) const
{
    return parcel.WriteInt32(uid) &&
        parcel.WriteInt32(static_cast<int32_t>(rendererInfo.contentType)) &&
        parcel.WriteInt32(static_cast<int32_t>(rendererInfo.streamUsage)) &&
        parcel.WriteInt32(static_cast<int32_t>(streamType)) &&
        parcel.WriteInt32(rendererInfo.rendererFlags) &&
        parcel.WriteInt32(streamId);
}

AudioRendererFilter *AudioRendererFilter::Unmarshalling(Parcel &parcel)
{
    auto info = new(std::nothrow) AudioRendererFilter();
    if (info == nullptr) {
        return nullptr;
    }
    info->uid = parcel.ReadInt32();
    info->rendererInfo.contentType = static_cast<ContentType>(parcel.ReadInt32());
    info->rendererInfo.streamUsage = static_cast<StreamUsage>(parcel.ReadInt32());
    info->streamType = static_cast<AudioStreamType>(parcel.ReadInt32());
    info->rendererInfo.rendererFlags = parcel.ReadInt32();
    info->streamId = parcel.ReadInt32();
    return info;
}

AudioCapturerFilter::AudioCapturerFilter()
{}

AudioCapturerFilter::~AudioCapturerFilter()
{}

bool AudioCapturerFilter::Marshalling(Parcel &parcel) const
{
    return parcel.WriteInt32(uid) &&
        parcel.WriteInt32(static_cast<int32_t>(capturerInfo.sourceType)) &&
        parcel.WriteInt32(capturerInfo.capturerFlags);
}

AudioCapturerFilter *AudioCapturerFilter::Unmarshalling(Parcel &in)
{
    auto audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    CHECK_AND_RETURN_RET(audioCapturerFilter != nullptr, nullptr);

    audioCapturerFilter->uid = in.ReadInt32();
    audioCapturerFilter->capturerInfo.sourceType = static_cast<SourceType>(in.ReadInt32());
    audioCapturerFilter->capturerInfo.capturerFlags = in.ReadInt32();

    return audioCapturerFilter;
}
} // namespace AudioStandard
} // namespace OHOS
