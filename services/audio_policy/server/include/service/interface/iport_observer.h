/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_PORT_OBSERVER_H
#define ST_AUDIO_PORT_OBSERVER_H

#include "audio_info.h"
#include "iaudio_policy_interface.h"

namespace OHOS {
namespace AudioStandard {
class IPortObserver {
public:
    virtual void OnAudioPolicyXmlParsingCompleted(const std::unordered_map<AdaptersType, AudioAdapterInfo>
        adapterInfoMap) = 0;
    virtual void OnXmlParsingCompleted(const std::unordered_map<ClassType, std::list<AudioModuleInfo>> &xmldata) = 0;
    virtual void OnUpdateRouteSupport(bool isSupported) = 0;
    virtual void OnAudioLatencyParsed(uint64_t latency) = 0;
    virtual void OnSinkLatencyParsed(uint32_t latency) = 0;
    virtual void OnVolumeGroupParsed(std::unordered_map<std::string, std::string>& volumeGroupData) = 0;
    virtual void OnInterruptGroupParsed(std::unordered_map<std::string, std::string>& interruptGroupData) = 0;
    virtual void OnGlobalConfigsParsed(GlobalConfigs &globalConfig) = 0;
    virtual void OnVoipConfigParsed(bool enableFastVoip) = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif
