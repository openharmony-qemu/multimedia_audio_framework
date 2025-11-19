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

#ifndef ST_PRO_AUDIO_SERVICE_ADAPTER_H
#define ST_PRO_AUDIO_SERVICE_ADAPTER_H
#include <mutex>
#include "safe_map.h"
#include <vector>
#include "audio_service_adapter.h"
#include "audio_module_info.h"
#include "audio_service_hpae_callback.h"

namespace OHOS {
namespace AudioStandard {

class ProAudioServiceAdapterImpl : public AudioServiceAdapter, public AudioServiceHpaeCallback,
                                   public std::enable_shared_from_this<ProAudioServiceAdapterImpl> {
public:
    explicit ProAudioServiceAdapterImpl(std::unique_ptr<AudioServiceAdapterCallback> &cb);
    ~ProAudioServiceAdapterImpl();

    bool Connect() override;
    uint32_t OpenAudioPort(std::string audioPortName, std::string moduleArgs) override;
    int32_t OpenAudioPort(std::string audioPortName,  const AudioModuleInfo& audioModuleInfo) override;
    int32_t ReloadAudioPort(const std::string &audioPortName, const AudioModuleInfo& audioModuleInfo) override;
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
    int32_t MoveSourceOutputByIndexOrName(
        uint32_t sourceOutputId, uint32_t sourceIndex, std::string sourceName) override;
    int32_t GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray) override;
    int32_t GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray) override;
    int32_t GetAudioEnhanceProperty(AudioEffectPropertyArrayV3 &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE) override;
    int32_t GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE) override;
    int32_t UpdateCollaborativeState(bool isCollaborationEnabled) override;
    int32_t SetAbsVolumeStateToEffect(const bool absVolumeState) override;
    int32_t SetSystemVolumeToEffect(AudioStreamType streamType, float volume) override;
    void AddCaptureInjector(const uint32_t &sinkPortIndex, const uint32_t &sourcePortIndex,
        const SourceType &sourceType) override;
    void RemoveCaptureInjector(const uint32_t &sinkPortIndex, const uint32_t &sourcePortIndex,
        const SourceType &sourceType) override;
    void UpdateAudioPortInfo(const uint32_t &sinkPortIndex, const AudioModuleInfo &audioPortInfo) override;
    // callback Member functions
    virtual void OnOpenAudioPortCb(int32_t portId) override;
    virtual void OnReloadAudioPortCb(int32_t portId) override;
    virtual void OnCloseAudioPortCb(int32_t result) override;
    virtual void OnSetSinkMuteCb(int32_t result) override;
    virtual void OnSetSourceOutputMuteCb(int32_t result) override;

    virtual void OnGetAllSinkInputsCb(int32_t result, std::vector<SinkInput> &sinkInputs) override;
    virtual void OnGetAllSourceOutputsCb(int32_t result, std::vector<SourceOutput> &sourceOutputs) override;
    virtual void OnGetAllSinksCb(int32_t result, std::vector<SinkInfo> &sinks) override;

    virtual void OnMoveSinkInputByIndexOrNameCb(int32_t result) override;
    virtual void OnMoveSourceOutputByIndexOrNameCb(int32_t result) override;

    virtual void OnGetAudioEffectPropertyCbV3(int32_t result) override;
    virtual void OnGetAudioEffectPropertyCb(int32_t result) override;
    virtual void OnGetAudioEnhancePropertyCbV3(int32_t result) override;
    virtual void OnGetAudioEnhancePropertyCb(int32_t result) override;
    virtual void HandleSourceAudioStreamRemoved(uint32_t sessionId) override;
    virtual bool IsChannelLayoutSupportedForDspEffect(AudioChannelLayout channelLayout) override;

private:
    std::mutex lock_;
    // for status operation wait and notify
    std::mutex callbackMutex_;
    std::condition_variable callbackCV_;
 
    bool isFinishOpenAudioPort_ = false;
    int32_t AudioPortIndex_ = 0;
    bool isFinishCloseAudioPort_ = false;
    bool isFinishReloadAudioPort_ = false;

    bool isFinishGetAllSinkInputs_ = false;
    std::vector<SinkInput> sinkInputs_;
    bool isFinishGetAllSourceOutputs_ = false;
    std::vector<SourceOutput> sourceOutputs_;
    bool isFinishGetAllSinks_ = false;
    std::vector<SinkInfo> sinks_;
    bool isFinishMoveSinkInputByIndexOrName_ = false;
    bool isFinishMoveSourceOutputByIndexOrName_ = false;

    int32_t SourceOutputMuteStreamSet_ = 0;
    bool isFinishSetSourceOutputMute_ = false;
    bool isFinishSetSinkMute_ = false;

    bool isFinishGetAudioEffectPropertyV3_ = false;
    bool isFinishGetAudioEffectProperty_ = false;
    bool isFinishGetAudioEnhancePropertyV3_ = false;
    bool isFinishGetAudioEnhanceProperty_ = false;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // ST_PULSEAUDIO_AUDIO_SERVICE_ADAPTER_H
