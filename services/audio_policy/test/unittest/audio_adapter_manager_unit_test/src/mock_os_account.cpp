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

#include "mock_os_account.h"
#include "os_account_manager.h"
#include "os_account_subprofile_client.h"

namespace OHOS {
namespace AccountSA {

MockOsAccountManager g_mockOsAccountManager;
MockOsAccountSubProfileClient g_mockOsAccountSubProfileClient;

OsAccountSubProfileClient::OsAccountSubProfileClient() {}

ErrCode OsAccountManager::QueryActiveOsAccountIds(std::vector<int32_t>& ids)
{
    return g_mockOsAccountManager.QueryActiveOsAccountIds(ids);
}

static OsAccountSubProfileClient g_stubSubProfileClient;

OsAccountSubProfileClient &OsAccountSubProfileClient::GetInstance()
{
    return g_stubSubProfileClient;
}

ErrCode OsAccountSubProfileClient::GetOsAccountForegroundSubProfileId(
    int32_t osAccountId, int32_t& subProfileId)
{
    return g_mockOsAccountSubProfileClient.GetOsAccountForegroundSubProfileId(osAccountId, subProfileId);
}

ErrCode OsAccountSubProfileClient::GetOsAccountForegroundSubProfileId(
    int32_t& subProfileId)
{
    return g_mockOsAccountSubProfileClient.GetOsAccountForegroundSubProfileId(subProfileId);
}

} // namespace AccountSA
} // namespace OHOS
