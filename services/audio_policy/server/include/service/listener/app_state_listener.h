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

#ifndef ST_APP_STATE_LISTENER_H
#define ST_APP_STATE_LISTENER_H

#include <memory>
#include "app_mgr_client.h"
#include "app_state_callback_host.h"

namespace OHOS {
namespace AudioStandard {

class AppStateListener : public AppExecFwk::AppStateCallbackHost {
public:
    AppStateListener();
    void OnAppStateChanged(const AppExecFwk::AppProcessData& appProcessData) override;
private:
    void HandleAppStateChange(int32_t pid, int32_t uid, int32_t state);
    AppExecFwk::BundleInfo GetBundleInfoFromUid(int32_t callingUid);

    static constexpr unsigned int GET_BUNDLE_TIME_OUT_SECONDS = 10;
};

}
}
#endif // ST_APP_STATE_LISTENER_H