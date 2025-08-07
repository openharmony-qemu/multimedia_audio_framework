/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef AUDIO_XML_NODE_MOCK_H
#define AUDIO_XML_NODE_MOCK_H

#include <gmock/gmock.h>
#include "audio_strategy_router_parser.h"

namespace OHOS {
namespace AudioStandard {
class MockAudioXmlNode : public AudioXmlNode {
public:
    MockAudioXmlNode() = default;
    virtual ~MockAudioXmlNode() override = default;
    MOCK_METHOD(std::shared_ptr<AudioXmlNode>, GetChildrenNode, (), (override));
    MOCK_METHOD(std::shared_ptr<AudioXmlNode>, GetCopyNode, (), (override));
    MOCK_METHOD(int32_t, Config, (const char *fileName, const char *encoding, int32_t options), (override));
    MOCK_METHOD(void, MoveToNext, (), (override));
    MOCK_METHOD(void, MoveToChildren, (), (override));
    MOCK_METHOD(bool, IsNodeValid, (), (override));
    MOCK_METHOD(bool, HasProp, (const char *propName), (override));
    MOCK_METHOD(int32_t, GetProp, (const char *propName, std::string &result), (override));
    MOCK_METHOD(int32_t, GetContent, (std::string &result), (override));
    MOCK_METHOD(std::string, GetName, (), (override));
    MOCK_METHOD(bool, CompareName, (const char *propName), (override));
    MOCK_METHOD(bool, IsElementNode, (), (override));
    MOCK_METHOD(void, FreeDoc, (), (override));
    MOCK_METHOD(void, FreeProp, (char *propName), (override));
};
}  // namespace AudioStandard
}  // namespace OHOS

#endif // AUDIO_XML_NODE_MOCK_H