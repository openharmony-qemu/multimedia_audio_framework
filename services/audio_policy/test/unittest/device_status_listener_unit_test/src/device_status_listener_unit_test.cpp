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

#include "audio_errors.h"
#include "device_status_listener_unit_test.h"
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void DeviceStatusListenerUnitTest::SetUpTestCase(void) {}
void DeviceStatusListenerUnitTest::TearDownTestCase(void) {}
void DeviceStatusListenerUnitTest::SetUp(void) {}
void DeviceStatusListenerUnitTest::TearDown(void) {}

namespace OHOS {
class MockIRemoteObject : public IRemoteObject {
public:
    MockIRemoteObject() : IRemoteObject(u"mock_i_remote_object") {}

    ~MockIRemoteObject() {}

    int32_t GetObjectRefCount() override
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    bool IsProxyObject() const override
    {
        return true;
    }

    bool CheckObjectLegality() const override
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface() override
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        return 0;
    }

    std::u16string GetObjectDescriptor() const
    {
        if (bExchange) {
            std::u16string descriptor = std::u16string();
            return descriptor;
        } else {
            std::u16string descriptor = std::u16string(u"testDescriptor");
            return descriptor;
        }
    }

    static void SetExchange(bool bEx) {
        bExchange = bEx;
    }
private:
    static bool  bExchange;
};
bool MockIRemoteObject::bExchange = true;
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_001
 * @tc.desc  : Test GetInternalDeviceType().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_001, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());

    const std::string info = "EVENT_TYPE=1;DEVICE_TYPE=4;";

    deviceStatusListenerPtr->OnMicrophoneBlocked(info);
    EXPECT_NE(deviceStatusListenerPtr, nullptr);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_002
 * @tc.desc  : Test GetInternalDeviceType().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_002, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());

    const std::string info = "EVENT_TYPE=1;DEVICE_TYPE=2;";

    deviceStatusListenerPtr->OnMicrophoneBlocked(info);
    EXPECT_NE(deviceStatusListenerPtr, nullptr);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_003
 * @tc.desc  : Test GetInternalDeviceType().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_003, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());

    const std::string info = "EVENT_TYPE=1;DEVICE_TYPE=8;";

    deviceStatusListenerPtr->OnMicrophoneBlocked(info);
    EXPECT_NE(deviceStatusListenerPtr, nullptr);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_004
 * @tc.desc  : Test GetInternalDeviceType().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_004, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());

    const std::string info = "EVENT_TYPE=1;DEVICE_TYPE=2048;";

    deviceStatusListenerPtr->OnMicrophoneBlocked(info);
    EXPECT_NE(deviceStatusListenerPtr, nullptr);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_005
 * @tc.desc  : Test GetInternalDeviceType().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_005, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());

    const std::string info = "EVENT_TYPE=1;DEVICE_TYPE=4096;";

    deviceStatusListenerPtr->OnMicrophoneBlocked(info);
    EXPECT_NE(deviceStatusListenerPtr, nullptr);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_006
 * @tc.desc  : Test GetInternalDeviceType().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_006, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());

    const std::string info = "EVENT_TYPE=1;DEVICE_TYPE=8192;";

    deviceStatusListenerPtr->OnMicrophoneBlocked(info);
    EXPECT_NE(deviceStatusListenerPtr, nullptr);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_007
 * @tc.desc  : Test GetInternalDeviceType().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_007, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());

    const std::string info = "EVENT_TYPE=1;DEVICE_TYPE=1;";

    deviceStatusListenerPtr->OnMicrophoneBlocked(info);
    EXPECT_NE(deviceStatusListenerPtr, nullptr);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_008
 * @tc.desc  : Test GetInternalDeviceType().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_008, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());

    const std::string info = "abc";

    deviceStatusListenerPtr->OnMicrophoneBlocked(info);
    EXPECT_NE(deviceStatusListenerPtr, nullptr);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_009
 * @tc.desc  : Test DeviceStatusListener::SetAudioDeviceAnahsCallback().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_009, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    EXPECT_NE(deviceStatusListenerPtr, nullptr);

    sptr<IRemoteObject> object = new OHOS::MockIRemoteObject();
    EXPECT_NE(object, nullptr);

    auto ret = deviceStatusListenerPtr->SetAudioDeviceAnahsCallback(object);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_010
 * @tc.desc  : Test DeviceStatusListener::OnPnpDeviceStatusChanged().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_010, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    EXPECT_NE(deviceStatusListenerPtr, nullptr);

    const std::string info = "abc";
    sptr<IRemoteObject> object = new OHOS::MockIRemoteObject();
    EXPECT_NE(object, nullptr);
    deviceStatusListenerPtr->audioDeviceAnahsCb_ = iface_cast<IStandardAudioAnahsManagerListener>(object);

    deviceStatusListenerPtr->OnPnpDeviceStatusChanged(info);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_011
 * @tc.desc  : Test DeviceStatusListener::OnPnpDeviceStatusChanged().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_011, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    EXPECT_NE(deviceStatusListenerPtr, nullptr);

    const std::string info = "ANAHS_NAME=test;EVENT_TYPE=1;DEVICE_TYPE=1;DEVICE_ADDRESS=1;";
    sptr<IRemoteObject> object = new OHOS::MockIRemoteObject();
    EXPECT_NE(object, nullptr);
    deviceStatusListenerPtr->audioDeviceAnahsCb_ = iface_cast<IStandardAudioAnahsManagerListener>(object);

    deviceStatusListenerPtr->OnPnpDeviceStatusChanged(info);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_012
 * @tc.desc  : Test DeviceStatusListener::OnPnpDeviceStatusChanged().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_012, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    EXPECT_NE(deviceStatusListenerPtr, nullptr);

    const std::string info = "ANAHS_NAME=test;EVENT_TYPE=1;DEVICE_TYPE=4096;DEVICE_ADDRESS=1;";
    sptr<IRemoteObject> object = new OHOS::MockIRemoteObject();
    EXPECT_NE(object, nullptr);
    deviceStatusListenerPtr->audioDeviceAnahsCb_ = iface_cast<IStandardAudioAnahsManagerListener>(object);

    deviceStatusListenerPtr->OnPnpDeviceStatusChanged(info);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_013
 * @tc.desc  : Test DeviceStatusListener::OnPnpDeviceStatusChanged().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_013, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    EXPECT_NE(deviceStatusListenerPtr, nullptr);

    const std::string info = "ANAHS_NAME=test;EVENT_TYPE=1;DEVICE_TYPE=4;DEVICE_ADDRESS=1;";
    sptr<IRemoteObject> object = new OHOS::MockIRemoteObject();
    EXPECT_NE(object, nullptr);
    deviceStatusListenerPtr->audioDeviceAnahsCb_ = iface_cast<IStandardAudioAnahsManagerListener>(object);

    deviceStatusListenerPtr->OnPnpDeviceStatusChanged(info);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_014
 * @tc.desc  : Test DeviceStatusListener::UnRegisterDeviceStatusListener().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_014, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    EXPECT_NE(deviceStatusListenerPtr, nullptr);

    HDIServiceManager hdiServiceManager;
    ServiceStatusListener listener;

    deviceStatusListenerPtr->hdiServiceManager_ = HDIServiceManagerGet();
    deviceStatusListenerPtr->listener_ = HdiServiceStatusListenerNewInstance();
    deviceStatusListenerPtr->audioPnpServer_ = &AudioPnpServer::GetAudioPnpServer();

    EXPECT_NE(deviceStatusListenerPtr->hdiServiceManager_, nullptr);
    EXPECT_NE(deviceStatusListenerPtr->listener_, nullptr);
    EXPECT_NE(deviceStatusListenerPtr->audioPnpServer_, nullptr);

    auto ret = deviceStatusListenerPtr->UnRegisterDeviceStatusListener();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_015
 * @tc.desc  : Test DeviceStatusListener::UnRegisterDeviceStatusListener().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_015, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    EXPECT_NE(deviceStatusListenerPtr, nullptr);

    HDIServiceManager hdiServiceManager;
    ServiceStatusListener listener;

    deviceStatusListenerPtr->hdiServiceManager_ = nullptr;
    deviceStatusListenerPtr->listener_ = HdiServiceStatusListenerNewInstance();
    deviceStatusListenerPtr->audioPnpServer_ = &AudioPnpServer::GetAudioPnpServer();

    EXPECT_NE(deviceStatusListenerPtr->listener_, nullptr);
    EXPECT_NE(deviceStatusListenerPtr->audioPnpServer_, nullptr);

    auto ret = deviceStatusListenerPtr->UnRegisterDeviceStatusListener();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_016
 * @tc.desc  : Test DeviceStatusListener::UnRegisterDeviceStatusListener().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_016, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    EXPECT_NE(deviceStatusListenerPtr, nullptr);

    HDIServiceManager hdiServiceManager;
    ServiceStatusListener listener;

    deviceStatusListenerPtr->hdiServiceManager_ = nullptr;
    deviceStatusListenerPtr->listener_ = nullptr;
    deviceStatusListenerPtr->audioPnpServer_ = &AudioPnpServer::GetAudioPnpServer();

    EXPECT_NE(deviceStatusListenerPtr->audioPnpServer_, nullptr);

    auto ret = deviceStatusListenerPtr->UnRegisterDeviceStatusListener();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}
} // namespace AudioStandard
} // namespace OHOS
