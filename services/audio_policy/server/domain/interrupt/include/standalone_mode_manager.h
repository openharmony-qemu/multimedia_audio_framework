/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef ST_STANDALONE_MODE_MANAGER_H
#define ST_STANDALONE_MODE_MANAGER_H


namespace OHOS {
namespace AudioStandard {

static constexpr int32_t INVALID_ID = INT_MIN;

class StandaloneModeManager {
public:

    int32_t SetAppConcurrencyMode(const int32_t ownerPid, const int32_t appUid, const int32_t mode);
    int32_t SetAppSlientOnDisplay(const int32_t displayId);

private:

};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_STANDALONE_MODE_MANAGER_H
