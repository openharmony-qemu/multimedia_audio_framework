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

#ifndef I_AUDIO_DEVICE_MANAGER_H
#define I_AUDIO_DEVICE_MANAGER_H

#include <map>
#include <mutex>

#include <v1_0/iaudio_manager.h>

#include "i_audio_device_adapter.h"

using OHOS::HDI::DistributedAudio::Audio::V1_0::AudioAdapterDescriptor;

namespace OHOS {
namespace AudioStandard {
class IAudioDeviceManager {
public:
    IAudioDeviceManager() = default;
    virtual ~IAudioDeviceManager() = default;

    virtual int32_t GetAllAdapters() = 0;
    virtual struct AudioAdapterDescriptor *GetTargetAdapterDesc(const std::string &adapterName, bool isMmap) = 0;
    virtual std::shared_ptr<IAudioDeviceAdapter> LoadAdapters(const std::string &adapterName, bool isMmap) = 0;
    virtual int32_t UnloadAdapter(const std::string &adapterName) = 0;
    virtual int32_t Release() = 0;
};

class AudioDeviceManagerFactory {
public:
    static AudioDeviceManagerFactory &GetInstance();
    std::shared_ptr<IAudioDeviceManager> CreatDeviceManager(const AudioDeviceManagerType audioMgrType);
    int32_t DestoryDeviceManager(const AudioDeviceManagerType audioMgrType);

private:
    AudioDeviceManagerFactory() = default;
    ~AudioDeviceManagerFactory() = default;

    AudioDeviceManagerFactory(const AudioDeviceManagerFactory &) = delete;
    AudioDeviceManagerFactory &operator=(const AudioDeviceManagerFactory &) = delete;
    AudioDeviceManagerFactory(AudioDeviceManagerFactory &&) = delete;
    AudioDeviceManagerFactory &operator=(AudioDeviceManagerFactory &&) = delete;

    std::shared_ptr<IAudioDeviceManager> InitLocalAudioMgr();
    std::shared_ptr<IAudioDeviceManager> InitRemoteAudioMgr();
    std::shared_ptr<IAudioDeviceManager> InitBluetoothAudioMgr();

private:
    std::mutex devMgrFactoryMtx_;
    std::map<AudioDeviceManagerType, std::shared_ptr<IAudioDeviceManager>> allHdiDevMgr_;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // I_AUDIO_DEVICE_MANAGER_H