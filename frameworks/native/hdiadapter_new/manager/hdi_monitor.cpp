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

#ifndef LOG_TAG
#define LOG_TAG "HdiMonitor"
#endif

#include "manager/hdi_monitor.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "audio_common_log.h"

#include "media_monitor_manager.h"

namespace OHOS {
namespace AudioStandard {
void HdiMonitor::ReportHdiException(HdiType hdiType, ErrorCase errorCase, int32_t errorMsg, const std::string &desc)
{
    AUDIO_WARNING_LOG("hdiType:%{public}d errorCase:%{public}d errorMsg:%{public}d desc:%{public}s", hdiType,
        errorCase, errorMsg, desc.c_str());
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::EventId::HDI_EXCEPTION,
        Media::MediaMonitor::EventType::FAULT_EVENT);
    bean->Add("HDI_TYPE", hdiType);
    bean->Add("ERROR_CASE", static_cast<int32_t>(errorCase));
    bean->Add("ERROR_MSG", errorMsg);
    bean->Add("ERROR_DESCRIPTION", desc);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}
} // namespace AudioStandard
} // namespace OHOS
