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

#ifndef AUDIO_STRATEGY_ROUTER_PARSER_H
#define AUDIO_STRATEGY_ROUTER_PARSER_H

#include <unordered_map>
#include <string>
#include <sstream>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "audio_policy_log.h"
#include "audio_info.h"
#include "iport_observer.h"
#include "parser.h"
#include "router_base.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
class AudioStrategyRouterParser : public Parser {
public:
    static constexpr char DEVICE_CONFIG_FILE[] = "system/etc/audio/audio_strategy_router.xml";

    bool LoadConfiguration() final;
    bool Parse() final;
    void Destroy() final;

    AudioStrategyRouterParser()
    {
        AUDIO_DEBUG_LOG("AudioStrategyRouterParser ctor");
    }

    ~AudioStrategyRouterParser()
    {
        AUDIO_DEBUG_LOG("AudioStrategyRouterParser dtor");
        Destroy();
    }

    std::vector<std::unique_ptr<RouterBase>> mediaRenderRouters_;
    std::vector<std::unique_ptr<RouterBase>> callRenderRouters_;
    std::vector<std::unique_ptr<RouterBase>> callCaptureRouters_;
    std::vector<std::unique_ptr<RouterBase>> ringRenderRouters_;
    std::vector<std::unique_ptr<RouterBase>> toneRenderRouters_;
    std::vector<std::unique_ptr<RouterBase>> recordCaptureRouters_;
    std::vector<std::unique_ptr<RouterBase>> voiceMessageRouters_;

private:
    bool ParseInternal(xmlNode *node);
    void ParserStrategyInfo(xmlNode *node);
    void AddRouters(std::vector<std::unique_ptr<RouterBase>> &routers, string &routeName);
    string ExtractPropertyValue(const string &propName, xmlNode *node);
    std::vector<std::string> split(const std::string &line, const std::string &sep);

    xmlDoc *doc_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_STRATEGY_ROUTER_PARSER_H
