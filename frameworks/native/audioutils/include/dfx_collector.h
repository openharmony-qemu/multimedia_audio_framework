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

#ifndef ST_DFX_COLLECTOR_H
#define ST_DFX_COLLECTOR_H

#include <map>
#include <list>

#include "dfx_stat.h"
#include "dfx_utils.h"
#include "audio_common_log.h"

namespace OHOS {
namespace AudioStandard {

template<typename T>
class DfxCollector {
public:
    virtual void FlushDfxMsg(uint32_t index, int32_t appUid) = 0;
    virtual ~DfxCollector() = default;

    void AddDfxMsg(uint32_t index, const T &info)
    {
        if (dfxInfos_.count(index) == 0) {
            std::list<T> vec{info};
            dfxInfos_.insert(std::make_pair(index, vec));
        } else {
            auto &item = dfxInfos_[index];
            item.push_back(info);
        }
    }

    bool IsExist(uint32_t index)
    {
        return dfxInfos_.count(index) != 0;
    }

    uint32_t dfxIndex_{0};
    std::map<uint32_t, std::list<T>> dfxInfos_{};
};

} // namespace AudioStandard
} // namespace OHOS

#endif // ST_DFX_COLLECTOR_H