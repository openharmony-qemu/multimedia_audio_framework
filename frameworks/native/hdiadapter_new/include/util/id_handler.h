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

#ifndef ID_HANDLER_H
#define ID_HANDLER_H

#include <iostream>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include "audio_info.h"
#include "common/hdi_adapter_info.h"

namespace OHOS {
namespace AudioStandard {
typedef struct InfoAttr {
    std::string info_ = HDI_ID_INFO_DEFAULT;
    std::atomic<uint32_t> useCount_ = 0;
} InfoAttr;

class IdHandler {
public:
    static IdHandler &GetInstance(void);

    uint32_t GetId(HdiIdBase base, HdiIdType type, const std::string &info = HDI_ID_INFO_DEFAULT);
    uint32_t GetRenderIdByDeviceClass(const std::string &deviceClass,
        const std::string &info = HDI_ID_INFO_DEFAULT);
    uint32_t GetCaptureIdByDeviceClass(const std::string &deviceClass, const SourceType sourceType,
        const std::string &info = HDI_ID_INFO_DEFAULT);

    void IncInfoIdUseCount(uint32_t id);
    void DecInfoIdUseCount(uint32_t id);

    bool CheckId(uint32_t id, HdiIdBase requireBase);

    uint32_t ParseBase(uint32_t id);
    uint32_t ParseType(uint32_t id);
    std::string ParseInfo(uint32_t id);

private:
    IdHandler() = default;
    ~IdHandler() = default;
    IdHandler(const IdHandler &) = delete;
    IdHandler &operator=(const IdHandler &) = delete;
    IdHandler(IdHandler &&) = delete;
    IdHandler &operator=(IdHandler &&) = delete;

    uint32_t CreateInfoId(void);

private:
    static constexpr uint32_t HDI_ID_BASE_OFFSET = 12;
    static constexpr uint32_t HDI_ID_TYPE_OFFSET = 8;
    static constexpr uint32_t HDI_ID_BASE_MASK = 0x3000;
    static constexpr uint32_t HDI_ID_TYPE_MASK = 0x0F00;
    static constexpr uint32_t HDI_ID_INFO_MASK = 0x00FF;

    std::unordered_map<uint32_t, InfoAttr> infoIdMap_;
    std::unordered_set<uint32_t> freeInfoIdSet_;
    std::mutex infoIdMtx_;
    std::mutex freeInfoIdMtx_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // ID_HANDLER_H
