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

#ifndef AUDIO_DEVICE_MANAGER_IMPL_H
#define AUDIO_DEVICE_MANAGER_IMPL_H

#include <map>
#include <mutex>

#include "audio_info.h"
#include "i_audio_device_manager.h"
#include "i_audio_device_adapter.h"

namespace OHOS {
namespace AudioStandard {
struct DeviceAdapterInfo {
    std::shared_ptr<IAudioDeviceAdapter> devAdp = nullptr;
    sptr<IAudioAdapter> audioAdapter = nullptr;
};

class AudioDeviceManagerImpl : public IAudioDeviceManager {
public:
    AudioDeviceManagerImpl(const AudioDeviceManagerType devMgrType, sptr<IAudioManager> audioMgr)
        : audioMgrType_(devMgrType), audioMgr_(audioMgr) {};
    ~AudioDeviceManagerImpl() = default;

    int32_t GetAllAdapters() override;
    struct AudioAdapterDescriptor *GetTargetAdapterDesc(const std::string &adapterName, bool isMmap) override;
    std::shared_ptr<IAudioDeviceAdapter> LoadAdapters(const std::string &adapterName, bool isMmap) override;
    int32_t UnloadAdapter(const std::string &adapterName) override;
    int32_t Release() override;

private:
    static constexpr int32_t INVALID_INDEX = -1;

    AudioDeviceManagerType audioMgrType_;
    sptr<IAudioManager> audioMgr_;
    std::mutex mtx_;
    std::map<std::string, DeviceAdapterInfo> enableAdapters_;
    std::vector<AudioAdapterDescriptor> descriptors_;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_DEVICE_MANAGER_IMPL_H
