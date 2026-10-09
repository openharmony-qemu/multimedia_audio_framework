/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "background_download"
#endif

#include "background_download.h"
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_utils.h"
#include "audio_xml_parser.h"
#include "common_event_support.h"
#include <libxml/tree.h>
#include <unistd.h>
#include <cinttypes>
#include <fcntl.h>
#include <cstdlib>
#include <cstdio>
#include <sys/stat.h>
#include <sstream>

#if !defined(AUDIOSUITE_SA_UT) && defined(AUDIO_SUITE_ROM_DOWNLOAD_NET_MANAGER)
#include "net_conn_client.h"
#endif

#if !defined(AUDIOSUITE_SA_UT) && defined(AUDIO_SUITE_ROM_DOWNLOAD_BATTERY_MANAGER)
#include "battery_info.h"
#include "battery_srv_client.h"
#endif

#if !defined(AUDIOSUITE_SA_UT) && defined(AUDIO_SUITE_ROM_DOWNLOAD_SCREENLOCK_MANAGER)
#include "screenlock_manager.h"
#endif

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

#if !defined(AUDIOSUITE_SA_UT) && defined(AUDIO_SUITE_ROM_DOWNLOAD_BATTERY_MANAGER)
static constexpr uint32_t BACKGROUND_DOWNLOAD_BATTERY_LEVEL = 50;
#endif

static constexpr int32_t MAX_XML_CONTENT_SIZE = 2048;
static constexpr int32_t MAX_RETRY_COUNT = 3;

static const std::string DOWNLOAD_STATUS_PARA_KEY = "persist.multimedia.audiosuite.download";
static const std::string DOWNLOAD_STATUS_PARA_KEY_WIFI = "persist.multimedia.audiosuite.download.wifi";
static const std::string XML_KEY_PENDING_TASKS = "audiosuite.pending_tasks";
static const std::string PERSISTENT_FILE_PATH =
    "/data/service/el2/public/audio_suite/download_pending_tasks.xml";

BackgroundDownload::EventSubscriber::EventSubscriber(BackgroundDownload& monitor,
    const EventFwk::CommonEventSubscribeInfo& subscribeInfo)
    : EventFwk::CommonEventSubscriber(subscribeInfo), monitor_(monitor)
{
}

void BackgroundDownload::EventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData& data)
{
    AUDIO_INFO_LOG("OnReceiveEvent: action=%{public}s, code=%{public}d, data=%{public}s",
        data.GetWant().GetAction().c_str(), data.GetCode(), data.GetData().c_str());
    monitor_.ProcessPendingDownloads();
}

BackgroundDownload::BackgroundDownload(std::function<int32_t(const std::string&)> downloadFunction)
    : downloadCallback_(downloadFunction)
{
}

BackgroundDownload::~BackgroundDownload()
{
    Release();
}

int32_t BackgroundDownload::Init()
{
    std::lock_guard<std::mutex> lock(mutex_);

    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_CONN_STATE);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_CHANGED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_LOCKED);

    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);

    subscriber_ = std::make_shared<EventSubscriber>(*this, subscribeInfo);
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_)) {
        AUDIO_ERR_LOG("Failed to subscribe common event, wifiConnState/batteryChanged/charging/screenLocked");
        return -1;
    }

    AUDIO_INFO_LOG("BackgroundDownload initialized successfully");
    return 0;
}

void BackgroundDownload::ProcessPendingDownloads()
{
    if (stopped_.load()) {
        AUDIO_INFO_LOG("BackgroundDownload stopped, skip ProcessPendingDownloads");
        return;
    }
    std::vector<std::pair<std::string, int32_t>> toProcess;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        CHECK_AND_RETURN_LOG(!processDownloadMap_.empty() && downloadCallback_ != nullptr,
                             "backgroundDownload map empty or callback null.");
        AUDIO_INFO_LOG("start process BackgroundDownload.");
        for (auto it = processDownloadMap_.begin(); it != processDownloadMap_.end();) {
            if (!AreConditionsMet(it->first)) {
                ++it;
                continue;
            }
            toProcess.emplace_back(it->first, it->second);
            it = processDownloadMap_.erase(it);
        }
    }

    std::vector<std::pair<std::string, int32_t>> failedFeatures;
    for (const auto& [featureName, oldRetryCount] : toProcess) {
        if (stopped_) {
            failedFeatures.emplace_back(featureName, oldRetryCount);
            continue;
        }
        int32_t ret = downloadCallback_(featureName);
        if (ret == SUCCESS) {
            AUDIO_INFO_LOG("Feature %{public}s download submitted successfully", featureName.c_str());
            continue;
        }
        AUDIO_ERR_LOG("Feature %{public}s download submit failed", featureName.c_str());
        int32_t retryCount = oldRetryCount + 1;
        if (retryCount >= MAX_RETRY_COUNT) {
            AUDIO_ERR_LOG("Feature %{public}s reached max retry count %{public}d, discarding",
                featureName.c_str(), MAX_RETRY_COUNT);
        } else {
            failedFeatures.emplace_back(featureName, retryCount);
        }
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& [featureName, retryCount] : failedFeatures) {
            if (processDownloadMap_.find(featureName) != processDownloadMap_.end()) {
                AUDIO_INFO_LOG("Feature %{public}s already in map, skip put back", featureName.c_str());
                continue;
            }
            processDownloadMap_[featureName] = retryCount;
        }
        AUDIO_INFO_LOG("finish process BackgroundDownload, size:%{public}zu", processDownloadMap_.size());
        PersistPendingQueueLocked();
    }
}

void BackgroundDownload::Release()
{
    std::shared_ptr<EventSubscriber> subscriber;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        CHECK_AND_RETURN_LOG(subscriber_ != nullptr,
                             "subscriber_ is nullptr, skip unsubscribe");
        subscriber = std::move(subscriber_);
    }
    EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber);
    AUDIO_INFO_LOG("Release finish, BackgroundDownload remove subscriber");
}

bool BackgroundDownload::AreConditionsMet(const std::string& featureName)
{
    const auto& config = GetFeatureConfig(featureName);
    bool met = true;
    if (config.requireWifi) {
        met = met && IsWifiConnected();
    }
    if (config.requireCharging) {
        met = met && IsCharging();
    }
    if (config.requireBatteryLevel) {
        met = met && IsBatteryLevelMet();
    }
    if (config.requireScreenLocked) {
        met = met && IsScreenLocked();
    }
    return met;
}

const DownloadConditionConfig& BackgroundDownload::GetFeatureConfig(const std::string& featureName)
{
    static const DownloadConditionConfig defaultConfig;
    static const std::unordered_map<std::string, DownloadConditionConfig> featureConfigs = {
        {"AudioSourceSeparationAlgoRes", DownloadConditionConfig {true, false, false, false}},
        {"RecordDefinition", DownloadConditionConfig {true, false, false, false}},
    };
    auto it = featureConfigs.find(featureName);
    if (it != featureConfigs.end()) {
        return it->second;
    }
    return defaultConfig;
}

bool BackgroundDownload::IsWifiConnected()
{
#if !defined(AUDIOSUITE_SA_UT) && defined(AUDIO_SUITE_ROM_DOWNLOAD_NET_MANAGER)
    NetManagerStandard::NetHandle handle;
    int32_t ret = NetManagerStandard::NetConnClient::GetInstance().GetDefaultNet(handle);
    CHECK_AND_RETURN_RET_LOG(ret == 0, false, "GetDefaultNet failed, ret:%{public}d", ret);
    NetManagerStandard::NetAllCapabilities netAllCap;
    int32_t capRet = NetManagerStandard::NetConnClient::GetInstance().GetNetCapabilities(handle, netAllCap);
    CHECK_AND_RETURN_RET_LOG(capRet == 0, false, "GetNetCapabilities failed, ret:%{public}d", capRet);
    const std::set<NetManagerStandard::NetBearType> &types = netAllCap.bearerTypes_;
    return types.count(NetManagerStandard::BEARER_WIFI);
#else
    return true;
#endif
}

bool BackgroundDownload::IsCharging()
{
#if !defined(AUDIOSUITE_SA_UT) && defined(AUDIO_SUITE_ROM_DOWNLOAD_BATTERY_MANAGER)
    auto &batteryService = OHOS::PowerMgr::BatterySrvClient::GetInstance();
    PowerMgr::BatteryChargeState chargeState = batteryService.GetChargingStatus();
    return chargeState == PowerMgr::BatteryChargeState::CHARGE_STATE_ENABLE ||
           chargeState == PowerMgr::BatteryChargeState::CHARGE_STATE_FULL;
#else
    return true;
#endif
}

bool BackgroundDownload::IsBatteryLevelMet()
{
#if !defined(AUDIOSUITE_SA_UT) && defined(AUDIO_SUITE_ROM_DOWNLOAD_BATTERY_MANAGER)
    auto &batteryService = OHOS::PowerMgr::BatterySrvClient::GetInstance();
    int32_t capacity = batteryService.GetCapacity();
    CHECK_AND_RETURN_RET_LOG(capacity >= 0, false,
        "GetCapacity failed, capacity:%{public}d", capacity);
    return static_cast<uint32_t>(capacity) > BACKGROUND_DOWNLOAD_BATTERY_LEVEL;
#else
    return true;
#endif
}

bool BackgroundDownload::IsScreenLocked()
{
#if !defined(AUDIOSUITE_SA_UT) && defined(AUDIO_SUITE_ROM_DOWNLOAD_SCREENLOCK_MANAGER)
    auto screenLockMgr = OHOS::ScreenLock::ScreenLockManager::GetInstance();
    return screenLockMgr->IsScreenLocked();
#else
    return true;
#endif
}

int32_t BackgroundDownload::AddFeature(const std::string& featureName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    bool alreadyExists = processDownloadMap_.find(featureName) != processDownloadMap_.end();
    processDownloadMap_[featureName] = 0;
    AUDIO_INFO_LOG("AddFeature featureName:%{public}s to map, alreadyExists:%{public}d, size:%{public}zu",
        featureName.c_str(), alreadyExists, processDownloadMap_.size());
    PersistPendingQueueLocked();
    return SUCCESS;
}

int32_t BackgroundDownload::RemoveFeature(const std::string& featureName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    processDownloadMap_.erase(featureName);
    AUDIO_INFO_LOG("featureName:%{public}s already remove from map, size:%{public}zu",
        featureName.c_str(), processDownloadMap_.size());
    PersistPendingQueueLocked();
    return SUCCESS;
}

void BackgroundDownload::PersistPendingQueueLocked()
{
    std::string featureList;
    for (const auto& [featureName, retryCount] : processDownloadMap_) {
        if (!featureList.empty()) {
            featureList += ";";
        }
        featureList += featureName + ":" + std::to_string(retryCount);
    }
    AUDIO_INFO_LOG("PersistPendingQueue: %{public}s", featureList.c_str());
    WritePendingTasksFile(featureList);
    UpdateDownloadStatusPara(featureList);
}

void BackgroundDownload::RestorePendingFeatures(const std::string& featureList)
{
    if (featureList.empty()) {
        AUDIO_INFO_LOG("RestorePendingFeatures: empty, skip");
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    std::istringstream stream(featureList);
    std::string token;
    while (std::getline(stream, token, ';')) {
        if (token.empty()) {
            continue;
        }
        size_t pos = token.find(':');
        std::string featureName = token.substr(0, pos);
        int32_t retryCount = atoi(token.substr(pos + 1).c_str());
        if (processDownloadMap_.find(featureName) == processDownloadMap_.end()) {
            processDownloadMap_[featureName] = retryCount;
            AUDIO_INFO_LOG("RestorePendingFeatures: restored %{public}s, retryCount=%{public}d",
                featureName.c_str(), retryCount);
        }
    }
    AUDIO_INFO_LOG("RestorePendingFeatures done, map size: %{public}zu", processDownloadMap_.size());
}

std::string BackgroundDownload::BuildXmlContent(const std::string& value)
{
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    CHECK_AND_RETURN_RET_LOG(doc != nullptr, "", "BuildXmlContent: xmlNewDoc failed");

    xmlNodePtr root = xmlNewDocNode(doc, nullptr, BAD_CAST "map", nullptr);
    if (root == nullptr) {
        AUDIO_ERR_LOG("BuildXmlContent: xmlNewDocNode failed");
        xmlFreeDoc(doc);
        return "";
    }
    xmlDocSetRootElement(doc, root);

    xmlNodePtr strNode = xmlNewChild(root, nullptr, BAD_CAST "string", BAD_CAST value.c_str());
    if (strNode == nullptr) {
        AUDIO_ERR_LOG("BuildXmlContent: xmlNewChild failed");
        xmlFreeDoc(doc);
        return "";
    }
    xmlSetProp(strNode, BAD_CAST "name", BAD_CAST XML_KEY_PENDING_TASKS.c_str());

    xmlChar* buffer = nullptr;
    int size = 0;
    xmlDocDumpMemory(doc, &buffer, &size);
    std::string result;
    if (buffer == nullptr) {
        AUDIO_ERR_LOG("BuildXmlContent: xmlDocDumpMemory failed, buffer is null");
        xmlFreeDoc(doc);
        return "";
    }
    if (size > MAX_XML_CONTENT_SIZE) {
        AUDIO_ERR_LOG("BuildXmlContent: size=%{public}d exceeds max=%{public}d", size, MAX_XML_CONTENT_SIZE);
        xmlFree(buffer);
        xmlFreeDoc(doc);
        return "";
    }
    if (size > 0) {
        result.assign(reinterpret_cast<char*>(buffer), static_cast<size_t>(size));
    }
    xmlFree(buffer);
    xmlFreeDoc(doc);
    return result;
}

std::string BackgroundDownload::ReadPendingTasksFromFile()
{
    struct stat st;
    CHECK_AND_RETURN_RET_LOG(
        stat(PERSISTENT_FILE_PATH.c_str(), &st) == 0, "", "ReadPendingTasksFromFile: No pending downloads file");
    CHECK_AND_RETURN_RET_LOG(st.st_size <= MAX_XML_CONTENT_SIZE,
                             "", "ReadPendingTasksFromFile: File size %{public}" PRId64 "exceeds max %{public}d, "
                             "possible tampering, skipping", st.st_size, MAX_XML_CONTENT_SIZE);

    std::shared_ptr<AudioXmlNode> rootNode = AudioXmlNode::Create();
    CHECK_AND_RETURN_RET_LOG(rootNode != nullptr, "", "ReadPendingTasksFromFile: Failed to create AudioXmlNode");
    CHECK_AND_RETURN_RET_LOG(rootNode->Config(PERSISTENT_FILE_PATH.c_str(), nullptr, 0) == SUCCESS,
                             "", "ReadPendingTasksFromFile: No pending downloads file");

    std::string value;
    std::shared_ptr<AudioXmlNode> childNode = rootNode->GetChildrenNode();
    for (; childNode && childNode->IsNodeValid(); childNode->MoveToNext()) {
        if (!childNode->IsElementNode()) {
            continue;
        }
        if (!childNode->CompareName("string")) {
            continue;
        }
        std::string name;
        if (childNode->GetProp("name", name) == SUCCESS && name == XML_KEY_PENDING_TASKS) {
            childNode->GetContent(value);
            break;
        }
    }
    rootNode->FreeDoc();
    return value;
}

void BackgroundDownload::UpdateDownloadStatusPara(const std::string& featureList)
{
    bool hasFullConditionFeature = false;
    bool hasWifiOnlyFeature = false;
    if (!featureList.empty()) {
        std::istringstream stream(featureList);
        std::string token;
        while (std::getline(stream, token, ';')) {
            if (token.empty()) {
                continue;
            }
            size_t pos = token.find(':');
            std::string featureName = token.substr(0, pos);
            const auto& config = GetFeatureConfig(featureName);
            if (config.requireWifi && !config.requireCharging &&
                !config.requireBatteryLevel && !config.requireScreenLocked) {
                hasWifiOnlyFeature = true;
            } else {
                hasFullConditionFeature = true;
            }
            if (hasWifiOnlyFeature && hasFullConditionFeature) {
                break;
            }
        }
    }
    SetSysPara(DOWNLOAD_STATUS_PARA_KEY, hasFullConditionFeature ? 1 : 0);
    SetSysPara(DOWNLOAD_STATUS_PARA_KEY_WIFI, hasWifiOnlyFeature ? 1 : 0);
    AUDIO_INFO_LOG("UpdateDownloadStatusPara: full=%{public}d, wifi=%{public}d",
        hasFullConditionFeature ? 1 : 0, hasWifiOnlyFeature ? 1 : 0);
}

void BackgroundDownload::DeletePendingTasksFile()
{
    CHECK_AND_RETURN_LOG(unlink(PERSISTENT_FILE_PATH.c_str()) == 0 || errno == ENOENT,
                         "Failed to delete XML file: errno=%{public}d", errno);
    AUDIO_INFO_LOG("Pending queue empty, XML file deleted");
}

void BackgroundDownload::ClearPersistedData()
{
    DeletePendingTasksFile();
    SetSysPara(DOWNLOAD_STATUS_PARA_KEY, 0);
    SetSysPara(DOWNLOAD_STATUS_PARA_KEY_WIFI, 0);
    AUDIO_INFO_LOG("ClearPersistedData done");
}

void BackgroundDownload::WritePendingTasksFile(const std::string& featureList)
{
    if (featureList.empty()) {
        DeletePendingTasksFile();
        return;
    }

    std::string xmlContent = BuildXmlContent(featureList);
    CHECK_AND_RETURN_LOG(!xmlContent.empty(),
                         "WritePendingTasksFile: BuildXmlContent failed, skip write to preserve existing file");
    FILE* fp = fopen(PERSISTENT_FILE_PATH.c_str(), "w");
    CHECK_AND_RETURN_LOG(fp != nullptr, "Failed to open file: errno=%{public}d", errno);
    size_t written = fwrite(xmlContent.c_str(), 1, xmlContent.size(), fp);
    if (written != xmlContent.size()) {
        AUDIO_ERR_LOG("Write failed: written=%{public}zu, expected=%{public}zu, errno=%{public}d",
            written, xmlContent.size(), errno);
        fclose(fp);
        unlink(PERSISTENT_FILE_PATH.c_str());
        return;
    }
    JUDGE_AND_ERR_LOG(fflush(fp) != 0, "fflush failed: errno=%{public}d", errno);
    JUDGE_AND_ERR_LOG(fsync(fileno(fp)) != 0, "fsync failed: errno=%{public}d", errno);
    CHECK_AND_RETURN_LOG(fclose(fp) == 0, "fclose failed: errno=%{public}d", errno);
    AUDIO_INFO_LOG("Persisted pending downloads: %{public}s", featureList.c_str());
}

void BackgroundDownload::RestoreFromFile(std::shared_ptr<BackgroundDownload> selfPtr,
    bool syncProcess)
{
    CHECK_AND_RETURN_LOG(selfPtr != nullptr, "BackgroundDownload is a null pointer.");
    std::string value = ReadPendingTasksFromFile();
    if (value.empty()) {
        AUDIO_INFO_LOG("No pending downloads to restore");
        std::lock_guard<std::mutex> lock(mutex_);
        PersistPendingQueueLocked();
        return;
    }
    RestorePendingFeatures(value);
    AUDIO_INFO_LOG("Restored pending downloads to map: %{public}s, syncProcess: %{public}d",
        value.c_str(), syncProcess);

    if (syncProcess) {
        AUDIO_INFO_LOG("syncProcess enabled, process pending downloads directly");
        ProcessPendingDownloads();
        return;
    }

    std::thread processThread([selfPtr]() {
        AUDIO_INFO_LOG("ProcessPendingDownloads start after 1min delay.");
        constexpr auto PROCESS_DELAY_MS = std::chrono::milliseconds(150000); // 2.5 minute
        std::this_thread::sleep_for(PROCESS_DELAY_MS);
        AUDIO_INFO_LOG("ProcessPendingDownloads Starting to process queue tasks.");
        selfPtr->ProcessPendingDownloads();
        AUDIO_INFO_LOG("ProcessPendingDownloads end after 1min delay.");
    });
    processThread.detach();
}

void BackgroundDownload::PersistOnExit()
{
    stopped_ = true;
    std::lock_guard<std::mutex> lock(mutex_);
    PersistPendingQueueLocked();
    AUDIO_INFO_LOG("PersistOnExit done, map size:%{public}zu", processDownloadMap_.size());
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS