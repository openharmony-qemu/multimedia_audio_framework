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

#ifndef ST_BACKGROUND_TASK_LISTENER_H
#define ST_BACKGROUND_TASK_LISTENER_H

#include <sys/types.h>
#include "background_mode.h"
#include "continuous_task_callback_info.h"
#include "background_task_subscriber.h"

namespace OHOS {
namespace AudioStandard {
using OHOS::BackgroundTaskMgr::BackgroundTaskSubscriber;
using OHOS::BackgroundTaskMgr::ContinuousTaskCallbackInfo;

class BackgroundTaskListener : public BackgroundTaskSubscriber {
public:
    BackgroundTaskListener() {}
    ~BackgroundTaskListener() {}
    void OnContinuousTaskStart(const std::shared_ptr<ContinuousTaskCallbackInfo> &ContinuousTaskCallbackInfo) override;
    void OnContinuousTaskStop(const std::shared_ptr<ContinuousTaskCallbackInfo> &ContinuousTaskCallbackInfo) override;

};

}
}
#endif // ST_BACKGROUND_TASK_LISTENER_H