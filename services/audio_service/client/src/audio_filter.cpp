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

sptr<AudioRendererFilter> AudioRendererFilter::Unmarshalling(Parcel &in)
{
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    if (audioRendererFilter == nullptr) {
        return nullptr;
    }

    audioRendererFilter->uid = in.ReadInt32();
    audioRendererFilter->rendererInfo.contentType = static_cast<ContentType>(in.ReadInt32());
    audioRendererFilter->rendererInfo.streamUsage = static_cast<StreamUsage>(in.ReadInt32());
    audioRendererFilter->streamType = static_cast<AudioStreamType>(in.ReadInt32());
    audioRendererFilter->rendererInfo.rendererFlags = in.ReadInt32();
    audioRendererFilter->streamId = in.ReadInt32();

    return audioRendererFilter;
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

sptr<AudioCapturerFilter> AudioCapturerFilter::Unmarshalling(Parcel &in)
{
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    CHECK_AND_RETURN_RET(audioCapturerFilter != nullptr, nullptr);

    audioCapturerFilter->uid = in.ReadInt32();
    audioCapturerFilter->capturerInfo.sourceType = static_cast<SourceType>(in.ReadInt32());
    audioCapturerFilter->capturerInfo.capturerFlags = in.ReadInt32();

    return audioCapturerFilter;
}
} // namespace AudioStandard
} // namespace OHOS
