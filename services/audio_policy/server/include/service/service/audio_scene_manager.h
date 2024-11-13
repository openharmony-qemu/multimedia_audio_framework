/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef ST_AUDIO_AUDIO_SCENE_MANAGER_H
#define ST_AUDIO_AUDIO_SCENE_MANAGER_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "singleton.h"
#include "audio_group_handle.h"
#include "audio_info.h"
#include "audio_manager_base.h"
#include "audio_module_info.h"
#include "audio_volume_config.h"
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_router_center.h"

#include "audio_active_device.h"

namespace OHOS {
namespace AudioStandard {

class AudioSceneManager {
public:
    static AudioSceneManager& GetInstance()
    {
        static AudioSceneManager instance;
        return instance;
    }
    int32_t SetAudioSceneAfter(AudioScene audioScene, BluetoothOffloadState state);
    void SetAudioScenePre(AudioScene audioScene);
    AudioScene GetAudioScene(bool hasSystemPermission = true) const;
    AudioScene GetLastAudioScene() const;
    bool IsSameAudioScene();
private:
    AudioSceneManager() :
        audioRouterCenter_(AudioRouterCenter::GetAudioRouterCenter()),
        audioActiveDevice_(AudioActiveDevice::GetInstance()) {}
    ~AudioSceneManager() {}
    void DealAudioSceneOutputDevices(const AudioScene &audioScene, std::vector<DeviceType> &activeOutputDevices,
        bool &haveArmUsbDevice);
private:
    AudioScene audioScene_ = AUDIO_SCENE_DEFAULT;
    AudioScene lastAudioScene_ = AUDIO_SCENE_DEFAULT;

    AudioRouterCenter& audioRouterCenter_;
    AudioActiveDevice& audioActiveDevice_;
};

}
}

#endif