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
#include "audio_policy_manager_device_unit_test.h"
#include "audio_unit_test.h"
#include "audio_info.h"
#include "audio_anahs_manager_listener.h"
#include "audio_routing_manager_listener.h"
#include "audio_zone_types.h"
#include "audio_general_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioDeviceAnahsTest : public AudioDeviceAnahs {
public:
    int32_t OnExtPnpDeviceStatusChanged(
        std::string anahsStatus, std::string anahsShowType) override
    {
        callCount++;
        lastStatus = anahsStatus;
        lastShowType = anahsShowType;
        return 0;
    }
    int callCount = 0;
    std::string lastStatus;
    std::string lastShowType;
};

HWTEST(AudioPolicyManagerDevice, AnahsSetCallback_NullCb, TestSize.Level1)
{
    auto listener = std::make_shared<AudioAnahsManagerListener>();
    std::shared_ptr<AudioDeviceAnahs> nullCb;
    listener->SetAudioDeviceAnahsCallback(nullCb);
    EXPECT_EQ(listener->audioDeviceAnahsCallback_.lock(), nullptr);
}

HWTEST(AudioPolicyManagerDevice, AnahsSetCallback_ValidCb, TestSize.Level1)
{
    auto listener = std::make_shared<AudioAnahsManagerListener>();
    auto cb = std::make_shared<AudioDeviceAnahsTest>();
    listener->SetAudioDeviceAnahsCallback(cb);
    EXPECT_NE(listener->audioDeviceAnahsCallback_.lock(), nullptr);
}

HWTEST(AudioPolicyManagerDevice, AnahsOnExtPnp_NullCb, TestSize.Level1)
{
    auto listener = std::make_shared<AudioAnahsManagerListener>();
    int32_t ret = listener->OnExtPnpDeviceStatusChanged(
        "status", "showType");
    EXPECT_EQ(ret, ERR_CALLBACK_NOT_REGISTERED);
}

HWTEST(AudioPolicyManagerDevice, AnahsOnExtPnp_ValidCb, TestSize.Level1)
{
    auto listener = std::make_shared<AudioAnahsManagerListener>();
    auto cb = std::make_shared<AudioDeviceAnahsTest>();
    listener->SetAudioDeviceAnahsCallback(cb);
    int32_t ret = listener->OnExtPnpDeviceStatusChanged(
        "status", "showType");
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(cb->callCount, 1);
    EXPECT_EQ(cb->lastStatus, "status");
    EXPECT_EQ(cb->lastShowType, "showType");
}

class AudioDistributedRoutingRoleCbTest
    : public AudioDistributedRoutingRoleCallback {
public:
    void OnDistributedRoutingRoleChange(
        std::shared_ptr<AudioDeviceDescriptor> descriptor,
        const CastType type) override
    {
        callCount++;
    }
    int callCount = 0;
};

class AudioDeviceRefinerTest : public AudioDeviceRefiner {
public:
    int32_t OnAudioOutputDeviceRefined(
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
        const FetchDeviceInfo &fetchDeviceInfo) override
    {
        outputCallCount++;
        return 0;
    }
    int32_t OnAudioDupDeviceRefined(
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
        const FetchDeviceInfo &fetchDeviceInfo) override
    {
        dupCallCount++;
        return 0;
    }
    int32_t OnAudioInputDeviceRefined(
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
        RouterType routerType, SourceType sourceType,
        int32_t clientUid, AudioPipeType audioPipeType) override
    {
        inputCallCount++;
        return 0;
    }
    int32_t GetSplitInfoRefined(std::string &splitInfo) override
    {
        splitCallCount++;
        return 0;
    }
    int32_t OnDistributedOutputChange(bool isRemote) override
    {
        distOutputCallCount++;
        return 0;
    }
    int32_t OnDistributedServiceOnline() override
    {
        onlineCallCount++;
        return 0;
    }
    int32_t SetSplitModeReadyRefined() override
    {
        splitModeCallCount++;
        return 0;
    }
    int outputCallCount = 0;
    int dupCallCount = 0;
    int inputCallCount = 0;
    int splitCallCount = 0;
    int distOutputCallCount = 0;
    int onlineCallCount = 0;
    int splitModeCallCount = 0;
};

HWTEST(AudioPolicyManagerDevice, RoutingSetDistRole_NullCb, TestSize.Level1)
{
    auto listener = std::make_shared<AudioRoutingManagerListener>();
    std::weak_ptr<AudioDistributedRoutingRoleCallback> nullCb;
    listener->SetDistributedRoutingRoleCallback(nullCb);
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    int32_t ret = listener->OnDistributedRoutingRoleChange(desc, 0);
    EXPECT_EQ(ret, ERR_CALLBACK_NOT_REGISTERED);
}

HWTEST(AudioPolicyManagerDevice, RoutingSetDistRole_ValidCb, TestSize.Level1)
{
    auto listener = std::make_shared<AudioRoutingManagerListener>();
    auto cb = std::make_shared<AudioDistributedRoutingRoleCbTest>();
    listener->SetDistributedRoutingRoleCallback(cb);
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    int32_t ret = listener->OnDistributedRoutingRoleChange(desc, 0);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(cb->callCount, 1);
}

HWTEST(AudioPolicyManagerDevice, RoutingSetRefiner_NullCb, TestSize.Level1)
{
    auto listener = std::make_shared<AudioRoutingManagerListener>();
    std::shared_ptr<AudioDeviceRefiner> nullCb;
    listener->SetAudioDeviceRefinerCallback(nullCb);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    FetchDeviceInfo info;
    int32_t ret = listener->OnAudioOutputDeviceRefined(descs, info);
    EXPECT_EQ(ret, ERR_CALLBACK_NOT_REGISTERED);
}

HWTEST(AudioPolicyManagerDevice, RoutingSetRefiner_ValidCb, TestSize.Level1)
{
    auto listener = std::make_shared<AudioRoutingManagerListener>();
    auto cb = std::make_shared<AudioDeviceRefinerTest>();
    listener->SetAudioDeviceRefinerCallback(cb);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    FetchDeviceInfo info;
    int32_t ret = listener->OnAudioOutputDeviceRefined(
        descs, info);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(cb->outputCallCount, 1);
}

HWTEST(AudioPolicyManagerDevice, RoutingOnDupRefined_NullCb, TestSize.Level1)
{
    auto listener = std::make_shared<AudioRoutingManagerListener>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    FetchDeviceInfo info;
    int32_t ret = listener->OnAudioDupDeviceRefined(descs, info);
    EXPECT_EQ(ret, ERR_CALLBACK_NOT_REGISTERED);
}

HWTEST(AudioPolicyManagerDevice, RoutingOnDupRefined_ValidCb, TestSize.Level1)
{
    auto listener = std::make_shared<AudioRoutingManagerListener>();
    auto cb = std::make_shared<AudioDeviceRefinerTest>();
    listener->SetAudioDeviceRefinerCallback(cb);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    FetchDeviceInfo info;
    int32_t ret = listener->OnAudioDupDeviceRefined(descs, info);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(cb->dupCallCount, 1);
}

HWTEST(AudioPolicyManagerDevice, RoutingOnDistOutput_NullCb, TestSize.Level1)
{
    auto listener = std::make_shared<AudioRoutingManagerListener>();
    int32_t ret = listener->OnDistributedOutputChange(false);
    EXPECT_EQ(ret, ERR_CALLBACK_NOT_REGISTERED);
}

HWTEST(AudioPolicyManagerDevice, RoutingOnDistOutput_ValidCb, TestSize.Level1)
{
    auto listener = std::make_shared<AudioRoutingManagerListener>();
    auto cb = std::make_shared<AudioDeviceRefinerTest>();
    listener->SetAudioDeviceRefinerCallback(cb);
    int32_t ret = listener->OnDistributedOutputChange(true);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(cb->distOutputCallCount, 1);
}

HWTEST(AudioPolicyManagerDevice, RoutingOnInputRefined_NullCb, TestSize.Level1)
{
    auto listener = std::make_shared<AudioRoutingManagerListener>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    int32_t ret = listener->OnAudioInputDeviceRefined(
        descs, 0, 0, 0, 0);
    EXPECT_EQ(ret, ERR_CALLBACK_NOT_REGISTERED);
}

HWTEST(AudioPolicyManagerDevice, RoutingOnInputRefined_ValidCb, TestSize.Level1)
{
    auto listener = std::make_shared<AudioRoutingManagerListener>();
    auto cb = std::make_shared<AudioDeviceRefinerTest>();
    listener->SetAudioDeviceRefinerCallback(cb);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    int32_t ret = listener->OnAudioInputDeviceRefined(
        descs, 0, 0, 0, 0);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(cb->inputCallCount, 1);
}

HWTEST(AudioPolicyManagerDevice, RoutingGetSplitInfo_NullCb, TestSize.Level1)
{
    auto listener = std::make_shared<AudioRoutingManagerListener>();
    std::string info;
    int32_t ret = listener->GetSplitInfoRefined(info);
    EXPECT_EQ(ret, ERR_CALLBACK_NOT_REGISTERED);
}

HWTEST(AudioPolicyManagerDevice, RoutingGetSplitInfo_ValidCb, TestSize.Level1)
{
    auto listener = std::make_shared<AudioRoutingManagerListener>();
    auto cb = std::make_shared<AudioDeviceRefinerTest>();
    listener->SetAudioDeviceRefinerCallback(cb);
    std::string info;
    int32_t ret = listener->GetSplitInfoRefined(info);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(cb->splitCallCount, 1);
}

HWTEST(AudioPolicyManagerDevice, RoutingOnDistOnline_NullCb, TestSize.Level1)
{
    auto listener = std::make_shared<AudioRoutingManagerListener>();
    int32_t ret = listener->OnDistributedServiceOnline();
    EXPECT_EQ(ret, ERR_CALLBACK_NOT_REGISTERED);
}

HWTEST(AudioPolicyManagerDevice, RoutingOnDistOnline_ValidCb, TestSize.Level1)
{
    auto listener = std::make_shared<AudioRoutingManagerListener>();
    auto cb = std::make_shared<AudioDeviceRefinerTest>();
    listener->SetAudioDeviceRefinerCallback(cb);
    int32_t ret = listener->OnDistributedServiceOnline();
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(cb->onlineCallCount, 1);
}

HWTEST(AudioPolicyManagerDevice, RoutingSetSplitMode_NullCb, TestSize.Level1)
{
    auto listener = std::make_shared<AudioRoutingManagerListener>();
    int32_t ret = listener->SetSplitModeReadyRefined();
    EXPECT_EQ(ret, ERR_CALLBACK_NOT_REGISTERED);
}

HWTEST(AudioPolicyManagerDevice, RoutingSetSplitMode_ValidCb, TestSize.Level1)
{
    auto listener = std::make_shared<AudioRoutingManagerListener>();
    auto cb = std::make_shared<AudioDeviceRefinerTest>();
    listener->SetAudioDeviceRefinerCallback(cb);
    int32_t ret = listener->SetSplitModeReadyRefined();
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(cb->splitModeCallCount, 1);
}

HWTEST(AudioPolicyManagerDevice, ZoneRegisterClient_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    int32_t ret = mgr->RegisterAudioZoneClient(nullptr);
    EXPECT_EQ(ret, 5);
}

HWTEST(AudioPolicyManagerDevice, ZoneCreate_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    AudioZoneContext ctx;
    int32_t ret = mgr->CreateAudioZone("test", ctx);
    EXPECT_EQ(ret, ERROR);
}

HWTEST(AudioPolicyManagerDevice, ZoneRelease_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    mgr->ReleaseAudioZone(1);
}

HWTEST(AudioPolicyManagerDevice, ZoneUpdateContext_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    AudioZoneContext ctx;
    mgr->UpdateContextForAudioZone(1, ctx);
}

HWTEST(AudioPolicyManagerDevice, ZoneGetZoneForApp_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    std::vector<int32_t> uids, retUids;
    int32_t ret = mgr->GetAudioZoneForApp(uids, retUids);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

HWTEST(AudioPolicyManagerDevice, ZoneGetAll_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    auto zones = mgr->GetAllAudioZone();
    EXPECT_EQ(zones.size(), 0);
}

HWTEST(AudioPolicyManagerDevice, ZoneGetById_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    auto zone = mgr->GetAudioZone(1);
    EXPECT_EQ(zone, nullptr);
}

HWTEST(AudioPolicyManagerDevice, ZoneGetByName_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    int32_t ret = mgr->GetAudioZoneByName("test");
    EXPECT_EQ(ret, ERROR);
}

HWTEST(AudioPolicyManagerDevice, ZoneBindDevice_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devs;
    int32_t ret = mgr->BindDeviceToAudioZone(1, devs);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZoneUnBindDevice_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devs;
    int32_t ret = mgr->UnBindDeviceToAudioZone(1, devs);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZoneEnableReport_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    int32_t ret = mgr->EnableAudioZoneReport(true);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

HWTEST(AudioPolicyManagerDevice, ZoneEnableChangeReport_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    int32_t ret = mgr->EnableAudioZoneChangeReport(1, true);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

HWTEST(AudioPolicyManagerDevice, ZoneAddUid_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    int32_t ret = mgr->AddUidToAudioZone(1, 1000);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

HWTEST(AudioPolicyManagerDevice, ZoneRemoveUid_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    int32_t ret = mgr->RemoveUidFromAudioZone(1, 1000);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

HWTEST(AudioPolicyManagerDevice, ZoneAddUidUsages_Invalid, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    std::set<StreamUsage> usages = {
        static_cast<StreamUsage>(0)};
    int32_t ret = mgr->AddUidUsagesToAudioZone(1, 1000, usages);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZoneAddUidUsages_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    std::set<StreamUsage> usages = {
        STREAM_USAGE_MUSIC};
    int32_t ret = mgr->AddUidUsagesToAudioZone(1, 1000, usages);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

HWTEST(AudioPolicyManagerDevice, ZoneRemoveUidUsages_Invalid, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    std::set<StreamUsage> usages = {
        static_cast<StreamUsage>(0)};
    int32_t ret = mgr->RemoveUidUsagesFromAudioZone(1, 1000, usages);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZoneRemoveUidUsages_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    std::set<StreamUsage> usages = {
        STREAM_USAGE_MUSIC};
    int32_t ret = mgr->RemoveUidUsagesFromAudioZone(1, 1000, usages);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

HWTEST(AudioPolicyManagerDevice, ZoneAddStream_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    int32_t ret = mgr->AddStreamToAudioZone(1, AudioZoneStream{});
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

HWTEST(AudioPolicyManagerDevice, ZoneAddStreams_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    std::vector<AudioZoneStream> streams;
    int32_t ret = mgr->AddStreamsToAudioZone(1, streams);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

HWTEST(AudioPolicyManagerDevice, ZoneAddStreamId_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    int32_t ret = mgr->AddStreamIdToAudioZone(1, 0);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZoneRemoveStreamId_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    int32_t ret = mgr->RemoveStreamIdFromAudioZone(1, 0);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZonePauseFocus_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    std::vector<int32_t> ids;
    int32_t ret = mgr->PauseAudioFocusForExtAudioZone(ids);
    EXPECT_EQ(ret, ERR_SYSTEM_PERMISSION_DENIED);
}

HWTEST(AudioPolicyManagerDevice, ZoneResumeFocus_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    std::vector<int32_t> ids;
    int32_t ret = mgr->ResumeAudioFocusForExtAudioZone(ids);
    EXPECT_EQ(ret, ERR_SYSTEM_PERMISSION_DENIED);
}

HWTEST(AudioPolicyManagerDevice, ZoneSetVisible_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    mgr->SetZoneDeviceVisible(true);
}

HWTEST(AudioPolicyManagerDevice, ZoneRemoveStream_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    int32_t ret = mgr->RemoveStreamFromAudioZone(
        1, AudioZoneStream{});
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

HWTEST(AudioPolicyManagerDevice, ZoneRemoveStreams_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    std::vector<AudioZoneStream> streams;
    int32_t ret = mgr->RemoveStreamsFromAudioZone(1, streams);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

HWTEST(AudioPolicyManagerDevice, ZoneEnableVolProxy_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    int32_t ret = mgr->EnableSystemVolumeProxy(
        1, DEVICE_TYPE_SPEAKER, true);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

HWTEST(AudioPolicyManagerDevice, ZoneGetInterrupt_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    auto list = mgr->GetAudioInterruptForZone(1);
    EXPECT_EQ(list.size(), 0);
}

HWTEST(AudioPolicyManagerDevice, ZoneGetRendererInfos_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> infos;
    int32_t ret = mgr->GetCurrentRendererChangeInfosForZone(
        1, infos);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

HWTEST(AudioPolicyManagerDevice, ZoneGetInterruptByTag_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    std::string tag = "tag";
    auto list = mgr->GetAudioInterruptForZone(1, tag);
    EXPECT_EQ(list.size(), 0);
}

HWTEST(AudioPolicyManagerDevice, ZoneEnableInterrupt_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    int32_t ret = mgr->EnableAudioZoneInterruptReport(
        1, "tag", true);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

HWTEST(AudioPolicyManagerDevice, ZoneInjectInterrupt_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> list;
    int32_t ret = mgr->InjectInterruptToAudioZone(1, list);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

HWTEST(AudioPolicyManagerDevice, ZoneInjectInterruptByTag_NullGsp, TestSize.Level1)
{
    auto mgr = std::make_shared<AudioPolicyManager>();
    mgr->audioPolicyClientStubCB_ = nullptr;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> list;
    int32_t ret = mgr->InjectInterruptToAudioZone(
        1, "tag", list);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

class ConcreteZoneChangeCb : public AudioZoneChangeCallback {
public:
    void OnAudioZoneChange(
        const AudioZoneDescriptor &zoneDescriptor,
        AudioZoneChangeReason reason) override {}
};

class ConcreteZoneInterruptCb
    : public AudioZoneInterruptCallback {
public:
    void OnInterruptEvent(
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
        AudioZoneInterruptReason reason) override {}
};

HWTEST(AudioPolicyManagerDevice, ZmCreateZone_EmptyName, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    AudioZoneContext ctx;
    int32_t ret = zm->CreateAudioZone("", ctx);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmGetByName_EmptyName, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->GetAudioZoneByName("");
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmBindDevice_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devs;
    int32_t ret = zm->BindDeviceToAudioZone(0, devs);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmBindDevice_EmptyDevs, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devs;
    int32_t ret = zm->BindDeviceToAudioZone(1, devs);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmUnBindDevice_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devs;
    int32_t ret = zm->UnBindDeviceToAudioZone(0, devs);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmUnBindDevice_EmptyDevs, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devs;
    int32_t ret = zm->UnBindDeviceToAudioZone(1, devs);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmReleaseZone_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    zm->ReleaseAudioZone(0);
}

HWTEST(AudioPolicyManagerDevice, ZmUpdateContext_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    AudioZoneContext ctx;
    zm->UpdateContextForAudioZone(-1, ctx);
}

HWTEST(AudioPolicyManagerDevice, ZmGetZoneForApp_EmptyUids, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    std::vector<int32_t> uids, retUids;
    int32_t ret = zm->GetAudioZoneForApp(uids, retUids);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmGetZone_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    auto zone = zm->GetAudioZone(0);
    EXPECT_EQ(zone, nullptr);
}

HWTEST(AudioPolicyManagerDevice, ZmRegZoneCb_NullCb, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->RegisterAudioZoneCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmRegZoneChangeCb_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    auto cb = std::make_shared<ConcreteZoneChangeCb>();
    int32_t ret = zm->RegisterAudioZoneChangeCallback(-1, cb);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmRegZoneChangeCb_NullCb, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->RegisterAudioZoneChangeCallback(0, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmUnregZoneChangeCb_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->UnRegisterAudioZoneChangeCallback(-1);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmAddUid_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->AddUidToAudioZone(0, 1000);
#ifdef MULTI_USER_ZONE
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
#else
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
#endif
}

HWTEST(AudioPolicyManagerDevice, ZmRemoveUid_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->RemoveUidFromAudioZone(0, 1000);
#ifdef MULTI_USER_ZONE
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
#else
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
#endif
}

HWTEST(AudioPolicyManagerDevice, ZmAddUidUsages_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    std::set<StreamUsage> usages = {STREAM_USAGE_MUSIC};
    int32_t ret = zm->AddUidUsagesToAudioZone(0, 1000, usages);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmAddUidUsages_EmptyUsages, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    std::set<StreamUsage> usages;
    int32_t ret = zm->AddUidUsagesToAudioZone(1, 1000, usages);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmRemoveUidUsages_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    std::set<StreamUsage> usages = {STREAM_USAGE_MUSIC};
    int32_t ret = zm->RemoveUidUsagesFromAudioZone(0, 1000, usages);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmRemoveUidUsages_EmptyUsages, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    std::set<StreamUsage> usages;
    int32_t ret = zm->RemoveUidUsagesFromAudioZone(1, 1000, usages);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmAddStream_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->AddStreamToAudioZone(0, AudioZoneStream{});
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmAddStreams_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    std::vector<AudioZoneStream> streams;
    int32_t ret = zm->AddStreamsToAudioZone(0, streams);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmAddStreams_EmptyStreams, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    std::vector<AudioZoneStream> streams;
    int32_t ret = zm->AddStreamsToAudioZone(1, streams);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmAddStreamId_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->AddStreamIdToAudioZone(0, 100000);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmAddStreamId_InvalidStreamId, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->AddStreamIdToAudioZone(1, 0);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmRemoveStreamId_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->RemoveStreamIdFromAudioZone(0, 100000);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmRemoveStreamId_InvalidId, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->RemoveStreamIdFromAudioZone(1, 0);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmRemoveStream_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->RemoveStreamFromAudioZone(0, AudioZoneStream{});
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmRemoveStreams_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    std::vector<AudioZoneStream> streams;
    int32_t ret = zm->RemoveStreamsFromAudioZone(0, streams);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmRemoveStreams_EmptyStreams, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    std::vector<AudioZoneStream> streams;
    int32_t ret = zm->RemoveStreamsFromAudioZone(1, streams);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmRegSysVolProxy_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->RegisterSystemVolumeProxy(
        -1, DEVICE_TYPE_SPEAKER, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmRegSysVolProxy_NullProxy, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->RegisterSystemVolumeProxy(
        0, DEVICE_TYPE_SPEAKER, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmUnregSysVolProxy_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->UnRegisterSystemVolumeProxy(
        -1, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmGetInterrupt_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    auto list = zm->GetAudioInterruptForZone(-1);
    EXPECT_EQ(list.size(), 0);
}

HWTEST(AudioPolicyManagerDevice, ZmGetRendererInfos_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> infos;
    int32_t ret = zm->GetCurrentRendererChangeInfosForZone(
        -1, infos);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmGetInterruptByTag_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    std::string tag = "tag";
    auto list = zm->GetAudioInterruptForZone(-1, tag);
    EXPECT_EQ(list.size(), 0);
}

HWTEST(AudioPolicyManagerDevice, ZmRegZoneIntCb_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->RegisterAudioZoneInterruptCallback(
        -1, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmRegZoneIntCb_NullCb, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->RegisterAudioZoneInterruptCallback(
        0, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmUnregZoneIntCb_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->UnRegisterAudioZoneInterruptCallback(-1);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmRegZoneIntCbByTag_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->RegisterAudioZoneInterruptCallback(
        -1, "tag", nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmRegZoneIntCbByTag_NullCb, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->RegisterAudioZoneInterruptCallback(
        0, "tag", nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmUnregZoneIntCbByTag_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    int32_t ret = zm->UnRegisterAudioZoneInterruptCallback(
        -1, "tag");
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmInjectInt_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    std::list<std::pair<AudioInterrupt, AudioFocuState>> list;
    int32_t ret = zm->InjectInterruptToAudioZone(-1, list);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmInjectIntByTag_InvalidZone, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    std::list<std::pair<AudioInterrupt, AudioFocuState>> list;
    int32_t ret = zm->InjectInterruptToAudioZone(
        -1, "tag", list);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmSetVolConfig_EmptyName, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    std::vector<AudioZoneVolumeConfig> configs;
    int32_t ret = zm->SetAudioZoneVolumeConfig("", configs);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, ZmSetVolConfig_EmptyConfigs, TestSize.Level1)
{
    auto *zm = AudioZoneManager::GetInstance();
    std::vector<AudioZoneVolumeConfig> configs;
    int32_t ret = zm->SetAudioZoneVolumeConfig(
        "zone", configs);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmSetRefinerCb_NullCb, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->SetAudioDeviceRefinerCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmSetClientInfoMgrCb_NullCb, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->SetAudioClientInfoMgrCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmSetPrefOutDevCb_NullCb, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    AudioRendererInfo info;
    int32_t ret = gm->SetPreferredOutputDeviceChangeCallback(
        info, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmRegFocusCb_NullCb, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->RegisterFocusInfoChangeCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmSetDevChangeCb_NullCb, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->SetDeviceChangeCallback(
        OUTPUT_DEVICES_FLAG, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmSetDevInfoCb_NullCb, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->SetDeviceInfoUpdateCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmSetQueryClientTypeCb_NullCb, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->SetQueryClientTypeCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmSetQueryDevVolCb_NullCb, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->SetQueryDeviceVolumeBehaviorCallback(
        nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmSetSceneChangeCb_NullCb, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->SetAudioSceneChangeCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmRegVolKeyCb_NullCb, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->RegisterVolumeKeyEventCallback(0, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmRegCapturerEvent_NullCb, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->RegisterAudioCapturerEventListener(
        0, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmRegRendererEvent_NullCb, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->RegisterAudioRendererEventListener(
        nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmSetPrefInDevCb_NullCb, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    AudioCapturerInfo info;
    int32_t ret = gm->SetPreferredInputDeviceChangeCallback(
        info, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmSetDevConnStatus_NullDesc, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> nullDesc;
    int32_t ret = gm->SetDeviceConnectionStatus(nullDesc, true);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmUpdateDevInfo_NullDesc, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> nullDesc;
    int32_t ret = gm->UpdateDeviceInfo(
        nullDesc, CONNECTSTATE_UPDATE);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmSelectOutput_EmptyDevs, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devs;
    int32_t ret = gm->SelectOutputDevice(devs);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmSelectOutput_NullDev, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devs = {
        nullptr};
    int32_t ret = gm->SelectOutputDevice(devs);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmSelectOutput_InputDev, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = INPUT_DEVICE;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devs = {desc};
    int32_t ret = gm->SelectOutputDevice(devs);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

HWTEST(AudioPolicyManagerDevice, GmSelectOutputFilter_NullFilter, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devs;
    int32_t ret = gm->SelectOutputDevice(
        nullptr, devs);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmSelectOutputFilter_NullDev, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    auto filter = sptr<AudioRendererFilter>::MakeSptr();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devs = {
        nullptr};
    int32_t ret = gm->SelectOutputDevice(filter, devs);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmSelectOutputFilter_InputDev, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    auto filter = sptr<AudioRendererFilter>::MakeSptr();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = INPUT_DEVICE;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devs = {
        desc};
    int32_t ret = gm->SelectOutputDevice(filter, devs);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

HWTEST(AudioPolicyManagerDevice, GmSetSleAudioCb_NullCb, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->SetSleAudioOperationCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmRegPrefDevSetCb_NullCb, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->RegisterPreferredDeviceSetCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmUnregPrefDevSetCb_NullCb, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->UnregisterPreferredDeviceSetCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST(AudioPolicyManagerDevice, GmGetVolume_NotSupported, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->GetVolume(
        static_cast<AudioVolumeType>(999));
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

HWTEST(AudioPolicyManagerDevice, GmUnregFocusCb_NullCb, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->UnregisterFocusInfoChangeCallback(nullptr);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST(AudioPolicyManagerDevice, GmGetCallingPid, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t pid = gm->GetCallingPid();
    EXPECT_GT(pid, 0);
}

HWTEST(AudioPolicyManagerDevice, GmGetDevicesInner, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    auto devs = gm->GetDevicesInner(OUTPUT_DEVICES_FLAG);
    EXPECT_GE(devs.size(), 0);
}

HWTEST(AudioPolicyManagerDevice, GmGetActiveOutputDevice, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    DeviceType dev = gm->GetActiveOutputDevice();
    EXPECT_NE(dev, DEVICE_TYPE_NONE);
}

HWTEST(AudioPolicyManagerDevice, GmGetAudioScene, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    AudioScene scene = gm->GetAudioScene();
    EXPECT_EQ(scene, AUDIO_SCENE_DEFAULT);
}

HWTEST(AudioPolicyManagerDevice, GmUnsetRefinerCb, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->UnsetAudioDeviceRefinerCallback();
    EXPECT_NE(ret, SUCCESS);
}

HWTEST(AudioPolicyManagerDevice, GmTriggerFetchDevice, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    AudioStreamDeviceChangeReasonExt reason;
    int32_t ret = gm->TriggerFetchDevice(reason);
    EXPECT_NE(ret, SUCCESS);
}

HWTEST(AudioPolicyManagerDevice, GmSetPreferredDevice, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    int32_t ret = gm->SetPreferredDevice(
        PreferredType::AUDIO_MEDIA_RENDER, desc);
    EXPECT_NE(ret, SUCCESS);
}

HWTEST(AudioPolicyManagerDevice, GmSetAsrVoiceMuteMode_NullGasp, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->SetAsrVoiceMuteMode(
        AsrVoiceMuteMode::OUTPUT_MUTE, true);
    EXPECT_EQ(ret, ERR_SYSTEM_PERMISSION_DENIED);
}

HWTEST(AudioPolicyManagerDevice, GmSetExtraParams_NullGasp, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    std::vector<std::pair<std::string, std::string>> kv;
    int32_t ret = gm->SetExtraParameters("key", kv);
    EXPECT_EQ(ret, ERR_SYSTEM_PERMISSION_DENIED);
}

HWTEST(AudioPolicyManagerDevice, GmNotifySensitive_NullGasp, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->NotifySensitiveRecordPermit(1, true);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

HWTEST(AudioPolicyManagerDevice, GmGetPrefOutDev, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    AudioRendererInfo info;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    int32_t ret = gm->GetPreferredOutputDeviceForRendererInfo(
        info, desc);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST(AudioPolicyManagerDevice, GmGetPrefInDev, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    AudioCapturerInfo info;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    int32_t ret = gm->GetPreferredInputDeviceForCapturerInfo(
        info, desc);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST(AudioPolicyManagerDevice, GmGetCurRendererInfos, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> infos;
    int32_t ret = gm->GetCurrentRendererChangeInfos(infos);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST(AudioPolicyManagerDevice, GmGetCurCapturerInfos, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> infos;
    int32_t ret = gm->GetCurrentCapturerChangeInfos(infos);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST(AudioPolicyManagerDevice, GmGetFocusInfoList, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    std::list<std::pair<AudioInterrupt, AudioFocuState>> list;
    int32_t ret = gm->GetAudioFocusInfoList(list);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST(AudioPolicyManagerDevice, GmSetActiveHfpDevice, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->SetActiveHfpDevice("00:00:00:00:00:00");
    EXPECT_NE(ret, SUCCESS);
}

HWTEST(AudioPolicyManagerDevice, GmRestoreDistributedInfo, TestSize.Level1)
{
    auto *gm = AudioGeneralManager::GetInstance();
    int32_t ret = gm->RestoreDistributedDeviceInfo();
    EXPECT_NE(ret, SUCCESS);
}

} // namespace AudioStandard
} // namespace OHOS
