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

#include "audio_errors.h"
#include "audio_server.h"
#include "gmock/gmock.h"
#include "manager/hdi_adapter_manager.h"

using namespace testing::ext;
using ::testing::StrEq;

namespace OHOS {
namespace AudioStandard {
class MockDeviceManagerForAudioServer : public IDeviceManager {
public:
    MOCK_METHOD(int32_t, SetAudioParameter,
        (const std::string &adapterName, const AudioParamKey key, const std::string &condition,
        const std::string &value), (override));
    MOCK_METHOD(std::string, GetAudioParameter,
        (const std::string &adapterName, const AudioParamKey key, const std::string &condition), (override));

    int32_t LoadAdapter(const std::string &adapterName) override
    {
        (void)adapterName;
        return SUCCESS;
    }

    void UnloadAdapter(const std::string &adapterName, bool force = false) override
    {
        (void)adapterName;
        (void)force;
    }

    void AllAdapterSetMicMute(bool isMute) override
    {
        (void)isMute;
    }

    int32_t SetVoiceVolume(const std::string &adapterName, float volume) override
    {
        (void)adapterName;
        (void)volume;
        return SUCCESS;
    }

    int32_t SetOutputRoute(const std::string &adapterName, const std::vector<DeviceType> &devices,
        int32_t streamId) override
    {
        (void)adapterName;
        (void)devices;
        (void)streamId;
        return SUCCESS;
    }

    int32_t SetInputRoute(const std::string &adapterName, DeviceType device, int32_t streamId,
        int32_t inputType) override
    {
        (void)adapterName;
        (void)device;
        (void)streamId;
        (void)inputType;
        return SUCCESS;
    }

    void SetMicMute(const std::string &adapterName, bool isMute) override
    {
        (void)adapterName;
        (void)isMute;
    }

    void *CreateRender(const std::string &adapterName, void *param, void *deviceDesc,
        uint32_t &hdiRenderId) override
    {
        (void)adapterName;
        (void)param;
        (void)deviceDesc;
        (void)hdiRenderId;
        return nullptr;
    }

    void DestroyRender(const std::string &adapterName, uint32_t hdiRenderId) override
    {
        (void)adapterName;
        (void)hdiRenderId;
    }

    void *CreateCapture(const std::string &adapterName, void *param, void *deviceDesc,
        uint32_t &hdiCaptureId) override
    {
        (void)adapterName;
        (void)param;
        (void)deviceDesc;
        (void)hdiCaptureId;
        return nullptr;
    }

    void DestroyCapture(const std::string &adapterName, uint32_t hdiCaptureId) override
    {
        (void)adapterName;
        (void)hdiCaptureId;
    }

    void DumpInfo(std::string &dumpString) override
    {
        (void)dumpString;
    }

    void SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType) override
    {
        (void)dmDeviceType;
        (void)deviceType;
    }

    void SetAudioScene(const AudioScene scene) override
    {
        (void)scene;
    }
};

/**
 * @tc.name  : Test SetExtraParameters API - Verify local HDI forwarding
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetExtraParameters_006
 * @tc.desc  : Verify AudioServer forwards parsed params to local device manager.
 */
HWTEST_F(AudioServerUnitTest, AudioServerSetExtraParameters_006, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    auto mockDeviceManager = std::make_shared<MockDeviceManagerForAudioServer>();
    HdiAdapterManager::GetInstance().deviceManagers_[HDI_DEVICE_MANAGER_TYPE_LOCAL] = mockDeviceManager;
    AudioServer::audioParameterKeys = {
        {
            "test_key", {
                {"subkey1", {"value1", "value2"}},
                {"subkey2", {"value3", "value4"}}
            }
        }
    };
    audioServer->isAudioParameterParsed_.store(true);

    std::vector<StringPair> kvpairs = {
        {"subkey1", "value1"},
        {"subkey2", "value3"}
    };

    EXPECT_CALL(*mockDeviceManager, SetAudioParameter(StrEq("primary"), AudioParamKey::NONE, StrEq(""),
        StrEq("subkey1=value1;subkey2=value3;"))).WillOnce(testing::Return(SUCCESS));

    int32_t ret = audioServer->SetExtraParameters("test_key", kvpairs);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetExtraParameters API - Verify HomeMusic remote forwarding
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetExtraParameters_007
 * @tc.desc  : Verify AudioServer forwards HomeMusic params to remote device manager.
 */
HWTEST_F(AudioServerUnitTest, AudioServerSetExtraParameters_007, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    auto mockDeviceManager = std::make_shared<MockDeviceManagerForAudioServer>();
    HdiAdapterManager::GetInstance().deviceManagers_[HDI_DEVICE_MANAGER_TYPE_REMOTE] = mockDeviceManager;

    std::vector<StringPair> kvpairs = {
        {"networkId", "network_id_001"},
        {"zoneId", "zone_id_002"}
    };

    EXPECT_CALL(*mockDeviceManager, SetAudioParameter(StrEq("network_id_001"), AudioParamKey::NONE,
        StrEq("zone_id_change"), StrEq("zone_id_002"))).WillOnce(testing::Return(SUCCESS));

    int32_t ret = audioServer->SetExtraParameters("HomeMusic", kvpairs);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetExtraParameters API - Verify local HDI forwarding
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetExtraParameters_011
 * @tc.desc  : Verify AudioServer forwards GetExtraParameters to local device manager.
 */
HWTEST_F(AudioServerUnitTest, AudioServerGetExtraParameters_011, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    auto mockDeviceManager = std::make_shared<MockDeviceManagerForAudioServer>();
    HdiAdapterManager::GetInstance().deviceManagers_[HDI_DEVICE_MANAGER_TYPE_LOCAL] = mockDeviceManager;
    AudioServer::audioParameterKeys = {
        {
            "test_key", {
                {"subkey1", {"value1", "value2"}}
            }
        }
    };
    audioServer->isAudioParameterParsed_.store(true);

    std::vector<StringPair> result;
    std::vector<std::string> subKeys = {"subkey1"};

    EXPECT_CALL(*mockDeviceManager, GetAudioParameter(StrEq("primary"), AudioParamKey::NONE, StrEq("subkey1")))
        .WillOnce(testing::Return("value1"));

    int32_t ret = audioServer->GetExtraParameters("test_key", subKeys, result);
    EXPECT_EQ(SUCCESS, ret);
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ("subkey1", result[0].firstParam);
    EXPECT_EQ("value1", result[0].secondParam);
}
} // namespace AudioStandard
} // namespace OHOS
