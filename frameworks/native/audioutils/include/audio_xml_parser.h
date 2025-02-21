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
#ifndef AUDIO_XML_NODE_H
#define AUDIO_XML_NODE_H

#include <memory>
#include <mutex>

namespace OHOS {
namespace AudioStandard {

struct XmlFuncHandle;

class DlopenUtils {
public:
    static bool Init();
    static void DeInit();
    static std::shared_ptr<XmlFuncHandle> GetHandle();
private:
    static std::atomic<int32_t> refCount_;
    static std::shared_ptr<XmlFuncHandle> xmlFuncHandle_;
    static std::mutex dlMutex_;
};

class AudioXmlNode {
public:
    static std::shared_ptr<AudioXmlNode> Create();

    AudioXmlNode() = default;
    virtual ~AudioXmlNode() = default;

    virtual std::shared_ptr<AudioXmlNode> GetChildrenNode() = 0;
    virtual std::shared_ptr<AudioXmlNode> GetCopyNode() = 0;
    virtual int32_t Config(const char *fileName, const char *encoding, int32_t options) = 0;
    virtual void MoveToNext() = 0;
    virtual void MoveToChildren() = 0;

    virtual bool IsNodeValid() = 0;
    virtual bool HasProp(const char *propName) = 0;
    virtual int32_t GetProp(const char *propName, std::string &result) = 0;
    virtual int32_t GetContent(std::string &result) = 0;
    virtual std::string GetName() = 0;
    virtual bool CompareName(const char *propName) = 0;
    virtual bool IsElementNode() = 0;

    virtual void FreeDoc() = 0;
    virtual void FreeProp(char *propName) = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_XML_NODE_H