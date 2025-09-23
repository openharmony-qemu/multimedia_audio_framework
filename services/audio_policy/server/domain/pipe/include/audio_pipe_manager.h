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
#ifndef AUDIO_PIPE_MANAGER_H
#define AUDIO_PIPE_MANAGER_H

#include <string>
#include <mutex>
#include <shared_mutex>
#include <set>

#include "audio_stream_descriptor.h"
#include "audio_module_info.h"
#include "audio_pipe_info.h"

namespace OHOS {
namespace AudioStandard {
class AudioPipeManager {
public:
    AudioPipeManager();
    ~AudioPipeManager();

    static std::shared_ptr<AudioPipeManager> GetPipeManager()
    {
        static std::shared_ptr<AudioPipeManager> instance = std::make_shared<AudioPipeManager>();
        return instance;
    }

    void AddAudioPipeInfo(std::shared_ptr<AudioPipeInfo> info);
    void RemoveAudioPipeInfo(std::shared_ptr<AudioPipeInfo> info);
    void RemoveAudioPipeInfo(AudioIOHandle id);
    void UpdateAudioPipeInfo(std::shared_ptr<AudioPipeInfo> newPipe);
    void Assign(std::shared_ptr<AudioPipeInfo> dst, std::shared_ptr<AudioPipeInfo> src);
    bool IsSamePipe(std::shared_ptr<AudioPipeInfo> info, std::shared_ptr<AudioPipeInfo> cmpInfo);

    const std::vector<std::shared_ptr<AudioPipeInfo>> GetPipeList();
    std::vector<std::shared_ptr<AudioPipeInfo>> GetUnusedPipe();
    std::vector<std::shared_ptr<AudioPipeInfo>> GetUnusedRecordPipe();
    std::shared_ptr<AudioPipeInfo> GetPipeinfoByNameAndFlag(const std::string adapterName, const uint32_t routeFlag);
    std::string GetAdapterNameBySessionId(uint32_t sessionId);
    std::shared_ptr<AudioDeviceDescriptor> GetProcessDeviceInfoBySessionId(uint32_t sessionId,
        AudioStreamInfo &streamInfo);

    void StartClient(uint32_t sessionId);
    void PauseClient(uint32_t sessionId);
    void StopClient(uint32_t sessionId);
    void RemoveClient(uint32_t sessionId);

    std::vector<std::shared_ptr<AudioStreamDescriptor>> GetAllOutputStreamDescs();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> GetAllInputStreamDescs();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> GetStreamDescsByIoHandle(AudioIOHandle id);
    std::shared_ptr<AudioStreamDescriptor> GetStreamDescById(uint32_t sessionId);
    std::shared_ptr<AudioStreamDescriptor> GetStreamDescByIdInner(uint32_t sessionId);
    int32_t GetStreamCount(const std::string adapterName, const uint32_t routeFlag);
    uint32_t GetPaIndexByIoHandle(AudioIOHandle id);
    void UpdateRendererPipeInfos(std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfos);
    void UpdateCapturerPipeInfos(std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfos);
    uint32_t PcmOffloadSessionCount();
    int32_t GetClientUidBySessionId(uint32_t sessionId);

    void Dump(std::string &dumpString);
    bool IsModemCommunicationIdExist();
    bool IsModemCommunicationIdExist(uint32_t sessionId);
    void AddModemCommunicationId(uint32_t sessionId, std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    void RemoveModemCommunicationId(uint32_t sessionId);
    std::shared_ptr<AudioStreamDescriptor> GetModemCommunicationStreamDescById(uint32_t sessionId);
    std::shared_ptr<AudioStreamDescriptor> GetModemCommunicationStreamDesc();
    std::unordered_map<uint32_t, std::shared_ptr<AudioStreamDescriptor>> GetModemCommunicationMap();
    void UpdateModemStreamStatus(AudioStreamStatus streamStatus);
    void UpdateModemStreamDevice(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs);
    bool IsModemStreamDeviceChanged(std::shared_ptr<AudioDeviceDescriptor> &deviceDescs);
    std::shared_ptr<AudioPipeInfo> GetNormalSourceInfo(bool isEcFeatureEnable);
    std::vector<uint32_t> GetStreamIdsByUidAndPid(int32_t uid, int32_t pid);
    void UpdateOutputStreamDescsByIoHandle(AudioIOHandle id,
        std::vector<std::shared_ptr<AudioStreamDescriptor>> &descs);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> GetAllCapturerStreamDescs();
    std::shared_ptr<AudioPipeInfo> FindPipeBySessionId(const std::vector<std::shared_ptr<AudioPipeInfo>> &pipeList,
        uint32_t sessionId);
    bool IsStreamUsageActive(const StreamUsage &usage);
    int32_t IsCaptureVoipCall();
    uint32_t GetPaIndexByName(std::string portName);

private:
    bool IsSpecialPipe(uint32_t routeFlag);
    bool IsNormalRecordPipe(std::shared_ptr<AudioPipeInfo> pipeInfo);
    std::shared_ptr<AudioPipeInfo> GetPipeByModuleAndFlag(const std::string moduleName, const uint32_t routeFlag);

    std::unordered_map<uint32_t, std::shared_ptr<AudioStreamDescriptor>> modemCommunicationIdMap_{};
    std::vector<std::shared_ptr<AudioPipeInfo>> curPipeList_{};
    std::shared_mutex pipeListLock_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PIPE_MANAGER_H
