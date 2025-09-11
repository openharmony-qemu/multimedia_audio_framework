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
#include "hpae_message_queue_monitor.h"
#include "media_monitor_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
void HpaeMessageQueueMonitor::ReportMessageQueueException(MessageQueueType type, const std::string &func,
    const std::string &error)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::EventId::HPAE_MESSAGE_QUEUE_EXCEPTION,
        Media::MediaMonitor::EventType::FAULT_EVENT);
    bean->Add("MSG_TYPE", type);
    bean->Add("MSG_FUNC_NAME", func);
    bean->Add("MSG_ERROR_DESCRIPTION", error);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}
}
}
}