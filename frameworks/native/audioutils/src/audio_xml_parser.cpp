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

#ifndef LOG_TAG
#define LOG_TAG "AudioXmlNode"
#endif

#include "audio_xml_parser.h"
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "audio_errors.h"
#include <dlfcn.h>
#include <string>
#include <map>
#include <set>
#include <atomic>

#include "audio_common_log.h"
namespace OHOS {
namespace AudioStandard {

const char *LIBXML_SO_PATH = "libxml2.z.so";

struct XmlFuncHandle {
    void *libHandle_ = nullptr;
    xmlDoc *(*xmlReadFile)(const char *fileName, const char *encoding, int32_t options);
    xmlNode *(*xmlDocGetRootElement)(xmlDoc *doc);
    bool (*xmlHasProp)(const xmlNode *node, const xmlChar *propName);
    xmlChar *(*xmlGetProp)(const xmlNode *node, const xmlChar *propName);
    void (*xmlFreeDoc)(xmlDoc *doc); // need check if works
    void (*xmlFree)(xmlChar *content); // need check if works
    void (*xmlCleanupParser)();
    int32_t (*xmlStrcmp)(const xmlChar *propName1, const xmlChar *propName2);
    xmlChar *(*xmlNodeGetContent)(const xmlNode *cur);
};

class DlopenUtils {
public:
    static bool  Init();
    static void DeInit();
    static std::shared_ptr<XmlFuncHandle> GetHandle();
private:
    static std::atomic<int32_t> g_refCount_;
    static std::shared_ptr<XmlFuncHandle> xmlFuncHandle_;
};

std::atomic<int32_t> DlopenUtils::g_refCount_{0};
std::shared_ptr<XmlFuncHandle> DlopenUtils::xmlFuncHandle_ = nullptr;

class AudioXmlNodeInner : public AudioXmlNode {
public:
    virtual std::shared_ptr<AudioXmlNode> GetChildrenNode() override;
    virtual std::shared_ptr<AudioXmlNode> GetCopyNode() override;
    AudioXmlNodeInner();
    AudioXmlNodeInner(const AudioXmlNodeInner &obj);
    AudioXmlNodeInner &operator=(const AudioXmlNodeInner &obj);
    ~AudioXmlNodeInner() override;

    int32_t Config(const char *fileName, const char *encoding, int32_t options) override;
    virtual void MoveToNext() override;
    virtual void MoveToChildren() override;
    virtual bool IsNodeValid() override;
    virtual int32_t GetNodeType() override;

    virtual bool HasProp(const char *propName) override;
    virtual int32_t GetProp(const char *propName, std::string &result) override;
    virtual int32_t GetContent(std::string &result) override;
    virtual std::string GetName() override;

    virtual void FreeDoc() override;
    virtual void FreeProp(char *propName) override;
    virtual void CleanUpParser() override;

    virtual bool CompareName(const char *propName) override;
    virtual bool IsElementNode() override;

    int32_t StrcmpXml(const xmlChar *propName1, const xmlChar *propName2);
    xmlDoc *doc_ = nullptr;
    xmlNode *curNode_ = nullptr;
    std::shared_ptr<XmlFuncHandle> xmlFuncHandle_ = nullptr;
};

bool DlopenUtils::Init()
{
    if (g_refCount_.load() == 0) {
        void *libHandle = dlopen(LIBXML_SO_PATH, RTLD_NOW);
        CHECK_AND_RETURN_RET_LOG(libHandle != nullptr, false, "dlopen failed!");
        xmlFuncHandle_ = std::make_shared<XmlFuncHandle>();
        xmlFuncHandle_->libHandle_ = libHandle;
        xmlFuncHandle_->xmlReadFile =
            reinterpret_cast<decltype(xmlFuncHandle_->xmlReadFile)>(dlsym(libHandle, "xmlReadFile")); 
        xmlFuncHandle_->xmlDocGetRootElement =
            reinterpret_cast<decltype(xmlFuncHandle_->xmlDocGetRootElement)>(dlsym(libHandle, "xmlDocGetRootElement")); 
        xmlFuncHandle_->xmlHasProp =
            reinterpret_cast<decltype(xmlFuncHandle_->xmlHasProp)>(dlsym(libHandle, "xmlHasProp")); 
        xmlFuncHandle_->xmlGetProp =
            reinterpret_cast<decltype(xmlFuncHandle_->xmlGetProp)>(dlsym(libHandle, "xmlGetProp"));
        xmlFuncHandle_->xmlFreeDoc =
            reinterpret_cast<decltype(xmlFuncHandle_->xmlFreeDoc)>(dlsym(libHandle, "xmlFreeDoc"));
        xmlFuncHandle_->xmlFree =
            reinterpret_cast<decltype(xmlFuncHandle_->xmlFree)>(dlsym(libHandle, "xmlFree"));
        xmlFuncHandle_->xmlCleanupParser =
            reinterpret_cast<decltype(xmlFuncHandle_->xmlCleanupParser)>(dlsym(libHandle, "xmlCleanupParser"));
        xmlFuncHandle_->xmlStrcmp =
            reinterpret_cast<decltype(xmlFuncHandle_->xmlStrcmp)>(dlsym(libHandle, "xmlStrcmp"));
        xmlFuncHandle_->xmlNodeGetContent =
            reinterpret_cast<decltype(xmlFuncHandle_->xmlNodeGetContent)>(dlsym(libHandle, "xmlNodeGetContent"));
        AUDIO_INFO_LOG("Libxml2 open success");
    }
    g_refCount_.store(g_refCount_.load() + 1);
    return true;
}

void DlopenUtils::DeInit()
{
    g_refCount_.store(g_refCount_.load() - 1);
    if (g_refCount_.load() == 0 && xmlFuncHandle_.use_count() == 1) {
        dlclose(xmlFuncHandle_->libHandle_);
        xmlFuncHandle_ = nullptr;
        AUDIO_INFO_LOG("Libxml2 close success");
    }
}

std::shared_ptr<XmlFuncHandle> DlopenUtils::GetHandle()
{
    return xmlFuncHandle_;
}

std::shared_ptr<AudioXmlNode> AudioXmlNode::Create()
{
    return std::make_shared<AudioXmlNodeInner>();
}

std::shared_ptr<AudioXmlNode> AudioXmlNodeInner::GetChildrenNode()
{
    std::shared_ptr<AudioXmlNodeInner> copyNode = std::make_shared<AudioXmlNodeInner>(*this);
    copyNode->MoveToChildren();
    return copyNode;
}

std::shared_ptr<AudioXmlNode> AudioXmlNodeInner::GetCopyNode()
{
    return std::make_shared<AudioXmlNodeInner>(*this);
}

AudioXmlNodeInner::AudioXmlNodeInner()
{
    CHECK_AND_RETURN_LOG(DlopenUtils::Init(), "open so fail!");
    xmlFuncHandle_ = DlopenUtils::GetHandle();
    CHECK_AND_RETURN_LOG(xmlFuncHandle_ != nullptr, "get xmlFuncHandle failed!");
}

AudioXmlNodeInner::AudioXmlNodeInner(const AudioXmlNodeInner &obj)
{
    // only the main node has doc and freedoc() when destruct
    doc_ = nullptr;
    curNode_ = obj.curNode_;
    CHECK_AND_RETURN_LOG(DlopenUtils::Init(), "open so fail!");
    xmlFuncHandle_ = DlopenUtils::GetHandle();
}

AudioXmlNodeInner &AudioXmlNodeInner::operator=(const AudioXmlNodeInner &obj)
{
    // only the main node has doc and freedoc() when destruct
    doc_ = nullptr;
    curNode_ = obj.curNode_;
    if (!DlopenUtils::Init()) {
        AUDIO_INFO_LOG("init openUtils fail!");
    }
    xmlFuncHandle_ = DlopenUtils::GetHandle();
    return *this;
}

AudioXmlNodeInner::~AudioXmlNodeInner()
{
    if (xmlFuncHandle_ != nullptr && doc_ != nullptr) {
        xmlFuncHandle_->xmlFreeDoc(doc_);
        doc_ = nullptr;
    }
    curNode_ = nullptr;
    xmlFuncHandle_ = nullptr;
    DlopenUtils::DeInit();
}

int32_t AudioXmlNodeInner::Config(const char *fileName, const char *encoding, int32_t options)
{
    CHECK_AND_RETURN_RET_LOG(xmlFuncHandle_ != nullptr, ERROR, "xmlFuncHandle is nullptr!");
    doc_ = xmlFuncHandle_->xmlReadFile(fileName, encoding, options);
    CHECK_AND_RETURN_RET_LOG(doc_ != nullptr, ERROR, "xmlReadFile failed!");
    curNode_ = xmlFuncHandle_->xmlDocGetRootElement(doc_);
    CHECK_AND_RETURN_RET_LOG(curNode_ != nullptr, ERROR, "xmlDocGetRootElement failed!"); 
    return SUCCESS;
}

void AudioXmlNodeInner::MoveToNext()
{
    curNode_ = curNode_->next;
}

void AudioXmlNodeInner::MoveToChildren()
{
    curNode_ = curNode_->children;
}

bool AudioXmlNodeInner::IsNodeValid()
{
    return curNode_ != nullptr;
}

int32_t AudioXmlNodeInner::GetNodeType()
{
    return curNode_->type;
}

// need check curNode_ isvalid before use
bool AudioXmlNodeInner::HasProp(const char *propName)
{
    CHECK_AND_RETURN_RET_LOG(xmlFuncHandle_ != nullptr, false, "xmlFuncHandle is nullptr!");
    return xmlFuncHandle_->xmlHasProp(curNode_, reinterpret_cast<const xmlChar*>(propName));
}

// need check curNode_ isvalid before use
int32_t AudioXmlNodeInner::GetProp(const char *propName, std::string &result)
{
    CHECK_AND_RETURN_RET_LOG(xmlFuncHandle_ != nullptr, ERROR, "xmlFuncHandle is nullptr!");
    auto xmlFunc = reinterpret_cast<xmlChar *(*)(const xmlNode *node, const xmlChar *propName)>
        (dlsym(xmlFuncHandle_->libHandle_, "xmlGetProp"));
    xmlChar *tempValue = xmlFunc(curNode_, reinterpret_cast<const xmlChar*>(propName));
    CHECK_AND_RETURN_RET_LOG(tempValue != nullptr, ERROR, "GetProp Fail! curNode has no prop: %{public}s", propName);
    result = reinterpret_cast<char*>(tempValue);
    xmlFuncHandle_->xmlFree(tempValue);
    return SUCCESS;
}

int32_t AudioXmlNodeInner::GetContent(std::string &result)
{
    CHECK_AND_RETURN_RET_LOG(xmlFuncHandle_ != nullptr, ERROR, "xmlFuncHandle is nullptr!");
    xmlChar *tempContent = xmlFuncHandle_->xmlNodeGetContent(curNode_);
    CHECK_AND_RETURN_RET_LOG(tempContent != nullptr, ERROR, "GetContent Fail!");
    result = reinterpret_cast<char*>(tempContent);
    xmlFuncHandle_->xmlFree(tempContent);
    return SUCCESS;
}

std::string AudioXmlNodeInner::GetName()
{
    return reinterpret_cast<char*>(curNode_->name);
}

void AudioXmlNodeInner::FreeDoc()
{
    CHECK_AND_RETURN_LOG(xmlFuncHandle_ != nullptr, "xmlFuncHandle is nullptr!");
    if (doc_ != nullptr) {
        xmlFuncHandle_->xmlFreeDoc(doc_);
    }
}

void AudioXmlNodeInner::FreeProp(char *propName)
{
    CHECK_AND_RETURN_LOG(xmlFuncHandle_ != nullptr, "xmlFuncHandle is nullptr!");
    xmlFuncHandle_->xmlFree(reinterpret_cast<xmlChar*>(propName));
}

void AudioXmlNodeInner::CleanUpParser()
{
    CHECK_AND_RETURN_LOG(xmlFuncHandle_ != nullptr, "xmlFuncHandle is nullptr!");
    xmlFuncHandle_->xmlCleanupParser();
}

int32_t AudioXmlNodeInner::StrcmpXml(const xmlChar *propName1, const xmlChar *propName2)
{
    CHECK_AND_RETURN_RET_LOG(xmlFuncHandle_ != nullptr, 1, "xmlFuncHandle is nullptr!");
    return xmlFuncHandle_->xmlStrcmp(propName1, propName2);
}

bool AudioXmlNodeInner::CompareName(const char *propName)
{
    return curNode_->type == XML_ELEMENT_NODE &&
        (StrcmpXml(curNode_->name, reinterpret_cast<const xmlChar*>(propName)) == 0);
}

bool AudioXmlNodeInner::IsElementNode()
{
    return curNode_->type == XML_ELEMENT_NODE;
}

} // namespace AudioStandard
} // namespace OHOS
