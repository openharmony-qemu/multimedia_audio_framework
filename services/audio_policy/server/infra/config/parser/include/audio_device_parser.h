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

#ifndef AUDIO_DEVICE_PARSER_H
#define AUDIO_DEVICE_PARSER_H

#include <list>
#include <unordered_map>
#include <string>
#include <sstream>

#include "audio_policy_log.h"
#include "parser.h"
#include "audio_device_manager.h"
#include "audio_xml_parser.h"

namespace OHOS {
namespace AudioStandard {

enum DeviceNodeName {
    UNKNOWN_NODE = -1,
    ADAPTER,
    DEVICES,
    DEVICE,
};

class AudioDeviceParser : public Parser {
public:
    static constexpr char PRIVACY_TYPE[] = "privacy";
    static constexpr char PUBLIC_TYPE[] = "public";
    static constexpr char DEVICE_CONFIG_FILE[] = "etc/audio/audio_device_privacy.xml";

    bool LoadConfiguration() final;
    void Destroy() final;

    AudioDeviceParser(AudioDeviceManager *audioDeviceManager)
    {
        curNode_ = AudioXmlNode::Create();
        audioDeviceManager_ = audioDeviceManager;
    }

    virtual ~AudioDeviceParser()
    {
        AUDIO_INFO_LOG("AudioDeviceParser dtor");
        Destroy();
        curNode_ = nullptr;
    }

private:
    DeviceNodeName GetDeviceNodeNameAsInt(std::shared_ptr<AudioXmlNode> curNode);
    bool ParseInternal(std::shared_ptr<AudioXmlNode> curNode);
    void ParseDevicePrivacyInfo(std::shared_ptr<AudioXmlNode> curNode, std::list<DevicePrivacyInfo> &deviceLists);
    void ParserDevicePrivacyInfoList(std::shared_ptr<AudioXmlNode> curNode, std::list<DevicePrivacyInfo> &deviceLists);
    void ParseAudioDevicePrivacyType(std::shared_ptr<AudioXmlNode> curNode, AudioDevicePrivacyType &deviceType);
    void ParseDeviceRole(const std::string &deviceRole, uint32_t &deviceRoleFlag);
    void ParseDeviceCategory(const std::string &deviceCategory, uint32_t &deviceCategoryFlag);
    void ParseDeviceUsage(const std::string &deviceUsage, uint32_t &deviceUsageFlag);
    AudioDevicePrivacyType GetDevicePrivacyType(const std::string &devicePrivacyType);

    std::shared_ptr<AudioXmlNode> curNode_ = nullptr;
    AudioDevicePrivacyType devicePrivacyType_ = {};
    AudioDeviceManager *audioDeviceManager_;
    std::unordered_map<AudioDevicePrivacyType, std::list<DevicePrivacyInfo>> devicePrivacyMaps_ = {};
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_DEVICE_PARSER_H
