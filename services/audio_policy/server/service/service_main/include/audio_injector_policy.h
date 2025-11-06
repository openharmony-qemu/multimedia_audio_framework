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
#ifndef AUDIO_INJECTOR_POLICY_H
#define AUDIO_INJECTOR_POLICY_H

#include <set>
#include "audio_module_info.h"
#include "audio_pipe_info.h"
#include "audio_iohandle_map.h"
#include "iaudio_policy_interface.h"
#include "audio_pipe_manager.h"
#include "audio_service_enum.h"

namespace OHOS {
namespace AudioStandard {
enum VoipType {
    NO_VOIP = 0,
    NORMAL_VOIP = 1,
    FAST_VOIP = 2,
};

class AudioPolicyServerHandler;
class AudioInjectorPolicy {
public:
    static AudioInjectorPolicy& GetInstance()
    {
        static AudioInjectorPolicy instance;
        return instance;
    }
    int32_t Init();
    int32_t DeInit();
    void UpdateAudioInfo(AudioModuleInfo &info);
    int32_t AddStreamDescriptor(uint32_t renderId, std::shared_ptr<AudioStreamDescriptor> desc);
    int32_t RemoveStreamDescriptor(uint32_t renderId);
    bool IsContainStream(uint32_t renderId);
    std::string GetAdapterName();
    int32_t GetRendererStreamCount();
    void SetCapturePortIdx(uint32_t idx);
    uint32_t GetCapturePortIdx();
    void SetRendererPortIdx(uint32_t idx);
    uint32_t GetRendererPortIdx();
    AudioModuleInfo& GetAudioModuleInfo();
    bool GetIsConnected();
    void SetVoipType(VoipType type);
    int32_t AddCaptureInjector();
    int32_t AddCaptureInjectorInner();
    int32_t RemoveCaptureInjector(bool noCapturer);
    int32_t RemoveCaptureInjectorInner(bool noCapturer);
    void ReleaseCaptureInjector();
    void RebuildCaptureInjector(uint32_t streamId);
    std::shared_ptr<AudioPipeInfo> FindCaptureVoipPipe(
        std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos, uint32_t &streamId);
    std::shared_ptr<AudioPipeInfo> FindPipeByStreamId(
        std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos, VoipType &type, uint32_t &streamId);
    void FetchCapDeviceInjectPreProc(std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos,
        bool &removeFlag, uint32_t &streamId);
    void FetchCapDeviceInjectPostProc(std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos,
        bool &removeFlag, uint32_t &streamId);
    bool HasRunningVoipStream(const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamVec);

    void AddInjectorStreamId(const uint32_t streamId);
    void DeleteInjectorStreamId(const uint32_t streamId);
    bool IsActivateInterruptStreamId(const uint32_t streamId);
    void SendInterruptEventToInjectorStreams(const std::shared_ptr<AudioPolicyServerHandler> &handler);
    void SetInjectStreamsMuteForInjection(uint32_t streamId);
    void SetInjectStreamsMuteForPlayback(uint32_t streamId);
    void SetInjectorStreamsMute(bool newMicrophoneMute);

private:
    AudioInjectorPolicy();
    AudioInjectorPolicy(const AudioInjectorPolicy&) = delete;
    AudioInjectorPolicy& operator=(const AudioInjectorPolicy&) = delete;
private:
    AudioModuleInfo moduleInfo_;
    uint32_t capturePortIdx_;
    uint32_t renderPortIdx_;
    AudioIOHandle ioHandle_;
    bool isOpened_;
    bool isConnected_;
    VoipType voipType_;
    std::unordered_map<uint32_t, std::shared_ptr<AudioStreamDescriptor>> rendererStreamMap_ = {};
    std::unordered_map<uint32_t, bool> rendererMuteStreamMap_ = {};
    AudioIOHandleMap &audioIOHandleMap_;
    IAudioPolicyInterface &audioPolicyManager_;
    std::shared_ptr<AudioPipeManager> pipeManager_ = nullptr;
    std::shared_mutex injectLock_;
    std::unordered_set<uint32_t> injectorStreamIds_;
    bool isNeedMuteRenderer_ = false;
};
} //  namespace AudioStandard
} //  namespace OHOS
#endif  // AUDIO_INJECTOR_POLICY_H