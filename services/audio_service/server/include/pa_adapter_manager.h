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

#ifndef PA_ADAPTER_MANAGER_H
#define PA_ADAPTER_MANAGER_H

#include <map>
#include <mutex>
#include <pulse/pulseaudio.h>
#include <pulse/thread-mainloop.h>
#include "audio_timer.h"
#include "i_stream_manager.h"
#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {

static std::map<uint8_t, AudioChannelLayout> defaultChCountToLayoutMap = {
    {1, CH_LAYOUT_MONO}, {2, CH_LAYOUT_STEREO}, {3, CH_LAYOUT_SURROUND},
    {4, CH_LAYOUT_2POINT0POINT2}, {5, CH_LAYOUT_5POINT0_BACK}, {6, CH_LAYOUT_5POINT1},
    {7, CH_LAYOUT_6POINT1_BACK}, {8, CH_LAYOUT_5POINT1POINT2}, {9, CH_LAYOUT_HOA_ORDER2_ACN_N3D},
    {10, CH_LAYOUT_7POINT1POINT2}, {12, CH_LAYOUT_7POINT1POINT4}, {14, CH_LAYOUT_9POINT1POINT4},
    {16, CH_LAYOUT_9POINT1POINT6}
};

static std::map<AudioChannelSet, pa_channel_position> chSetToPaPositionMap = {
    {FRONT_LEFT, PA_CHANNEL_POSITION_FRONT_LEFT}, {FRONT_RIGHT, PA_CHANNEL_POSITION_FRONT_RIGHT},
    {FRONT_CENTER, PA_CHANNEL_POSITION_FRONT_CENTER}, {LOW_FREQUENCY, PA_CHANNEL_POSITION_LFE},
    {SIDE_LEFT, PA_CHANNEL_POSITION_SIDE_LEFT}, {SIDE_RIGHT, PA_CHANNEL_POSITION_SIDE_RIGHT},
    {BACK_LEFT, PA_CHANNEL_POSITION_REAR_LEFT}, {BACK_RIGHT, PA_CHANNEL_POSITION_REAR_RIGHT},
    {FRONT_LEFT_OF_CENTER, PA_CHANNEL_POSITION_FRONT_LEFT_OF_CENTER},
    {FRONT_RIGHT_OF_CENTER, PA_CHANNEL_POSITION_FRONT_RIGHT_OF_CENTER},
    {BACK_CENTER, PA_CHANNEL_POSITION_REAR_CENTER}, {TOP_CENTER, PA_CHANNEL_POSITION_TOP_CENTER},
    {TOP_FRONT_LEFT, PA_CHANNEL_POSITION_TOP_FRONT_LEFT}, {TOP_FRONT_CENTER, PA_CHANNEL_POSITION_TOP_FRONT_CENTER},
    {TOP_FRONT_RIGHT, PA_CHANNEL_POSITION_TOP_FRONT_RIGHT}, {TOP_BACK_LEFT, PA_CHANNEL_POSITION_TOP_REAR_LEFT},
    {TOP_BACK_CENTER, PA_CHANNEL_POSITION_TOP_REAR_CENTER}, {TOP_BACK_RIGHT, PA_CHANNEL_POSITION_TOP_REAR_RIGHT},
    /** Channel layout positions below do not have precise mapped pulseaudio positions */
    {STEREO_LEFT, PA_CHANNEL_POSITION_FRONT_LEFT}, {STEREO_RIGHT, PA_CHANNEL_POSITION_FRONT_RIGHT},
    {WIDE_LEFT, PA_CHANNEL_POSITION_FRONT_LEFT}, {WIDE_RIGHT, PA_CHANNEL_POSITION_FRONT_RIGHT},
    {SURROUND_DIRECT_LEFT, PA_CHANNEL_POSITION_SIDE_LEFT}, {SURROUND_DIRECT_RIGHT, PA_CHANNEL_POSITION_SIDE_LEFT},
    {BOTTOM_FRONT_CENTER, PA_CHANNEL_POSITION_FRONT_CENTER},
    {BOTTOM_FRONT_LEFT, PA_CHANNEL_POSITION_FRONT_LEFT}, {BOTTOM_FRONT_RIGHT, PA_CHANNEL_POSITION_FRONT_RIGHT},
    {TOP_SIDE_LEFT, PA_CHANNEL_POSITION_TOP_REAR_LEFT}, {TOP_SIDE_RIGHT, PA_CHANNEL_POSITION_TOP_REAR_RIGHT},
    {LOW_FREQUENCY_2, PA_CHANNEL_POSITION_LFE},
};

class PaAdapterManager : public IStreamManager {
public:
    PaAdapterManager(ManagerType type);

    int32_t CreateRender(AudioProcessConfig processConfig, std::shared_ptr<IRendererStream> &stream) override;
    int32_t ReleaseRender(uint32_t streamIndex_) override;
    int32_t StartRender(uint32_t streamIndex) override;
    int32_t StopRender(uint32_t streamIndex) override;
    int32_t PauseRender(uint32_t streamIndex) override;
    int32_t GetStreamCount() const noexcept override;
    int32_t TriggerStartIfNecessary() override;
    int32_t CreateCapturer(AudioProcessConfig processConfig, std::shared_ptr<ICapturerStream> &stream) override;
    int32_t ReleaseCapturer(uint32_t streamIndex_) override;
    uint32_t ConvertChLayoutToPaChMap(const uint64_t &channelLayout, pa_channel_map &paMap);
    uint64_t GetLatency() noexcept override;

private:
    // audio channel index
    static const uint8_t CHANNEL1_IDX = 0;
    static const uint8_t CHANNEL2_IDX = 1;
    static const uint8_t CHANNEL3_IDX = 2;
    static const uint8_t CHANNEL4_IDX = 3;
    static const uint8_t CHANNEL5_IDX = 4;
    static const uint8_t CHANNEL6_IDX = 5;
    static const uint8_t CHANNEL7_IDX = 6;
    static const uint8_t CHANNEL8_IDX = 7;

    int32_t ResetPaContext();
    int32_t InitPaContext();
    int32_t HandleMainLoopStart();
    pa_stream *InitPaStream(AudioProcessConfig processConfig, uint32_t sessionId, bool isRecording);
    bool IsEffectNone(StreamUsage streamUsage);
    int32_t SetPaProplist(pa_proplist *propList, pa_channel_map &map, AudioProcessConfig &processConfig,
        const std::string &streamName, uint32_t sessionId);
    std::shared_ptr<IRendererStream> CreateRendererStream(AudioProcessConfig processConfig, pa_stream *paStream);
    std::shared_ptr<ICapturerStream> CreateCapturerStream(AudioProcessConfig processConfig, pa_stream *paStream);
    int32_t ConnectStreamToPA(pa_stream *paStream, pa_sample_spec sampleSpec,
        SourceType source, const std::string &deviceName = "");
    void ReleasePaStream(pa_stream *paStream);
    int32_t ConnectRendererStreamToPA(pa_stream *paStream, pa_sample_spec sampleSpec);
    int32_t ConnectCapturerStreamToPA(pa_stream *paStream, pa_sample_spec sampleSpec,
        SourceType source, const std::string &deviceName);

    int32_t SetStreamAudioEnhanceMode(pa_stream *paStream, AudioEffectMode audioEnhanceMode);
    const std::string GetEnhanceModeName(AudioEffectMode audioEnhanceMode);
    const std::string GetEnhanceSceneName(SourceType sourceType);

    // Callbacks to be implemented
    static void PAStreamStateCb(pa_stream *stream, void *userdata);
    static void PAContextStateCb(pa_context *context, void *userdata);

    static void PAStreamUpdateStreamIndexSuccessCb(pa_stream *stream, int32_t success, void *userdata);

    const std::string GetStreamName(AudioStreamType audioType);
    pa_sample_spec ConvertToPAAudioParams(AudioProcessConfig processConfig);

    int32_t GetDeviceNameForConnect(AudioProcessConfig processConfig,
        uint32_t sessionId, std::string &deviceName);

    void SetHighResolution(pa_proplist *propList, AudioProcessConfig &processConfig, uint32_t sessionId);
    bool CheckHighResolution(const AudioProcessConfig &processConfig);
    void SetRecordProplist(pa_proplist *propList, AudioProcessConfig &processConfig);

    std::mutex paElementsMutex_;
    pa_threaded_mainloop *mainLoop_;
    pa_mainloop_api *api_;
    pa_context *context_;
    std::mutex streamMapMutex_;
    std::map<int32_t, std::shared_ptr<IRendererStream>> rendererStreamMap_;
    std::map<int32_t, std::shared_ptr<ICapturerStream>> capturerStreamMap_;
    bool isContextConnected_;
    bool isMainLoopStarted_;
    ManagerType managerType_ = PLAYBACK;
    bool waitConnect_ = true;
    AudioEffectMode enhanceMode_ = EFFECT_NONE;
    uint32_t highResolutionIndex_ = 0;
    bool isHighResolutionExist_ = false;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // PA_ADAPTER_MANAGER_H
