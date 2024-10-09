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

#ifndef OH_AUDIO_STREAM_BUILDER_H
#define OH_AUDIO_STREAM_BUILDER_H

#include <cstdint>
#include "native_audiostream_base.h"
#include "audio_interrupt_info.h"
#include "audio_info.h"
#include "OHAudioRenderer.h"

namespace OHOS {
namespace AudioStandard {
class OHAudioStreamBuilder {
public:
    explicit OHAudioStreamBuilder(const int32_t type);

    ~OHAudioStreamBuilder();

    OH_AudioStream_Result Generate(OH_AudioRenderer **renderer);
    OH_AudioStream_Result Generate(OH_AudioCapturer **capturer);

    OH_AudioStream_Result SetSamplingRate(int32_t rate);
    OH_AudioStream_Result SetChannelCount(int32_t channelCount);
    OH_AudioStream_Result SetSampleFormat(AudioSampleFormat sampleFormat);
    OH_AudioStream_Result SetEncodingType(AudioEncodingType encodingType);
    OH_AudioStream_Result SetPreferredFrameSize(int32_t frameSize);
    OH_AudioStream_Result SetLatencyMode(int32_t latencyMode);
    OH_AudioStream_Result SetChannelLayout(AudioChannelLayout channelLayout);

    OH_AudioStream_Result SetRendererInfo(StreamUsage usage);
    OH_AudioStream_Result SetRendererCallback(OH_AudioRenderer_Callbacks callbacks, void *userData);
    OH_AudioStream_Result SetRendererOutputDeviceChangeCallback(OH_AudioRenderer_OutputDeviceChangeCallback callback,
    void *userData);
    OH_AudioStream_Result SetRendererPrivacy(AudioPrivacyType privacyType);
    OH_AudioStream_Result SetWriteDataWithMetadataCallback(OH_AudioRenderer_WriteDataWithMetadataCallback callback,
        void *userData);
    OH_AudioStream_Result SetRendererWriteDataCallback(OH_AudioRenderer_OnWriteDataCallback callback,
        void* userData);

    OH_AudioStream_Result SetSourceType(SourceType type);
    OH_AudioStream_Result SetCapturerCallback(OH_AudioCapturer_Callbacks callbacks, void *userData);
    OH_AudioStream_Result SetInterruptMode(InterruptMode mode);

private:
    int32_t streamType_;
    int32_t latencyMode_ = 0; // default value is normal mode
    int32_t preferredFrameSize_ = -1; // undefined clientBufferSizeInFrame

    // stream params
    int32_t samplingRate_ = SAMPLE_RATE_48000;
    int32_t channelCount_ = STEREO;
    AudioEncodingType encodingType_ = ENCODING_PCM;
    AudioSampleFormat sampleFormat_ = SAMPLE_S16LE;
    AudioChannelLayout channelLayout_ = CH_LAYOUT_UNKNOWN;

    // renderer params
    StreamUsage usage_ = STREAM_USAGE_UNKNOWN;
    AudioPrivacyType privacyType_ = PRIVACY_TYPE_PUBLIC;

    // capturer params
    SourceType sourceType_ = SOURCE_TYPE_MIC;

    OH_AudioCapturer_Callbacks capturerCallbacks_ = {
        NULL
    };
    WriteDataCallbackType writeDataCallbackType_ = WRITE_DATA_CALLBACK_WITHOUT_RESULT;
    RendererCallback rendererCallbacks_ = {
        {nullptr},

        nullptr,

        nullptr
    };
    void *userData_ = nullptr;

    OH_AudioRenderer_OutputDeviceChangeCallback outputDeviceChangecallback_ = nullptr;
    void *outputDeviceChangeuserData_ = nullptr;
    InterruptMode interruptMode_ = SHARE_MODE;
    void *metadataUserData_ = nullptr;
};
}  // namespace AudioStandard
}  // namespace OHOS

#endif // OH_AUDIO_STREAM_BUILDER_H
