/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef AUDIO_STREAM_TYPES_H
#define AUDIO_STREAM_TYPES_H

#include "audio_stutter.h"
#include "audio_stream_info.h"

namespace OHOS {
namespace AudioStandard {
/**
 * @brief AudioRendererFilter is used for select speficed AudioRenderer.
 */
class AudioRendererFilter : public Parcelable {
public:
    AudioRendererFilter();
    virtual ~AudioRendererFilter();

    int32_t uid = -1;
    AudioRendererInfo rendererInfo = {};
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    int32_t streamId = -1;

    bool Marshalling(Parcel &parcel) const override;
    static AudioRendererFilter* Unmarshalling(Parcel &parcel);
};

/**
 * @brief AudioCapturerFilter is used for select speficed audiocapturer.
 */
class AudioCapturerFilter : public Parcelable {
public:
    AudioCapturerFilter();
    virtual ~AudioCapturerFilter();

    int32_t uid = -1;
    AudioCapturerInfo capturerInfo = {SOURCE_TYPE_INVALID, 0};

    bool Marshalling(Parcel &parcel) const override;
    static AudioCapturerFilter *Unmarshalling(Parcel &in);
};

class AudioParameterCallback {
public:
    virtual ~AudioParameterCallback() = default;
    /**
     * @brief AudioParameterCallback will be executed when parameter change.
     *
     * @param networkId networkId
     * @param key  Audio paramKey
     * @param condition condition
     * @param value value
     * @since 9
     */
    virtual void OnAudioParameterChange(const std::string networkId, const AudioParamKey key,
        const std::string& condition, const std::string& value) = 0;

    virtual void OnHdiRouteStateChange(const std::string &networkId, bool enable) = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_STREAM_TYPES_H
