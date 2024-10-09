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
#ifndef AUDIO_VOLUME_PARSER_H
#define AUDIO_VOLUME_PARSER_H

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <map>
#include <string>
#include <vector>
#include <unordered_map>
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_policy_log.h"
#include "audio_volume_config.h"

namespace OHOS {
namespace AudioStandard {

#ifdef USE_CONFIG_POLICY
static constexpr char AUDIO_VOLUME_CONFIG_FILE[] = "etc/audio/audio_volume_config.xml";
#else
static constexpr char AUDIO_VOLUME_CONFIG_FILE[] = "system/etc/audio/audio_volume_config.xml";
#endif
class AudioVolumeParser {
public:
    AudioVolumeParser();
    virtual ~AudioVolumeParser();
    int32_t LoadConfig(StreamVolumeInfoMap &streamVolumeInfoMap);
private:
    std::map<std::string, AudioVolumeType> audioStreamMap_;
    std::map<std::string, DeviceVolumeType> audioDeviceMap_;

    void ParseStreamInfos(xmlNode *node, StreamVolumeInfoMap &streamVolumeInfoMap);
    void ParseStreamVolumeInfoAttr(xmlNode *node, std::shared_ptr<StreamVolumeInfo> &streamVolInfo);
    void ParseDeviceVolumeInfos(xmlNode *node, std::shared_ptr<StreamVolumeInfo> &streamVolInfo);
    void ParseVolumePoints(xmlNode *node, std::shared_ptr<DeviceVolumeInfo> &deviceVolInfo);
    int32_t ParseVolumeConfig(const char *path, StreamVolumeInfoMap &streamVolumeInfoMap);
    void WriteVolumeConfigErrorEvent();
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_VOLUME_PARSER_H
