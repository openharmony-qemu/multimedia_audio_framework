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

#ifndef AUDIO_DEVICE_ADAPTER_IMPL_H
#define AUDIO_DEVICE_ADAPTER_IMPL_H

#include <map>
#include <mutex>

#include <v1_0/iaudio_manager.h>

#include "audio_info.h"
#include "audio_utils.h"
#include "i_audio_device_adapter.h"

namespace OHOS {
namespace AudioStandard {
struct DevicePortInfo {
    IAudioDeviceAdapterCallback *devAdpCb;
    uint32_t portId = 0;
};

class AudioDeviceAdapterImpl : public IAudioDeviceAdapter {
public:
    AudioDeviceAdapterImpl(const std::string &adapterName, sptr<IAudioAdapter> audioAdapter)
        : adapterName_(adapterName), audioAdapter_(audioAdapter) {}
    ~AudioDeviceAdapterImpl() = default;

    static int32_t ParamEventCallback(AudioExtParamKey key, const char *condition, const char *value, void *reserved,
        std::shared_ptr<AudioDeviceAdapterImpl> cookie);
    void SetParamCallback(std::shared_ptr<AudioDeviceAdapterImpl> callback);
    static std::shared_ptr<AudioDeviceAdapterImpl> GetParamCallback();

    int32_t Init() override;
    int32_t RegExtraParamObserver() override;
    int32_t CreateRender(const AudioDeviceDescriptor &devDesc, const AudioSampleAttributes &attr,
        sptr<IAudioRender> &audioRender, IAudioDeviceAdapterCallback *renderCb, uint32_t &renderId) override;
    void DestroyRender(sptr<IAudioRender> audioRender, uint32_t &renderId) override;
    int32_t CreateCapture(const AudioDeviceDescriptor &devDesc, const AudioSampleAttributes &attr,
        sptr<IAudioCapture> &audioCapture, IAudioDeviceAdapterCallback *captureCb, uint32_t &captureId) override;
    void DestroyCapture(sptr<IAudioCapture> audioCapture, uint32_t &captureId) override;
    void SetAudioParameter(const AudioParamKey key, const std::string &condition, const std::string &value) override;
    std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) override;
    int32_t UpdateAudioRoute(const AudioRoute &route) override;
    int32_t Release() override;

private:
    static int32_t HandleStateChangeEvent(std::shared_ptr<AudioDeviceAdapterImpl> devAdapter, const AudioParamKey key,
        const char *condition, const char *value);
    static int32_t HandleRenderParamEvent(std::shared_ptr<AudioDeviceAdapterImpl> devAdapter,
        const AudioParamKey audioKey, const char *condition, const char *value);
    static int32_t HandleCaptureParamEvent(std::shared_ptr<AudioDeviceAdapterImpl> devAdapter,
        const AudioParamKey audioKey, const char *condition, const char *value);
    size_t GetRenderPortsNum();
    size_t GetCapturePortsNum();

private:
    static constexpr uint32_t REMOTE_OUTPUT_STREAM_ID = 29; // 13 + 2 * 8
    static constexpr int32_t INVALID_ROUT_HANDLE = -1;
    static constexpr int32_t EVENT_DES_SIZE = 60;
    static constexpr int32_t ADAPTER_STATE_CONTENT_DES_SIZE = 60;
    static constexpr int32_t PARAMS_STATE_NUM = 2;
    static constexpr char DAUDIO_DEV_TYPE_SPK = '1';
    static constexpr char DAUDIO_DEV_TYPE_MIC = '2';

    std::string adapterName_;
    sptr<IAudioAdapter> audioAdapter_;
    int32_t routeHandle_ = INVALID_ROUT_HANDLE;
#ifdef FEATURE_DISTRIBUTE_AUDIO
    std::mutex regParamCbMtx_;
    std::atomic<bool> isParamCbReg_ = false;
#endif
    std::mutex renderPortsMtx_;
    std::mutex capturePortsMtx_;
    std::map<sptr<IAudioRender>, DevicePortInfo> renderPorts_;
    std::map<sptr<IAudioCapture>, DevicePortInfo> capturePorts_;
    sptr<IAudioCallback> callbackStub_ = nullptr;
    static std::shared_ptr<AudioDeviceAdapterImpl> paramCallback_;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_DEVICE_ADAPTER_IMPL_H