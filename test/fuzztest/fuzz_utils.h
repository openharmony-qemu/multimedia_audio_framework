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
#ifndef FUZZ_UTILS_H
#define FUZZ_UTILS_H

#include <cstdint>
#include <securec.h>
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {

typedef void (*TestFuncs)();
class FuzzUtils {
public:
    static FuzzUtils& GetInstance()
    {
        static FuzzUtils instance;
        return instance;
    }

    void fuzzTest(const uint8_t *rawData, size_t size, std::vector<TestFuncs> &testFunctions)
    {
        if (rawData == nullptr) {
            return;
        }

        rawData_ = rawData;
        dataSize_ = size;
        pos_ = 0;

        uint32_t code = GetData<uint32_t>();
        uint32_t len = testFunctions.size();
        if (len > 0) {
            testFunctions[code % len]();
        } else {
            AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
        }
        return;
    }

    template<class T>
    T GetData()
    {
        T object {};
        size_t objectSize = sizeof(object);
        if (dataSize_ <= pos_) {
            return object;
        }
        if (rawData_ == nullptr || objectSize > dataSize_ - pos_) {
            return object;
        }
        errno_t ret = memcpy_s(&object, objectSize, rawData_ + pos_, objectSize);
        if (ret != EOK) {
            return {};
        }
        pos_ += objectSize;
        return object;
    }

private:
    FuzzUtils() = default;
    ~FuzzUtils() = default;

    // Disable copy and move
    FuzzUtils(const FuzzUtils&) = delete;
    FuzzUtils& operator=(const FuzzUtils&) = delete;
    FuzzUtils(FuzzUtils&&) = delete;
    FuzzUtils& operator=(FuzzUtils&&) = delete;

    const uint8_t *rawData_ = nullptr;
    size_t dataSize_ = 0;
    size_t pos_ = 0;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // FUZZ_UTILS_H