/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_AUDIOSERVER_SA_LOAD_H
#define OHOS_AUDIOSERVER_SA_LOAD_H

#include "iremote_object.h"
#include "system_ability_definition.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS {
namespace AudioStandard {

class AudioLoadCallback : public SystemAbilityLoadCallbackStub {
public:
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
        const sptr<IRemoteObject> &remoteObject) override;
    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
};

class AudioServiceLoad {
public:
    static AudioServiceLoad *GetInstance();
    int32_t LoadAudioService(void);
    void SetLoadFinish(void);
private:
    std::mutex mutex_;
    std::atomic<bool> isAudioServiceLoading_ = false;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // OHOS_AUDIOSERVER_SA_LOAD_H
