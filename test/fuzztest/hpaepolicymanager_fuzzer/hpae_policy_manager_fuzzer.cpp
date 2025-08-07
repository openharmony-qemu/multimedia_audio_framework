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
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"

#include "audio_policy_utils.h"
#include "audio_stream_descriptor.h"
#include "audio_limiter_manager.h"
#include "dfx_msg_manager.h"

#include "audio_source_clock.h"
#include "capturer_clock_manager.h"
#include "hpae_policy_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint8_t TESTSIZE = 22;
static int32_t NUM_2 = 2;

typedef void (*TestFuncs)();

vector<AudioSpatializationSceneType> AudioSpatializationSceneTypeVec {
    SPATIALIZATION_SCENE_TYPE_DEFAULT,
    SPATIALIZATION_SCENE_TYPE_MUSIC,
    SPATIALIZATION_SCENE_TYPE_MOVIE,
    SPATIALIZATION_SCENE_TYPE_AUDIOBOOK,
    SPATIALIZATION_SCENE_TYPE_MAX,
};

vector<DeviceType> DeviceTypeVec = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_INVALID,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WAKEUP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_DP,
    DEVICE_TYPE_REMOTE_CAST,
    DEVICE_TYPE_USB_DEVICE,
    DEVICE_TYPE_ACCESSORY,
    DEVICE_TYPE_REMOTE_DAUDIO,
    DEVICE_TYPE_HDMI,
    DEVICE_TYPE_LINE_DIGITAL,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_NEARLINK_IN,
    DEVICE_TYPE_FILE_SINK,
    DEVICE_TYPE_FILE_SOURCE,
    DEVICE_TYPE_EXTERN_CABLE,
    DEVICE_TYPE_DEFAULT,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_MAX,
};

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

void UpdateSpatializationStateFuzzTest()
{
    AudioSpatializationState spatializationState = {};
    HPAE::HpaePolicyManager hpaePolicyManager;
    hpaePolicyManager.GetInstance().UpdateSpatializationState(spatializationState);
}

void UpdateSpatialDeviceTypeFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    constexpr int32_t spatialDeviceTypeCount = static_cast<int32_t>(AudioSpatialDeviceType::EARPHONE_TYPE_OTHERS) + 1;
    AudioSpatialDeviceType spatialDeviceType =
        static_cast<AudioSpatialDeviceType>(GetData<uint8_t>() % spatialDeviceTypeCount);
    hpaePolicyManager.GetInstance().UpdateSpatialDeviceType(spatialDeviceType);
}

void SetSpatializationSceneTypeFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    uint32_t audioSpatializationSceneTypeCount = GetData<uint32_t>() % AudioSpatializationSceneTypeVec.size();
    AudioSpatializationSceneType spatializationSceneType =
        AudioSpatializationSceneTypeVec[audioSpatializationSceneTypeCount];
    hpaePolicyManager.GetInstance().SetSpatializationSceneType(spatializationSceneType);
}

void EffectRotationUpdateFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    uint32_t rotationState = GetData<uint32_t>();
    hpaePolicyManager.GetInstance().EffectRotationUpdate(rotationState);
}

void SetEffectSystemVolumeFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    int32_t systemVolumeType = GetData<int32_t>();
    float systemVolume = GetData<float>();
    hpaePolicyManager.GetInstance().SetEffectSystemVolume(systemVolumeType, systemVolume);
}

void SetAudioEffectPropertyFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    AudioEffectPropertyArrayV3 propertyArray;
    hpaePolicyManager.GetInstance().SetAudioEffectProperty(propertyArray);
}

void GetAudioEffectPropertyFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    AudioEffectPropertyArray propertyArray1;
    hpaePolicyManager.GetInstance().GetAudioEffectProperty(propertyArray1);
}

void SetInputDeviceFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    uint32_t captureId = GetData<uint32_t>();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType inputDevice = DeviceTypeVec[deviceTypeCount];
    hpaePolicyManager.GetInstance().SetInputDevice(captureId, inputDevice, "Built_in_mic");
}

void SetOutputDeviceFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    uint32_t renderId = GetData<uint32_t>();
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType outputDevice = DeviceTypeVec[deviceTypeCount];
    hpaePolicyManager.GetInstance().SetOutputDevice(renderId, outputDevice);
}

void SetVolumeInfoFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(0);
    float systemVolume = GetData<float>();
    hpaePolicyManager.GetInstance().SetVolumeInfo(volumeType, systemVolume);
}

void SetMicrophoneMuteInfoFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    bool isMute = GetData<uint32_t>() % NUM_2;
    hpaePolicyManager.GetInstance().SetMicrophoneMuteInfo(isMute);
}

void SetStreamVolumeInfoFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    uint32_t sessionId = GetData<uint32_t>();
    float streamVol = GetData<float>();
    hpaePolicyManager.GetInstance().SetStreamVolumeInfo(sessionId, streamVol);
}

void SetAudioEnhancePropertyFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    AudioEffectPropertyArrayV3 propertyArray2;
    propertyArray2.property.push_back({"invalidEffect", "property1"});
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    hpaePolicyManager.GetInstance().SetAudioEnhanceProperty(propertyArray2, deviceType);
}

void GetAudioEnhancePropertyFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    AudioEffectPropertyArrayV3 propertyArray3;
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    hpaePolicyManager.GetInstance().GetAudioEnhanceProperty(propertyArray3, deviceType);
}

void UpdateExtraSceneTypeFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    std::string mainkey = "other_mainkey";
    std::string subkey = "other_subkey";
    std::string extraSceneType = "extra_scene";
    hpaePolicyManager.GetInstance().UpdateExtraSceneType(mainkey, subkey, extraSceneType);
}

void InitAudioEffectChainManagerFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    std::vector<EffectChain> effectChains = {{"EFFECTCHAIN_SPK_MUSIC", {}, ""}, {"EFFECTCHAIN_BT_MUSIC", {}, ""}};
    EffectChainManagerParam effectChainManagerParam;
    std::vector<std::shared_ptr<AudioEffectLibEntry>> effectLibraryList;
    hpaePolicyManager.GetInstance().InitAudioEffectChainManager(effectChains,
        effectChainManagerParam, effectLibraryList);
}

void SetOutputDeviceSinkFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    int32_t device = GetData<int32_t>();
    std::string sinkName = "abc";
    hpaePolicyManager.GetInstance().SetOutputDeviceSink(device, sinkName);
}

void SetAbsVolumeStateToEffectFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    bool absVolumeState = GetData<bool>();
    hpaePolicyManager.GetInstance().SetAbsVolumeStateToEffect(absVolumeState);
}

void InitHdiStateFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    hpaePolicyManager.GetInstance().InitHdiState();
}

void UpdateParamExtraFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    std::string mainkey = "device_status";
    std::string subkey = "update_audio_effect_type";
    std::string value = "test";
    hpaePolicyManager.GetInstance().UpdateParamExtra(mainkey, subkey, value);
}

void InitAudioEnhanceChainManagerFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    std::vector<EffectChain> enhanceChains = {{"EFFECTCHAIN_SPK_MUSIC", {}, ""}, {"EFFECTCHAIN_BT_MUSIC", {}, ""}};
    EffectChainManagerParam managerParam;
    std::vector<std::shared_ptr<AudioEffectLibEntry>> enhanceLibraryList;
    hpaePolicyManager.GetInstance().InitAudioEnhanceChainManager(enhanceChains, managerParam, enhanceLibraryList);
}

void SetAudioParameterFuzzTest()
{
    HPAE::HpaePolicyManager hpaePolicyManager;
    std::string adapterName = "abc";
    AudioParamKey key = GetData<AudioParamKey>();
    std::string condition = "123456";
    std::string value = "123456";
    hpaePolicyManager.GetInstance().SetAudioParameter(adapterName, key, condition, value);
}

TestFuncs g_testFuncs[TESTSIZE] = {
    UpdateSpatializationStateFuzzTest,
    UpdateSpatialDeviceTypeFuzzTest,
    SetSpatializationSceneTypeFuzzTest,
    EffectRotationUpdateFuzzTest,
    SetEffectSystemVolumeFuzzTest,
    SetAudioEffectPropertyFuzzTest,
    GetAudioEffectPropertyFuzzTest,
    SetInputDeviceFuzzTest,
    SetOutputDeviceFuzzTest,
    SetVolumeInfoFuzzTest,
    SetMicrophoneMuteInfoFuzzTest,
    SetStreamVolumeInfoFuzzTest,
    SetAudioEnhancePropertyFuzzTest,
    GetAudioEnhancePropertyFuzzTest,
    UpdateExtraSceneTypeFuzzTest,
    InitAudioEffectChainManagerFuzzTest,
    SetOutputDeviceSinkFuzzTest,
    SetAbsVolumeStateToEffectFuzzTest,
    InitHdiStateFuzzTest,
    UpdateParamExtraFuzzTest,
    InitAudioEnhanceChainManagerFuzzTest,
    SetAudioParameterFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return true;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
