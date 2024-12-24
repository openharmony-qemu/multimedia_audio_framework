/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef LOG_TAG
#define LOG_TAG "AudioDeviceManagerImplUnitTest"
#endif

#include "audio_device_manager_impl_unit_test.h"

#include <chrono>
#include <thread>

#include "audio_errors.h"
#include "audio_device_manager_impl.h"

using namespace std;
using namespace std::chrono;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

namespace {
    AudioDeviceManagerFactory &audioDeviceManagerFactory = AudioDeviceManagerFactory::GetInstance();

#ifdef FEATURE_DISTRIBUTE_AUDIO
    sptr<IAudioManager> audioMgr = IAudioManager::Get("daudio_primary_service", false);
#else
    sptr<IAudioManager> audioMgr = nullptr;
#endif // FEATURE_DISTRIBUTE_AUDIO
    std::shared_ptr<AudioDeviceManagerImpl> audioDeviceManagerIml =
        std::make_shared<AudioDeviceManagerImpl>(LOCAL_DEV_MGR, audioMgr);
}

class IAudioDeviceManagerInner : public IAudioDeviceManager {
public:
    IAudioDeviceManagerInner() {};
    ~IAudioDeviceManagerInner() {};

    int32_t GetAllAdapters() override
    {
        return SUCCESS;
    };

    struct AudioAdapterDescriptor *GetTargetAdapterDesc(const std::string &adapterName, bool isMmap) override
    {
        return nullptr;
    };

    std::shared_ptr<IAudioDeviceAdapter> LoadAdapters(const std::string &adapterName, bool isMmap) override
    {
        return nullptr;
    };

    int32_t UnloadAdapter(const std::string &adapterName) override
    {
        return SUCCESS;
    };

    int32_t Release() override
    {
        return SUCCESS;
    };
};

class IAudioAdapterFork : public IAudioAdapter {
public:
    DECLARE_HDI_DESCRIPTOR(u"ohos.hdi.distributed_audio.audio.v1_0.IAudioAdapter");

    IAudioAdapterFork() {};
    ~IAudioAdapterFork() {};

    int32_t InitAllPorts() override
    {
        return SUCCESS;
    };

    int32_t CreateRender(const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioDeviceDescriptor& desc,
        const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSampleAttributes& attrs,
            sptr<OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioRender>& render, uint32_t& renderId) override
    {
        return SUCCESS;
    };

    int32_t DestroyRender(uint32_t renderId) override
    {
        return SUCCESS;
    };

    int32_t CreateCapture(const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioDeviceDescriptor& desc,
        const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSampleAttributes& attrs,
            sptr<OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioCapture>& capture, uint32_t& captureId) override
    {
        return SUCCESS;
    };

    int32_t DestroyCapture(uint32_t captureId) override
    {
        return SUCCESS;
    };

    int32_t GetPortCapability(const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPort& port,
        OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPortCapability& capability) override
    {
        return SUCCESS;
    };

    int32_t SetPassthroughMode(const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPort& port,
        OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPortPassthroughMode mode) override
    {
        return SUCCESS;
    };

    int32_t GetPassthroughMode(const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPort& port,
        OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPortPassthroughMode& mode) override
    {
        return SUCCESS;
    };

    int32_t GetDeviceStatus(OHOS::HDI::DistributedAudio::Audio::V1_0::AudioDeviceStatus& status) override
    {
        return SUCCESS;
    };

    int32_t UpdateAudioRoute(const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioRoute& route,
         int32_t& routeHandle) override
    {
        return SUCCESS;
    };

    int32_t ReleaseAudioRoute(int32_t routeHandle) override
    {
        return SUCCESS;
    };

    int32_t SetMicMute(bool mute) override
    {
        return SUCCESS;
    };

    int32_t GetMicMute(bool& mute) override
    {
        return SUCCESS;
    };

    int32_t SetVoiceVolume(float volume) override
    {
        return SUCCESS;
    };

    int32_t SetExtraParams(OHOS::HDI::DistributedAudio::Audio::V1_0::AudioExtParamKey key,
        const std::string& condition, const std::string& value) override
    {
        return SUCCESS;
    };

    int32_t GetExtraParams(OHOS::HDI::DistributedAudio::Audio::V1_0::AudioExtParamKey key,
        const std::string& condition, std::string& value) override
    {
        return SUCCESS;
    };

    int32_t RegExtraParamObserver(const sptr<OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioCallback>& audioCallback,
        int8_t cookie) override
    {
        return SUCCESS;
    };

    int32_t GetVersion(uint32_t& majorVer, uint32_t& minorVer) override
    {
        majorVer = 1;
        minorVer = 0;
        return HDF_SUCCESS;
    }

    bool IsProxy() override
    {
        return false;
    }

    const std::u16string GetDesc() override
    {
        return metaDescriptor_;
    }
};

class IAudioDeviceAdapterInner : public IAudioDeviceAdapter {
public:
    IAudioDeviceAdapterInner() {};
    ~IAudioDeviceAdapterInner() {};

    int32_t Init() override
    {
        return SUCCESS;
    };

    int32_t RegExtraParamObserver() override
    {
        return SUCCESS;
    };

    int32_t CreateRender(const AudioDeviceDescriptor &devDesc, const AudioSampleAttributes &attr,
        sptr<IAudioRender> &audioRender, IAudioDeviceAdapterCallback *renderCb, uint32_t &renderId) override
    {
        return SUCCESS;
    };

    void DestroyRender(sptr<IAudioRender> audioRender, uint32_t &renderId) override
    {
    };

    int32_t CreateCapture(const AudioDeviceDescriptor &devDesc, const AudioSampleAttributes &attr,
        sptr<IAudioCapture> &audioCapture, IAudioDeviceAdapterCallback *captureCb, uint32_t &captureId) override
    {
        return SUCCESS;
    };

    void DestroyCapture(sptr<IAudioCapture> audioCapture, uint32_t &captureId) override
    {
    };

    void SetAudioParameter(const AudioParamKey key, const std::string &condition,
        const std::string &value) override
    {
    };

    std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) override
    {
        return nullptr;
    };

    int32_t UpdateAudioRoute(const AudioRoute &route) override
    {
        return SUCCESS;
    };

    int32_t Release() override
    {
        return SUCCESS;
    };
};

void AudioDeviceManagerImplUnitTest::SetUpTestCase(void) {}
void AudioDeviceManagerImplUnitTest::TearDownTestCase(void) {}
void AudioDeviceManagerImplUnitTest::SetUp(void) {}
void AudioDeviceManagerImplUnitTest::TearDown(void) {}

/**
* @tc.name   : Test DestoryDeviceManager API
* @tc.number : AudioDeviceManagerImpl_001
* @tc.desc   : Test DestoryDeviceManager interface.
*/
HWTEST(AudioDeviceManagerImplUnitTest, AudioDeviceManagerImpl_001, TestSize.Level1)
{
    ASSERT_TRUE(&audioDeviceManagerFactory);
    int32_t result = audioDeviceManagerFactory.DestoryDeviceManager(LOCAL_DEV_MGR);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test DestoryDeviceManager API
* @tc.number : AudioDeviceManagerImpl_002
* @tc.desc   : Test DestoryDeviceManager interface.
*/
HWTEST(AudioDeviceManagerImplUnitTest, AudioDeviceManagerImpl_002, TestSize.Level1)
{
    ASSERT_TRUE(&audioDeviceManagerFactory);
    std::shared_ptr<IAudioDeviceManager> audioManager = std::make_shared<IAudioDeviceManagerInner>();
    ASSERT_TRUE(audioManager != nullptr);
    audioDeviceManagerFactory.allHdiDevMgr_.emplace(LOCAL_DEV_MGR, audioManager);
    int32_t result = audioDeviceManagerFactory.DestoryDeviceManager(LOCAL_DEV_MGR);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test CreatDeviceManager API
* @tc.number : AudioDeviceManagerImpl_003
* @tc.desc   : Test CreatDeviceManager interface.
*/
HWTEST(AudioDeviceManagerImplUnitTest, AudioDeviceManagerImpl_003, TestSize.Level1)
{
    ASSERT_TRUE(&audioDeviceManagerFactory);
    std::shared_ptr<IAudioDeviceManager> audioDevMgr = audioDeviceManagerFactory.CreatDeviceManager(DEV_MGR_UNKNOW);
    EXPECT_EQ(nullptr, audioDevMgr);
}

/**
* @tc.name   : Test CreatDeviceManager API
* @tc.number : AudioDeviceManagerImpl_004
* @tc.desc   : Test CreatDeviceManager interface.
*/
HWTEST(AudioDeviceManagerImplUnitTest, AudioDeviceManagerImpl_004, TestSize.Level1)
{
    ASSERT_TRUE(&audioDeviceManagerFactory);
    std::shared_ptr<IAudioDeviceManager> audioDevMgr = audioDeviceManagerFactory.CreatDeviceManager(LOCAL_DEV_MGR);
    EXPECT_EQ(nullptr, audioDevMgr);
}

/**
* @tc.name   : Test CreatDeviceManager API
* @tc.number : AudioDeviceManagerImpl_005
* @tc.desc   : Test CreatDeviceManager interface.
*/
HWTEST(AudioDeviceManagerImplUnitTest, AudioDeviceManagerImpl_005, TestSize.Level1)
{
    ASSERT_TRUE(&audioDeviceManagerFactory);
    std::shared_ptr<IAudioDeviceManager> audioDevMgr = audioDeviceManagerFactory.CreatDeviceManager(REMOTE_DEV_MGR);
    EXPECT_EQ(nullptr, audioDevMgr);
}

/**
* @tc.name   : Test CreatDeviceManager API
* @tc.number : AudioDeviceManagerImpl_006
* @tc.desc   : Test CreatDeviceManager interface.
*/
HWTEST(AudioDeviceManagerImplUnitTest, AudioDeviceManagerImpl_006, TestSize.Level1)
{
    ASSERT_TRUE(&audioDeviceManagerFactory);
    std::shared_ptr<IAudioDeviceManager> audioDevMgr = audioDeviceManagerFactory.CreatDeviceManager(BLUETOOTH_DEV_MGR);
    EXPECT_EQ(nullptr, audioDevMgr);
}

/**
* @tc.name   : Test CreatDeviceManager API
* @tc.number : AudioDeviceManagerImpl_007
* @tc.desc   : Test CreatDeviceManager interface.
*/
HWTEST(AudioDeviceManagerImplUnitTest, AudioDeviceManagerImpl_007, TestSize.Level1)
{
    ASSERT_TRUE(&audioDeviceManagerFactory);
    std::shared_ptr<IAudioDeviceManager> audioManager = std::make_shared<IAudioDeviceManagerInner>();
    ASSERT_TRUE(audioManager != nullptr);
    audioDeviceManagerFactory.allHdiDevMgr_.emplace(BLUETOOTH_DEV_MGR, audioManager);
    std::shared_ptr<IAudioDeviceManager> audioDevMgr = audioDeviceManagerFactory.CreatDeviceManager(BLUETOOTH_DEV_MGR);
    EXPECT_NE(nullptr, audioDevMgr);
}

/**
* @tc.name   : Test GetTargetAdapterDesc API
* @tc.number : AudioDeviceManagerImpl_008
* @tc.desc   : Test GetTargetAdapterDesc interface.
*/
HWTEST(AudioDeviceManagerImplUnitTest, AudioDeviceManagerImpl_008, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceManagerIml != nullptr);
#ifdef FEATURE_DISTRIBUTE_AUDIO
    audioDeviceManagerIml->audioMgr_ = IAudioManager::Get("daudio_primary_service", false);
#endif // FEATURE_DISTRIBUTE_AUDIO
    if (audioDeviceManagerIml->audioMgr_ == nullptr) {
        GTEST_SKIP();
    }

    AudioAdapterDescriptor audioAdapterDescriptor;
    audioAdapterDescriptor.adapterName = "";
    audioDeviceManagerIml->descriptors_.push_back(audioAdapterDescriptor);
    std::string adapterName = "";
    struct AudioAdapterDescriptor *desc = audioDeviceManagerIml->GetTargetAdapterDesc(adapterName, true);
    EXPECT_NE(nullptr, desc);
}

/**
* @tc.name   : Test GetTargetAdapterDesc API
* @tc.number : AudioDeviceManagerImpl_009
* @tc.desc   : Test GetTargetAdapterDesc interface.
*/
HWTEST(AudioDeviceManagerImplUnitTest, AudioDeviceManagerImpl_009, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceManagerIml != nullptr);
#ifdef FEATURE_DISTRIBUTE_AUDIO
    audioDeviceManagerIml->audioMgr_ = IAudioManager::Get("daudio_primary_service", false);
#endif // FEATURE_DISTRIBUTE_AUDIO
    if (audioDeviceManagerIml->audioMgr_ == nullptr) {
        GTEST_SKIP();
    }

    AudioAdapterDescriptor audioAdapterDescriptor;
    audioAdapterDescriptor.adapterName = "adapterName";
    audioDeviceManagerIml->descriptors_.push_back(audioAdapterDescriptor);
    std::string adapterName = "adapterName";
    struct AudioAdapterDescriptor *desc = audioDeviceManagerIml->GetTargetAdapterDesc(adapterName, true);
    EXPECT_NE(nullptr, desc);
}

/**
* @tc.name   : Test GetTargetAdapterDesc API
* @tc.number : AudioDeviceManagerImpl_010
* @tc.desc   : Test GetTargetAdapterDesc interface.
*/
HWTEST(AudioDeviceManagerImplUnitTest, AudioDeviceManagerImpl_010, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceManagerIml != nullptr);
#ifdef FEATURE_DISTRIBUTE_AUDIO
    audioDeviceManagerIml->audioMgr_ = IAudioManager::Get("daudio_primary_service", false);
#endif // FEATURE_DISTRIBUTE_AUDIO
    if (audioDeviceManagerIml->audioMgr_ == nullptr) {
        GTEST_SKIP();
    }

    AudioAdapterDescriptor audioAdapterDescriptor;
    audioAdapterDescriptor.adapterName = "adapterName";
    audioDeviceManagerIml->descriptors_.push_back(audioAdapterDescriptor);
    std::string adapterName = "";
    struct AudioAdapterDescriptor *desc = audioDeviceManagerIml->GetTargetAdapterDesc(adapterName, true);
    EXPECT_NE(nullptr, desc);
}

/**
* @tc.name   : Test LoadAdapters API
* @tc.number : AudioDeviceManagerImpl_011
* @tc.desc   : Test LoadAdapters interface.
*/
HWTEST(AudioDeviceManagerImplUnitTest, AudioDeviceManagerImpl_011, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceManagerIml != nullptr);
#ifdef FEATURE_DISTRIBUTE_AUDIO
    audioDeviceManagerIml->audioMgr_ = IAudioManager::Get("daudio_primary_service", false);
#endif // FEATURE_DISTRIBUTE_AUDIO
    if (audioDeviceManagerIml->audioMgr_ == nullptr) {
        GTEST_SKIP();
    }

    std::string enableAdapters = "";
    DeviceAdapterInfo deviceAdapterInfo;
    deviceAdapterInfo.devAdp = std::make_shared<IAudioDeviceAdapterInner>();
    audioDeviceManagerIml->enableAdapters_.emplace(enableAdapters, deviceAdapterInfo);
    std::string adapterName = "";
    auto audioDevAdp = audioDeviceManagerIml->LoadAdapters(adapterName, true);
    EXPECT_NE(nullptr, audioDevAdp);
}

/**
* @tc.name   : Test LoadAdapters API
* @tc.number : AudioDeviceManagerImpl_012
* @tc.desc   : Test LoadAdapters interface.
*/
HWTEST(AudioDeviceManagerImplUnitTest, AudioDeviceManagerImpl_012, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceManagerIml != nullptr);
#ifdef FEATURE_DISTRIBUTE_AUDIO
    audioDeviceManagerIml->audioMgr_ = IAudioManager::Get("daudio_primary_service", false);
#endif // FEATURE_DISTRIBUTE_AUDIO
    if (audioDeviceManagerIml->audioMgr_ == nullptr) {
        GTEST_SKIP();
    }

    std::string enableAdapters = "";
    DeviceAdapterInfo deviceAdapterInfo;
    audioDeviceManagerIml->enableAdapters_.emplace(enableAdapters, deviceAdapterInfo);
    std::string adapterName = "adapterName";
    auto audioDevAdp = audioDeviceManagerIml->LoadAdapters(adapterName, true);
    EXPECT_NE(nullptr, audioDevAdp);
}

/**
* @tc.name   : Test UnloadAdapter API
* @tc.number : AudioDeviceManagerImpl_013
* @tc.desc   : Test UnloadAdapter interface.
*/
HWTEST(AudioDeviceManagerImplUnitTest, AudioDeviceManagerImpl_013, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceManagerIml != nullptr);
    std::string enableAdapters = "";
    DeviceAdapterInfo deviceAdapterInfo;
    audioDeviceManagerIml->enableAdapters_.emplace(enableAdapters, deviceAdapterInfo);
    std::string adapterName = "adapterName";
    int32_t result = audioDeviceManagerIml->UnloadAdapter(adapterName);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test UnloadAdapter API
* @tc.number : AudioDeviceManagerImpl_014
* @tc.desc   : Test UnloadAdapter interface.
*/
HWTEST(AudioDeviceManagerImplUnitTest, AudioDeviceManagerImpl_014, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceManagerIml != nullptr);
#ifdef FEATURE_DISTRIBUTE_AUDIO
    audioDeviceManagerIml->audioMgr_ = IAudioManager::Get("daudio_primary_service", false);
#endif // FEATURE_DISTRIBUTE_AUDIO
    if (audioDeviceManagerIml->audioMgr_ == nullptr) {
        GTEST_SKIP();
    }

    std::string enableAdapters = "adapterName";
    DeviceAdapterInfo deviceAdapterInfo;
    deviceAdapterInfo.devAdp = std::make_shared<IAudioDeviceAdapterInner>();
    deviceAdapterInfo.audioAdapter = new IAudioAdapterFork();
    audioDeviceManagerIml->enableAdapters_.emplace(enableAdapters, deviceAdapterInfo);
    std::string adapterName = "adapterName";
    int32_t result = audioDeviceManagerIml->UnloadAdapter(adapterName);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test UnloadAdapter API
* @tc.number : AudioDeviceManagerImpl_015
* @tc.desc   : Test UnloadAdapter interface.
*/
HWTEST(AudioDeviceManagerImplUnitTest, AudioDeviceManagerImpl_015, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceManagerIml != nullptr);
#ifdef FEATURE_DISTRIBUTE_AUDIO
    audioDeviceManagerIml->audioMgr_ = IAudioManager::Get("daudio_primary_service", false);
#endif // FEATURE_DISTRIBUTE_AUDIO
    if (audioDeviceManagerIml->audioMgr_ == nullptr) {
        GTEST_SKIP();
    }

    std::string enableAdapters = "adapterName";
    DeviceAdapterInfo deviceAdapterInfo;
    deviceAdapterInfo.audioAdapter = new IAudioAdapterFork();
    audioDeviceManagerIml->enableAdapters_.emplace(enableAdapters, deviceAdapterInfo);
    std::string adapterName = "adapterName";
    int32_t result = audioDeviceManagerIml->UnloadAdapter(adapterName);
    EXPECT_EQ(SUCCESS, result);
}
} // namespace AudioStandard
} // namespace OHOS