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
#define LOG_TAG "AudioDialogAbilityConnection"
#endif

#include "audio_dialog_ability_connection.h"
#include "audio_policy_service.h"

namespace OHOS {
namespace AudioStandard {
std::atomic<bool> AudioDialogAbilityConnection::isDialogDestroy_(true);
std::condition_variable AudioDialogAbilityConnection::dialogCondition_;

void AudioDialogAbilityConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    AUDIO_INFO_LOG("element: %{public}s", element.GetURI().c_str());
    std::unique_lock<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(remoteObject != nullptr, "remoteObject is nullptr");

    if (!isDialogDestroy_.load()) {
        auto status = dialogCondition_.wait_for(lock, std::chrono::seconds(WAIT_DIALOG_CLOSE_TIME_S),
            [] () { return isDialogDestroy_.load(); });
        if (status) {
            AUDIO_INFO_LOG("wait safe dialog close failed.");
            return;
        }
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInt32(MESSAGE_PARCEL_KEY_SIZE);
    data.WriteString16(u"bundleName");
    data.WriteString16(u"com.huawei.hmos.mediacontroller");
    data.WriteString16(u"abilityName");
    data.WriteString16(u"SafeVolumeAbility");
    data.WriteString16(u"parameters");
    nlohmann::json param;
    param["ability.want.params.uiExtensionType"] = "sys/commonUI";
    param["sysDialogZOrder"] = SYS_DIALOG_TYPE_UPPER;
    std::string paramStr = param.dump();
    data.WriteString16(Str8ToStr16(paramStr));
    int32_t errCode = remoteObject->SendRequest(IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
    CHECK_AND_RETURN_LOG(errCode == SUCCESS, "send ON_ABILITY_CONNECT_DONE failed");
    isDialogDestroy_.store(false);
}

void AudioDialogAbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    AUDIO_INFO_LOG("element: %{public}s, resultCode:%{public}d", element.GetURI().c_str(), resultCode);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        isDialogDestroy_.store(true);
        dialogCondition_.notify_all();
    }
    AudioPolicyService::GetAudioPolicyService().SafeVolumeDialogDisapper();
}
} // namespace AudioStandard
} // namespace OHOS