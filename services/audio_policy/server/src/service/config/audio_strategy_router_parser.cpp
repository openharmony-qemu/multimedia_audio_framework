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
#ifndef LOG_TAG
#define LOG_TAG "AudioStrategyRouterParser"
#endif

#include "audio_strategy_router_parser.h"
#include "user_select_router.h"
#include "app_select_router.h"
#include "privacy_priority_router.h"
#include "public_priority_router.h"
#include "stream_filter_router.h"
#include "cockpit_phone_router.h"
#include "pair_device_router.h"
#include "default_router.h"

#include "media_monitor_manager.h"

namespace OHOS {
namespace AudioStandard {
bool AudioStrategyRouterParser::LoadConfiguration()
{
    curNode_ = AudioXmlNode::Create();
    int32_t ret = curNode_->Config(DEVICE_CONFIG_FILE, nullptr, 0);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Not found audio_strategy_router.xml!");
        std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::LOAD_CONFIG_ERROR, Media::MediaMonitor::FAULT_EVENT);
        bean->Add("CATEGORY", Media::MediaMonitor::AUDIO_STRATEGY_ROUTER);
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
        return false;
    }
    bool result = ParseInternal(curNode_->GetCopyNode());
    CHECK_AND_RETURN_RET_LOG(result, false, "Audio strategy router xml parse failed.");
    return true;
}

void AudioStrategyRouterParser::Destroy()
{
    curNode_->FreeDoc();
}

bool AudioStrategyRouterParser::ParseInternal(std::shared_ptr<AudioXmlNode> curNode)
{
    for (; curNode->IsNodeValid(); curNode->MoveToNext()) {
        if (!curNode->IsElementNode()) {
            continue;
        }
        if (curNode->CompareName("strategy")) {
            ParserStrategyInfo(curNode->GetCopyNode());
        } else {
            ParseInternal(curNode->GetChildrenNode());
        }
    }
    return true;
}

void AudioStrategyRouterParser::ParserStrategyInfo(std::shared_ptr<AudioXmlNode> curNode)
{
    string name;
    string routers;
    curNode->GetProp("name", name);
    curNode->GetProp("routers", routers);

    if (name == "MEDIA_RENDER") {
        AddRouters(mediaRenderRouters_, routers);
    } else if (name == "CALL_RENDER") {
        AddRouters(callRenderRouters_, routers);
    } else if (name == "RING_RENDER") {
        AddRouters(ringRenderRouters_, routers);
    } else if (name == "TONE_RENDER") {
        AddRouters(toneRenderRouters_, routers);
    } else if (name == "RECORD_CAPTURE") {
        AddRouters(recordCaptureRouters_, routers);
    } else if (name == "CALL_CAPTURE") {
        AddRouters(callCaptureRouters_, routers);
    } else if (name == "VOICE_MESSAGE_CAPTURE") {
        AddRouters(voiceMessageRouters_, routers);
    }
}

std::vector<std::string> AudioStrategyRouterParser::split(const std::string &line, const std::string &sep)
{
    std::vector<std::string> buf;
    size_t temp = 0;
    std::string::size_type pos = 0;
    while (true) {
        pos = line.find(sep, temp);
        if (pos == std::string::npos) {
            break;
        }
        buf.push_back(line.substr(temp, pos-temp));
        temp = pos + sep.length();
    }
    buf.push_back(line.substr(temp, line.length()));
    return buf;
}

void AudioStrategyRouterParser::AddRouters(std::vector<std::unique_ptr<RouterBase>> &routers, string &routeName)
{
    vector<string> buf = split(routeName, ",");
    for (const auto &name : buf) {
        if (name == "AppSelectRouter") {
            routers.push_back(make_unique<AppSelectRouter>());
        } else if (name == "UserSelectRouter") {
            routers.push_back(make_unique<UserSelectRouter>());
        } else if (name == "PrivacyPriorityRouter") {
            routers.push_back(make_unique<PrivacyPriorityRouter>());
        } else if (name == "PublicPriorityRouter") {
            routers.push_back(make_unique<PublicPriorityRouter>());
        } else if (name == "StreamFilterRouter") {
            routers.push_back(make_unique<StreamFilterRouter>());
        } else if (name == "DefaultRouter") {
            routers.push_back(make_unique<DefaultRouter>());
        } else if (name == "CockpitPhoneRouter") {
            routers.push_back(make_unique<CockpitPhoneRouter>());
        } else if (name == "PairDeviceRouter") {
            routers.push_back(make_unique<PairDeviceRouter>());
        }
    }
}
} // namespace AudioStandard
} // namespace OHOS
