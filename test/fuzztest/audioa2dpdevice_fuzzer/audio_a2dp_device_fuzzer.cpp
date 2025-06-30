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
#include "audio_policy_state_monitor.h"
#include "audio_device_info.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint8_t TESTSIZE = 8;
static int32_t NUM_2 = 2;

typedef void (*TestFuncs)();

vector<AudioSamplingRate> AudioSamplingRateVec = {
    SAMPLE_RATE_8000,
    SAMPLE_RATE_11025,
    SAMPLE_RATE_12000,
    SAMPLE_RATE_16000,
    SAMPLE_RATE_22050,
    SAMPLE_RATE_24000,
    SAMPLE_RATE_32000,
    SAMPLE_RATE_44100,
    SAMPLE_RATE_48000,
    SAMPLE_RATE_64000,
    SAMPLE_RATE_88200,
    SAMPLE_RATE_96000,
    SAMPLE_RATE_176400,
    SAMPLE_RATE_192000,
};

vector<AudioSampleFormat> AudioSampleFormatVec = {
    SAMPLE_U8,
    SAMPLE_S16LE,
    SAMPLE_S24LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE,
    INVALID_WIDTH,
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

void GetA2dpDeviceInfoFuzzTest()
{
    uint32_t samplingRateCount = GetData<uint32_t>() % AudioSamplingRateVec.size();
    AudioSamplingRate samplingRate = AudioSamplingRateVec[samplingRateCount];
    int32_t encodingCount =
        static_cast<int32_t>(AudioEncodingType::ENCODING_EAC3 - AudioEncodingType::ENCODING_INVALID) + 1;
    AudioEncodingType encoding = static_cast<AudioEncodingType>(GetData<int32_t>() % encodingCount - 1);
    uint32_t formatCount = GetData<uint32_t>() % AudioSampleFormatVec.size();
    AudioSampleFormat format = AudioSampleFormatVec[formatCount];
    int32_t channelsCount = static_cast<int32_t>(AudioChannel::CHANNEL_16) + 1;
    AudioChannel channels = static_cast<AudioChannel>(GetData<int32_t>() % channelsCount);
    DeviceStreamInfo streamInfo(samplingRate, encoding, format, channels);
    A2dpDeviceConfigInfo configInfo;
    configInfo.streamInfo = streamInfo;
    configInfo.absVolumeSupport = GetData<uint32_t>() % NUM_2;
    configInfo.volumeLevel = GetData<int32_t>();
    configInfo.mute = GetData<uint32_t>() % NUM_2;
    string device = "test_device";
    AudioA2dpDevice::GetInstance().AddA2dpDevice(device, configInfo);
    A2dpDeviceConfigInfo info;
    AudioA2dpDevice::GetInstance().GetA2dpDeviceInfo(device, info);
    AudioA2dpDevice::GetInstance().DelA2dpDevice(device);
}

void GetA2dpInDeviceInfoFuzzTest()
{
    uint32_t samplingRateCount = GetData<uint32_t>() % AudioSamplingRateVec.size();
    AudioSamplingRate samplingRate = AudioSamplingRateVec[samplingRateCount];
    int32_t encodingCount =
        static_cast<int32_t>(AudioEncodingType::ENCODING_EAC3 - AudioEncodingType::ENCODING_INVALID) + 1;
    AudioEncodingType encoding = static_cast<AudioEncodingType>(GetData<int32_t>() % encodingCount - 1);
    uint32_t formatCount = GetData<uint32_t>() % AudioSampleFormatVec.size();
    AudioSampleFormat format = AudioSampleFormatVec[formatCount];
    int32_t channelsCount = static_cast<int32_t>(AudioChannel::CHANNEL_16) + 1;
    AudioChannel channels = static_cast<AudioChannel>(GetData<int32_t>() % channelsCount);
    DeviceStreamInfo streamInfo(samplingRate, encoding, format, channels);
    A2dpDeviceConfigInfo configInfo;
    configInfo.streamInfo = streamInfo;
    configInfo.absVolumeSupport = GetData<uint32_t>() % NUM_2;
    configInfo.volumeLevel = GetData<int32_t>();
    configInfo.mute = GetData<uint32_t>() % NUM_2;
    std::string device = "test_device";
    AudioA2dpDevice::GetInstance().AddA2dpInDevice(device, configInfo);
    A2dpDeviceConfigInfo info;
    AudioA2dpDevice::GetInstance().GetA2dpInDeviceInfo(device, info);
    AudioA2dpDevice::GetInstance().DelA2dpInDevice(device);
}

void GetA2dpDeviceVolumeLevelFuzzTest()
{
    A2dpDeviceConfigInfo configInfo;
    configInfo.volumeLevel = GetData<int32_t>();
    std::string device = "test_device";
    AudioA2dpDevice::GetInstance().AddA2dpDevice(device, configInfo);
    int32_t volumeLevel;
    AudioA2dpDevice::GetInstance().GetA2dpDeviceVolumeLevel(device, volumeLevel);
    string nonDevice = "non_existent_device";
    int32_t volumeLeve2;
    AudioA2dpDevice::GetInstance().GetA2dpDeviceVolumeLevel(nonDevice, volumeLeve2);
}

void CheckA2dpDeviceExistFuzzTest()
{
    A2dpDeviceConfigInfo configInfo;
    std::string device = "test_device";
    AudioA2dpDevice::GetInstance().AddA2dpDevice(device, configInfo);
    AudioA2dpDevice::GetInstance().CheckA2dpDeviceExist(device);
    AudioA2dpDevice::GetInstance().DelA2dpDevice(device);
}

void SetA2dpDeviceMuteFuzzTest()
{
    A2dpDeviceConfigInfo configInfo;
    configInfo.absVolumeSupport = GetData<uint32_t>() % NUM_2;
    std::string device = "test_device";
    AudioA2dpDevice::GetInstance().AddA2dpDevice(device, configInfo);
    AudioA2dpDevice::GetInstance().SetA2dpDeviceMute(device, GetData<uint32_t>() % NUM_2);
    A2dpDeviceConfigInfo info;
    AudioA2dpDevice::GetInstance().GetA2dpDeviceInfo(device, info);
    AudioA2dpDevice::GetInstance().DelA2dpDevice(device);
}

void GetA2dpDeviceMuteFuzzTest()
{
    A2dpDeviceConfigInfo configInfo;
    configInfo.absVolumeSupport = GetData<uint32_t>() % NUM_2;
    configInfo.mute = GetData<uint32_t>() % NUM_2;
    std::string device = "test_device";
    AudioA2dpDevice::GetInstance().AddA2dpDevice(device, configInfo);
    bool isMute = GetData<uint32_t>() % NUM_2;
    AudioA2dpDevice::GetInstance().GetA2dpDeviceMute(device, isMute);
    AudioA2dpDevice::GetInstance().DelA2dpDevice(device);
}

void SetA2dpDeviceAbsVolumeSupportFuzzTest()
{
    A2dpDeviceConfigInfo configInfo;
    configInfo.absVolumeSupport = GetData<uint32_t>() % NUM_2;
    std::string device = "test_device";
    AudioA2dpDevice::GetInstance().AddA2dpDevice(device, configInfo);
    bool support = GetData<uint32_t>() % NUM_2;
    int32_t volume = GetData<int32_t>();
    bool mute = GetData<uint32_t>() % NUM_2;
    AudioA2dpDevice::GetInstance().SetA2dpDeviceAbsVolumeSupport(device, support, volume, mute);
    A2dpDeviceConfigInfo info;
    AudioA2dpDevice::GetInstance().GetA2dpDeviceInfo(device, info);
    AudioA2dpDevice::GetInstance().DelA2dpDevice(device);
}

void SetA2dpDeviceVolumeLevelFuzzTest()
{
    A2dpDeviceConfigInfo configInfo;
    configInfo.absVolumeSupport = GetData<uint32_t>() % NUM_2;
    std::string device = "test_device";
    AudioA2dpDevice::GetInstance().AddA2dpDevice(device, configInfo);
    int32_t volumeLevel = GetData<int32_t>();
    bool result = AudioA2dpDevice::GetInstance().SetA2dpDeviceVolumeLevel(device, volumeLevel);
    A2dpDeviceConfigInfo info;
    bool getInfoResult = AudioA2dpDevice::GetInstance().GetA2dpDeviceInfo(device, info);
    AudioA2dpDevice::GetInstance().DelA2dpDevice(device);
}

TestFuncs g_testFuncs[TESTSIZE] = {
    GetA2dpDeviceInfoFuzzTest,
    GetA2dpInDeviceInfoFuzzTest,
    GetA2dpDeviceVolumeLevelFuzzTest,
    CheckA2dpDeviceExistFuzzTest,
    SetA2dpDeviceMuteFuzzTest,
    GetA2dpDeviceMuteFuzzTest,
    SetA2dpDeviceAbsVolumeSupportFuzzTest,
    SetA2dpDeviceVolumeLevelFuzzTest,
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
