/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef ST_PULSEAUDIO_AUDIO_SERVICE_ADAPTER_H
#define ST_PULSEAUDIO_AUDIO_SERVICE_ADAPTER_H
#include <mutex>
#include "safe_map.h"

#include <pulse/pulseaudio.h>
#include <vector>

#include "audio_service_adapter.h"

namespace OHOS {
namespace AudioStandard {
class PulseAudioServiceAdapterImpl : public AudioServiceAdapter {
public:
    explicit PulseAudioServiceAdapterImpl(std::unique_ptr<AudioServiceAdapterCallback> &cb);
    ~PulseAudioServiceAdapterImpl();

    bool Connect() override;
    uint32_t OpenAudioPort(std::string audioPortName, std::string moduleArgs) override;
    int32_t CloseAudioPort(int32_t audioHandleIndex) override;
    int32_t SetDefaultSink(std::string name) override;
    int32_t SetDefaultSource(std::string name) override;
    int32_t SetSourceOutputMute(int32_t uid, bool setMute) override;
    int32_t SuspendAudioDevice(std::string &audioPortName, bool isSuspend) override;
    bool SetSinkMute(const std::string &sinkName, bool isMute, bool isSync = false) override;
    std::vector<SinkInput> GetAllSinkInputs() override;
    std::vector<SourceOutput> GetAllSourceOutputs() override;
    void Disconnect() override;

    std::vector<uint32_t> GetTargetSinks(std::string adapterName) override;
    std::vector<SinkInfo> GetAllSinks() override;
    int32_t SetLocalDefaultSink(std::string name) override;
    int32_t MoveSinkInputByIndexOrName(uint32_t sinkInputId, uint32_t sinkIndex, std::string sinkName) override;
    int32_t MoveSourceOutputByIndexOrName(uint32_t sourceOutputId,
        uint32_t sourceIndex, std::string sourceName) override;

    // Static Member functions
    static void PaGetSinksCb(pa_context *c, const pa_sink_info *i, int eol, void *userdata);
    static void PaMoveSinkInputCb(pa_context *c, int success, void *userdata);
    static void PaMoveSourceOutputCb(pa_context *c, int success, void *userdata);
    static void PaContextStateCb(pa_context *c, void *userdata);
    static void PaModuleLoadCb(pa_context *c, uint32_t idx, void *userdata);
    static void PaSubscribeCb(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *userdata);
    static void PaGetAllSinkInputsCb(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata);
    static void PaGetAllSourceOutputsCb(pa_context *c, const pa_source_output_info *i, int eol, void *userdata);
    static void PaGetSourceOutputNoSignalCb(pa_context *c, const pa_source_output_info *i, int eol, void *userdata);
    static void ProcessSourceOutputEvent(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *userdata);
    static void PaSinkMuteCb(pa_context *c, int success, void *userdata);
private:
    struct UserData {
        PulseAudioServiceAdapterImpl *thiz;
        AudioStreamType streamType;
        float volume;
        bool mute;
        bool isCorked;
        uint32_t idx;
        std::vector<SinkInput> sinkInputList;
        std::vector<SourceOutput> sourceOutputList;
        std::vector<SinkInfo> sinkInfos;
        int32_t moveResult;
        bool isSubscribingCb = false;
    };

    bool ConnectToPulseAudio();
    AudioStreamType GetIdByStreamType(std::string streamType);

    static constexpr uint32_t PA_CONNECT_RETRY_SLEEP_IN_MICRO_SECONDS = 500000;
    pa_context *mContext = NULL;
    pa_threaded_mainloop *mMainLoop = NULL;
    static SafeMap<uint32_t, uint32_t> sinkIndexSessionIDMap;
    static SafeMap<uint32_t, uint32_t> sourceIndexSessionIDMap;
    std::mutex lock_;
    bool isSetDefaultSink_ = false;
    bool isSetDefaultSource_ = false;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // ST_PULSEAUDIO_AUDIO_SERVICE_ADAPTER_H
