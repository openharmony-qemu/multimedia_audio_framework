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

#ifndef AUDIO_DEVICE_PARSER_H
#define AUDIO_DEVICE_PARSER_H

#include <list>
#include <unordered_map>
#include <string>
#include <sstream>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "audio_policy_log.h"
#include "audio_info.h"
#include "iport_observer.h"
#include "parser.h"
#include "audio_device_manager.h"

namespace OHOS {
namespace AudioStandard {
static const char* PRIVACY_TYPE = "privacy";
static const char* PUBLIC_TYPE = "public";
static const char* NEGATIVE_TYPE = "negative";

enum DeviceNodeName {
    UNKNOWN_NODE = -1,
    ADAPTER,
    DEVICES,
    DEVICE,
};

class AudioDeviceParser : public Parser {
public:
    static constexpr char DEVICE_CONFIG_FILE[] = "system/etc/audio/audio_device_privacy.xml";

    bool LoadConfiguration() final;
    bool Parse() final;
    void Destroy() final;

    AudioDeviceParser(AudioDeviceManager *audioDeviceManager)
    {
        audioDeviceManager_ = audioDeviceManager;
    }

    virtual ~AudioDeviceParser()
    {
        AUDIO_INFO_LOG("AudioDeviceParser dtor");
        Destroy();
    }

private:
    DeviceNodeName GetDeviceNodeNameAsInt(xmlNode *node);
    bool ParseInternal(xmlNode *node);
    void ParseDevicePrivacyInfo(xmlNode *node, std::list<DevicePrivacyInfo> &deviceLists);
    void ParserDevicePrivacyInfoList(xmlNode *node, std::list<DevicePrivacyInfo> &deviceLists);
    void ParseAudioDevicePrivacyType(xmlNode *node, AudioDevicePrivacyType &deviceType);
    void ParseDeviceRole(const std::string &deviceRole, uint32_t &deviceRoleFlag);
    void ParseDeviceCategory(const std::string &deviceCategory, uint32_t &deviceCategoryFlag);
    void ParseDeviceUsage(const std::string &deviceUsage, uint32_t &deviceUsageFlag);
    std::string ExtractPropertyValue(const std::string &propName, xmlNode *node);
    AudioDevicePrivacyType GetDevicePrivacyType(const std::string &devicePrivacyType);

    xmlDoc *mDoc_ = nullptr;
    AudioDevicePrivacyType devicePrivacyType_ = {};
    AudioDeviceManager *audioDeviceManager_;
    std::unordered_map<AudioDevicePrivacyType, std::list<DevicePrivacyInfo>> devicePrivacyMaps_ = {};
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_DEVICE_PARSER_H
