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
#ifndef TAIHE_WORK_H
#define TAIHE_WORK_H

#include <memory>

#include "ohos.multimedia.audio.proj.hpp"
#include "ohos.multimedia.audio.impl.hpp"
#include "taihe/runtime.hpp"
#include "audio_log.h"

namespace ANI::Audio {

struct AutoRef {
    AutoRef(std::shared_ptr<uintptr_t> cb)
    {
        if (cb != nullptr) {
            cb_ = cb;
        }
    }
    ~AutoRef()
    {
        cb_ = nullptr;
    }
    std::shared_ptr<uintptr_t> cb_;
};
} // namespace ANI::Audio
#endif // TAIHE_WORK_H