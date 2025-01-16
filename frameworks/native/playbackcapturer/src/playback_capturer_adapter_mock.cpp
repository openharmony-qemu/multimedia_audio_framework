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
#include "playback_capturer_adapter.h"

bool IsStreamSupportInnerCapturer(int32_t streamUsage)
{
    (void)streamUsage;
    return false;
}

bool IsPrivacySupportInnerCapturer(int32_t privacyType)
{
    (void)privacyType;
    return false;
}

bool IsCaptureSilently()
{
    return false;
}

extern "C" __attribute__((visibility("default"))) bool GetInnerCapturerState()
{
    return false;
}

extern "C" __attribute__((visibility("default"))) void SetInnerCapturerState(bool state)
{
    (void)state;
}
