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
#ifndef MOCK_UPDATE_ENGINE_CALLBACK_H
#define MOCK_UPDATE_ENGINE_CALLBACK_H

#include <gmock/gmock.h>
#include <vector>
#include <string>
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "audio_suite_sa_types.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class MockRemoteObject : public IRemoteObject {
public:
    MockRemoteObject() : IRemoteObject(u"MockRemoteObject") {}

    int32_t GetObjectRefCount() override { return 0; }

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option) override
    {
        return 0;
    }

    bool AddDeathRecipient(const sptr<IRemoteObject::DeathRecipient> &recipient) override
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<IRemoteObject::DeathRecipient> &recipient) override
    {
        return true;
    }

    int Dump(int fd, const std::vector<std::u16string> &args) override { return 0; }

    DECLARE_INTERFACE_DESCRIPTOR(u"MockRemoteObject");
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // MOCK_UPDATE_ENGINE_CALLBACK_H
