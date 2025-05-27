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

#ifndef KV_PAIR_H
#define KV_PAIR_H

#include <iostream>
#include <unordered_map>
#include <mutex>
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
class Value {
public:
    Value() = default;
    template<typename T>
    explicit Value(const T &v) : dataWrapper_(new DataWrapperImpl<T>(v)) {}
    ~Value() = default;
    Value(const Value &) = delete;
    Value &operator=(const Value &) = delete;
    Value(const Value &&) = delete;
    Value &operator=(const Value &&) = delete;

    template<typename T>
    void SetData(const T &v)
    {
        dataWrapper_.reset(new DataWrapperImpl<T>(v));
    }
    template<typename T>
    int32_t GetData(T &v)
    {
        if (dataWrapper_ == nullptr) {
            return ERR_INVALID_HANDLE;
        }
        v = static_cast<DataWrapperImpl<T> *>(dataWrapper_.get())->data_;
        return SUCCESS;
    }

private:
    struct DataWrapper {
        virtual ~DataWrapper() = default;
    };

    template<typename T>
    struct DataWrapperImpl : DataWrapper {
        explicit DataWrapperImpl(const T &data) : data_(data) {}

        T data_;
    };
    std::unique_ptr<DataWrapper> dataWrapper_;
};

template<typename K>
class KvPair {
public:
    template<typename V>
    void Set(const K &key, const V &value)
    {
        std::lock_guard<std::mutex> lock(kvMtx_);
        kv_[key].template SetData<V>(value);
    }
    template<typename V>
    int32_t Get(const K &key, V &value)
    {
        std::lock_guard<std::mutex> lock(kvMtx_);
        if (kv_.count(key) == 0) {
            return ERR_INVALID_HANDLE;
        }
        return kv_[key].template GetData<V>(value);
    }
    void Erase(const K &key)
    {
        std::lock_guard<std::mutex> lock(kvMtx_);
        kv_.erase(key);
    }

private:
    std::unordered_map<K, Value> kv_;
    std::mutex kvMtx_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // KV_PAIR_H
