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

#ifndef ST_DATA_SHARE_OBSERVER_CALLBACK_H
#define ST_DATA_SHARE_OBSERVER_CALLBACK_H

#include "datashare_helper.h"
#include "audio_policy_service.h"
#include "data_ability_observer_stub.h"

namespace OHOS {
namespace AudioStandard {
class DataShareObserverCallBack : public AAFwk::DataAbilityObserverStub {
public:
    explicit DataShareObserverCallBack();
    ~DataShareObserverCallBack() override {};
    void OnChange() override;

private:
    AudioPolicyService& audioPolicyService_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_DATA_SHARE_OBSERVER_CALLBACK_H
