/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <cstdio>
#include "napi_dfx_utils.h"
#include "audio_common_log.h"
#include "media_monitor_manager.h"
#include "media_monitor_info.h"

namespace OHOS {
namespace AudioStandard {

void NapiDfxUtils::SendVolumeApiInvokeEvent(int32_t uid, std::string functionName, int32_t paramValue)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::VOLUME_API_INVOKE,
        Media::MediaMonitor::EventType::FREQUENCY_AGGREGATION_EVENT);
    bean->Add("CLIENT_UID", uid);
    bean->Add("FUNC_NAME", functionName);
    bean->Add("PARAM_VALUE", paramValue);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}
} // namespace AudioStandard
} // namespace OHOS