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

#ifndef ST_AUDIO_PNP_SERVER_H
#define ST_AUDIO_PNP_SERVER_H

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "hdf_types.h"
#include "audio_info.h"
#include "audio_pnp_param.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

class AudioPnpServer {
public:
    static AudioPnpServer& GetAudioPnpServer()
    {
        static AudioPnpServer audioPnpServer;
        return audioPnpServer;
    }
    ~AudioPnpServer();
    bool init(void);
    int32_t RegisterPnpStatusListener(std::shared_ptr<AudioPnpDeviceChangeCallback> callback);
    int32_t UnRegisterPnpStatusListener();
    void OnPnpDeviceStatusChanged(const std::string &info);
    void StopPnpServer();

private:
    std::string eventInfo_;
    std::mutex pnpMutex_;
    std::shared_ptr<AudioPnpDeviceChangeCallback> pnpCallback_ = nullptr;
    std::unique_ptr<std::thread> socketThread_ = nullptr;
    std::unique_ptr<std::thread> inputThread_ = nullptr;
    void OpenAndReadWithSocket();
    void OpenAndReadInput();
    void DetectAudioDevice();
#ifdef AUDIO_DOUBLE_PNP_DETECT
    void UpdateUsbHeadset();
#endif
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_PNP_SERVER_H