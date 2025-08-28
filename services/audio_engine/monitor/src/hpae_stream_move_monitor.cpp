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
#include "hpae_stream_move_monitor.h"
#include "media_monitor_manager.h"
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
void HpaeStreamMoveMonitor::ReportStreamMoveException(int32_t clientId, uint32_t sessionId, uint32_t streamType,
    const std::string &srcName, const std::string &desName, const std::string &error)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::EventId::STREAM_MOVE_EXCEPTION,
        Media::MediaMonitor::EventType::FAULT_EVENT);
    bean->Add("CLIENT_UID", clientId);
    bean->Add("SESSION_ID", static_cast<int32_t>(sessionId));
    bean->Add("CURRENT_NAME", srcName);
    bean->Add("DES_NAME", desName);
    bean->Add("STREAM_TYPE", static_cast<int32_t>(streamType));
    bean->Add("ERROR_DESCRIPTION", error);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}
}
}
}
