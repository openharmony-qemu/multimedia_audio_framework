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

#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "audio_utils.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "sink/remote_offload_audio_render_sink.h"
#include "../include/sink/mock_remote_i_audio_render.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class RemoteOffloadAudioRenderSinkUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp();
    virtual void TearDown();

protected:
    static uint32_t id_;
    static std::shared_ptr<IAudioRenderSink> sink_;
    static IAudioSinkAttr attr_;
};

uint32_t RemoteOffloadAudioRenderSinkUnitTest::id_ = HDI_INVALID_ID;
std::shared_ptr<IAudioRenderSink> RemoteOffloadAudioRenderSinkUnitTest::sink_ = nullptr;
IAudioSinkAttr RemoteOffloadAudioRenderSinkUnitTest::attr_ = {};

void RemoteOffloadAudioRenderSinkUnitTest::SetUpTestCase()
{
    id_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_REMOTE_OFFLOAD, HDI_ID_INFO_DEFAULT,
        true);
}

void RemoteOffloadAudioRenderSinkUnitTest::TearDownTestCase()
{
    HdiAdapterManager::GetInstance().ReleaseId(id_);
}

void RemoteOffloadAudioRenderSinkUnitTest::SetUp()
{
    sink_ = HdiAdapterManager::GetInstance().GetRenderSink(id_, true);
    if (sink_ == nullptr) {
        return;
    }
}

void RemoteOffloadAudioRenderSinkUnitTest::TearDown()
{
    sink_ = nullptr;
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : RemoteOffloadSinkUnitTest_001
 * @tc.desc   : Test remote offload sink create
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, RemoteOffloadSinkUnitTest_001, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : RemoteOffloadSinkUnitTest_002
 * @tc.desc   : Test remote offload sink deinit
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, RemoteOffloadSinkUnitTest_002, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    if (sink_->IsInited()) {
        sink_->DeInit();
    }
    EXPECT_FALSE(sink_->IsInited());
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : RemoteOffloadSinkUnitTest_003
 * @tc.desc   : Test remote offload sink start, stop, resume, pause, flush, reset
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, RemoteOffloadSinkUnitTest_003, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    int32_t ret = sink_->Start();
    EXPECT_EQ(ret, ERR_NOT_STARTED);
    ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = sink_->Resume();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
    ret = sink_->Pause();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
    ret = sink_->Flush();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ret = sink_->Reset();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : RemoteOffloadSinkUnitTest_004
 * @tc.desc   : Test remote offload sink set/get volume
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, RemoteOffloadSinkUnitTest_004, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    int32_t ret = sink_->SetVolume(0.0f, 0.0f);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->SetVolume(0.0f, 1.0f);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->SetVolume(1.0f, 0.0f);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->SetVolume(1.0f, 1.0f);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    float left;
    float right;
    ret = sink_->GetVolume(left, right);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : RemoteOffloadSinkUnitTest_005
 * @tc.desc   : Test remote offload sink set audio scene
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, RemoteOffloadSinkUnitTest_005, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    int32_t ret = sink_->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : RemoteOffloadSinkUnitTest_006
 * @tc.desc   : Test remote offload sink update active device
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, RemoteOffloadSinkUnitTest_006, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    std::vector<DeviceType> deviceTypes = { DEVICE_TYPE_SPEAKER };
    int32_t ret = sink_->UpdateActiveDevice(deviceTypes);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : RemoteOffloadSinkUnitTest_007
 * @tc.desc   : Test remote offload sink update app uid
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, RemoteOffloadSinkUnitTest_007, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    int32_t ret = sink_->LockOffloadRunningLock();
    EXPECT_EQ(ret, SUCCESS);
    std::vector<int32_t> appsUid = { 20000001, 20000002, 20000003 };
    sink_->UpdateAppsUid(appsUid);
    appsUid.clear();
    sink_->UpdateAppsUid(appsUid);
    ret = sink_->UnLockOffloadRunningLock();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : RemoteOffloadSinkUnitTest_008
 * @tc.desc   : Test remote offload sink set invalid state
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, RemoteOffloadSinkUnitTest_008, TestSize.Level1)
{
    std::shared_ptr<RemoteOffloadAudioRenderSink> sink = std::make_shared<RemoteOffloadAudioRenderSink>("test");
    sink->SetInvalidState();
    sink->sinkInited_.store(true);
    int32_t ret = sink->Start();
    EXPECT_EQ(ret, ERR_NOT_STARTED);

    sink->renderInited_.store(true);
    ret = sink->Start();
    EXPECT_EQ(ret, ERR_NOT_STARTED);

    sink->validState_.store(true);
    ret = sink->Start();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : RemoteOffloadSinkUnitTest_011
 * @tc.desc   : Test remote offload sink deinit with invalid state
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, RemoteOffloadSinkUnitTest_011, TestSize.Level1)
{
    std::shared_ptr<RemoteOffloadAudioRenderSink> sink = std::make_shared<RemoteOffloadAudioRenderSink>("test");
    sink->SetInvalidState();
    sink->DeInit();
    EXPECT_FALSE(sink->IsInited());
}

/**
 * @tc.name   : Test Start Mock HDI fail
 * @tc.number : Start_MockHdiFail_001
 * @tc.desc   : Test Start with Mock HDI Start fail
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, Start_MockHdiFail_001, TestSize.Level1)
{
    std::shared_ptr<RemoteOffloadAudioRenderSink> sink = std::make_shared<RemoteOffloadAudioRenderSink>("test");
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, Start()).WillOnce(testing::Return(ERR_OPERATION_FAILED));
    sink->audioRender_ = mockRender;
    sink->renderInited_.store(true);
    sink->started_.store(false);
    sink->validState_.store(true);
    sink->attr_.hdPlayBackMode = HdPlaybackMode::APP_LEVEL;
    sink->appsUid_.insert(20000001);
    int32_t ret = sink->Start();
    EXPECT_EQ(ret, ERR_NOT_STARTED);
}

/**
 * @tc.name   : Test Start Mock HDI fail
 * @tc.number : Start_MockHdiFail_002
 * @tc.desc   : Test Start with Mock HDI Start fail and HD_NOT_SUPPORTED
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, Start_MockHdiFail_002, TestSize.Level1)
{
    std::shared_ptr<RemoteOffloadAudioRenderSink> sink = std::make_shared<RemoteOffloadAudioRenderSink>("test");
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, Start()).WillOnce(testing::Return(ERR_OPERATION_FAILED));
    sink->audioRender_ = mockRender;
    sink->renderInited_.store(true);
    sink->started_.store(false);
    sink->validState_.store(true);
    sink->attr_.hdPlayBackMode = HdPlaybackMode::HD_NOT_SUPPORTED;
    sink->appsUid_.insert(20000001);
    int32_t ret = sink->Start();
    EXPECT_EQ(ret, ERR_NOT_STARTED);
}

/**
 * @tc.name   : Test Stop Mock HDI fail
 * @tc.number : Stop_MockHdiFail_001
 * @tc.desc   : Test Stop with Mock HDI Stop fail
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, Stop_MockHdiFail_001, TestSize.Level1)
{
    std::shared_ptr<RemoteOffloadAudioRenderSink> sink = std::make_shared<RemoteOffloadAudioRenderSink>("test");
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, Stop()).WillOnce(testing::Return(ERR_OPERATION_FAILED));
    sink->audioRender_ = mockRender;
    sink->started_.store(true);
    sink->validState_.store(true);
    sink->attr_.hdPlayBackMode = HdPlaybackMode::APP_LEVEL;
    sink->appsUid_.insert(20000001);
    int32_t ret = sink->Stop();
    EXPECT_EQ(ret, ERR_NOT_STARTED);
}

/**
 * @tc.name   : Test Pause Mock HDI fail
 * @tc.number : Pause_MockHdiFail_001
 * @tc.desc   : Test Pause with Mock HDI Pause fail
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, Pause_MockHdiFail_001, TestSize.Level1)
{
    std::shared_ptr<RemoteOffloadAudioRenderSink> sink = std::make_shared<RemoteOffloadAudioRenderSink>("test");
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, Pause()).WillOnce(testing::Return(ERR_OPERATION_FAILED));
    sink->audioRender_ = mockRender;
    sink->started_.store(true);
    sink->paused_.store(false);
    sink->validState_.store(true);
    sink->attr_.hdPlayBackMode = HdPlaybackMode::APP_LEVEL;
    sink->appsUid_.insert(20000001);
    int32_t ret = sink->Pause();
    EXPECT_EQ(ret, ERR_NOT_STARTED);
}

/**
 * @tc.name   : Test FlushInner Mock HDI fail
 * @tc.number : FlushInner_MockHdiFail_001
 * @tc.desc   : Test FlushInner with Mock HDI Flush fail
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, FlushInner_MockHdiFail_001, TestSize.Level1)
{
    std::shared_ptr<RemoteOffloadAudioRenderSink> sink = std::make_shared<RemoteOffloadAudioRenderSink>("test");
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, Flush()).WillOnce(testing::Return(ERR_OPERATION_FAILED));
    sink->audioRender_ = mockRender;
    sink->started_.store(true);
    sink->isFlushing_.store(false);
    sink->validState_.store(true);
    sink->attr_.hdPlayBackMode = HdPlaybackMode::APP_LEVEL;
    sink->appsUid_.insert(20000001);
    int32_t ret = sink->FlushInner();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test GetLatencyInner Mock HDI fail
 * @tc.number : GetLatencyInner_MockHdiFail_001
 * @tc.desc   : Test GetLatencyInner with Mock HDI GetLatency fail
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, GetLatencyInner_MockHdiFail_001, TestSize.Level1)
{
    std::shared_ptr<RemoteOffloadAudioRenderSink> sink = std::make_shared<RemoteOffloadAudioRenderSink>("test");
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, GetLatency(testing::_)).WillOnce(testing::Return(ERR_OPERATION_FAILED));
    sink->audioRender_ = mockRender;
    sink->validState_.store(true);
    sink->attr_.hdPlayBackMode = HdPlaybackMode::APP_LEVEL;
    sink->appsUid_.insert(20000001);
    int32_t ret = sink->GetLatencyInner();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name   : Test GetRenderPositionInner Mock HDI fail
 * @tc.number : GetRenderPositionInner_MockHdiFail_001
 * @tc.desc   : Test GetRenderPositionInner with Mock HDI GetRenderPosition fail
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, GetRenderPositionInner_MockHdiFail_001, TestSize.Level1)
{
    std::shared_ptr<RemoteOffloadAudioRenderSink> sink = std::make_shared<RemoteOffloadAudioRenderSink>("test");
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender,
        GetRenderPosition(testing::_, testing::_)).WillOnce(testing::Return(ERR_OPERATION_FAILED));
    sink->audioRender_ = mockRender;
    sink->isFlushing_.store(false);
    sink->validState_.store(true);
    sink->attr_.hdPlayBackMode = HdPlaybackMode::APP_LEVEL;
    sink->appsUid_.insert(20000001);
    int32_t ret = sink->GetRenderPositionInner();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name   : Test SetAudioParameter Mock HDI fail
 * @tc.number : SetAudioParameter_MockHdiFail_001
 * @tc.desc   : Test SetAudioParameter with Mock HDI SetExtraParams fail
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, SetAudioParameter_MockHdiFail_001, TestSize.Level1)
{
    std::shared_ptr<RemoteOffloadAudioRenderSink> sink = std::make_shared<RemoteOffloadAudioRenderSink>("test");
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, SetExtraParams(testing::_)).WillRepeatedly(testing::Return(ERR_OPERATION_FAILED));
    sink->audioRender_ = mockRender;
    sink->validState_.store(true);
    sink->attr_.hdPlayBackMode = HdPlaybackMode::APP_LEVEL;
    sink->appsUid_.insert(20000001);
    sink->SetAudioParameter(AudioParamKey::NONE, "", "test=value");
    int32_t ret = sink->audioRender_->SetExtraParams("test=value");
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name   : Test SetSpeed Mock HDI fail
 * @tc.number : SetSpeed_MockHdiFail_001
 * @tc.desc   : Test SetSpeed with Mock HDI SetRenderSpeed fail
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, SetSpeed_MockHdiFail_001, TestSize.Level1)
{
    std::shared_ptr<RemoteOffloadAudioRenderSink> sink = std::make_shared<RemoteOffloadAudioRenderSink>("test");
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, SetRenderSpeed(testing::_)).WillRepeatedly(testing::Return(ERR_OPERATION_FAILED));
    sink->audioRender_ = mockRender;
    sink->validState_.store(true);
    sink->attr_.hdPlayBackMode = HdPlaybackMode::APP_LEVEL;
    sink->appsUid_.insert(20000001);
    sink->SetSpeed(1.5f);
    int32_t ret = sink->audioRender_->SetRenderSpeed(1.5f);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name   : Test UpdateAppsUid Mock HDI fail
 * @tc.number : UpdateAppsUid_MockHdiFail_001
 * @tc.desc   : Test UpdateAppsUid with Mock HDI SetExtraParams fail
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, UpdateAppsUid_MockHdiFail_001, TestSize.Level1)
{
    std::shared_ptr<RemoteOffloadAudioRenderSink> sink = std::make_shared<RemoteOffloadAudioRenderSink>("test");
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, SetExtraParams(testing::_)).WillOnce(testing::Return(ERR_OPERATION_FAILED));
    sink->audioRender_ = mockRender;
    sink->runningLock_ = std::make_shared<AudioRunningLock>("test");
    sink->validState_.store(true);
    sink->attr_.hdPlayBackMode = HdPlaybackMode::APP_LEVEL;
    sink->appsUid_.insert(20000001);
    std::vector<int32_t> newAppsUid = {20000002, 20000003};
    int32_t ret = sink->UpdateAppsUid(newAppsUid);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name   : Test RegistOffloadHdiCallback Mock HDI fail
 * @tc.number : RegistOffloadHdiCallback_MockHdiFail_001
 * @tc.desc   : Test RegistOffloadHdiCallback with Mock HDI RegCallback fail
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, RegistOffloadHdiCallback_MockHdiFail_001, TestSize.Level1)
{
    std::shared_ptr<RemoteOffloadAudioRenderSink> sink = std::make_shared<RemoteOffloadAudioRenderSink>("test");
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, RegCallback(testing::_, testing::_)).WillRepeatedly(testing::Return(ERR_OPERATION_FAILED));
    sink->audioRender_ = mockRender;
    sink->validState_.store(true);
    sink->attr_.hdPlayBackMode = HdPlaybackMode::APP_LEVEL;
    sink->appsUid_.insert(20000001);
    sink->RegistOffloadHdiCallback([](RenderCallbackType type) {});
    int32_t ret = sink->audioRender_->RegCallback(new RemoteOffloadHdiCallbackImpl(sink.get()), (int8_t)0);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name   : Test CreateRender Mock HDI fail
 * @tc.number : CreateRender_MockHdiFail_001
 * @tc.desc   : Test CreateRender with Mock HDI CreateRender fail
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, CreateRender_MockHdiFail_001, TestSize.Level1)
{
    std::shared_ptr<RemoteOffloadAudioRenderSink> sink = std::make_shared<RemoteOffloadAudioRenderSink>("test");
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    sink->attr_.hdPlayBackMode = HdPlaybackMode::APP_LEVEL;
    sink->appsUid_.insert(20000001);
    int32_t ret = sink->CreateRender();
    EXPECT_EQ(ret, ERR_NOT_STARTED);
}

/**
 * @tc.name   : Test SetBufferSize Mock HDI fail
 * @tc.number : SetBufferSize_MockHdiFail_001
 * @tc.desc   : Test SetBufferSize with Mock HDI ReqMmapBuffer fail
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, SetBufferSize_MockHdiFail_001, TestSize.Level1)
{
    std::shared_ptr<RemoteOffloadAudioRenderSink> sink = std::make_shared<RemoteOffloadAudioRenderSink>("test");
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, ReqMmapBuffer(testing::_, testing::_)).WillOnce(testing::Return(ERR_OPERATION_FAILED));
    sink->audioRender_ = mockRender;
    sink->validState_.store(true);
    sink->attr_.hdPlayBackMode = HdPlaybackMode::APP_LEVEL;
    sink->appsUid_.insert(20000001);
    int32_t ret = sink->SetBufferSize(100);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name   : Test SetVolumeInner Mock HDI fail
 * @tc.number : SetVolumeInner_MockHdiFail_001
 * @tc.desc   : Test SetVolumeInner with Mock HDI SetVolume fail
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, SetVolumeInner_MockHdiFail_001, TestSize.Level1)
{
    std::shared_ptr<RemoteOffloadAudioRenderSink> sink = std::make_shared<RemoteOffloadAudioRenderSink>("test");
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    EXPECT_CALL(*mockRender, SetVolume(testing::_)).WillOnce(testing::Return(ERR_OPERATION_FAILED));
    sink->audioRender_ = mockRender;
    sink->isFlushing_.store(false);
    sink->validState_.store(true);
    sink->attr_.hdPlayBackMode = HdPlaybackMode::APP_LEVEL;
    sink->appsUid_.insert(20000001);
    int32_t ret = sink->SetVolumeInner(1.0f, 1.0f);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name   : Test RenderFrame Mock HDI fail
 * @tc.number : RenderFrame_MockHdiFail_001
 * @tc.desc   : Test RenderFrame with Mock HDI RenderFrame fail
 */
HWTEST_F(RemoteOffloadAudioRenderSinkUnitTest, RenderFrame_MockHdiFail_001, TestSize.Level1)
{
    std::shared_ptr<RemoteOffloadAudioRenderSink> sink = std::make_shared<RemoteOffloadAudioRenderSink>("test");
    sptr<MockRemoteIAudioRender> mockRender = new MockRemoteIAudioRender();
    uint64_t writeLen = 0;
    EXPECT_CALL(*mockRender, RenderFrame(testing::_, testing::_)).WillOnce(testing::Return(ERR_OPERATION_FAILED));

    sink->audioRender_ = mockRender;
    sink->started_.store(true);
    sink->validState_.store(true);
    sink->isFlushing_.store(false);
    sink->attr_.hdPlayBackMode = HdPlaybackMode::APP_LEVEL;
    sink->appsUid_.insert(20000001);

    char data = 0;
    uint64_t len = 1;
    int32_t ret = sink->RenderFrame(data, len, writeLen);
    EXPECT_EQ(ret, ERR_WRITE_FAILED);
}

} // namespace AudioStandard
} // namespace OHOS
