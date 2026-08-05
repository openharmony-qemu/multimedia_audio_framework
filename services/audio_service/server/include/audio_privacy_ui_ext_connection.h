/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef AUDIO_PRIVACY_UI_EXT_CONNECTION_H
#define AUDIO_PRIVACY_UI_EXT_CONNECTION_H

#include <cstdint>
#include <string>
#include <string_view>
#include "cJSON.h"

#include "ability_connect_callback_interface.h"
#include "ability_connect_callback_stub.h"
#include "audio_capturer_log.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
namespace AudioStandard {
static constexpr std::string_view BUNDLE_NAME = "com.hmos.mediacontroller";
static constexpr std::string_view ABILITY_NAME = "DialogExtension";
const uint32_t PARAM_SIZE = 4;
const uint32_t ORDER_SIZE = 2;
class AudioPrivacyUIExtConnection : public OHOS::AAFwk::AbilityConnectionStub {
public:
    explicit AudioPrivacyUIExtConnection(int32_t sessionId, uint32_t uid)
        : sessionId_(sessionId), uid_(uid)
    {
        AUDIO_INFO_LOG("AudioPrivacyUIExtConnection created");
    }

    ~AudioPrivacyUIExtConnection() override
    {
        AUDIO_INFO_LOG("AudioPrivacyUIExtConnection destroyed");
    }

    void OnAbilityConnectDone(const OHOS::AppExecFwk::ElementName &element,
        const OHOS::sptr<OHOS::IRemoteObject> &remoteObject, int32_t resultCode) override
    {
        CHECK_AND_RETURN_LOG(remoteObject != nullptr, "remoteObject is nullptr");
        outRemote_ = remoteObject;

        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        data.WriteInt32(PARAM_SIZE);
        data.WriteString16(u"bundleName");
        std::u16string bundleNameU16(BUNDLE_NAME.begin(), BUNDLE_NAME.end());
        data.WriteString16(bundleNameU16);

        data.WriteString16(u"abilityName");
        std::u16string abilityNameU16(ABILITY_NAME.begin(), ABILITY_NAME.end());
        data.WriteString16(abilityNameU16);

        data.WriteString16(u"parameters");
        cJSON *param = cJSON_CreateObject();
        CHECK_AND_RETURN_LOG(param != nullptr, "cJSON_CreateObject failed");
        cJSON_AddStringToObject(param, "ability.want.params.uiExtensionType", "sys/commonUI");
        cJSON_AddNumberToObject(param, "sysDialogZOrder", static_cast<double>(ORDER_SIZE));
        cJSON_AddNumberToObject(param, "sessionId", static_cast<double>(sessionId_));
        cJSON_AddNumberToObject(param, "uid", static_cast<double>(uid_));

        char *pChar = cJSON_PrintUnformatted(param);
        std::string paramStr = (pChar != nullptr) ? std::string(pChar) : std::string{};
        cJSON_free(pChar);
        cJSON_Delete(param);
        std::u16string paramStrU16(paramStr.begin(), paramStr.end());
        data.WriteString16(paramStrU16);
    
        auto ret = remoteObject->SendRequest(IAbilityConnection::ON_ABILITY_CONNECT_DONE, data, reply, option);
        AUDIO_INFO_LOG("SendRequest ret=%{public}d", ret);
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override
    {
        outRemote_ = nullptr;
    }

private:
    int32_t sessionId_;
    uint32_t uid_;
    OHOS::sptr<OHOS::IRemoteObject> outRemote_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_PRIVACY_UI_EXT_CONNECTION_H
