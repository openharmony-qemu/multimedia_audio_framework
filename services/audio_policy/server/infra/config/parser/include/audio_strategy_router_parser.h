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

#ifndef AUDIO_STRATEGY_ROUTER_PARSER_H
#define AUDIO_STRATEGY_ROUTER_PARSER_H

#include <unordered_map>
#include <string>
#include <sstream>

#include "audio_policy_log.h"
#include "parser.h"
#include "router_base.h"
#include "audio_xml_parser.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
class AudioStrategyRouterParser : public Parser {
public:
#ifdef USE_CONFIG_POLICY
    static constexpr char DEVICE_CONFIG_FILE[] = "etc/audio/audio_strategy_router.xml";
#else
    static constexpr char DEVICE_CONFIG_FILE[] = "/system/etc/audio/audio_strategy_router.xml";
#endif

    bool LoadConfiguration() final;
    void Destroy() final;

    AudioStrategyRouterParser()
    {
        curNode_ = AudioXmlNode::Create();
        AUDIO_DEBUG_LOG("AudioStrategyRouterParser ctor");
    }

    ~AudioStrategyRouterParser()
    {
        AUDIO_DEBUG_LOG("AudioStrategyRouterParser dtor");
        Destroy();
        curNode_ = nullptr;
    }

    std::vector<std::unique_ptr<RouterBase>> mediaRenderRouters_;
    std::vector<std::unique_ptr<RouterBase>> callRenderRouters_;
    std::vector<std::unique_ptr<RouterBase>> callCaptureRouters_;
    std::vector<std::unique_ptr<RouterBase>> ringRenderRouters_;
    std::vector<std::unique_ptr<RouterBase>> toneRenderRouters_;
    std::vector<std::unique_ptr<RouterBase>> recordCaptureRouters_;
    std::vector<std::unique_ptr<RouterBase>> voiceMessageRouters_;

private:
    bool ParseInternal(std::shared_ptr<AudioXmlNode> curNode);
    void ParserStrategyInfo(std::shared_ptr<AudioXmlNode> curNode);
    void AddRouters(std::vector<std::unique_ptr<RouterBase>> &routers, string &routeName);
    std::vector<std::string> split(const std::string &line, const std::string &sep);
    std::shared_ptr<AudioXmlNode> curNode_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_STRATEGY_ROUTER_PARSER_H
