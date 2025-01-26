/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef AUDIO_AFFINITY_PARSER_H
#define AUDIO_AFFINITY_PARSER_H

#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>

#include "iport_observer.h"
#include "parser.h"
#include "audio_affinity_manager.h"
#include "audio_xml_parser.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

class AudioAffinityParser : public Parser {
public:
    static constexpr char AFFINITY_CONFIG_FILE[] = "/system/etc/audio/audio_affinity_config.xml";

    bool LoadConfiguration() final;
    void Destroy() final;

    AudioAffinityParser(AudioAffinityManager *affinityManager)
    {
        audioAffinityManager_ = affinityManager;
        curNode_ = AudioXmlNode::Create();
    }

    virtual ~AudioAffinityParser()
    {
        Destroy();
        curNode_ = nullptr;
    }

    std::vector<AffinityDeviceInfo>& GetAffinityDeviceInfo()
    {
        return affinityDeviceInfoArray_;
    }

private:
    bool ParseInternal(std::shared_ptr<AudioXmlNode> curNode);
    void ParserAffinityGroups(std::shared_ptr<AudioXmlNode> curNode, const DeviceFlag& deviceFlag);
    void ParserAffinityGroupAttribute(std::shared_ptr<AudioXmlNode> curNode, const DeviceFlag& deviceFlag);
    void ParserAffinityGroupDeviceInfos(std::shared_ptr<AudioXmlNode> curNode, AffinityDeviceInfo& deviceInfo);

    std::shared_ptr<AudioXmlNode> curNode_ = nullptr;
    AudioAffinityManager* audioAffinityManager_ = nullptr;
    std::vector<AffinityDeviceInfo> affinityDeviceInfoArray_ = {};
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_DEVICE_AFFINITY_PARSER_H
