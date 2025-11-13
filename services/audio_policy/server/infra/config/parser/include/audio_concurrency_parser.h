/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef SERVICE_CONFIG_AUDIO_CONCURRENCY_PARSER_H
#define SERVICE_CONFIG_AUDIO_CONCURRENCY_PARSER_H

#include <map>
#include <string>
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_policy_log.h"
#include "audio_xml_parser.h"

namespace OHOS {
namespace AudioStandard {

enum ConcurrencyAction {
    PLAY_BOTH,
    CONCEDE_INCOMING,
    CONCEDE_EXISTING,
    CONCEDE_BOTH,
};

class AudioConcurrencyParser {
public:
    static constexpr char AUDIO_CONCURRENCY_CONFIG_FILE[] = "/vendor/etc/audio/audio_concurrency_config.xml";
    AudioConcurrencyParser()
    {
        curNode_ = AudioXmlNode::Create();
        AUDIO_INFO_LOG("AudioConcurrencyParser ctor");
    }
    virtual ~AudioConcurrencyParser()
    {
        curNode_ = nullptr;
        AUDIO_DEBUG_LOG("AudioConcurrencyParser dtor");
    }
    int32_t LoadConfig(std::map<std::pair<AudioPipeType, AudioPipeType>, ConcurrencyAction> &concurrencyMap);

private:
    void ParseInternal(std::map<std::pair<AudioPipeType, AudioPipeType>, ConcurrencyAction> &concurrencyMap,
        std::shared_ptr<AudioXmlNode> audioXmlNode);
    void ParseIncoming(const std::string &existing, std::shared_ptr<AudioXmlNode> audioXmlNode,
        std::map<std::pair<AudioPipeType, AudioPipeType>, ConcurrencyAction> &concurrencyMap);
    std::shared_ptr<AudioXmlNode> curNode_ = nullptr;
    std::map<std::string, AudioPipeType> audioPipeTypeMap_ = {
        {"primary out", PIPE_TYPE_OUT_NORMAL},
        {"primary in", PIPE_TYPE_IN_NORMAL},
        {"fast out normal", PIPE_TYPE_OUT_LOWLATENCY},
        {"fast in normal", PIPE_TYPE_IN_LOWLATENCY},
        {"offload out", PIPE_TYPE_OUT_OFFLOAD},
        {"multichannel out", PIPE_TYPE_OUT_MULTICHANNEL},
        {"direct out normal", PIPE_TYPE_OUT_DIRECT_NORMAL},
        {"voip out", PIPE_TYPE_OUT_VOIP},
        {"voip in", PIPE_TYPE_IN_VOIP},
        {"cellular call out", PIPE_TYPE_OUT_CELLULAR_CALL},
        {"cellular call in", PIPE_TYPE_IN_CELLULAR_CALL},
        {"primary in AI", PIPE_TYPE_IN_NORMAL_AI}
    };
};
} // namespace AudioStandard
} // namespace OHOS
#endif // SERVICE_CONFIG_AUDIO_CONCURRENCY_PARSER_H