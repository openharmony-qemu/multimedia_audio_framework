/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#ifndef AUDIO_THERMAL_LISTENER_H
#define AUDIO_THERMAL_LISTENER_H

#include <memory>
#include "securec.h"
#include "event_handler.h"
#include "event_runner.h"
#include "thermal_mgr_listener.h"

namespace OHOS {
namespace AudioStandard {

class AudioThermalLevelListener : public PowerMgr::ThermalMgrListener::ThermalLevelEvent {
public:
    AudioThermalLevelListener() = default;
    virtual ~AudioThermalLevelListener() = default;

    void RegisterThermalLevelListener();
    void UnregisterThermalLevelListener();

    void OnThermalLevelResult(const PowerMgr::ThermalLevel &level) override;

private:
    void PostThermalLevelTask(const PowerMgr::ThermalLevel &thermalLevel);
};

class AudioThermalLevelListenerHandler : public AppExecFwk::EventHandler {
public:
    AudioThermalLevelListenerHandler();
    ~AudioThermalLevelListenerHandler() override;
    AudioThermalLevelListenerHandler(const AudioThermalLevelListenerHandler &) = delete;
    AudioThermalLevelListenerHandler &operator=(const AudioThermalLevelListenerHandler &) = delete;
};

void RegisterAudioThermalLevelListener();
void UnregisterAudioThermalLevelListener();

}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_THERMAL_LISTENER_H