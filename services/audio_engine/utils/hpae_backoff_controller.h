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

#ifndef HPAE_BACKOFF_CONTROLLER_H
#define HPAE_BACKOFF_CONTROLLER_H
#include <cstdint>
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class HpaeBackoffController {
public:
    explicit HpaeBackoffController(int32_t minDelay = 0, int32_t maxDelay = 20, int32_t increment = 1); // max 20ms
    ~HpaeBackoffController() = default;
    void HandleResult(bool result);

private:
    void Reset();
private:
    int32_t minDelay_ = 0;
    int32_t maxDelay_ = 0;
    int32_t increment_ = 0;
    int32_t delay_ = 0;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif