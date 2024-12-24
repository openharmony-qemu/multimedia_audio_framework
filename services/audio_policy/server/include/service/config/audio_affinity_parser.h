/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "audio_info.h"
#include "iport_observer.h"
#include "parser.h"
#include "audio_affinity_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

class audioAffinityParser : public Parser {
public:
    static constexpr char AFFINITY_CONFIG_FILE[] = "/system/etc/audio/audio_affinity_config.xml";

    bool LoadConfiguration() final;
    bool Parse() final;
    void Destroy() final;

    audioAffinityParser(AudioAffinityManager *affinityManager)
    {
        audioAffinityManager_ = affinityManager;
    }

    virtual ~audioAffinityParser()
    {
        Destroy();
    }

    std::vector<AffinityDeviceInfo>& GetAffinityDeviceInfo()
    {
        return affinityDeviceInfoArray_;
    }

private:
    bool ParseInternal(xmlNode *node);
    void ParserAffinityGroups(xmlNode *node, const DeviceFlag& deviceFlag);
    void ParserAffinityGroupAttribute(xmlNode *node, const DeviceFlag& deviceFlag);
    void ParserAffinityGroupDeviceInfos(xmlNode *node, AffinityDeviceInfo& deviceInfo);

    xmlDoc *mDoc_ = nullptr;
    AudioAffinityManager* audioAffinityManager_ = nullptr;
    std::vector<AffinityDeviceInfo> affinityDeviceInfoArray_ = {};
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_DEVICE_AFFINITY_PARSER_H
