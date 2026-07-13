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

#ifndef AUDIO_AIUT_UTILS_H
#define AUDIO_AIUT_UTILS_H

#include "nativetoken_kit.h"
#include "token_setproc.h"

namespace AIUT {

/*
 * Grant a single permission to the current test process via AccessToken.
 *
 * Usage:
 *   AIUT::GrantPermission("ohos.permission.MICROPHONE");
 *
 * Typically called in SetUpTestCase() of a GTest fixture.
 */
inline void GrantPermission(const char* perm)
{
    const char* perms[] = {perm};
    NativeTokenInfoParams info = {
        .dcapsNum = 0,
        .permsNum = 1,
        .perms = perms,
        .aplStr = "system_basic",
    };
    uint64_t tokenID = GetAccessTokenId(&info);
    SetSelfTokenID(tokenID);
}

/*
 * Grant multiple permissions to the current test process via AccessToken.
 *
 * Usage:
 *   const char* perms[] = {
 *       "ohos.permission.MICROPHONE",
 *       "ohos.permission.MODIFY_AUDIO_SETTINGS",
 *   };
 *   AIUT::GrantPermissions(perms, sizeof(perms) / sizeof(perms[0]));
 */
inline void GrantPermissions(const char** perms, int count)
{
    NativeTokenInfoParams info = {
        .dcapsNum = 0,
        .permsNum = count,
        .perms = perms,
        .aplStr = "system_basic",
    };
    uint64_t tokenID = GetAccessTokenId(&info);
    SetSelfTokenID(tokenID);
}

} // namespace AIUT

#endif // AUDIO_AIUT_UTILS_H
