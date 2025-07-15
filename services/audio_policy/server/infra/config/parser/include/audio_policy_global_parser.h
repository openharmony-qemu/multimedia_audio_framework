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

#ifndef AUDIO_POLICY_GLOBAL_PARSER_H
#define AUDIO_POLICY_GLOBAL_PARSER_H

#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "audio_info.h"
#include "parser.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

class AudioPolicyGlobalParser : public Parser {
public:
    static constexpr char POLICY_GLOBAL_CONFIG_FILE[] = "/system/etc/audio/audio_policy_global_config.xml";

    bool LoadConfiguration() final;
    bool Parse();
    void Destroy() final;

    AudioPolicyGlobalParser() {}

    int32_t GetConfigByKeyName(std::string keyName, uint32_t &value);

    virtual ~AudioPolicyGlobalParser()
    {
        Destroy();
    }

private:
    bool ParseInternal(xmlNode *node);
    void ParserAttribute(xmlNode *node);

    xmlDoc *mDoc_ = nullptr;
    std::unordered_map<std::string, uint32_t> globalConfigs_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_DEVICE_AFFINITY_PARSER_H
