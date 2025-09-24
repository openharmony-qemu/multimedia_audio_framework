/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include "audio_source_strategy_parser.h"
 
namespace OHOS {
namespace AudioStandard {
class MockAudioXmlNode : public AudioXmlNode {
public:
    MockAudioXmlNode() = default;
    ~MockAudioXmlNode() = default;
    MOCK_METHOD(std::shared_ptr<AudioXmlNode>, GetChildrenNode, (), ());
    MOCK_METHOD(std::shared_ptr<AudioXmlNode>, GetCopyNode, (), ());
    MOCK_METHOD(int32_t, Config, (const char *fileName, const char *encoding, int32_t options), ());
    MOCK_METHOD(void, MoveToNext, (), ());
    MOCK_METHOD(void, MoveToChildren, (), ());
    MOCK_METHOD(bool, IsNodeValid, (), ());
    MOCK_METHOD(bool, HasProp, (const char *propName), ());
    MOCK_METHOD(int32_t, GetProp, (const char *propName, std::string &result), ());
    MOCK_METHOD(int32_t, GetContent, (std::string &result), ());
    MOCK_METHOD(std::string, GetName, (), ());
    MOCK_METHOD(bool, CompareName, (const char *propName), ());
    MOCK_METHOD(bool, IsElementNode, (), ());
    MOCK_METHOD(void, FreeDoc, (), ());
    MOCK_METHOD(void, FreeProp, (char *propName), ());
};
}  // namespace AudioStandard
}  // namespace OHOS
 
#endif // AUDIO_XML_NODE_MOCK_H