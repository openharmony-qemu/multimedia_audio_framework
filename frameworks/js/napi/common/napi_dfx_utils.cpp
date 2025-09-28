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

#include <cstdio>
#include "napi_dfx_utils.h"
#include "hisysevent.h"
#include "audio_common_log.h"
#include "audio_info.h"
#ifndef CROSS_PLATFORM
#include "media_monitor_manager.h"
#include "media_monitor_info.h"
#endif

namespace OHOS {
namespace AudioStandard {

void NapiDfxUtils::SendVolumeApiInvokeEvent(int32_t uid, const std::string &functionName, int32_t paramValue)
{
#ifndef CROSS_PLATFORM
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::VOLUME_API_INVOKE,
        Media::MediaMonitor::EventType::FREQUENCY_AGGREGATION_EVENT);
    bean->Add("CLIENT_UID", uid);
    bean->Add("FUNC_NAME", functionName);
    bean->Add("PARAM_VALUE", paramValue);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
#endif
}

static const char* GetFuncReadable(bool direction, uint8_t functionType)
{
    if (direction == NapiDfxUtils::SteamDirection::PLAYBACK) {
        switch (functionType) {
            case 0: return "Write";
            case 1: return "RegisterRendererWriteDataCallback";
            default: return "UnknownRendererFunc";
        }
    } else {
        switch (functionType) {
            case 0: return "Read";
            case 1: return "RegisterCaptureReadDataCallback";
            default: return "UnknownCaptureFunc";
        }
    }
}
 
void NapiDfxUtils::ReportAudioMainThreadEvent(std::string bundleName, bool direction,
        uint8_t usageOrSourceType, uint8_t functionType)
{
    const char* typeStr = direction ? "Capture" : "Renderer";
    const char* keyStr  = direction ? "sourceType" : "usage";
    const char* funcStr = GetFuncReadable(direction, functionType);
 
    AUDIO_INFO_LOG("type=%{public}s, %{public}s=%{public}d, funcId=%{public}d(%{public}s)",
        typeStr, keyStr, usageOrSourceType, functionType, funcStr);

    auto ret = HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::AUDIO, "PROCESS_AUDIO_BY_MAINTHREAD",
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "BUNDLENAME", bundleName,
        "AUDIODIRECTION", direction,
        "AUDIOSTREAM", usageOrSourceType,
        "CALLFUNC", functionType);
    if (ret) {
        AUDIO_ERR_LOG("write event fail: PROCESS_AUDIO_BY_MAINTHREAD, ret = %{public}d", ret);
    }
}
} // namespace AudioStandard
} // namespace OHOS