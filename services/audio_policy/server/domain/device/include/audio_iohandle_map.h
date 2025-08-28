/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_IOHANDLE_MAP_H
#define ST_AUDIO_IOHANDLE_MAP_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "singleton.h"
#include "audio_group_handle.h"
#include "audio_manager_base.h"
#include "audio_module_info.h"
#include "audio_pipe_info.h"

namespace OHOS {
namespace AudioStandard {

class AudioIOHandleMap {
public:
    static AudioIOHandleMap& GetInstance()
    {
        static AudioIOHandleMap instance;
        return instance;
    }

    void DeInit();
    std::unordered_map<std::string, AudioIOHandle> GetCopy();
    bool GetModuleIdByKey(std::string moduleName, AudioIOHandle& moduleId);
    void DelIOHandleInfo(std::string moduleName);
    void AddIOHandleInfo(std::string moduleName, const AudioIOHandle& moduleId);
    AudioIOHandle GetSinkIOHandle(DeviceType deviceType);
    AudioIOHandle GetSourceIOHandle(DeviceType deviceType);
    bool CheckIOHandleExist(std::string moduleName);

    int32_t OpenPortAndInsertIOHandle(const std::string &moduleName, const AudioModuleInfo &moduleInfo);
    int32_t ClosePortAndEraseIOHandle(const std::string &moduleName);
    int32_t ReloadPortAndUpdateIOHandle(std::shared_ptr<AudioPipeInfo> &pipeInfo, const AudioModuleInfo &moduleInfo,
        bool softLinkFlag = false);

    void NotifyUnmutePort();
    void MuteSinkPort(const std::string &portName, int32_t duration, bool isSync, bool isSleepEnabled = true);
    void SetMoveFinish(bool flag);
    void MuteDefaultSinkPort(std::string networkID, std::string sinkName);
    void UnmutePortAfterMuteDuration(int32_t muteDuration, const std::string &portName);
    void DoUnmutePort(int32_t muteDuration, const std::string &portName);

private:
    AudioIOHandleMap() {}
    ~AudioIOHandleMap() {}

private:
    std::mutex ioHandlesMutex_;
    std::unordered_map<std::string, AudioIOHandle> IOHandles_ = {};

    std::mutex moveDeviceMutex_;
    std::condition_variable moveDeviceCV_;
    std::atomic<bool> moveDeviceFinished_ = false;

    static std::map<std::string, std::string> sinkPortStrToClassStrMap_;
};
}
}

#endif