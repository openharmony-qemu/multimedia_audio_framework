/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "audio_concurrency_parser.h"

namespace OHOS {
namespace AudioStandard {

int32_t AudioConcurrencyParser::LoadConfig(std::map<std::pair<AudioPipeType, AudioPipeType>,
    ConcurrencyAction> &concurrencyMap)
{
    AUDIO_INFO_LOG("start.");
    CHECK_AND_RETURN_RET_LOG(curNode_->Config(AUDIO_CONCURRENCY_CONFIG_FILE, nullptr, 0) == SUCCESS, ERROR,
        "AudioConcurrencyParser loadXmlFile Failed!");
    if (!curNode_->CompareName("audioConcurrencyPolicy")) {
        AUDIO_ERR_LOG("Missing tag - audioConcurrencyPolicy");
        curNode_->FreeDoc();
        return ERR_OPERATION_FAILED;
    }
    ParseInternal(concurrencyMap, curNode_);
    return SUCCESS;
}

void AudioConcurrencyParser::ParseInternal(std::map<std::pair<AudioPipeType, AudioPipeType>,
    ConcurrencyAction> &concurrencyMap, std::shared_ptr<AudioXmlNode> curNode)
{
    for (; curNode->IsNodeValid(); curNode->MoveToNext()) {
        if (curNode->CompareName("existingStream")) {
            std::string existingStream;
            curNode->GetProp("name", existingStream);
            AUDIO_DEBUG_LOG("existingStream: %{public}s", existingStream.c_str());

            std::shared_ptr<AudioXmlNode> childrenNode = curNode->GetChildrenNode();
            ParseIncoming(existingStream, childrenNode, concurrencyMap);
        } else {
            std::shared_ptr<AudioXmlNode> childrenNode = curNode->GetChildrenNode();
            ParseInternal(concurrencyMap, childrenNode);
        }
    }
    return;
}

void AudioConcurrencyParser::ParseIncoming(const std::string &existing, std::shared_ptr<AudioXmlNode> curNode,
    std::map<std::pair<AudioPipeType, AudioPipeType>, ConcurrencyAction> &concurrencyMap)
{
    while (curNode->IsNodeValid()) {
        if (curNode->CompareName("incomingStream")) {
            std::string incoming;
            std::string action;
            curNode->GetProp("name", incoming);
            curNode->GetProp("action", action);
            AUDIO_DEBUG_LOG("existing: %{public}s %{public}d, incoming: %{public}s %{public}d, action: %{public}s",
                existing.c_str(), audioPipeTypeMap_[existing], incoming.c_str(),
                audioPipeTypeMap_[incoming], action.c_str());
            std::pair<AudioPipeType, AudioPipeType> concurrencyPair =
                std::make_pair(audioPipeTypeMap_[existing], audioPipeTypeMap_[incoming]);
            ConcurrencyAction concurrencyAction = (action == "play both" || action == "mix") ? PLAY_BOTH :
                (action == "concede existing" ? CONCEDE_EXISTING : CONCEDE_INCOMING);
            concurrencyMap.emplace(concurrencyPair, concurrencyAction);
        }
        curNode->MoveToNext();
    }
}
} // namespace AudioStandard
} // namespace OHOS