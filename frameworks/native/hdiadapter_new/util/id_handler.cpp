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
#define LOG_TAG "IdHandler"
#endif

#include "util/id_handler.h"
#include "audio_hdi_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
IdHandler &IdHandler::GetInstance(void)
{
    static IdHandler instance;
    return instance;
}

uint32_t IdHandler::GetId(HdiIdBase base, HdiIdType type, const std::string &info)
{
    CHECK_AND_RETURN_RET_LOG(base < HDI_ID_BASE_NUM, HDI_INVALID_ID, "invalid id base %{public}u", base);
    CHECK_AND_RETURN_RET_LOG(type < HDI_ID_TYPE_NUM, HDI_INVALID_ID, "invalid id type %{public}u", type);

    uint32_t id = (base << HDI_ID_BASE_OFFSET) | (type << HDI_ID_TYPE_OFFSET);
    std::lock_guard<std::mutex> lock(infoIdMtx_);
    for (auto &attr : infoIdMap_) {
        if (attr.second.info_ == info) {
            id |= attr.first;
            return id;
        }
    }
    uint32_t infoId = CreateInfoId();
    infoIdMap_[infoId].info_ = info;
    id |= infoId;
    return id;
}

uint32_t IdHandler::GetRenderIdByDeviceClass(const std::string &deviceClass, const std::string &info)
{
    CHECK_AND_RETURN_RET_LOG(!deviceClass.empty(), HDI_INVALID_ID, "invalid device class");

    if (deviceClass == "primary") {
        return GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_DEFAULT);
    } else if (deviceClass == "usb") {
        return GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_USB);
    } else if (deviceClass == "dp") {
        return GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_DP);
    } else if (deviceClass == "a2dp") {
        return GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_BLUETOOTH, HDI_ID_INFO_DEFAULT);
    } else if (deviceClass == "a2dp_fast") {
        return GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_BLUETOOTH, HDI_ID_INFO_MMAP);
    } else if (deviceClass == "hearing_aid") {
        return GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_BLUETOOTH, HDI_ID_INFO_HEARING_AID);
#ifdef FEATURE_FILE_IO
    } else if (deviceClass == "file_io") {
        return GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_FILE, HDI_ID_INFO_DEFAULT);
#endif
#ifdef FEATURE_DISTRIBUTE_AUDIO
    } else if (deviceClass == "remote") {
        return GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_REMOTE, info);
    } else if (deviceClass == "remote_offload") {
        return GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_REMOTE_OFFLOAD, info);
#endif
    } else if (deviceClass == "offload") {
        return GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_OFFLOAD, HDI_ID_INFO_DEFAULT);
    } else if (deviceClass == "multichannel") {
        return GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_MULTICHANNEL, HDI_ID_INFO_DEFAULT);
    } else if (deviceClass == "dp_multichannel") {
        return GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_MULTICHANNEL, HDI_ID_INFO_DP);
    } else if (deviceClass == "primary_direct_voip") {
        return GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_VOIP);
    } else if (deviceClass == "primary_mmap_voip") {
        return GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_FAST, HDI_ID_INFO_VOIP);
    } else if (deviceClass == "primary_mmap") {
        return GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_FAST, HDI_ID_INFO_DEFAULT);
    } else if (deviceClass == "Virtual_Injector") {
        return GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_VIRTUAL_INJECTOR, HDI_ID_INFO_DEFAULT);
    } else if (deviceClass == "primary_direct") {
        return GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_DIRECT);
    }
    AUDIO_ERR_LOG("invalid param, deviceClass: %{public}s, info: %{public}s", deviceClass.c_str(), info.c_str());
    return HDI_INVALID_ID;
}

uint32_t IdHandler::GetCaptureIdByDeviceClass(const std::string &deviceClass, const SourceType sourceType,
    const std::string &info)
{
    CHECK_AND_RETURN_RET_LOG(!deviceClass.empty(), HDI_INVALID_ID, "invalid device class");
    AUDIO_INFO_LOG("deviceClass: %{public}s, sourceType: %{public}d, info: %{public}s", deviceClass.c_str(), sourceType,
        info.c_str());

    if (deviceClass == "primary") {
        if (sourceType == SOURCE_TYPE_WAKEUP) {
            return GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_WAKEUP, info);
        }
        if (info == HDI_ID_INFO_EC || info == HDI_ID_INFO_MIC_REF) {
            return GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY, info);
        }
        if (sourceType == SOURCE_TYPE_VOICE_TRANSCRIPTION) {
            return GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_AI, HDI_ID_INFO_DEFAULT);
        }
        return GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_DEFAULT);
    } else if (deviceClass == "va") {
        return GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_VA, HDI_ID_INFO_VA);
    } else if (deviceClass == "usb") {
        return GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_USB);
    } else if (deviceClass == "a2dp") {
        return GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_BLUETOOTH, HDI_ID_INFO_DEFAULT);
#ifdef FEATURE_FILE_IO
    } else if (deviceClass == "file_io") {
        return GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_FILE, HDI_ID_INFO_DEFAULT);
#endif
#ifdef FEATURE_DISTRIBUTE_AUDIO
    } else if (deviceClass == "remote") {
        return GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_REMOTE, info);
#endif
    } else if (deviceClass == "accessory") {
        return GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_ACCESSORY, HDI_ID_INFO_ACCESSORY);
    } else if (deviceClass == "offload") {
        return GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_OFFLOAD, HDI_ID_INFO_DEFAULT);
    }

    AUDIO_ERR_LOG("invalid param, deviceClass: %{public}s, sourceType: %{public}d, info: %{public}s",
        deviceClass.c_str(), sourceType, info.c_str());
    return HDI_INVALID_ID;
}

void IdHandler::IncInfoIdUseCount(uint32_t id)
{
    uint32_t infoId = id & HDI_ID_INFO_MASK;
    std::lock_guard<std::mutex> lock(infoIdMtx_);
    CHECK_AND_RETURN_LOG(infoIdMap_.count(infoId) != 0, "invalid id %{public}u", id);
    std::lock_guard<std::mutex> useIdLock(infoIdMap_[infoId].useIdMtx_);
    infoIdMap_[infoId].useIdSet_.insert(id);
    AUDIO_INFO_LOG("infoId: %{public}u, useCount: %{public}zu", infoId, infoIdMap_[infoId].useIdSet_.size());
}

void IdHandler::DecInfoIdUseCount(uint32_t id)
{
    uint32_t infoId = id & HDI_ID_INFO_MASK;
    std::lock_guard<std::mutex> lock(infoIdMtx_);
    CHECK_AND_RETURN_LOG(infoIdMap_.count(infoId) != 0, "invalid id %{public}u", id);
    std::unique_lock<std::mutex> useIdLock(infoIdMap_[infoId].useIdMtx_);
    infoIdMap_[infoId].useIdSet_.erase(id);
    AUDIO_INFO_LOG("infoId: %{public}u, useCount: %{public}zu", infoId, infoIdMap_[infoId].useIdSet_.size());
    CHECK_AND_RETURN(infoIdMap_[infoId].useIdSet_.size() == 0);
    useIdLock.unlock();
    infoIdMap_.erase(infoId);
    std::lock_guard<std::mutex> freeLock(freeInfoIdMtx_);
    freeInfoIdSet_.emplace(infoId);
}

bool IdHandler::CheckId(uint32_t id, HdiIdBase requireBase)
{
    CHECK_AND_RETURN_RET_LOG(ParseBase(id) == requireBase, false, "invalid id base, id: %{public}u, "
        "requireBase: %{public}u", id, requireBase);
    CHECK_AND_RETURN_RET_LOG(ParseType(id) < HDI_ID_TYPE_NUM, false, "invalid id type");
    CHECK_AND_RETURN_RET_LOG(!ParseInfo(id).empty(), false, "invalid id info");
    return true;
}

uint32_t IdHandler::ParseBase(uint32_t id)
{
    return (id & HDI_ID_BASE_MASK) >> HDI_ID_BASE_OFFSET;
}

uint32_t IdHandler::ParseType(uint32_t id)
{
    return (id & HDI_ID_TYPE_MASK) >> HDI_ID_TYPE_OFFSET;
}

std::string IdHandler::ParseInfo(uint32_t id)
{
    std::lock_guard<std::mutex> lock(infoIdMtx_);
    uint32_t infoId = id & HDI_ID_INFO_MASK;
    CHECK_AND_RETURN_RET_LOG(infoIdMap_.count(infoId) != 0, "", "invalid id %{public}u", id);
    return infoIdMap_[infoId].info_;
}

uint32_t IdHandler::CreateInfoId(void)
{
    std::lock_guard<std::mutex> lock(freeInfoIdMtx_);

    if (freeInfoIdSet_.empty()) {
        return infoIdMap_.size();
    }

    uint32_t infoId = *freeInfoIdSet_.begin();
    freeInfoIdSet_.erase(infoId);
    return infoId;
}

} // namespace AudioStandard
} // namespace OHOS
