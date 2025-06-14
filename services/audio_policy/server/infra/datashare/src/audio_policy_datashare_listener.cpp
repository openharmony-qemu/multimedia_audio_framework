/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "AudioSettingProvider"
#endif

#include "audio_policy_datashare_listener.h"

#include "iservice_registry.h"
#include "audio_errors.h"
#include "system_ability_definition.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {

void AudioPolicyDataShareListener::RegisterAccessiblilityBalance()
{
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    AudioSettingObserver::UpdateFunc updateFuncBalance = [&](const std::string &key) {
        AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
        float balance = 0;
        int32_t ret = settingProvider.GetFloatValue(CONFIG_AUDIO_BALANACE_KEY, balance, "secure");
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "get balance value failed");
        if (balance < -1.0f || balance > 1.0f) {
            AUDIO_WARNING_LOG("audioBalance value is out of range [-1.0, 1.0]");
        } else {
           AudioServerProxy::GetInstance().SetAudioBalanceValueProxy(balance);
        }
    };

    sptr observer = settingProvider.CreateObserver(CONFIG_AUDIO_BALANACE_KEY, updateFuncBalance);
    ErrCode ret = settingProvider.RegisterObserver(observer, "secure");
    if (ret != ERR_OK) {
        AUDIO_ERR_LOG("RegisterObserver balance failed");
    } else {
        AUDIO_INFO_LOG("Register accessibility balance successfully");
    }

    updateFuncBalance(CONFIG_AUDIO_BALANACE_KEY);
}

void AudioPolicyDataShareListener::RegisterAccessiblilityMono()
{
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    AudioSettingObserver::UpdateFunc updateFuncMono = [&](const std::string &key) {
        AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
        int32_t value = 0;
        ErrCode ret = settingProvider.GetIntValue(CONFIG_AUDIO_MONO_KEY, value, "secure");
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "get mono value failed");
        AudioServerProxy::GetInstance().SetAudioMonoStateProxy(value != 0);
    };
    sptr observer = settingProvider.CreateObserver(CONFIG_AUDIO_MONO_KEY, updateFuncMono);
    ErrCode ret = settingProvider.RegisterObserver(observer, "secure");
    if (ret != ERR_OK) {
        AUDIO_ERR_LOG("RegisterObserver mono failed");
    } else {
        AUDIO_INFO_LOG("Register accessibility mono successfully");
    }
    AUDIO_INFO_LOG("Register accessibility mono successfully");
    updateFuncMono(CONFIG_AUDIO_MONO_KEY);
}

} // namespace AudioStandard
} // namespace OHOS
