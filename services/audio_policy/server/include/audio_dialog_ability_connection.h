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
#ifndef AUDIO_DIALOG_ABILITY_CONNECTION_H
#define AUDIO_DIALOG_ABILITY_CONNECTION_H

#include <queue>
#include <string>
#include <atomic>
#include <mutex>
#include <element_name.h>
#include <ability_connect_callback_interface.h>
#include "ability_connect_callback_stub.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {
constexpr int32_t WAIT_DIALOG_CLOSE_TIME_S = 3;
constexpr int32_t MESSAGE_PARCEL_KEY_SIZE = 3;
constexpr int32_t SYS_DIALOG_TYPE_UPPER = 2;
class AudioDialogAbilityConnection : public OHOS::AAFwk::AbilityConnectionStub {
public:
    AudioDialogAbilityConnection() = default;
    virtual ~AudioDialogAbilityConnection() = default;

    void OnAbilityConnectDone(const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject,
        int resultCode) override;

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;

    static std::atomic<bool> isDialogDestroy_;
    static std::condition_variable dialogCondition_;
private:
    std::mutex mutex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_DIALOG_ABILITY_CONNECTION_H
