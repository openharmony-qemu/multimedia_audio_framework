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

#ifndef MOCK_AUDIO_ZONE_SERVICE
#define MOCK_AUDIO_ZONE_SERVICE

#include "audio_zone_service.h"
#include "gmock/gmock.h"

namespace OHOS {
namespace AudioStandard {

class MockAudioZoneService : public AudioZoneService {
public:
    MockAudioZoneService() = default;
    virtual ~MockAudioZoneService() = default;
    MOCK_METHOD(std::string, FindAudioZoneNameByUid, (int32_t uid), (override));
};
} // namespace AudioStandard
} // namespace OHOS

#endif