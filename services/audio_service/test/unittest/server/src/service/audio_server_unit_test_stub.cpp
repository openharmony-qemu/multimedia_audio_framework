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

#include "audio_server_unit_test.h"

namespace OHOS {
namespace AudioStandard {

sptr<AudioServer> audioServer = nullptr;

void AudioServerUnitTest::SetUpTestCase(void)
{
    constexpr int32_t systemAbilityId = 100;
    audioServer = sptr<AudioServer>::MakeSptr(systemAbilityId, true);
}

void AudioServerUnitTest::TearDownTestCase(void)
{
    audioServer = nullptr;
}

void AudioServerUnitTest::SetUp(void) {}

void AudioServerUnitTest::TearDown(void) {}

} // namespace AudioStandard
} // namespace OHOS
